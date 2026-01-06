# NeoOS - Memory Manager
El Memory Manager de NeoOS es un componente esencial del sistema operativo que se encarga de la gestión eficiente de la memoria del sistema. Está implementado en `src/kernel/memory/` y consta de tres subsistemas principales: PMM (Physical Memory Manager), VMM (Virtual Memory Manager) y el Heap del kernel.

## Arquitectura del Memory Manager

El Memory Manager se compone de tres capas:

### 1. Physical Memory Manager (PMM)
**Ubicación**: `src/kernel/memory/src/pmm.c` (302 líneas)

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
1. Verifica que Multiboot proporciona información de memoria (flag `MULTIBOOT_INFO_MEMORY`)
2. Si no hay información, retorna `E_INVAL` y muestra mensaje de error
3. Calcula memoria total desde `multiboot_info_t` (mem_lower + mem_upper)
4. Calcula el tamaño del bitmap necesario (1 bit por página de 4KB)
5. Coloca el bitmap después del kernel (símbolo `kernel_end`)
6. Marca todas las páginas como ocupadas inicialmente
7. Parsea el mmap de Multiboot buscando regiones `MULTIBOOT_MEMORY_AVAILABLE`
8. Marca regiones disponibles como libres, excepto las páginas del kernel y bitmap
9. Muestra estadísticas si `kdebug` está activo
10. Retorna `E_OK` si todo es exitoso

**Manejo de Errores**:
```c
int pmm_init(multiboot_info_t* mbi, bool kdebug, bool kverbose) {
    // Verificar flags de Multiboot
    if (!(mbi->flags & MULTIBOOT_INFO_MEMORY)) {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_write("[PMM] [FAIL] No hay informacion de memoria de Multiboot\n");
        return E_INVAL;  // Argumento inválido
    }
    
    // ... inicialización ...
    
    return E_OK;  // Éxito
}
```

**Estadísticas**:
- Calcula y muestra páginas totales, libres y usadas
- Muestra ubicación del bitmap y kernel en memoria
- Información solo visible con `--debug`

### 2. Virtual Memory Manager (VMM)
**Ubicación**: `src/kernel/memory/src/vmm.c` (262 líneas)

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

**Implementación del Identity Mapping**:
```c
// 32 tablas estáticas, alineadas a 4KB
static page_table_t kernel_tables[32] __attribute__((aligned(4096)));

// Directorio estático, alineado a 4KB
static page_directory_t kernel_directory_data __attribute__((aligned(4096)));
```

Cada tabla mapea 4MB (1024 páginas × 4KB), así que 32 tablas mapean exactamente 128MB (0x00000000 - 0x08000000).

**Funciones principales**:
- `vmm_init()`: Configura el directorio de páginas del kernel, crea identity mapping y habilita paginación
- `vmm_map_page()`: Mapea una dirección virtual a una física
- `vmm_unmap_page()`: Desmapea una página virtual
- `vmm_get_physical()`: Obtiene la dirección física de una dirección virtual
- `vmm_switch_directory()`: Cambia el directorio de páginas activo (carga CR3)
- `vmm_get_kernel_directory()`: Obtiene el directorio de páginas del kernel

**Proceso de Inicialización**:
1. Usa el directorio estático en `.bss`
2. Limpia el directorio y las 32 tablas estáticas con `memset()`
3. Crea identity mapping para 0-128MB usando las tablas estáticas:
   ```c
   for (i = 0; i < 32; i++) {
       for (j = 0; j < 1024; j++) {
           // Calcular dirección física
           uint32_t phys = (i * 4 * 1024 * 1024) + (j * 4096);
           // Mapear con flags: PRESENT | WRITE
           kernel_tables[i].pages[j] = phys | PAGE_PRESENT | PAGE_WRITE;
       }
       // Agregar tabla al directorio
       uint32_t table_phys = (uint32_t)&kernel_tables[i];
       kernel_directory_data.tables[i] = table_phys | PAGE_PRESENT | PAGE_WRITE;
   }
   ```
4. Carga el directorio en CR3:
   ```c
   vmm_switch_directory(&kernel_directory_data);
   ```
5. Habilita paginación (bit 31 de CR0):
   ```c
   uint32_t cr0;
   __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));
   cr0 |= 0x80000000;  // Bit 31: Paging Enable
   __asm__ volatile("mov %0, %%cr0" : : "r"(cr0));
   ```
