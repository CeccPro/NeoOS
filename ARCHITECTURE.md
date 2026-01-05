# NeoOS Kernel - Arquitectura y Estructura

## Visión General

```
┌─────────────────────────────────────────────────────────────┐
│                      MODO USUARIO                            │
│                                                              │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐   │
│  │  NeoUI   │  │  NeoSH   │  │ Proceso  │  │ Proceso  │   │
│  │ (WebView)│  │ (Shell)  │  │    1     │  │    2     │   │
│  └────┬─────┘  └────┬─────┘  └────┬─────┘  └────┬─────┘   │
│       │             │             │             │           │
│       └─────────────┴─────────────┴─────────────┘           │
│                            │                                 │
│                     [SYSCALL Interface]                      │
└────────────────────────────┼────────────────────────────────┘
                             │
┌────────────────────────────┼────────────────────────────────┐
│                      MODO KERNEL                             │
│                            │                                 │
│    ┌───────────────────────▼─────────────────────────┐      │
│    │         Syscall Handler (syscall.h)             │      │
│    └───────────────────────┬─────────────────────────┘      │
│                            │                                 │
│    ┌───────────────────────┴─────────────────────────┐      │
│    │           NeoCore Core (kernel_main.c)        │      │
│    │  ┌────────────┐  ┌──────────┐  ┌────────────┐  │      │
│    │  │    IDT     │  │   GDT    │  │    PIC     │  │      │
│    │  │ Interrupts │  │ Segments │  │  Timer     │  │      │
│    │  └────────────┘  └──────────┘  └────────────┘  │      │
│    └───────────────────────┬─────────────────────────┘      │
│                            │                                 │
│    ┌───────────────────────┴─────────────────────────┐      │
│    │         Subsistemas del Kernel                  │      │
│    │                                                  │      │
│    │  ┌──────────────────────────────────────────┐   │      │
│    │  │   Memory Manager (memory.h)              │   │      │
│    │  │   - PMM (Physical Memory Manager)        │   │      │
│    │  │   - VMM (Virtual Memory Manager)         │   │      │
│    │  │   - Heap (kmalloc/kfree)                 │   │      │
│    │  └──────────────────────────────────────────┘   │      │
│    │                                                  │      │
│    │  ┌──────────────────────────────────────────┐   │      │
│    │  │   Process Scheduler (scheduler.h)        │   │      │
│    │  │   - Process Management                   │   │      │
│    │  │   - Context Switching                    │   │      │
│    │  │   - Priority Queues                      │   │      │
│    │  └──────────────────────────────────────────┘   │      │
│    │                                                  │      │
│    │  ┌──────────────────────────────────────────┐   │      │
│    │  │   IPC Manager (ipc.h)                    │   │      │
│    │  │   - Message Queues                       │   │      │
│    │  │   - Signals                              │   │      │
│    │  │   - Shared Memory                        │   │      │
│    │  └──────────────────────────────────────────┘   │      │
│    │                                                  │      │
│    │  ┌──────────────────────────────────────────┐   │      │
│    │  │   Module Manager (module.h)              │   │      │
│    │  │   - Dynamic Module Loading               │   │      │
│    │  │   - Module Dependencies                  │   │      │
│    │  │   - Hotplug Support                      │   │      │
│    │  └──────────────────────────────────────────┘   │      │
│    │                                                  │      │
│    └──────────────────────────────────────────────────┘      │
│                                                              │
│    ┌─────────────────────────────────────────────────┐      │
│    │              Drivers (drivers/)                 │      │
│    │  ┌─────────┐  ┌─────────┐  ┌─────────┐        │      │
│    │  │   VGA   │  │Keyboard │  │  Disk   │  ...   │      │
│    │  └─────────┘  └─────────┘  └─────────┘        │      │
│    └─────────────────────────────────────────────────┘      │
└─────────────────────────────────────────────────────────────┘
                             │
┌────────────────────────────┼────────────────────────────────┐
│                        HARDWARE                              │
│    ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐ │
│    │   CPU    │  │  Memory  │  │   Disk   │  │   I/O    │ │
│    └──────────┘  └──────────┘  └──────────┘  └──────────┘ │
└─────────────────────────────────────────────────────────────┘
```

## Flujo de Arranque

