# NeoOS - Kernel Initialization
La inicialización del kernel en NeoOS es un proceso fundamental que establece las bases para el funcionamiento del sistema operativo. Durante esta fase, el kernel configura los recursos del sistema, inicializa los controladores de hardware esenciales y prepara el entorno necesario para futuros subsistemas.

## Proceso de Inicialización
El proceso de inicialización del kernel de NeoOS se lleva a cabo en las siguientes etapas:

### 1. Entry Point en Assembly (`_start`)
- Ubicado en `src/kernel/arch/x86/boot/kmain.S`
- Configura la pila del kernel (16KB)
- Preserva el magic number de Multiboot (EAX) y el puntero a multiboot_info (EBX)
- Llama a `kernel_main()` pasando estos parámetros

### 2. Inicialización del Driver VGA
- **Función**: `vga_init()` en `src/kernel/drivers/src/vga.c`
- Configura el terminal VGA en modo texto 80x25
- Establece el color predeterminado (gris claro sobre negro)
- Limpia la pantalla

### 3. Parseo de Parámetros de Línea de Comandos
- Lee la estructura `multiboot_info_t` para obtener cmdline
- Busca los flags `--debug` y `--verbose`
- Almacena temporalmente estos valores

### 4. Inicialización de Configuración del Kernel
- **Función**: `kconfig_init()` en `src/kernel/core/src/kconfig.c`
- Establece las variables globales:
  - `kernel_debug_mode`: controla mensajes de depuración
  - `kernel_verbose_mode`: controla salida detallada
- Proporciona funciones helper: `is_kdebug()` y `is_kverbose()`

### 5. Verificación del Bootloader Multiboot
- Verifica que el magic number sea `0x2BADB002`
- Si falla, muestra error en rojo y detiene el kernel con `hlt`

### 6. Banner de Bienvenida (si verbose está activo)
- Muestra ASCII art del logo de NeoOS
- Muestra la versión del kernel (definida en `kmain.h` como `KERNEL_VERSION`)
- Muestra la fecha de compilación (`BUILD_DATE`)

### 7. Información de Memoria
- Lee información de memoria de `multiboot_info_t`:
  - `mem_lower`: memoria inferior en KB (0-640KB)
  - `mem_upper`: memoria superior en KB (desde 1MB)
- Calcula y muestra memoria total en MB o GB (si verbose está activo)

### 8. Inicialización del Memory Manager
- **Función**: `memory_init()` en `src/kernel/memory/src/memory.c`
- Inicializa tres subsistemas:
  1. **PMM** (Physical Memory Manager): Gestiona páginas físicas usando bitmap
  2. **VMM** (Virtual Memory Manager): Configura paginación con identity mapping de 128MB
  3. **Heap**: Configura heap del kernel en `0x00400000` (4MB) con tamaño de 4MB
- Si falla cualquier componente, muestra error y detiene el kernel

### 9. Estado Actual
El kernel actualmente se detiene aquí y entra en un bucle infinito con `hlt`. Los siguientes subsistemas están **pendientes de implementación**:
- Planificador de procesos
- Sistema de archivos (NeoFS)
- IPC (Comunicación entre procesos)
- Module Manager
- Drivers adicionales (teclado, timer, disco)
- Transición a modo usuario
- Proceso init

## Archivos Involucrados
- `src/kernel/arch/x86/boot/kmain.S`: Entry point en assembly
- `src/kernel/core/src/kmain.c`: Función principal del kernel
- `src/kernel/core/include/kmain.h`: Definiciones y prototipos
- `src/kernel/core/src/kconfig.c`: Configuración global del kernel
- `src/kernel/core/include/kconfig.h`: Variables de configuración
- `src/kernel/drivers/src/vga.c`: Driver VGA
- `src/kernel/memory/src/memory.c`: Inicialización del Memory Manager
- `src/kernel/memory/src/pmm.c`: Physical Memory Manager
- `src/kernel/memory/src/vmm.c`: Virtual Memory Manager
- `src/kernel/memory/src/heap.c`: Heap del kernel

## Notas Adicionales
- La inicialización del kernel es crítica para la estabilidad del sistema. Cualquier fallo durante esta fase resulta en un kernel panic (detención con `hlt`).
- El kernel utiliza un enfoque simple y directo en esta fase inicial, sin manejo complejo de errores.
- Los mensajes de depuración y verbose son útiles para desarrollo y troubleshooting.
- El sistema de módulos dinámicos mencionado en documentación antigua **no está implementado aún**.

## Véase también
- [Boot Process](./Boot%20Process.md)
- [Memory Manager](./Memory%20Manager.md)
- [Errors](./Errors.md)