6. Retorna `E_OK`

**Ventajas del Identity Mapping**:
- Simplifica la inicialización del kernel
- Permite usar direcciones físicas directamente
- El kernel puede acceder a hardware mapeado en memoria (como VGA 0xB8000)
- No requiere traducción de direcciones durante boot

### 3. Kernel Heap
**Ubicación**: `src/kernel/memory/src/heap.c` (359 líneas)

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
    uint32_t magic;                 // Verificación de integridad (0x12345678)
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

**Algoritmo de Asignación (First-Fit)**:
1. Busca el primer bloque libre suficientemente grande
2. Si se encuentra:
   - Verifica si vale la pena dividir el bloque (espacio sobrante ≥ `HEAP_MIN_BLOCK_SIZE`)
   - Si sí, divide y crea un nuevo bloque libre con el espacio sobrante
   - Marca el bloque como ocupado y devuelve puntero a los datos
3. Si no se encuentra ningún bloque:
   - Llama a `heap_expand()` para asignar más páginas del PMM
   - Crea un nuevo bloque con las páginas recién asignadas
   - Intenta fusionar con el último bloque si este está libre
4. Retorna puntero o `NULL` si falla

**Algoritmo de Liberación**:
1. Verifica el magic number del bloque para detectar corrupción
2. Marca el bloque como libre
3. Intenta fusionar con el bloque siguiente si está libre (coalescing hacia adelante)
4. Intenta fusionar con el bloque anterior si está libre (coalescing hacia atrás)
5. Actualiza la lista enlazada para mantener consistencia

**Manejo de Errores**:
```c
int heap_init(uint32_t start, uint32_t size, bool kdebug, bool kverbose) {
    // ... configuración inicial ...
    
    // Intentar expandir con páginas iniciales
    if (!heap_expand(HEAP_INITIAL_PAGES)) {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_write("[HEAP] [FAIL] No se pudo expandir el heap inicial\n");
        return E_NOMEM;  // Sin memoria
    }
    
    // ... crear bloque inicial ...
    
    return E_OK;  // Éxito
}
```

**Función `heap_expand()`**:
- Asigna páginas físicas usando `pmm_alloc_page()`
- Mapea las páginas físicas al espacio virtual del heap usando `vmm_map_page()`
- Actualiza el tamaño actual del heap
- Retorna `true` si éxito, `false` si no hay memoria

## Coordinador: memory_init()

**Ubicación**: `src/kernel/memory/src/memory.c`

La función `memory_init()` coordina la inicialización de los tres subsistemas en orden:

```c
int memory_init(multiboot_info_t* mbi, bool kdebug, bool kverbose) {
    int result;
    
    // 1. Inicializar Physical Memory Manager
    result = pmm_init(mbi, kdebug, kverbose);
    if (result != E_OK) {
        return result;  // Propagar error
    }
    
    // 2. Inicializar Virtual Memory Manager
    result = vmm_init(kdebug, kverbose);
    if (result != E_OK) {
        return result;  // Propagar error
    }
    
    // 3. Inicializar Kernel Heap
    result = heap_init(KERNEL_HEAP_START, KERNEL_HEAP_SIZE, kdebug, kverbose);
    if (result != E_OK) {
        return result;  // Propagar error
    }
    
    return E_OK;  // Todo inicializado correctamente
}
```

**Comportamiento ante Errores**:
- Si cualquier subsistema falla, retorna inmediatamente el código de error
- El error se propaga a `kernel_main()` que muestra el mensaje y detiene el kernel
- No se intenta continuar con subsistemas posteriores si uno falla
- Los mensajes de error específicos son mostrados por cada subsistema

**Orden de Inicialización** (crítico):
1. **PMM primero**: Se necesita saber qué páginas físicas están disponibles
2. **VMM segundo**: Requiere páginas del PMM y debe estar activo antes del heap
3. **Heap último**: Necesita paginación habilitada y PMM funcional para expandirse

## Información de Memoria

La función `memory_get_info()` proporciona estadísticas:

```c
void memory_get_info(uint32_t* total_kb, uint32_t* used_kb, uint32_t* free_kb) {
    uint32_t total_pages = pmm_get_total_pages();
    uint32_t free_pages = pmm_get_free_pages();
    uint32_t used_pages = total_pages - free_pages;
    
    *total_kb = (total_pages * PAGE_SIZE) / 1024;
    *used_kb = (used_pages * PAGE_SIZE) / 1024;
    *free_kb = (free_pages * PAGE_SIZE) / 1024;
}
```

