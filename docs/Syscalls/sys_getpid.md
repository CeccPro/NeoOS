# NeoOS - sys_getpid
La syscall `sys_getpid()` en NeoOS se utiliza para obtener el ID del proceso (PID) actual. Esta syscall es útil cuando un proceso necesita conocer su propio identificador, por ejemplo, para registros, depuración o comunicación entre procesos.

## Prototipo
```c
int sys_getpid(void);
```

## Parámetros
- Ninguno.

## Comportamiento
Cuando un proceso llama a `sys_getpid`, el sistema operativo simplemente devuelve el PID del proceso que realizó la llamada. Esta es una operación muy rápida que no requiere acceso a estructuras de datos complejas.

## Valor de Retorno
- Devuelve el PID del proceso actual. El valor siempre es un entero positivo mayor que 0.
- Esta syscall nunca falla, por lo que siempre devuelve un PID válido.

## Ejemplo de Uso
```c
#include <syscalls.h>
void main() {
    int my_pid = sys_getpid();
    
    // Usar el PID para registro o depuración
    char message[50];
    sprintf(message, "Mi PID es: %d\n", my_pid);
    sys_write(STDOUT, message, strlen(message));
}
```

## Notas
- El PID es único para cada proceso en el sistema durante su tiempo de vida.
- Los PIDs pueden ser reutilizados después de que un proceso termina y sus recursos son liberados completamente.
- Esta syscall es útil para identificar procesos en logs y mensajes de depuración.
- En sistemas donde se utiliza comunicación entre procesos (IPC), el PID es esencial para dirigir mensajes al proceso correcto.

## Véase también
- [sys_run](./sys_run.md)
- [sys_kill](./sys_kill.md)
- [sys_getpriority](./sys_getpriority.md)
- [sys_setpriority](./sys_setpriority.md)