```
1. BIOS/UEFI
   │
   ▼
2. GRUB (Bootloader)
   │
   ├── Carga neoos en memoria (0x00100000 - 1MB)
   ├── Pasa información Multiboot (magic + multiboot_info)
   └── Puede pasar parámetros: --debug, --verbose
   │
   ▼
3. kmain.S (_start)
   │
   ├── Configura stack (16KB)
   ├── Preserva magic number (EAX) y multiboot_info (EBX)
   └── Salta a kernel_main()
   │
   ▼
4. kernel_main() [src/kernel/core/src/kmain.c]
   │
   ├── Inicializa VGA Terminal (modo texto 80x25)
   ├── Parsea cmdline (--debug, --verbose)
   ├── Inicializa kconfig (configuración global)
   ├── Verifica Multiboot Magic (0x2BADB002)
   ├── Muestra banner de bienvenida (si verbose)
   ├── Muestra información de memoria
   │
   ▼
5. Inicialización del Memory Manager [IMPLEMENTADO]
   │
   ├── PMM (Physical Memory Manager)
   │   ├── Parsea mapa de memoria de Multiboot
   │   ├── Crea bitmap de páginas físicas
   │   └── Marca regiones disponibles/ocupadas
   │
   ├── VMM (Virtual Memory Manager)
   │   ├── Configura directorio de páginas del kernel
   │   ├── Crea identity mapping (0-128MB)
   │   └── Habilita paginación (bit PG en CR0)
   │
   └── Heap del Kernel
       ├── Inicializa en 0x00400000 (4MB)
       ├── Tamaño: 4MB
       └── Proporciona kmalloc/kfree
   │
   ▼
6. Bucle Infinito (hlt) [ESTADO ACTUAL]


=== PENDIENTES DE IMPLEMENTACIÓN ===

7. GDT, IDT, Interrupciones [PENDIENTE]
   │
   ├── Inicializa GDT (Global Descriptor Table)
   ├── Inicializa IDT (Interrupt Descriptor Table)
   ├── Configura PIC (Programmable Interrupt Controller)
   ├── Inicializa timer PIT (Programmable Interval Timer)
   └── Habilita interrupciones

8. Process Scheduler [PENDIENTE]
   │
   ├── Crea proceso IDLE (PID 0)
   └── Prepara estructuras de procesos

9. IPC Manager [PENDIENTE]
   │
   └── Inicializa colas de mensajes

10. Module Manager [PENDIENTE]
    │
    └── Sistema de carga dinámica de módulos

11. Sistema de Archivos (NeoFS) [PENDIENTE]

12. Transición a Modo Usuario [PENDIENTE]
    │
    └── Inicia proceso init (PID 1)
        └── Shell/UI
```

## Estructura de Archivos Detallada

### Estructura Real del Código

