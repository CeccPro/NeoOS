# NeoOS - Kernel Initialization
La inicialización del kernel en NeoOS es un proceso fundamental que establece las bases para el funcionamiento del sistema operativo. Durante esta fase, el kernel configura los recursos del sistema, inicializa los controladores de hardware esenciales y prepara el entorno necesario para futuros subsistemas.

## Proceso de Inicialización
El proceso de inicialización del kernel de NeoOS se lleva a cabo en las siguientes etapas:

### 1. Entry Point en Assembly (`_start`)
- **Ubicación**: `src/kernel/arch/x86/boot/kmain.S`
- **Función**: Primer código ejecutado después del bootloader
- Configura la pila del kernel (16KB en la sección `.bss`)
- Preserva el magic number de Multiboot (EAX) y el puntero a `multiboot_info_t` (EBX)
- Llama a `kernel_main(uint32_t magic, multiboot_info_t* mbi)` con estos parámetros
- Si `kernel_main()` retorna, deshabilita interrupciones y entra en bucle infinito con `hlt`

### 2. Inicialización del Driver VGA
- **Función**: `vga_init()` en `src/kernel/drivers/src/vga.c`
- **Importancia**: Primer subsistema inicializado para poder mostrar mensajes
- Configura el terminal VGA en modo texto 80x25
- Buffer VGA ubicado en `0xB8000`
- Establece el color predeterminado (gris claro sobre negro)
- Limpia la pantalla completa
- Inicializa cursor en posición (0, 0)

### 3. Parseo de Parámetros de Línea de Comandos
Lee la estructura `multiboot_info_t` para obtener `cmdline` y busca los siguientes flags:
- `--debug`: Activa `kdebug`, muestra mensajes de depuración detallados
- `--verbose`: Activa `kverbose`, muestra salida detallada de inicialización
- `--no-subsystems`: Activa `ksubsystems = false`, detiene el kernel después de Memory Manager (útil para testing)

**Implementación**:
```c
if (mbi->flags & MULTIBOOT_INFO_CMDLINE) {
    if (strstr((const char*)mbi->cmdline, "--debug"))
        kdebug = true;
    if (strstr((const char*)mbi->cmdline, "--verbose"))
        kverbose = true;
    if (strstr((const char*)mbi->cmdline, "--no-subsystems"))
        ksubsystems = false;
}
```

### 4. Inicialización de Configuración del Kernel
- **Función**: `kconfig_init(kdebug, kverbose)` en `src/kernel/core/src/kconfig.c`
- Establece las variables globales:
  - `kernel_debug_mode`: controla mensajes de depuración
  - `kernel_verbose_mode`: controla salida detallada
- Proporciona funciones helper:
  - `is_kdebug()`: retorna el estado del modo debug
  - `is_kverbose()`: retorna el estado del modo verbose
- Estas variables son usadas en todo el kernel para controlar la salida de diagnóstico

### 5. Verificación del Bootloader Multiboot
- Verifica que el magic number sea `0x2BADB002`
- Si falla:
  - Muestra error en rojo (VGA_COLOR_LIGHT_RED)
  - Muestra el magic esperado y el recibido en hexadecimal
  - Detiene el kernel con bucle infinito: `while(1) { __asm__ volatile("hlt"); }`
- Esta verificación es crítica para asegurar que el bootloader pasó información válida

### 6. Banner de Bienvenida
Solo se muestra si `kverbose == true`:
- Banner con ASCII art del logo de NeoOS
- Versión del kernel: `KERNEL_VERSION` (definida como "0.1.0" en `kmain.h`)
- Fecha de compilación: `BUILD_DATE` (definida como `__DATE__ " " __TIME__`)
- Colores: cian claro para el banner, blanco para el texto

Ejemplo de salida:
```
================================================
          _   _             ___  ____  
         | \ | | ___  ___  / _ \/ ___| 
         |  \| |/ _ \/ _ \| | \___ \_
         | |\  |  __/ (_) | |_| ___) |
         |_| \_|\___|\___/ \___/____/ 
              NeoOS Kernel v0.1.0
         (Build: Jan 5 2026 10:30:00)
================================================
```

### 7. Información de Memoria
Lee información de memoria de `multiboot_info_t`:
- **mem_lower**: memoria inferior en KB (0-640KB, memoria convencional)
- **mem_upper**: memoria superior en KB (desde 1MB)

**Salida en modo debug y verbose**:
- Memoria inferior y superior en KB
- Memoria total calculada en MB o GB (si > 1GB)