Calcula estadísticas basándose en las páginas del PMM:
- **total_kb**: Memoria total en KB (total_pages × 4KB)
- **used_kb**: Memoria usada en KB (used_pages × 4KB)
- **free_kb**: Memoria libre en KB (free_pages × 4KB)

## Mapa de Memoria del Kernel

Después de la inicialización completa, el mapa de memoria es:

```
0x00000000 - 0x000FFFFF  : 1MB reservado (BIOS, Video, etc.)
0x00100000 - kernel_end  : Kernel code/data/bss
kernel_end - bitmap_end  : PMM Bitmap
bitmap_end - 0x003FFFFF  : Memoria libre (uso futuro)
0x00400000 - 0x007FFFFF  : Kernel Heap (4MB)
0x00800000 - 0x07FFFFFF  : Memoria libre (identity mapped)
0x08000000 - 0xFFFFFFFF  : No mapeado (requiere tablas dinámicas)
```

**Áreas Especiales**:
- **0xB8000**: VGA text buffer (mapeado, presente físicamente)
- **0x00100000 (1MB)**: Inicio del kernel (cargado por GRUB)
- **Stack**: 16KB después de `_start` en boot (crece hacia abajo)

## Limitaciones Actuales

- **Sin swapping**: No se implementa intercambio de páginas a disco
- **Identity mapping limitado**: Solo los primeros 128MB están mapeados 1:1
- **Heap fijo máximo**: El heap puede crecer hasta 4MB máximo
- **Sin demand paging**: Todas las páginas se asignan al solicitarlas, no bajo demanda
- **Algoritmo simple**: El heap usa first-fit, no hay best-fit ni buddy system
- **Sin estadísticas de heap**: No se pueden consultar estadísticas detalladas del heap
- **Sin protección de memoria**: Todas las páginas son RW, no hay enforcement de permisos
- **Tablas estáticas**: Las 32 page tables son estáticas, no se pueden crear más dinámicamente

## Futuras Mejoras

- **Demand Paging**: Asignar páginas solo cuando se acceden
- **Copy-on-Write**: Para procesos que comparten memoria
- **Mapeo dinámico**: Crear page tables bajo demanda para direcciones >128MB
- **Protección**: Implementar páginas de solo lectura y ejecutables
- **Estadísticas avanzadas**: Fragmentación del heap, hit rate del cache de páginas
- **Mejor algoritmo de heap**: Considerar buddy system o slab allocator

## Notas de Implementación

- El bitmap del PMM se coloca en memoria física inmediatamente después del kernel
- El VMM usa estructuras estáticas para evitar problemas de asignación dinámica antes de tener paginación habilitada
- Con el identity mapping de 128MB, las direcciones virtuales bajas son iguales a las físicas
- El heap se expande bajo demanda asignando páginas del espacio pre-mapeado (0x00400000-0x007FFFFF)
- El heap NO puede crecer más allá de su límite de 4MB definido en `KERNEL_HEAP_SIZE`
- Todas las estructuras de paginación están alineadas a 4KB (requisito de x86)

## Debugging y Verificación

Con la opción `--debug`:
- PMM muestra: páginas totales, libres, usadas, ubicación del bitmap
- VMM muestra: inicio/fin del identity mapping, dirección del directorio
- Heap muestra: ubicación, tamaño, número de páginas iniciales

Ejemplo de salida:
```
[PMM] Inicializando Physical Memory Manager...
[PMM] Memoria total: 131072 KB (128 MB)
[PMM] Paginas totales: 32768
[PMM] Paginas libres: 31000
[PMM] Paginas usadas: 1768
[PMM] Bitmap en: 0x00110000

[VMM] Inicializando Virtual Memory Manager...
[VMM] Identity mapping: 0x00000000 - 0x08000000
[VMM] Page Directory: 0x00115000

[HEAP] Inicializando Kernel Heap...
[HEAP] Heap start: 0x00400000
[HEAP] Heap size: 4 MB
[HEAP] Paginas iniciales: 16
```

## Véase también
- [Kernel Initialization](./Kernel%20Initialization.md) - Proceso completo de inicialización
- [Errors](./Errors.md) - Sistema de manejo de errores
- [Boot Process](./Boot%20Process.md) - Proceso de arranque
- Header file: `src/kernel/memory/include/memory.h`
