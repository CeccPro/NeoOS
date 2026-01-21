# NeoOS - IPC (Inter-Process Communication)
La Comunicación entre Procesos (IPC) en NeoOS es el mecanismo fundamental que permite a los procesos intercambiar datos y señales de manera eficiente y segura. Es especialmente crítico en la arquitectura de microkernel de NeoOS, donde los servicios del sistema se comunican mediante IPC.

## Estado de Implementación: COMPLETAMENTE FUNCIONAL

**Ubicación**: `src/kernel/core/src/ipc.c` y `src/kernel/core/include/ipc.h`

## Filosofía de Diseño
En un microkernel como NeoOS:
- **IPC es la piedra angular** del sistema
- Los servidores (VFS, Process Manager, drivers) se comunican mediante IPC
- Todo proceso puede enviar/recibir mensajes a otros procesos
- La comunicación es asíncrona con colas de mensajes

## Arquitectura del Sistema IPC

### 1. Estructura de Mensajes
```c
typedef struct {
    pid_t sender_pid;        // PID del proceso emisor
    size_t size;             // Tamaño del mensaje en bytes
    void* buffer;            // Puntero al contenido del mensaje
} ipc_message_t;
```

**Características**:
- Tamaño máximo por mensaje: **4KB** (4096 bytes)
- Cada proceso tiene su propia cola de mensajes
- Cola máxima: **32 mensajes** por proceso

### 2. Cola de Mensajes por Proceso
```c
typedef struct {
    ipc_message_t messages[IPC_MAX_QUEUE_SIZE];  // Array de mensajes (32)
    uint32_t count;                               // Número de mensajes en cola
    uint32_t read_index;                          // Índice de lectura
    uint32_t write_index;                         // Índice de escritura
} ipc_queue_t;
```

Cada PCB (Process Control Block) contiene una cola IPC integrada.

## API del Sistema IPC

### Inicialización
```c
int ipc_init(bool kverbose);
```
- Inicializa el sistema IPC
- Configura las estructuras necesarias
- Retorna `E_OK` si tiene éxito

### Envío de Mensajes
```c
int ipc_send(pid_t dest_pid, const void* msg, size_t size);
```

**Parámetros**:
- `dest_pid`: PID del proceso destino
- `msg`: Puntero al buffer con el mensaje
- `size`: Tamaño del mensaje en bytes

**Retorna**:
- `E_OK`: Mensaje enviado correctamente
- `E_INVAL`: PID inválido o mensaje NULL
- `E_NOMEM`: Cola del receptor llena (32 mensajes)
- `E_NOENT`: Proceso destino no existe

**Funcionamiento**:
1. Valida parámetros (dest_pid, msg no NULL, size <= 4KB)
2. Busca el PCB del proceso destino
3. Verifica que la cola no esté llena
4. Copia el mensaje a la cola del destino
5. Si el destino estaba bloqueado esperando, lo desbloquea
6. Retorna código de éxito/error

**Ejemplo**:
```c
const char* mensaje = "Hola desde el proceso A";
int resultado = ipc_send(target_pid, mensaje, strlen(mensaje) + 1);
if (resultado == E_OK) {
    vga_write("Mensaje enviado\n");
}
```

### Recepción de Mensajes
```c
int ipc_recv(ipc_message_t* msg, int flags);
```

**Parámetros**:
- `msg`: Puntero donde se almacenará el mensaje recibido
- `flags`: Modo de operación (`IPC_BLOCK` o `IPC_NONBLOCKING`)

**Flags**:
- `IPC_BLOCK (0x00)`: Bloquea el proceso hasta recibir un mensaje
- `IPC_NONBLOCKING (0x01)`: Retorna inmediatamente si no hay mensajes

**Retorna**:
- `E_OK`: Mensaje recibido correctamente
- `E_INVAL`: Puntero msg es NULL
- `E_NOMEM`: No hay mensajes (solo en modo no bloqueante)

**Funcionamiento**:
1. Obtiene el proceso actual del scheduler
2. Verifica si hay mensajes en su cola
3. Si no hay mensajes:
   - Modo bloqueante: bloquea el proceso hasta que llegue uno
   - Modo no bloqueante: retorna `E_NOMEM`
4. Si hay mensajes: copia el más antiguo al buffer
5. Actualiza índices de la cola (circular)
6. Retorna `E_OK`

**Ejemplo Bloqueante**:
```c
ipc_message_t msg;
int resultado = ipc_recv(&msg, IPC_BLOCK);
if (resultado == E_OK) {
    vga_write("Mensaje de PID ");
    vga_write_hex(msg.sender_pid);
    vga_write(": ");
    vga_write((const char*)msg.buffer);
    vga_write("\n");
    ipc_free(&msg);
}
```

**Ejemplo No Bloqueante**:
```c
ipc_message_t msg;
int resultado = ipc_recv(&msg, IPC_NONBLOCKING);
if (resultado == E_OK) {
    // Procesar mensaje
    ipc_free(&msg);
} else if (resultado == E_NOMEM) {
    // No hay mensajes, continuar con otras tareas
}
```