**Cálculo**:
```c
uint32_t total_mem_kb = mbi->mem_lower + mbi->mem_upper;
uint32_t total_mem_mb = total_mem_kb / 1024;
if (total_mem_mb > 1024) {
    int total_mem_gb = total_mem_mb / 1024;
    // Muestra en GB
} else {
    // Muestra en MB
}
```

### 8. Inicialización del Memory Manager
- **Función**: `memory_init(mbi, kdebug, kverbose)` en `src/kernel/memory/src/memory.c`
- **Componentes**: Inicializa tres subsistemas en orden específico

#### 8.1. PMM (Physical Memory Manager)
- **Función**: `pmm_init(mbi, kdebug, kverbose)` en `src/kernel/memory/src/pmm.c`
- **Propósito**: Gestión de páginas físicas mediante bitmap

**Proceso de inicialización**:
1. Verifica que haya información de memoria en Multiboot (`MULTIBOOT_INFO_MEMORY`)
2. Calcula memoria total: `(mem_lower + mem_upper + 1024) * 1024` bytes
3. Calcula número total de páginas: `memoria_total / 4096`
4. Calcula tamaño del bitmap: 1 bit por página, redondeado a DWORDs
5. Coloca el bitmap después del kernel usando `extern uint32_t kernel_end`
6. Inicializa todas las páginas como ocupadas (bitmap = 0xFFFFFFFF)
7. Parsea el mapa de memoria de Multiboot:
   - Busca regiones `MULTIBOOT_MEMORY_AVAILABLE` (tipo 1)
   - Marca esas páginas como libres en el bitmap
   - **Excepción**: No marca como libres las páginas del kernel y bitmap
8. Reserva páginas desde `KERNEL_START / PAGE_SIZE` hasta `(bitmap_end + PAGE_SIZE - 1) / PAGE_SIZE`

**Funciones principales**:
- `pmm_alloc_page()`: Asigna una página física, retorna dirección o 0
- `pmm_free_page(uint32_t page)`: Libera una página física
- `pmm_get_free_pages()`: Retorna número de páginas libres
- `pmm_get_total_pages()`: Retorna número total de páginas

**Código de error**: Retorna `E_INVAL` si no hay información de memoria, `E_OK` si es exitoso

#### 8.2. VMM (Virtual Memory Manager)
- **Función**: `vmm_init(kdebug, kverbose)` en `src/kernel/memory/src/vmm.c`
- **Propósito**: Implementar paginación de 2 niveles (x86 32-bit)

**Arquitectura de paginación**:
- **Page Directory**: 1024 entradas (cada una mapea 4MB)
- **Page Tables**: 1024 entradas cada una (cada una mapea 4KB)
- **Dirección virtual**: `[31-22: Dir Index | 21-12: Table Index | 11-0: Offset]`

**Proceso de inicialización**:
1. Usa directorio estático `kernel_directory_data` alineado a 4KB
2. Usa 32 tablas de páginas estáticas `kernel_tables[32]` (128MB total)
3. Limpia el directorio y las tablas con `memset()`
4. **Identity mapping de 0-128MB**:
   - Para cada una de las 32 tablas:
     - Configura entrada del directorio: `table_phys | PAGE_PRESENT | PAGE_WRITE`
     - Llena tabla con 1024 entradas: `phys_addr | PAGE_PRESENT | PAGE_WRITE`
     - Dirección física = (tabla * 1024 + página) * 4096
5. Carga el directorio en CR3: `mov %cr3, kernel_directory_phys`
6. Habilita paginación: `or $0x80000000, %cr0; mov %cr0, ...` (bit 31)

**Funciones principales**:
- `vmm_map_page()`: Mapea dirección virtual a física
- `vmm_unmap_page()`: Desmapea una página virtual
- `vmm_get_physical()`: Obtiene dirección física de una virtual
- `vmm_switch_directory()`: Cambia el directorio activo (carga CR3)
- `vmm_get_kernel_directory()`: Retorna puntero al directorio del kernel

**Código de error**: Retorna `E_OK` si es exitoso

#### 8.3. Heap del Kernel
- **Función**: `heap_init(KERNEL_HEAP_START, KERNEL_HEAP_SIZE, kdebug, kverbose)` en `src/kernel/memory/src/heap.c`
- **Propósito**: Asignación dinámica de memoria (kmalloc/kfree)

