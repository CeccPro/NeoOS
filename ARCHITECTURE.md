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
   ├── Carga initimage.img (módulos esenciales)
   └── Pasa información Multiboot
   │
   ▼
3. boot.asm (_start)
   │
   ├── Configura stack
   ├── Guarda información Multiboot
   └── Salta a kernel_main()
   │
   ▼
4. kernel_main.c
   │
   ├── Inicializa VGA Terminal
   ├── Verifica Multiboot Magic
   ├── Inicializa GDT
   ├── Inicializa IDT
   ├── Configura PIC
   ├── Habilita interrupciones
   │
   ▼
5. Inicialización de Subsistemas
   │
   ├── Memory Manager
   │   ├── Parsea mapa de memoria de Multiboot
   │   ├── Inicializa PMM (bitmap de páginas físicas)
   │   ├── Inicializa VMM (paginación)
   │   └── Configura heap del kernel
   │
   ├── Process Scheduler
   │   ├── Crea proceso IDLE (PID 0)
   │   └── Prepara estructuras de procesos
   │
   ├── IPC Manager
   │   └── Inicializa colas de mensajes
   │
   └── Module Manager
       ├── Carga InitImage
       └── Inicializa módulos esenciales
   │
   ▼
6. Transición a Modo Usuario
   │
   └── Inicia proceso init (PID 1)
       └── Shell/UI
```

## Estructura de Archivos Detallada

### Core Kernel
- `boot/boot.asm` - Código de arranque en Assembly
- `core/kernel_main.c` - Punto de entrada principal del kernel

### Headers
- `include/kernel.h` - Definiciones globales del kernel
- `include/multiboot.h` - Protocolo Multiboot
- `include/errors.h` - Códigos de error del sistema
- `include/process.h` - PCB y estructuras de proceso
- `include/scheduler.h` - API del planificador
- `include/memory.h` - API del gestor de memoria
- `include/ipc.h` - API de comunicación entre procesos
- `include/module.h` - API del gestor de módulos
- `include/syscall.h` - Definiciones de syscalls
- `include/vga.h` - Driver de terminal VGA

### Drivers
- `drivers/vga.c` - Terminal VGA en modo texto (✅ Implementado)
- `drivers/keyboard.c` - Driver de teclado (⏳ Pendiente)
- `drivers/timer.c` - Driver de timer PIT (⏳ Pendiente)

### Subsistemas (Pendientes de Implementar)
- `mm/` - Memory Manager
  - `pmm.c` - Physical Memory Manager
  - `vmm.c` - Virtual Memory Manager
  - `heap.c` - Heap del kernel
- `scheduler/` - Process Scheduler
  - `scheduler.c` - Planificador de procesos
  - `process.c` - Gestión de procesos
  - `context_switch.asm` - Cambio de contexto
- `ipc/` - IPC Manager
  - `ipc.c` - Comunicación entre procesos
  - `message_queue.c` - Colas de mensajes
- `modules/` - Module Manager
  - `module_loader.c` - Cargador de módulos
- `core/` - Core del Kernel
  - `gdt.c` - Global Descriptor Table
  - `idt.c` - Interrupt Descriptor Table
  - `interrupts.c` - Handlers de interrupciones
  - `syscall.c` - Handler de syscalls

## Syscalls Definidas (29 total)

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

## Códigos de Error

- `E_OK` (0) - Éxito
- `E_NOMEM` (1) - Sin memoria
- `E_INVAL` (2) - Argumento inválido
- `E_PERM` (3) - Sin permisos
- `E_NOENT` (4) - No existe
- `E_IO` (5) - Error I/O
- `E_BUSY` (6) - Recurso ocupado
- `E_EXISTS` (7) - Ya existe
- `E_TIMEOUT` (8) - Timeout
- `E_NOT_IMPL` (9) - No implementado
- `E_NOT_SUPPORTED` (10) - No soportado
- `E_MODULE_ERR` (11) - Error de módulo
- `E_UNKNOWN` (255) - Error desconocido

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
