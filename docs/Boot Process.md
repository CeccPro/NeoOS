# NeoOS - Boot Process
El proceso de arranque de NeoOS es una secuencia crítica que inicia el sistema operativo desde un estado apagado hasta que está completamente operativo y listo para ejecutar aplicaciones. Este proceso involucra varias etapas, desde la carga del bootloader hasta la inicialización completa del kernel.

## Etapas del Proceso de Arranque

### 1. Bootloader (GRUB)
El bootloader es el primer código que se ejecuta cuando la máquina se enciende. NeoOS utiliza GRUB (GRand Unified Bootloader), que es compatible con el estándar Multiboot 1.

**Especificación Multiboot**:
- **Magic Number**: `0x1BADB002` (definido en el header Multiboot de `kmain.S`)
- **Flags**: `ALIGN | MEMINFO` (bit 0 y bit 1)
  - `ALIGN`: Alinear módulos cargados en límites de página
  - `MEMINFO`: Proporcionar mapa de memoria al kernel
- **Checksum**: `-(MAGIC + FLAGS)` para validación

**Función del GRUB**:
- Detecta el header Multiboot en el binario del kernel
- Carga el kernel en la dirección `0x00100000` (1MB)
- Configura el CPU en modo protegido de 32 bits
- Deshabilita las interrupciones
- Deshabilita la paginación (el kernel la habilitará después)
- Pasa control al entry point del kernel con:
  - **EAX**: Magic number de Multiboot (`0x2BADB002`)
  - **EBX**: Dirección física de la estructura `multiboot_info_t`

### 2. Entry Point en Assembly (`_start`)
**Ubicación**: `src/kernel/arch/x86/boot/kmain.S`

El entry point `_start` es la primera función que se ejecuta en el kernel:

1. **Configuración de la Pila**:
   - Reserva 16KB (16384 bytes) en la sección `.bss`
   - Configura ESP para apuntar a `stack_top` (la pila crece hacia abajo)
   - Esta pila será usada durante toda la inicialización del kernel

2. **Preservación de Parámetros**:
   - Guarda EAX (magic number) en la pila
   - Guarda EBX (puntero a `multiboot_info_t`) en la pila
   - Estos valores se pasan como argumentos a `kernel_main()`

3. **Llamada a kernel_main**:
   - Ejecuta `call kernel_main`
   - Pasa los dos argumentos según la convención de llamada de C (cdecl)

4. **Bucle Infinito de Seguridad**:
   - Si `kernel_main()` retorna (no debería), ejecuta `cli` para deshabilitar interrupciones
   - Entra en un bucle infinito con `hlt` y `jmp` para manejar NMIs

### 3. Inicialización del Driver VGA
**Función**: `vga_init()` en `src/kernel/drivers/src/vga.c`

Lo primero que hace `kernel_main()` es inicializar el driver VGA para poder mostrar mensajes:
- Configura el buffer VGA en `0xB8000`
- Establece el modo texto 80x25
- Inicializa el cursor en (0, 0)
- Establece el color predeterminado: gris claro sobre negro
- Limpia la pantalla completa

### 4. Parseo de Parámetros de Línea de Comandos
El kernel lee la estructura `multiboot_info_t` para obtener los parámetros:

**Parámetros disponibles**:
- `--debug`: Activa mensajes de depuración detallados
- `--verbose`: Activa salida detallada de inicialización  
- `--no-subsystems`: Detiene el kernel antes de inicializar subsistemas (útil para testing)

### 5. Inicialización de Configuración del Kernel
**Función**: `kconfig_init()` en `src/kernel/core/src/kconfig.c`

Establece las variables globales de configuración:
- `kernel_debug_mode`: Almacena el estado del modo debug
- `kernel_verbose_mode`: Almacena el estado del modo verbose

Proporciona funciones helper inline:
- `is_kdebug()`: Retorna true si debug está activo
- `is_kverbose()`: Retorna true si verbose está activo

### 6. Verificación del Bootloader Multiboot
Valida que el bootloader sea compatible con Multiboot. Si el magic number no es `0x2BADB002`, muestra error en rojo y detiene el kernel con `hlt`.

### 7. Banner de Bienvenida  
Si el modo verbose está activo, muestra:
- ASCII art del logo de NeoOS
- Versión del kernel (`KERNEL_VERSION` = "0.1.0")
- Fecha de compilación (`BUILD_DATE`)

### 8. Información de Memoria
Lee y muestra información de memoria desde Multiboot:
- `mem_lower`: Memoria inferior (0-640KB)
- `mem_upper`: Memoria superior (desde 1MB)
- Calcula y muestra memoria total en MB o GB

### 9. Inicialización del Memory Manager
**Función**: `memory_init()` en `src/kernel/memory/src/memory.c`

Inicializa tres subsistemas críticos en orden:

#### a) PMM (Physical Memory Manager)
**Función**: `pmm_init()` en `src/kernel/memory/src/pmm.c`
- Parsea el mapa de memoria de Multiboot para identificar regiones disponibles
- Crea un bitmap donde cada bit representa una página de 4KB
- Coloca el bitmap inmediatamente después del kernel (`kernel_end`)
- Marca como ocupadas las páginas del kernel y del bitmap
- Marca como libres las regiones `MULTIBOOT_MEMORY_AVAILABLE`

