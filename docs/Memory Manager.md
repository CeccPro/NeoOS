# NeoOS - Memory Manager
El Memory Manager de NeoOS es un componente esencial del sistema operativo que se encarga de la gestión eficiente de la memoria del sistema. Está implementado en `src/kernel/memory/` y consta de tres subsistemas principales: PMM (Physical Memory Manager), VMM (Virtual Memory Manager) y el Heap del kernel.

## Arquitectura del Memory Manager

El Memory Manager se compone de tres capas:

### 1. Physical Memory Manager (PMM)
**Ubicación**: `src/kernel/memory/src/pmm.c`

El PMM gestiona las páginas físicas de memoria usando un bitmap donde cada bit representa una página de 4KB:
- **0** = página libre
- **1** = página ocupada

**Características**:
- **Inicialización**: `pmm_init()` parsea el mapa de memoria de Multiboot para identificar regiones disponibles
- **Ubicación del Bitmap**: Se coloca inmediatamente después del kernel en memoria (usando el símbolo `kernel_end` del linker script)
- **Tamaño de Página**: 4KB (4096 bytes), definido por `PAGE_SIZE`
- **Funciones principales**:
  - `pmm_alloc_page()`: Asigna una página física, devuelve su dirección o 0 si no hay memoria
  - `pmm_free_page(uint32_t page)`: Libera una página física
  - `pmm_get_free_pages()`: Obtiene el número de páginas libres
  - `pmm_get_total_pages()`: Obtiene el número total de páginas

**Proceso de Inicialización**:
1. Calcula memoria total desde `multiboot_info_t` (mem_lower + mem_upper)
2. Calcula el tamaño del bitmap necesario
3. Coloca el bitmap después del kernel
4. Marca todas las páginas como ocupadas inicialmente
5. Parsea el mapa de memoria de Multiboot buscando regiones `MULTIBOOT_MEMORY_AVAILABLE`
6. Marca regiones disponibles como libres, excepto las páginas del kernel y bitmap

### 2. Virtual Memory Manager (VMM)
**Ubicación**: `src/kernel/memory/src/vmm.c`

El VMM implementa paginación de 2 niveles (arquitectura x86 32-bit):
- **Page Directory**: 1024 entradas (cada una mapea 4MB)
- **Page Tables**: 1024 entradas cada una (cada una mapea 4KB)

**Dirección virtual de 32 bits**: `[31-22: Dir Index | 21-12: Table Index | 11-0: Offset]`

**Características**:
- **Identity Mapping**: Los primeros 128MB están mapeados 1:1 (dirección virtual = dirección física)
- **Estructuras Estáticas**: Usa 32 tablas de páginas estáticas (`kernel_tables[32]`) para mapear 128MB
- **Directorio del Kernel**: Estructura estática `kernel_directory_data` alineada a 4KB
- **Flags de Página**:
  - `PAGE_PRESENT` (bit 0): Página presente en memoria
  - `PAGE_WRITE` (bit 1): Página escribible
  - `PAGE_USER` (bit 2): Accesible desde modo usuario
  - `PAGE_ACCESSED` (bit 5): Página accedida
  - `PAGE_DIRTY` (bit 6): Página modificada

**Funciones principales**:
- `vmm_init()`: Configura el directorio de páginas del kernel, crea identity mapping y habilita paginación
- `vmm_map_page()`: Mapea una dirección virtual a una física
- `vmm_unmap_page()`: Desmapea una página virtual
- `vmm_get_physical()`: Obtiene la dirección física de una dirección virtual
- `vmm_switch_directory()`: Cambia el directorio de páginas activo (carga CR3)
- `vmm_get_kernel_directory()`: Obtiene el directorio de páginas del kernel

**Proceso de Inicialización**:
1. Usa el directorio estático en `.bss`
2. Limpia el directorio y las 32 tablas estáticas
3. Crea identity mapping para 0-128MB usando las tablas estáticas
4. Cada tabla mapea 4MB (1024 páginas de 4KB)
5. Carga el directorio en CR3
6. Habilita paginación (bit 31 de CR0)