**src/kernel/** - Código del kernel

#### Arquitectura (arch/)
- `arch/x86/boot/kmain.S` - Entry point en Assembly [Implementado]
- `arch/arm/boot/kmain.S` - Entry point ARM [Pendiente]

#### Core (core/)
- `core/src/kmain.c` - Función principal del kernel [Implementado]
- `core/src/kconfig.c` - Configuración global del kernel [Implementado]
- `core/src/error.c` - Conversión de códigos de error a strings [Implementado]
- `core/include/kmain.h` - Definiciones del kernel main [Implementado]
- `core/include/kconfig.h` - Variables de configuración [Implementado]
- `core/include/error.h` - Códigos de error del sistema [Implementado]

#### Drivers (drivers/)
- `drivers/src/vga.c` - Terminal VGA en modo texto 80x25 [Implementado]
- `drivers/include/vga.h` - API del driver VGA [Implementado]
- `drivers/keyboard.c` - Driver de teclado [Pendiente]
- `drivers/timer.c` - Driver de timer PIT [Pendiente]

#### Librería (lib/)
- `lib/src/string.c` - Funciones de strings [Implementado]
- `lib/include/string.h` - API de strings [Implementado]
- `lib/include/types.h` - Tipos básicos del sistema [Implementado]
- `lib/include/multiboot.h` - Protocolo Multiboot [Implementado]

#### Memory Manager (memory/) - [IMPLEMENTADO]
- `memory/src/memory.c` - Coordinador del Memory Manager [Implementado]
- `memory/src/pmm.c` - Physical Memory Manager [Implementado]
- `memory/src/vmm.c` - Virtual Memory Manager [Implementado]
- `memory/src/heap.c` - Heap del kernel [Implementado]
- `memory/include/memory.h` - API completa del Memory Manager [Implementado]

#### Subsistemas Pendientes
- `scheduler/` - Process Scheduler [NO EXISTE AÚN]
- `ipc/` - IPC Manager [NO EXISTE AÚN]
- `modules/` - Module Manager [NO EXISTE AÚN]
- `fs/` - Sistema de archivos [NO EXISTE AÚN]
- `syscalls/` - Syscall handler [NO EXISTE AÚN]

#### Otros
- `linker.ld` - Linker script [Implementado]
- `grub.cfg` - Configuración de GRUB [Implementado]
- `Makefile` - Build system del kernel [Implementado]

## Syscalls Definidas (29 total) - [PENDIENTE DE IMPLEMENTACIÓN]

**NOTA**: Las syscalls están definidas conceptualmente en la documentación, pero **NO están implementadas** en el código actual. El handler de syscalls y el mecanismo de invocación aún no existen.

### Gestión de Procesos
- `SYS_EXIT` (0) - Terminar proceso
- `SYS_GETPID` (5) - Obtener PID
- `SYS_FORK` (6) - Fork/Clone proceso
- `SYS_YIELD` (7) - Ceder CPU
- `SYS_SLEEP` (8) - Dormir proceso
- `SYS_KILL` (19) - Terminar otro proceso
- `SYS_WAIT` (20) - Esperar por proceso hijo
- `SYS_CLONE` (21) - Clonar proceso
- `SYS_RUN` (22) - Ejecutar programa
- `SYS_SIGNAL` (23) - Enviar señal

### Memoria
- `SYS_MMAP` (11) - Mapear memoria
- `SYS_MUNMAP` (12) - Desmapear memoria
- `SYS_SBRK` (13) - Ajustar heap

### IPC
- `SYS_IPC_SEND` (14) - Enviar mensaje
- `SYS_IPC_RECV` (15) - Recibir mensaje
- `SYS_IPC_FREE` (16) - Liberar mensaje

### Prioridad
- `SYS_SETPRIORITY` (17) - Establecer prioridad
- `SYS_GETPRIORITY` (18) - Obtener prioridad

### Tiempo
- `SYS_GETTIME` (9) - Obtener tiempo actual
- `SYS_UPTIME` (10) - Tiempo de actividad del sistema

### Archivos
- `SYS_READ` (1) - Leer archivo
- `SYS_WRITE` (2) - Escribir archivo
- `SYS_OPEN` (3) - Abrir archivo
- `SYS_CLOSE` (4) - Cerrar archivo
- `SYS_STAT` (24) - Información de archivo
- `SYS_UNLINK` (28) - Eliminar archivo

### Directorios
- `SYS_MKDIR` (25) - Crear directorio
- `SYS_RMDIR` (26) - Eliminar directorio
- `SYS_CHDIR` (27) - Cambiar directorio

## Códigos de Error - [IMPLEMENTADOS]

Definidos en `src/kernel/core/include/error.h` e implementados en `src/kernel/core/src/error.c`:

- `E_OK` (0) - Éxito
- `E_UNKNOWN` (-1) - Error desconocido
- `E_NOMEM` (-2) - Sin memoria
- `E_INVAL` (-3) - Argumento inválido
- `E_NOENT` (-4) - No existe
- `E_EXISTS` (-5) - Ya existe
- `E_BUSY` (-6) - Recurso ocupado
- `E_IO` (-7) - Error I/O
- `E_PERM` (-8) - Sin permisos
- `E_TIMEOUT` (-9) - Timeout
- `E_MODULE_ERR` (-10) - Error de módulo
- `E_NOT_IMPL` (-11) - No implementado
- `E_NOT_SUPPORTED` (-12) - No soportado

La función `error_to_string(int error)` convierte códigos de error a sus nombres como strings.

## Convenciones de Código

### Nombres de Archivos
- Headers: `nombre.h`
- Implementación: `nombre.c`
- Assembly: `nombre.asm`

### Funciones
- Públicas: `subsistema_funcion()` ej: `memory_init()`, `scheduler_create_process()`
- Privadas/estáticas: `subsistema_funcion_interna()` o simplemente usar `static`

### Comentarios
- Usar Doxygen para documentar funciones públicas
- Comentarios inline para lógica compleja

### Tipos
- Usar typedefs para claridad: `pid_t`, `mid_t`, `error_t`
- Structs terminan en `_t`: `process_t`, `module_t`

## Recursos de Desarrollo

### Herramientas Necesarias
- GCC 11+ (cross-compiler para i686)
- NASM 2.15+
- LD (GNU Binutils)
- GRUB 2.x
- QEMU (para testing)

### Referencias
- [OSDev Wiki](https://wiki.osdev.org/)
- [Intel Software Developer Manuals](https://www.intel.com/sdm)
- [Multiboot Specification](https://www.gnu.org/software/grub/manual/multiboot/)