#### b) VMM (Virtual Memory Manager)
**Función**: `vmm_init()` en `src/kernel/memory/src/vmm.c`
- Usa estructuras estáticas para el directorio y las tablas de páginas
- Crea identity mapping de los primeros 128MB (dirección virtual = dirección física)
- Utiliza 32 tablas de páginas estáticas (cada una mapea 4MB)
- Configura cada entrada con flags: `PAGE_PRESENT | PAGE_WRITE`
- Carga el directorio de páginas en CR3
- Habilita la paginación (bit 31 de CR0)

#### c) Heap del Kernel
**Función**: `heap_init()` en `src/kernel/memory/src/heap.c`
- Configura el heap en `0x00400000` (4MB) con tamaño de 4MB
- Implementa asignación dinámica con lista enlazada de bloques
- Cada bloque tiene un header con magic number (`0x12345678`) para validación
- Soporta expansión dinámica del heap bajo demanda

**Nota**: Si cualquiera de estos pasos falla, el kernel muestra el error y se detiene.

### 10. Inicialización del Sistema de Interrupciones
**Funciones**: `gdt_init()`, `idt_init()`, `interrupts_init()` en `src/kernel/core/src/`

#### a) GDT (Global Descriptor Table)
Configura 5 entradas básicas:
- Entrada 0: Descriptor nulo (requerido por x86)
- Entrada 1: Segmento de código del kernel (Ring 0, 4GB)
- Entrada 2: Segmento de datos del kernel (Ring 0, 4GB)
- Entrada 3: Segmento de código de usuario (Ring 3, 4GB)
- Entrada 4: Segmento de datos de usuario (Ring 3, 4GB)

#### b) IDT (Interrupt Descriptor Table)
Inicializa la tabla con 256 entradas para:
- ISR 0-31: Excepciones del CPU
- IRQ 0-15: Interrupciones de hardware (mapeadas a ISR 32-47)

#### c) Sistema de Interrupciones
- Remapea el PIC (Programmable Interrupt Controller)
  - PIC1: IRQ 0-7 → ISR 32-39
  - PIC2: IRQ 8-15 → ISR 40-47
- Configura los handlers de ISR e IRQ en assembly (`src/kernel/arch/x86/isr.S`)
- Habilita interrupciones con `sti`

### 11. Estado Final
Después de completar la inicialización:
- Si el flag `--no-subsystems` está presente, el kernel se detiene aquí
- Muestra mensaje "Subsistemas del kernel iniciados correctamente"
- Entra en un bucle infinito con `hlt`

**Subsistemas pendientes de implementación**:
- Planificador de procesos
- Sistema de archivos (NeoFS)
- IPC (Comunicación entre procesos)
- Module Manager
- Drivers de hardware (timer, teclado, disco)
- Transición a modo usuario (Ring 3)
- Proceso init

## Configuración de GRUB
**Archivo**: `src/kernel/grub.cfg`

```cfg
set timeout=5
set default=0

menuentry "NeoOS" {
multiboot /boot/neoos
boot
}
```

### Personalización de Parámetros
Para activar debug y verbose durante el arranque:
```cfg
menuentry "NeoOS (Debug & Verbose)" {
multiboot /boot/neoos --debug --verbose
boot
}
```

Para testing sin subsistemas:
```cfg
menuentry "NeoOS (Test Mode)" {
multiboot /boot/neoos --debug --verbose --no-subsystems
boot
}
```

## Características de la Implementación Actual

### Seguridad y Validación
- Verificación del magic number de Multiboot en cada arranque
- Validación de estructuras con magic numbers (`0x12345678` en heap)
- Detención segura del kernel con `cli; hlt` ante errores críticos

### Identity Mapping
Los primeros 128MB de memoria están mapeados 1:1:
- Dirección virtual = Dirección física
- Simplifica el acceso inicial a hardware y estructuras
- Usa 32 tablas de páginas estáticas (4MB cada una)

### Manejo de Errores
- Códigos de error estandarizados (ver `error.h`)
- Función `error_to_string()` para mensajes legibles
- Mensajes de error en color rojo con información detallada
- Kernel panic con información de registros en excepciones

## Notas Adicionales
- El kernel arranca en modo protegido de 32 bits (x86)
- La pila del kernel es de 16KB, suficiente para la inicialización
- El bitmap del PMM se coloca dinámicamente después del kernel
- El identity mapping de 128MB cubre el kernel, el heap y estructuras críticas
- Las interrupciones están deshabilitadas hasta que el sistema de interrupciones esté completamente inicializado
- El kernel no retorna nunca de `kernel_main()`

## Véase También
- [Kernel Initialization](./Kernel%20Initialization.md) - Detalles de cada paso de inicialización
- [Memory Manager](./Memory%20Manager.md) - Arquitectura del gestor de memoria
- [Interrupts](./Interrupts.md) - Sistema de interrupciones y excepciones
- [Errors](./Errors.md) - Códigos de error y manejo
- [Multiboot Specification](https://www.gnu.org/software/grub/manual/multiboot/multiboot.html) - Especificación oficial
- [GRUB Manual](https://www.gnu.org/software/grub/manual/grub/grub.html) - Documentación de GRUB