**Configuración**:
- **Dirección de inicio**: `0x00400000` (4MB) - `KERNEL_HEAP_START`
- **Tamaño**: `0x00400000` (4MB) - `KERNEL_HEAP_SIZE`
- **Magic number**: `0x12345678` para validación de bloques
- **Alineación mínima**: 16 bytes
- **Tamaño mínimo de bloque**: 32 bytes

**Estructura de bloque**:
```c
typedef struct heap_block {
    uint32_t magic;                 // 0x12345678
    size_t size;                    // Tamaño sin incluir header
    bool is_free;                   // Estado del bloque
    struct heap_block* next;        // Siguiente bloque
    struct heap_block* prev;        // Bloque anterior
} heap_block_t;
```

**Proceso de inicialización**:
1. Establece `heap_start`, `heap_end`, `heap_current`
2. Llama a `heap_expand(PAGE_SIZE)` para crear el primer bloque
3. Marca `heap_initialized = true`

**Funciones de asignación**:
- `kmalloc(size_t size)`: Asignación básica
- `kmalloc_a(size_t size)`: Asignación alineada a página
- `kmalloc_p(size_t size, uint32_t* phys)`: Asignación con dirección física
- `kmalloc_ap(size_t size, uint32_t* phys)`: Asignación alineada con dirección física
- `kfree(void* ptr)`: Liberación de memoria

**Algoritmo first-fit**:
1. Busca el primer bloque libre suficientemente grande
2. Si se encuentra, divide el bloque si el resto es útil
3. Si no se encuentra, expande el heap asignando más páginas
4. Marca el bloque como ocupado y retorna puntero

**Código de error**: Retorna `E_NOMEM` si no puede expandir el heap, `E_OK` si es exitoso

### 9. Inicialización del Sistema de Interrupciones
Esta fase configura la arquitectura de interrupciones x86. Si `kverbose` está activo, muestra mensajes detallados de progreso.

#### 9.1. GDT (Global Descriptor Table)
- **Función**: `gdt_init()` en `src/kernel/core/src/gdt.c`
- **Propósito**: Configurar la segmentación del procesador

**Configuración de 5 entradas**:
1. **Entrada 0 (Null Descriptor)**: Requerido por arquitectura x86, todo en 0
2. **Entrada 1 (Kernel Code)**: Segmento de código Ring 0, base 0, límite 4GB
   - Selector: `0x08` (`GDT_KERNEL_CODE_SEGMENT`)
   - Acceso: `PRESENT | PRIV_0 | CODE_SEG | READ_WRITE`
   - Granularidad: `4K | 32BIT`
3. **Entrada 2 (Kernel Data)**: Segmento de datos Ring 0, base 0, límite 4GB
   - Selector: `0x10` (`GDT_KERNEL_DATA_SEGMENT`)
   - Acceso: `PRESENT | PRIV_0 | DATA_SEG | READ_WRITE`
   - Granularidad: `4K | 32BIT`
4. **Entrada 3 (User Code)**: Segmento de código Ring 3, base 0, límite 4GB
   - Selector: `0x18` (`GDT_USER_CODE_SEGMENT`)
   - Con RPL: `0x1B` (`GDT_USER_CODE_SELECTOR = 0x18 | 0x03`)
   - Acceso: `PRESENT | PRIV_3 | CODE_SEG | READ_WRITE`
   - Granularidad: `4K | 32BIT`
5. **Entrada 4 (User Data)**: Segmento de datos Ring 3, base 0, límite 4GB
   - Selector: `0x20` (`GDT_USER_DATA_SEGMENT`)
   - Con RPL: `0x23` (`GDT_USER_DATA_SELECTOR = 0x20 | 0x03`)
   - Acceso: `PRESENT | PRIV_3 | DATA_SEG | READ_WRITE`
   - Granularidad: `4K | 32BIT`

**Estructura de entrada GDT**:
```c
struct gdt_entry {
    uint16_t limit_low;    // Límite bits 0-15
    uint16_t base_low;     // Base bits 0-15
    uint8_t  base_middle;  // Base bits 16-23
    uint8_t  access;       // Flags de acceso
    uint8_t  granularity;  // Granularidad y límite bits 16-19
    uint8_t  base_high;    // Base bits 24-31
} __attribute__((packed));
```

**Carga de la GDT**: Llama a `gdt_flush()` (en assembly) que ejecuta `lgdt` y recarga selectores de segmento

