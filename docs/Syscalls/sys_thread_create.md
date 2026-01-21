# sys_thread_create - Crear Thread/Proceso

## Sinopsis
```c
pid_t sys_thread_create(void *entry, void *stack_top, int flags);
```

## Descripción
Crea un nuevo thread o proceso. El thread comienza ejecutando en la dirección `entry` con su propio stack.

## Parámetros
- **entry**: Puntero a la función de entrada del thread (`void (*func)(void)`)
- **stack_top**: Puntero al tope del stack del nuevo thread (debe estar alineado a 16 bytes)
- **flags**: Flags de creación
  - `0`: Thread normal (PROCESS_PRIORITY_NORMAL)
  - Otros flags pendientes de definir

## Valor de Retorno
- **> 0**: PID del thread creado
- **E_NOMEM**: Sin memoria para el PCB o recursos del thread
- **E_INVAL**: Parámetros inválidos

## Ejemplo

### Crear un Thread Simple
```c
#include <neoos/scheduler.h>

void mi_thread(void) {
    printf("Thread ejecutándose!\n");
    sys_thread_exit(0);
}

// Asignar stack de 4KB alineado
uint8_t stack[4096] __attribute__((aligned(16)));

pid_t thread_pid = sys_thread_create(mi_thread, &stack[4096], 0);
if (thread_pid > 0) {
    printf("Thread creado con PID: %d\n", thread_pid);
}
```

### Implementar fork() en libneo
```c
pid_t fork(void) {
    // Reservar stack para el hijo
    void *child_stack = malloc(STACK_SIZE);
    if (!child_stack) return E_NOMEM;
    
    // Copiar estado actual
    // ... (copiar memoria, file descriptors, etc.)
    
    // Crear thread hijo
    return sys_thread_create(child_entry, child_stack + STACK_SIZE, 0);
}
```

## Notas
- El thread se crea en estado `PROCESS_STATE_READY` y puede ejecutarse inmediatamente
- El stack debe ser **suficientemente grande** (mínimo 4KB recomendado)
- El stack debe estar **alineado a 16 bytes** (requisito x86)
- El thread hereda el directorio de páginas del padre (mismo espacio de direcciones)
- Para procesos con espacio de direcciones separado, usar `sys_map` después de crear el thread

## Ver También
- [sys_thread_exit](sys_thread_exit.md) - Terminar thread
- [sys_yield](sys_yield.md) - Ceder CPU
- [Process Scheduler.md](../Process%20Scheduler.md) - Planificador
