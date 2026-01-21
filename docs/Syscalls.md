# NeoOS - Syscalls (Microkernel Architecture)

NeoOS implementa un diseño de **microkernel minimalista** donde solo las operaciones críticas residen en el kernel. El resto de funcionalidades (filesystem, gestión avanzada de procesos, drivers) se implementan como **servidores en modo usuario** que se comunican mediante IPC.

## Filosofía de Diseño

**En un microkernel:**
- El kernel solo proporciona mecanismos, no políticas
- Todo lo que pueda ejecutarse en modo usuario, debe hacerlo
- IPC eficiente es la piedra angular del sistema
- Seguridad por reducción de superficie de ataque

**Ventajas:**
- **Seguridad**: Menos código en kernel = menos vulnerabilidades
- **Modularidad**: Componentes reemplazables sin reiniciar
- **Debugging**: Los servidores crashean sin afectar el kernel
- **Flexibilidad**: Cambiar políticas sin recompilar el kernel

---

## Mecanismo de Syscalls

Las syscalls en NeoOS se invocan mediante la instrucción `int 0x80` (x86) o equivalente según arquitectura:

1. El proceso coloca el **número de syscall** en `EAX`
2. Los **parámetros** van en `EBX, ECX, EDX, ESI, EDI` (hasta 5 parámetros)
3. Se ejecuta `int 0x80` → transfer al kernel
4. El kernel verifica permisos, ejecuta la operación y retorna
5. El **valor de retorno** se devuelve en `EAX`

```c
// Ejemplo: sys_send(dest_pid, buffer, size, flags)
int result;
asm volatile(
    "int $0x80"
    : "=a"(result)
    : "a"(SYSCALL_SEND), "b"(dest_pid), "c"(buffer), "d"(size), "S"(flags)
);
```

---

## Syscalls del Kernel (~15 syscalls)

### IPC / Comunicación (ipc.h)
Las syscalls más importantes del microkernel. Todo el resto del sistema se construye sobre IPC.

| # | Syscall | Descripción |
|---|---------|-------------|
| 1 | `sys_send(pid_t dest, void *msg, size_t len, int flags)` | Envía un mensaje a otro proceso |
| 2 | `sys_recv(pid_t *src, void *buf, size_t len, int flags)` | Recibe un mensaje (bloqueante o no bloqueante) |
| 3 | `sys_call(pid_t dest, void *req, void *resp, size_t len)` | RPC: send + recv atómico |
| 4 | `sys_signal(pid_t pid, int sig)` | Envía una señal a un proceso |

**Flags soportados:**
- `IPC_BLOCK` (0x00): Bloquea hasta recibir mensaje
- `IPC_NONBLOCKING` (0x01): Retorna inmediatamente si no hay mensajes

### Scheduler / Threads (scheduler.h)

| # | Syscall | Descripción |
|---|---------|-------------|
| 5 | `sys_thread_create(void *entry, void *stack, int flags)` | Crea un nuevo thread/proceso |
| 6 | `sys_thread_exit(int status)` | Termina el thread actual |
| 7 | `sys_yield()` | Cede voluntariamente la CPU |
| 8 | `sys_setpriority(pid_t pid, int priority)` | Establece prioridad de un proceso |
| 9 | `sys_getpriority(pid_t pid)` | Obtiene prioridad de un proceso |
| 10 | `sys_wait(int *event_mask, timeout_t timeout)` | Espera eventos/IRQs |

**Prioridades:**
- `PROCESS_PRIORITY_IDLE` (0): Proceso idle
- `PROCESS_PRIORITY_LOW` (1): Baja prioridad
- `PROCESS_PRIORITY_NORMAL` (2): Normal (por defecto)
- `PROCESS_PRIORITY_HIGH` (3): Alta prioridad
- `PROCESS_PRIORITY_REALTIME` (4): Tiempo real

### Memory Management (memory.h)

| # | Syscall | Descripción |
|---|---------|-------------|
| 11 | `sys_map(void *addr, size_t len, int prot, int flags)` | Mapea memoria en el espacio de direcciones |
| 12 | `sys_unmap(void *addr, size_t len)` | Desmapea región de memoria |
| 13 | `sys_grant(pid_t dest, void *addr, size_t len, int prot)` | Comparte memoria con otro proceso |

**Flags de protección:**
- `PAGE_PRESENT`: Página presente en memoria
- `PAGE_WRITE`: Página escribible
- `PAGE_USER`: Accesible desde modo usuario

### Sistema (kmain.h)

| # | Syscall | Descripción |
|---|---------|-------------|
| 14 | `sys_getinfo(int type, void *buf)` | Obtiene información del sistema (PID, tiempo, etc.) |
| 15 | `sys_debug(const char *msg)` | Imprime mensaje de debug (solo en builds debug) |

**Tipos de info:**
- `INFO_PID`: PID del proceso actual
- `INFO_UPTIME`: Tiempo desde el boot
- `INFO_MEMORY`: Estadísticas de memoria

---