#### 9.2. IDT (Interrupt Descriptor Table)
- **Función**: `idt_init()` en `src/kernel/core/src/idt.c`
- **Propósito**: Configurar tabla de 256 vectores de interrupción

**Estructura de entrada IDT**:
```c
struct idt_entry {
    uint16_t base_low;     // Handler address bits 0-15
    uint16_t selector;     // Selector de segmento (GDT_KERNEL_CODE_SEGMENT)
    uint8_t  always0;      // Siempre 0
    uint8_t  flags;        // Tipo y atributos
    uint16_t base_high;    // Handler address bits 16-31
} __attribute__((packed));
```

**Proceso de inicialización**:
1. Inicializa todas las 256 entradas a 0 con `memset()`
2. Las entradas se configuran después en `interrupts_init()` mediante `idt_set_gate()`
3. Carga la IDT con `idt_flush()` (ejecuta `lidt`)

**Tipos de interrupciones configuradas**:
- ISR 0-31: Excepciones del CPU
- IRQ 0-15: Interrupciones de hardware (mapeadas a ISR 32-47)

#### 9.3. Sistema de Interrupciones
- **Función**: `interrupts_init(kverbose)` en `src/kernel/core/src/interrupts.c`
- **Propósito**: Configurar handlers y remapear el PIC

**Proceso de inicialización**:
1. **Inicializa array de handlers**: `memset(&interrupt_handlers, 0, 256 * sizeof(isr_handler_t))`
2. **Remapea el PIC** (`pic_remap()`):
   - Por defecto, PIC mapea IRQ 0-7 a INT 8-15 (se superpone con excepciones CPU)
   - Remapea: PIC1 IRQs 0-7 → INT 32-39, PIC2 IRQs 8-15 → INT 40-47
   - Secuencia de inicialización: ICW1 → ICW2 → ICW3 → ICW4
3. **Configura entradas IDT para ISR 0-31**:
   - Llama `idt_set_gate(num, isr_handler_addr, GDT_KERNEL_CODE_SEGMENT, IDT_TYPE_INTERRUPT)` para cada ISR
   - Los handlers `isr0` a `isr31` están definidos en `src/kernel/arch/x86/isr.S`
4. **Configura entradas IDT para IRQ 0-15**:
   - Llama `idt_set_gate(32+num, irq_handler_addr, GDT_KERNEL_CODE_SEGMENT, IDT_TYPE_INTERRUPT)`
   - Los handlers `irq0` a `irq15` están definidos en `src/kernel/arch/x86/isr.S`
5. **Habilita interrupciones**: `__asm__ volatile("sti")`

**Handlers en Assembly** (`src/kernel/arch/x86/isr.S`):
- **ISR stub**: Empuja número de interrupción y código de error, salta a `isr_common_stub`
- **IRQ stub**: Empuja número de interrupción (32-47), salta a `irq_common_stub`
- **Common stub**: Guarda registros (`pusha`), llama a handler C, restaura registros, ejecuta `iret`

**Handlers en C**:
- `isr_handler(registers_t* regs)`: Maneja excepciones del CPU
  - Si hay handler registrado, lo llama
  - Si no, muestra **KERNEL PANIC** con información de registros
- `irq_handler(registers_t* regs)`: Maneja interrupciones de hardware
  - Envía EOI al PIC con `pic_send_eoi()`
  - Llama al handler registrado si existe

**Registro de handlers custom**: `interrupts_register_handler(uint8_t num, isr_handler_t handler)`

### 10. Estado Actual
Después de completar la inicialización:
- Si el flag `--no-subsystems` está presente:
  - Muestra mensaje indicando modo sin subsistemas
  - Entra en bucle infinito con `hlt`
- Si no:
  - Muestra mensaje "Subsistemas del kernel iniciados correctamente"
  - Entra en bucle infinito con `hlt`

**Subsistemas pendientes de implementación**:
- Planificador de procesos (scheduler)
- Sistema de archivos NeoFS
- IPC (Comunicación entre procesos)
- Module Manager para carga dinámica de módulos
- Drivers de hardware (timer PIT, teclado, disco ATA)
- Transición a modo usuario (Ring 3)
- Proceso init y shell

## Manejo de Errores Durante la Inicialización

Cada componente puede fallar y retornar un código de error definido en `error.h`:
- `E_OK` (0): Operación exitosa
- `E_NOMEM` (-2): Memoria insuficiente
- `E_INVAL` (-3): Argumento inválido