### 3. Kernel Heap
**Ubicación**: `src/kernel/memory/src/heap.c`

El heap proporciona asignación dinámica de memoria para el kernel mediante una implementación simple con lista enlazada de bloques.

**Configuración**:
- **Inicio**: `0x00400000` (4MB) - definido por `KERNEL_HEAP_START`
- **Tamaño**: `0x00400000` (4MB) - definido por `KERNEL_HEAP_SIZE`
- **Magic Number**: `0x12345678` - para detección de corrupción
- **Alineación mínima**: 16 bytes
- **Tamaño mínimo de bloque**: 32 bytes

**Estructura de Bloque**:
```c
typedef struct heap_block {
    uint32_t magic;                 // Verificación de integridad
    size_t size;                    // Tamaño sin incluir header
    bool is_free;                   // Estado del bloque
    struct heap_block* next;        // Siguiente en la lista
    struct heap_block* prev;        // Anterior en la lista
} heap_block_t;
```

**Funciones principales**:
- `heap_init()`: Inicializa el heap del kernel
- `kmalloc(size_t size)`: Asigna memoria del heap
- `kmalloc_a(size_t size)`: Asigna memoria alineada a página (4KB)
- `kmalloc_p(size_t size, uint32_t* phys)`: Asigna memoria y devuelve dirección física
- `kmalloc_ap(size_t size, uint32_t* phys)`: Asigna memoria alineada y devuelve dirección física
- `kfree(void* ptr)`: Libera memoria del heap

**Algoritmo de Asignación**:
1. Busca el primer bloque libre suficientemente grande (first-fit)
2. Si se encuentra, divide el bloque si el espacio sobrante es útil
3. Si no se encuentra, expande el heap asignando más páginas
4. Marca el bloque como ocupado y devuelve puntero a los datos

**Algoritmo de Liberación**:
1. Verifica el magic number del bloque
2. Marca el bloque como libre
3. Intenta fusionar con bloques adyacentes libres (coalescing)

## Proceso de Inicialización del Memory Manager

La función `memory_init()` en `src/kernel/memory/src/memory.c` coordina la inicialización:

1. **Inicializar PMM**: `pmm_init(mbi, kdebug, kverbose)`
2. **Inicializar VMM**: `vmm_init(kdebug, kverbose)`
3. **Inicializar Heap**: `heap_init(KERNEL_HEAP_START, KERNEL_HEAP_SIZE, kdebug, kverbose)`

Si cualquier paso falla, se devuelve el código de error correspondiente y el kernel se detiene.

## Información de Memoria

La función `memory_get_info()` proporciona estadísticas:
- **total_kb**: Memoria total en KB
- **used_kb**: Memoria usada en KB
- **free_kb**: Memoria libre en KB

Calcula estos valores basándose en las estadísticas del PMM (páginas totales vs páginas libres).

## Limitaciones Actuales

- **Sin swapping**: No se implementa intercambio de páginas a disco
- **Sin configuración externa**: No hay archivos de configuración como `memory_manager.conf`
- **Identity mapping limitado**: Solo los primeros 128MB están mapeados
- **Heap fijo**: El heap tiene tamaño fijo de 4MB
- **Sin herramientas de monitoreo**: Comandos como `memstat` y `memtop` no están implementados
- **Algoritmo simple**: El heap usa first-fit, no hay optimizaciones avanzadas

## Notas de Implementación

- El bitmap del PMM se coloca en memoria física inmediatamente después del kernel
- El VMM usa estructuras estáticas para evitar problemas de asignación dinámica antes de tener paginación habilitada
- Con el identity mapping de 128MB, las direcciones virtuales bajas son iguales a las físicas
- El heap se expande bajo demanda asignando páginas del espacio pre-mapeado

## Véase también
- [Kernel Initialization](./Kernel%20Initialization.md)
- [Errors](./Errors.md)
- Header file: `src/kernel/memory/include/memory.h`