### Liberación de Mensajes
```c
void ipc_free(ipc_message_t* msg);
```
- Libera la memoria del buffer del mensaje
- Debe llamarse después de procesar cada mensaje recibido
- Usa `kfree()` internamente

## Demo Funcional: Marco-Polo

NeoOS incluye una demo completa de IPC implementada en `kernel_main()`:

```c
// Proceso Marco (PID 2)
void marco_process(void) {
    for (int i = 0; i < 5; i++) {
        const char* marco_msg = "Marco";
        ipc_send(3, marco_msg, strlen(marco_msg) + 1);  // Envía a Polo
        
        ipc_message_t response;
        ipc_recv(&response, IPC_BLOCK);  // Espera respuesta
        vga_write("[Marco] Recibi: ");
        vga_write((const char*)response.buffer);
        vga_write("\n");
        ipc_free(&response);
    }
}

// Proceso Polo (PID 3)
void polo_process(void) {
    for (int i = 0; i < 5; i++) {
        ipc_message_t msg;
        ipc_recv(&msg, IPC_BLOCK);  // Espera mensaje
        vga_write("[Polo] Recibi: ");
        vga_write((const char*)msg.buffer);
        vga_write("\n");
        ipc_free(&msg);
        
        const char* polo_msg = "Polo";
        ipc_send(2, polo_msg, strlen(polo_msg) + 1);  // Responde a Marco
    }
}
```

**Resultado**: 5 rondas de comunicación IPC entre dos procesos (funcional)

## Integración con el Scheduler

El IPC está íntimamente integrado con el scheduler:

1. **Bloqueo automático**: Si un proceso llama a `ipc_recv(IPC_BLOCK)` y no hay mensajes, el scheduler lo marca como `PROCESS_STATE_BLOCKED`
2. **Desbloqueo automático**: Cuando llega un mensaje, `ipc_send()` cambia el estado del receptor a `PROCESS_STATE_READY`
3. **Context switching**: El scheduler automáticamente cambia a otro proceso cuando uno se bloquea

## Syscalls IPC

Las syscalls del kernel exponen el IPC a modo usuario:

```c
// sys_send: Syscall 0
int sys_send(pid_t dest, void* msg, size_t len, int flags);

// sys_recv: Syscall 1  
int sys_recv(pid_t* src, void* buf, size_t len, int flags);

// sys_call: Syscall 2 (RPC: send + recv atómico)
int sys_call(pid_t dest, void* req, void* resp, size_t len);
```

Ver [Syscalls.md](Syscalls.md) para más detalles.

## Ventajas del Sistema IPC de NeoOS

- **Simplicidad**: API minimalista de 3 funciones
- **Asincronía**: No bloquea emisores, solo receptores
- **Integrado**: Cada proceso tiene su cola automáticamente
- **Eficiente**: Copia directa de memoria sin buffering innecesario
- **Flexible**: Modo bloqueante y no bloqueante  

## Limitaciones Actuales

- **Sin Prioridad**: Los mensajes se procesan FIFO (First In, First Out)
- **Colas Fijas**: Máximo 32 mensajes por proceso
- **Sin Filtrado**: No se puede recibir selectivamente por remitente
- **Sin Timeout**: El modo bloqueante espera indefinidamente
- **Sin Seguridad**: No hay validación de permisos (todos los procesos pueden comunicarse)  

## Futuras Mejoras

1. **Priorización de Mensajes**: Colas con múltiples prioridades
2. **Mensajes Urgentes**: Canal separado para señales críticas
3. **Recv Selectivo**: `ipc_recv_from(pid)` para filtrar por emisor
4. **Timeout**: `ipc_recv_timeout(timeout_ms)` con límite de espera
5. **Shared Memory**: Alternativa para transferencias grandes
6. **Permisos**: Control de acceso basado en capabilities
7. **RPC Framework**: Abstracciones de alto nivel para cliente-servidor

## Casos de Uso en NeoOS

### 1. Servidor VFS
```c
// El proceso hace una petición al VFS Server
file_request_t req = {.op = OP_OPEN, .path = "/etc/config"};
ipc_send(VFS_SERVER_PID, &req, sizeof(req));

ipc_message_t response;
ipc_recv(&response, IPC_BLOCK);
file_handle_t* handle = (file_handle_t*)response.buffer;
```

### 2. Driver en Userspace
```c
// El kernel envía notificación de hardware al driver
irq_event_t event = {.irq = 14, .data = 0x1F0};
ipc_send(DISK_DRIVER_PID, &event, sizeof(event));
```

### 3. Signals
```c
// Enviar señal SIGTERM a un proceso
signal_t sig = {.signal = SIGTERM};
ipc_send(target_pid, &sig, sizeof(sig));
```

## Mecanismos de Sincronización (Futuro)

Actualmente NeoOS no implementa semáforos o mutexes nativos. La sincronización se logra mediante:
- **Mensajes IPC**: Coordinación por intercambio de mensajes
- **Busy waiting**: Polling activo (ineficiente pero funcional)
- **Scheduler**: `scheduler_yield()` para cooperación

**Planeado para futuras versiones**:
- Semáforos binarios y contadores
- Mutexes con ownership
- Condition variables
- Read-write locks