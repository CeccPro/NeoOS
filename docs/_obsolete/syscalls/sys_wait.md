# NeoOS - sys_wait
La syscall `sys_wait(int *status)` en NeoOS se utiliza para que un proceso padre espere a que uno de sus procesos hijos termine su ejecución. Esta syscall es fundamental para la sincronización entre procesos padre e hijo, permitiendo al padre obtener el estado de salida del hijo y evitar procesos zombies.

## Prototipo
```c
int sys_wait(int *status);
```

## Parámetros
- `status`: Un puntero a un entero donde se almacenará el estado de salida del proceso hijo que terminó. Si este parámetro es `NULL`, el estado de salida no se almacena pero el proceso padre igualmente espera a que un hijo termine.

## Comportamiento
Cuando un proceso padre llama a `sys_wait`, el sistema operativo realiza las siguientes acciones:
1. Verifica si el proceso tiene procesos hijos.
2. Si hay un proceso hijo que ya ha terminado, devuelve inmediatamente su PID y estado de salida.
3. Si no hay procesos hijos terminados pero sí procesos hijos en ejecución, el proceso padre se bloquea hasta que uno de ellos termine.
4. Limpia los recursos del proceso hijo terminado (lo elimina de la tabla de procesos como zombie).
5. Devuelve el PID del proceso hijo que terminó.

## Valor de Retorno
- Devuelve el PID del proceso hijo que terminó si la syscall se ejecuta correctamente.
- Devuelve un código de error si ocurre un problema, como:
  - `E_NOENT`: Si el proceso no tiene procesos hijos.
  - `E_INVAL`: Si el parámetro `status` apunta a una dirección de memoria inválida.
  - Otros códigos de error definidos en `error.h`.

## Ejemplo de Uso
```c
#include <error.h>
#include <syscalls.h>
void main() {
    char *args[] = {"mi_programa", NULL};
    int pid = sys_run("/bin/mi_programa", args);
    
    if (pid > 0) {
        int status;
        int child_pid = sys_wait(&status);
        
        if (child_pid > 0) {
            // El proceso hijo ha terminado
            if (status == 0) {
                // Terminó exitosamente
            } else {
                // Terminó con error
            }
        } else {
            // Manejar error
        }
    }
}
```

## Notas
- Es importante que los procesos padre llamen a `sys_wait` para evitar la acumulación de procesos zombies en el sistema.
- Si un proceso padre termina sin hacer `sys_wait` de sus hijos, estos pueden ser reasignados al proceso init o quedar como huérfanos.
- La syscall bloqueará al proceso padre hasta que un hijo termine, por lo que debe usarse con cuidado en aplicaciones que requieren respuesta inmediata.

## Véase también
- [sys_exit](./sys_exit.md)
- [sys_run](./sys_run.md)
- [sys_kill](./sys_kill.md)
- [sys_clone](./sys_clone.md)
