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
   ├── Parsea cmdline (--debug, --verbose, --no-subsystems)
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
6. Inicialización del Sistema de Interrupciones [IMPLEMENTADO]
   │
   ├── GDT (Global Descriptor Table)
   │   ├── Segmento de código del kernel (ring 0)
   │   ├── Segmento de datos del kernel (ring 0)
   │   ├── Segmento de código de usuario (ring 3) [pendiente]
   │   └── Segmento de datos de usuario (ring 3) [pendiente]
   │
   ├── IDT (Interrupt Descriptor Table)
   │   ├── 256 entradas de interrupciones
   │   ├── ISR (Interrupt Service Routines) 0-31
   │   └── IRQ (Hardware Interrupts) 32-47
   │
   ├── PIC (Programmable Interrupt Controller)
   │   ├── Remapeo de IRQs (evitar conflicto con excepciones)
   │   ├── IRQ0-15 → IDT 32-47
   │   └── Máscaras de interrupciones
   │
   └── Habilita interrupciones (STI)
   │
   ▼
7. Inicialización del PIT (Timer) [IMPLEMENTADO]
   │
   ├── Frecuencia: 100 Hz (configurable)
   ├── Genera IRQ0 cada 10ms
   └── Utilizado por el scheduler para preemption
   │
   ▼
8. Inicialización del Scheduler [IMPLEMENTADO]
   │
   ├── Crea proceso IDLE (PID 1)
   │   ├── Prioridad: PROCESS_PRIORITY_IDLE
   │   └── Ejecuta HLT en loop infinito
   │
   ├── Algoritmo: Round Robin con prioridades
   │   ├── 5 niveles de prioridad (IDLE, LOW, NORMAL, HIGH, REALTIME)
   │   ├── Quantum por prioridad (10ms - 100ms)
   │   └── Context switching automático por IRQ0
   │
   ├── PCB (Process Control Block)
   │   ├── PID, nombre, estado, prioridad
   │   ├── Contexto CPU (ESP, EBP, EIP, EFLAGS, etc.)
   │   ├── Cola IPC integrada
   │   └── Información de memoria
   │
   └── Funciones disponibles:
       ├── scheduler_create_process()
       ├── scheduler_terminate_process()
       ├── scheduler_yield()
       ├── scheduler_set_priority()
       └── scheduler_block/unblock_process()
   │
   ▼
9. Inicialización del IPC [IMPLEMENTADO]
   │
   ├── Sistema de mensajería asíncrona
   │   ├── Tamaño máximo de mensaje: 4KB
   │   ├── Cola por proceso: hasta 32 mensajes
   │   └── Modo bloqueante y no bloqueante
   │
   ├── Funciones implementadas:
   │   ├── ipc_send(dest_pid, msg, size)
   │   ├── ipc_recv(msg, flags)
   │   └── ipc_free(msg)
   │
   └── Demo funcional: Marco-Polo
       ├── Proceso Marco (PID 2) envía "Marco"
       ├── Proceso Polo (PID 3) responde "Polo"
       └── 5 rounds de comunicación IPC
   │
   ▼
10. Inicialización de Syscalls [IMPLEMENTADO]
    │
    ├── Instala handler para int 0x80
    ├── Configura IDT entrada 128 (0x80)
    └── Proporciona 15 syscalls del microkernel
    │
    ▼
11. Demo Marco-Polo [IMPLEMENTADO]
    │
    ├── Crea proceso Marco (PID 2)
    ├── Crea proceso Polo (PID 3)
    ├── Ejecuta 5 rondas de comunicación IPC
    └── Demuestra funcionalidad completa de IPC
    │
    ▼
12. Transferencia de Control al Scheduler [IMPLEMENTADO]
    │
    ├── scheduler_switch() nunca retorna
    ├── Sistema ejecuta procesos en Round Robin
    └── Preemption automática cada 10ms (IRQ0)


=== ESTADO ACTUAL ===
El kernel está funcional con:
- Memory Manager (PMM, VMM, Heap)
- Sistema de interrupciones (GDT, IDT, PIC, PIT)
- Scheduler multitarea con prioridades
- IPC completamente funcional
- Syscall Handler (int 0x80)
- Demo Marco-Polo funcionando

[EJECUTANDO PROCESOS EN MODO KERNEL]


=== PENDIENTES DE IMPLEMENTACIÓN ===

13. Transición a Modo Usuario [PENDIENTE]
    │
    ├── Configurar segmentos de usuario en GDT
    ├── TSS (Task State Segment)
    ├── Cambio de ring 0 → ring 3
    └── Stack de usuario separado

14. Libneo (Librería en Userspace) [PENDIENTE]
    │
    ├── Wrappers de syscalls
    ├── Funciones estándar: run(), clone(), kill()
    ├── Gestión de heap: malloc(), free(), sbrk()
    └── Compatibilidad POSIX básica

