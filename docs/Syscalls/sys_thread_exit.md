# sys_thread_exit - Terminar Thread

## Sinopsis
```c
void sys_thread_exit(int status);
```

## Descripción
Termina el thread actual. El thread pasa a estado `PROCESS_STATE_TERMINATED` y sus recursos son liberados. Esta syscall **nunca retorna**.

## Parámetros
- **status**: Código de salida del thread (0 = éxito, != 0 = error)

## Valor de Retorno
Esta syscall **no retorna**. El thread es removido del scheduler.

## Ejemplo
```c
#include <neoos/scheduler.h>

void worker_thread(void) {
    // Hacer trabajo...
    printf("Trabajo completado\n");
    
    // Terminar thread exitosamente
    sys_thread_exit(0);
    
    // Esta línea nunca se ejecuta
    printf("Nunca impreso\n");
}
```

## Notas
- El PID del thread queda disponible para reutilización
- La memoria del stack **no se libera automáticamente** (debe ser liberada por el padre o el gestor de procesos)
- Si hay threads bloqueados esperando este thread (vía `sys_wait`), serán desbloqueados
- El proceso init (PID 1) **nunca debe llamar** esta syscall

## Ver También
- [sys_thread_create](sys_thread_create.md) - Crear thread
- [sys_wait](sys_wait.md) - Esperar eventos