## Funciones en Userspace (libneo)

Estas funciones **NO son syscalls** sino wrappers en `libneo.so` que usan las syscalls básicas:

### Gestión de Procesos (implementadas con sys_thread_create + sys_signal)
```c
pid_t run(const char *path, char *const argv[]);   // Carga ejecutable NEO y crea proceso (formato NEO pendiente de definir)
pid_t clone(int flags, void *stack);                // Fork/clone del proceso actual
int kill(pid_t pid);                                 // Envía SIGKILL usando sys_signal
int wait(int *status);                               // Espera usando sys_wait + IPC
```

### Gestión de Memoria (implementadas con sys_map/sys_unmap)
```c
void *sbrk(intptr_t increment);  // Gestión de heap usando sys_map/sys_unmap
void *malloc(size_t size);        // Asignación dinámica en heap
void free(void *ptr);             // Liberación de memoria
```

---

## Servidores en Userspace

### 📁 VFS Server (vfs_server)
Maneja el sistema de archivos. Recibe peticiones vía IPC:

```c
// Aplicación en userspace:
int fd = open("file.txt", O_RDONLY);  // libneo envía mensaje IPC a vfs_server
read(fd, buf, 100);                   // IPC a vfs_server
close(fd);                            // IPC a vfs_server
```

**Operaciones soportadas:**
- `open, read, write, close`
- `mkdir, rmdir, unlink, stat`
- `chdir, getcwd`

### ⚙️ Process Server (proc_server)
Gestiona el ciclo de vida de procesos:
- Carga de ejecutables NEO (formato pendiente de definir)
- Fork/exec
- Gestión de PIDs
- Terminación de procesos

### Device Manager (dev_server)
Gestiona drivers y dispositivos:
- Mapeo de IRQs usando `sys_wait`
- Acceso a puertos I/O (con permisos)
- Hot-plug de dispositivos

---

## Ejemplos de Uso

### Enviar un mensaje IPC
```c
#include <neoos/ipc.h>

const char *msg = "Hola, servidor!";
int result = sys_send(server_pid, msg, strlen(msg) + 1, IPC_BLOCK);
if (result == E_OK) {
    printf("Mensaje enviado correctamente\n");
}
```

### Crear un thread
```c
#include <neoos/scheduler.h>

void thread_func(void) {
    printf("Thread iniciado\n");
    sys_thread_exit(0);
}

uint8_t stack[4096] __attribute__((aligned(16)));
pid_t thread_pid = sys_thread_create(thread_func, &stack[4096], 0);
```

### Mapear memoria compartida
```c
#include <neoos/memory.h>

// Proceso A: crea región compartida
void *shared = sys_map(NULL, 4096, PAGE_PRESENT | PAGE_WRITE | PAGE_USER, 0);
sys_grant(process_b_pid, shared, 4096, PAGE_WRITE);

// Proceso B: accede a la región
// (la región ya está mapeada gracias a sys_grant)
strcpy(shared, "Datos compartidos");
```

---

## Estado de Implementación

| Componente | Estado | Notas |
|------------|--------|-------|
| IPC (send/recv/free) | ✅ Implementado | Funcionando con demo Marco-Polo |
| Scheduler (create/exit/yield) | ✅ Implementado | Round-robin con prioridades |
| Memory (PMM/VMM/Heap) | ✅ Implementado | Paginación activa |
| Priority syscalls | ✅ Implementado | 5 niveles de prioridad |
| Syscall dispatcher | ⏳ Pendiente | Necesita implementar int 0x80 |
| sys_map/unmap/grant | ⏳ Pendiente | API de VMM disponible |
| sys_wait (eventos) | ⏳ Pendiente | Para IRQs y sincronización |
| libneo (userspace) | ❌ No iniciado | Wrappers y libc básica |
| VFS Server | ❌ No iniciado | Servidor de archivos |
| Process Server | ❌ No iniciado | Gestor de procesos |

---

## Comparación con Otros Sistemas

| Sistema | # Syscalls | Filosofía |
|---------|-----------|-----------|
| **NeoOS** | ~15 | Microkernel puro |
| seL4 | 10 | Microkernel verificado formalmente |
| L4 | 7 | Microkernel minimalista |
| Minix 3 | ~50 | Microkernel modular |
| Linux | ~400 | Kernel monolítico |

---

## Conclusión

El diseño minimalista de syscalls en NeoOS permite:
- **Kernel pequeño y verificable**: Menos código = menos bugs
- **Flexibilidad máxima**: Políticas en userspace
- **Mejor seguridad**: Aislamiento de componentes
- **Desarrollo ágil**: Servidores reemplazables sin recompilar kernel

Para detalles de implementación, consulte:
- [ARCHITECTURE.md](../ARCHITECTURE.md) - Arquitectura general del sistema
- [IPC.md](./IPC.md) - Detalles del sistema de mensajería
- [Scheduler.md](./Process%20Scheduler.md) - Planificador de procesos
- [Memory Manager.md](./Memory%20Manager.md) - Gestión de memoria