15. Servidores en Userspace [PENDIENTE]
    │
    ├── VFS Server: Gestión de archivos
    ├── Process Server: Fork, exec, gestión de PIDs
    └── Device Manager: Drivers en userspace

16. Module Manager [FUTURO]
    │
    └── Sistema de carga dinámica de módulos

17. Sistema de Archivos (NeoFS) [FUTURO]
    │
    └── Implementado como VFS Server en userspace
```

## Estructura de Archivos Detallada

### Estructura Real del Código

**src/kernel/** - Código del kernel

#### Arquitectura (arch/)
- `arch/x86/boot/kmain.S` - Entry point en Assembly [Implementado]
- `arch/arm/boot/kmain.S` - Entry point ARM [Pendiente]

#### Core (core/) - [IMPLEMENTADO]
- `core/src/kmain.c` - Función principal del kernel [Implementado]
- `core/src/kconfig.c` - Configuración global del kernel [Implementado]
- `core/src/error.c` - Conversión de códigos de error a strings [Implementado]
- `core/src/gdt.c` - Global Descriptor Table [Implementado]
- `core/src/idt.c` - Interrupt Descriptor Table [Implementado]
- `core/src/interrupts.c` - Manejo de ISR/IRQ [Implementado]
- `core/src/timer.c` - Programmable Interval Timer [Implementado]
- `core/src/scheduler.c` - Planificador de procesos [Implementado]
- `core/src/ipc.c` - Sistema de mensajería [Implementado]
- `core/src/syscall.c` - Syscall Dispatcher [Implementado]
- `core/include/kmain.h` - Definiciones del kernel main [Implementado]
- `core/include/kconfig.h` - Variables de configuración [Implementado]
- `core/include/error.h` - Códigos de error del sistema [Implementado]
- `core/include/gdt.h` - API de GDT [Implementado]
- `core/include/idt.h` - API de IDT [Implementado]
- `core/include/interrupts.h` - API de interrupciones [Implementado]
- `core/include/timer.h` - API del timer [Implementado]
- `core/include/scheduler.h` - API del scheduler [Implementado]
- `core/include/ipc.h` - API de IPC [Implementado]
- `core/include/syscall.h` - API de Syscalls [Implementado]

#### Arquitectura (arch/) - [IMPLEMENTADO]
- `arch/x86/boot/kmain.S` - Entry point en Assembly [Implementado]
- `arch/x86/context_switch.S` - Context switching [Implementado]
- `arch/x86/isr.S` - ISR/IRQ handlers [Implementado]
- `arch/arm/boot/kmain.S` - Entry point ARM [Pendiente]

#### Drivers (drivers/) - [IMPLEMENTADO]
- `drivers/src/early_vga.c` - Terminal VGA en modo texto 80x25 [Implementado]
- `drivers/include/early_vga.h` - API del driver VGA [Implementado]

#### Librería (lib/) - [IMPLEMENTADO]
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
- `modules/` - Module Manager [FUTURO]
- `fs/` - Sistema de archivos [FUTURO - Mover a userspace]
- Modo Usuario (ring 3) [PENDIENTE]
- Libneo (userspace library) [PENDIENTE]

#### Otros
- `linker.ld` - Linker script [Implementado]
- `grub.cfg` - Configuración de GRUB [Implementado]
- `Makefile` - Build system del kernel [Implementado]

## Syscalls de NeoOS (Microkernel - 15 syscalls)

**NOTA**: El dispatcher de syscalls mediante `int 0x80` está **IMPLEMENTADO y FUNCIONAL** en `src/kernel/core/src/syscall.c`.

### IPC/Comunicación (4 syscalls)
- `SYS_SEND` (0) - Enviar mensaje IPC
- `SYS_RECV` (1) - Recibir mensaje IPC
- `SYS_CALL` (2) - RPC: send+recv atómico
- `SYS_SIGNAL` (3) - Enviar señal a proceso

### Scheduler/Threads (6 syscalls)
- `SYS_THREAD_CREATE` (4) - Crear thread/proceso
- `SYS_THREAD_EXIT` (5) - Terminar thread actual
- `SYS_YIELD` (6) - Ceder CPU voluntariamente
- `SYS_SETPRIORITY` (7) - Establecer prioridad
- `SYS_GETPRIORITY` (8) - Obtener prioridad
- `SYS_WAIT` (9) - Esperar eventos/IRQs

### Memory Management (3 syscalls)
- `SYS_MAP` (10) - Mapear memoria
- `SYS_UNMAP` (11) - Desmapear memoria
- `SYS_GRANT` (12) - Compartir memoria con otro proceso

### Sistema (2 syscalls)
- `SYS_GETINFO` (13) - Obtener info del sistema (PID, tiempo, etc.)
- `SYS_DEBUG` (14) - Imprimir debug (solo builds debug)

**El resto de funcionalidad (filesystem, fork/exec, drivers) se implementa en userspace** mediante servidores que se comunican con IPC.

Ver [docs/Syscalls.md](docs/Syscalls.md) para detalles completos.

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