**Respuesta a errores**:
1. El componente que falla retorna un código de error
2. `kernel_main()` verifica el retorno
3. Si hay error:
   - Muestra mensaje en rojo con `vga_set_color(VGA_COLOR_LIGHT_RED, ...)`
   - Muestra el componente que falló y el código de error usando `error_to_string()`
   - Detiene el kernel con bucle infinito: `while(1) { __asm__ volatile("hlt"); }`

**Ejemplo de manejo de error**:
```c
int mm_result = memory_init(mbi, kdebug, kverbose);
if (mm_result != E_OK) {
    vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
    vga_write("[FAIL] Error al inicializar el Memory Manager\n");
    vga_write("Codigo de error: ");
    vga_write(error_to_string(mm_result));
    vga_write("\n");
    
    while(1) {
        __asm__ volatile("hlt");
    }
}
```

## Archivos Involucrados

### Core del Kernel
- `src/kernel/arch/x86/boot/kmain.S`: Entry point en assembly
- `src/kernel/core/src/kmain.c`: Función principal `kernel_main()`
- `src/kernel/core/include/kmain.h`: Definiciones (KERNEL_VERSION, BUILD_DATE)
- `src/kernel/core/src/kconfig.c`: Configuración global (debug, verbose)
- `src/kernel/core/include/kconfig.h`: Variables de configuración
- `src/kernel/core/src/error.c`: Implementación de `error_to_string()`
- `src/kernel/core/include/error.h`: Definiciones de códigos de error

### Drivers
- `src/kernel/drivers/src/vga.c`: Driver VGA modo texto 80x25
- `src/kernel/drivers/include/vga.h`: API del driver VGA

### Memory Manager
- `src/kernel/memory/src/memory.c`: Coordinador, `memory_init()`
- `src/kernel/memory/src/pmm.c`: Physical Memory Manager
- `src/kernel/memory/src/vmm.c`: Virtual Memory Manager
- `src/kernel/memory/src/heap.c`: Heap del kernel (kmalloc/kfree)
- `src/kernel/memory/include/memory.h`: API completa del Memory Manager

### Sistema de Interrupciones
- `src/kernel/core/src/gdt.c`: Implementación de GDT
- `src/kernel/core/include/gdt.h`: Definiciones de GDT
- `src/kernel/core/src/idt.c`: Implementación de IDT
- `src/kernel/core/include/idt.h`: Definiciones de IDT
- `src/kernel/core/src/interrupts.c`: Sistema de interrupciones
- `src/kernel/core/include/interrupts.h`: Definiciones de ISR e IRQ
- `src/kernel/arch/x86/isr.S`: Handlers en assembly

### Biblioteca
- `src/kernel/lib/src/string.c`: Funciones de strings (memset, memcpy, strstr, etc.)
- `src/kernel/lib/include/string.h`: API de strings
- `src/kernel/lib/include/types.h`: Tipos básicos (uint32_t, bool, size_t)
- `src/kernel/lib/include/multiboot.h`: Estructuras de Multiboot

## Notas Adicionales

### Orden de Inicialización
El orden de inicialización es crítico y no puede ser modificado arbitrariamente:
1. VGA debe ser primero para poder mostrar mensajes
2. Memory Manager debe estar antes que cualquier subsistema que use kmalloc()
3. Interrupciones se inicializan después del Memory Manager porque los handlers pueden necesitar memoria dinámica

### Modos de Operación
- **Modo Normal**: Sin flags, solo muestra mensajes esenciales
- **Modo Verbose**: Con `--verbose`, muestra mensajes de progreso detallados
- **Modo Debug**: Con `--debug`, muestra información técnica (direcciones, tamaños)
- **Modo Test**: Con `--no-subsystems`, detiene después de Memory Manager

### Identity Mapping
El identity mapping de 128MB es crucial:
- Permite que el kernel acceda a memoria física directamente
- Cubre el kernel (1MB+), el bitmap del PMM, y el heap (4MB-8MB)
- Simplifica el acceso a hardware (VGA buffer en 0xB8000)

### Sistema de Módulos Dinámicos
**Nota importante**: El Module Manager mencionado en documentación antigua **no está implementado**. No existe carga dinámica de módulos actualmente.

## Véase También
- [Boot Process](./Boot%20Process.md) - Proceso completo de arranque
- [Memory Manager](./Memory%20Manager.md) - Arquitectura detallada del Memory Manager
- [Interrupts](./Interrupts.md) - Sistema de interrupciones y excepciones
- [Errors](./Errors.md) - Códigos de error del kernel
