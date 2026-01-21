# NeoOS - Process Scheduler
El Process Scheduler de NeoOS es un componente fundamental del sistema operativo que se encarga de gestionar la ejecución de los procesos en el sistema. Implementa un algoritmo de planificación Round Robin con 5 niveles de prioridad, proporcionando multitarea preemptiva eficiente.

## Estado de Implementación: COMPLETAMENTE FUNCIONAL

**Ubicación**: `src/kernel/core/src/scheduler.c` y `src/kernel/core/include/scheduler.h`

## Características Principales

### 1. Algoritmo de Planificación
El scheduler utiliza **Round Robin con Prioridades**:
- Cada proceso recibe un quantum de tiempo de CPU
- Los procesos de mayor prioridad reciben quantums más largos
- Context switching automático mediante IRQ0 del timer (cada 10ms)
- Garantiza que todos los procesos reciben tiempo de CPU (no starvation)

### 2. Niveles de Prioridad
```c
typedef enum {
    PROCESS_PRIORITY_IDLE = 0,      // Proceso idle del sistema
    PROCESS_PRIORITY_LOW = 1,       // Baja prioridad
    PROCESS_PRIORITY_NORMAL = 2,    // Prioridad normal (por defecto)
    PROCESS_PRIORITY_HIGH = 3,      // Alta prioridad
    PROCESS_PRIORITY_REALTIME = 4   // Tiempo real
} process_priority_t;
```

**Quantums por Prioridad**:
- IDLE: 10ms (1 tick)
- LOW: 20ms (2 ticks)
- NORMAL: 50ms (5 ticks)
- HIGH: 80ms (8 ticks)
- REALTIME: 100ms (10 ticks)

### 3. Process Control Block (PCB)
Cada proceso tiene un PCB que almacena:
```c
typedef struct process {
    pid_t pid;                          // Process ID único
    char name[32];                      // Nombre del proceso
    process_state_t state;              // Estado actual
    process_priority_t priority;        // Prioridad
    uint32_t quantum;                   // Quantum restante en ticks
    cpu_context_t context;              // Contexto de CPU (registros)
    ipc_queue_t ipc_queue;             // Cola de mensajes IPC
    struct process* next;               // Siguiente en la lista
} process_t;
```

### 4. Estados de Proceso
```c
typedef enum {
    PROCESS_STATE_READY,     // Listo para ejecutar
    PROCESS_STATE_RUNNING,   // Ejecutándose actualmente
    PROCESS_STATE_BLOCKED,   // Bloqueado esperando evento
    PROCESS_STATE_TERMINATED // Terminado
} process_state_t;
```

## Funciones del API

### Inicialización
```c
void scheduler_init(bool verbose);
```
- Inicializa el scheduler
- Crea el proceso IDLE (PID 1) que ejecuta `hlt` en loop
- Configura las estructuras de datos necesarias

### Gestión de Procesos
```c
pid_t scheduler_create_process(const char* name, void (*entry)(), process_priority_t priority);
```
- Crea un nuevo proceso
- Asigna un PID único
- Configura el contexto inicial (stack, instruction pointer)
- Agrega el proceso a la cola de listos

```c
void scheduler_terminate_process(pid_t pid);
```
- Marca el proceso como TERMINATED
- Libera sus recursos
- Lo elimina de las colas de planificación

```c
void scheduler_yield(void);
```
- El proceso actual cede voluntariamente la CPU
- Útil para procesos cooperativos

### Gestión de Prioridades
```c
void scheduler_set_priority(pid_t pid, process_priority_t priority);
process_priority_t scheduler_get_priority(pid_t pid);
```

### Bloqueo/Desbloqueo
```c
void scheduler_block_process(pid_t pid);
void scheduler_unblock_process(pid_t pid);
```
- Bloquea un proceso (por ejemplo, esperando I/O o IPC)
- Desbloquea cuando el evento ocurre

### Context Switching
```c
void scheduler_switch(void);
```
- Nunca retorna (primera llamada)
- Inicia la multitarea
- Selecciona el siguiente proceso según prioridad y estado
- Realiza el cambio de contexto

```c
void scheduler_tick(void);
```
- Llamada desde IRQ0 (timer)
- Decrementa el quantum del proceso actual
- Si quantum = 0, llama a `scheduler_switch()`

## Proceso de Context Switching

1. **IRQ0 del timer** se dispara cada 10ms
2. **scheduler_tick()** decrementa el quantum del proceso actual
3. Si quantum llega a 0:
   - Guarda el contexto del proceso actual (registros en PCB)
   - Busca el siguiente proceso listo con mayor prioridad
   - Restaura su contexto desde su PCB
   - Salta a su instruction pointer
4. El proceso continúa ejecutándose donde se quedó

## Proceso IDLE
- **PID**: 1
- **Prioridad**: IDLE
- **Función**: Ejecuta `hlt` en loop infinito
- **Propósito**: Proceso fallback cuando no hay otros procesos listos

## Ejemplo de Uso

```c
// Función que ejecutará el proceso
void proceso_ejemplo(void) {
    while (1) {
        vga_write("Proceso ejecutándose\n");
        scheduler_yield();  // Ceder CPU voluntariamente
    }
}

// Crear el proceso
pid_t pid = scheduler_create_process("ejemplo", proceso_ejemplo, PROCESS_PRIORITY_NORMAL);

// Cambiar prioridad más adelante
scheduler_set_priority(pid, PROCESS_PRIORITY_HIGH);

// Terminar el proceso
scheduler_terminate_process(pid);
```

## Integración con Otros Subsistemas

### Con IPC
- Cada PCB contiene una cola IPC
- `ipc_recv()` bloquea el proceso si no hay mensajes
- `ipc_send()` desbloquea procesos esperando

### Con Timer
- El PIT genera IRQ0 a 100Hz
- Cada IRQ0 llama a `scheduler_tick()`
- Implementa la preemption

### Con Syscalls
- `sys_yield()` → `scheduler_yield()`
- `sys_thread_create()` → `scheduler_create_process()`
- `sys_setpriority()` → `scheduler_set_priority()`

## Limitaciones Actuales
- Todos los procesos corren en ring 0 (modo kernel)
- No hay separación de espacios de memoria por proceso
- No hay protección contra procesos maliciosos
- Quantum fijo por prioridad (no adaptativo)
- Funciona perfectamente para procesos cooperativos del kernel

## Próximas Mejoras
1. **Modo Usuario**: Migrar procesos a ring 3
2. **Espacios de Memoria**: Page directory por proceso
3. **Algoritmo Adaptativo**: Ajustar quantum según comportamiento
4. **CPU Affinity**: Soporte para múltiples núcleos (futuro)
5. **Grupos de Procesos**: Control groups para límites de recursos