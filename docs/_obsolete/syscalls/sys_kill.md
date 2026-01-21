# NeoOS - sys_kill
La syscall `sys_kill(int pid)` en NeoOS se utiliza para terminar forzosamente un proceso especificado por su PID. Esta syscall es importante para la gestión de procesos, permitiendo a un proceso (usualmente con permisos adecuados) finalizar la ejecución de otro proceso.

## Prototipo
```c
int sys_kill(int pid);
```

## Parámetros
- `pid`: El ID del proceso (PID) que se desea terminar. Debe ser un PID válido de un proceso existente en el sistema.

## Comportamiento
Cuando un proceso llama a `sys_kill`, el sistema operativo realiza las siguientes acciones:
1. Verifica que el PID especificado corresponda a un proceso válido en el sistema.
2. Verifica que el proceso que realiza la llamada tenga los permisos necesarios para terminar el proceso objetivo.
3. Marca el proceso objetivo para terminación.
4. Interrumpe cualquier operación bloqueante que el proceso objetivo pueda estar realizando.
5. Libera los recursos asociados al proceso objetivo (memoria, descriptores de archivos, etc.).
6. Actualiza la tabla de procesos para reflejar que el proceso ha sido terminado.
7. Si el proceso objetivo tiene un padre, notifica al padre sobre la terminación.

## Valor de Retorno
- Devuelve `E_OK` (0) si la syscall se ejecuta correctamente y el proceso fue terminado.
- Devuelve un código de error si ocurre un problema, como:
  - `E_NOENT`: Si el PID especificado no corresponde a ningún proceso existente.
  - `E_PERM`: Si el proceso que realiza la llamada no tiene permisos para terminar el proceso objetivo.
  - `E_INVAL`: Si el PID especificado es inválido (por ejemplo, negativo o 0).
  - Otros códigos de error definidos en `error.h`.

## Ejemplo de Uso
```c
#include <error.h>
#include <syscalls.h>
void main() {
    // Crear un proceso hijo
    char *args[] = {"mi_programa", NULL};
    int pid = sys_run("/bin/mi_programa", args);
    
    if (pid > 0) {
        // Esperar un poco
        sys_sleep(100);
        
        // Terminar el proceso hijo
        int result = sys_kill(pid);
        
        if (result == E_OK) {
            // El proceso fue terminado exitosamente
        } else {
            // Manejar error
        }
    }
}
```

## Notas
- La terminación forzosa de un proceso no le da oportunidad de limpiar recursos o guardar estado, por lo que debe usarse con precaución.
- En sistemas multiusuario, generalmente solo el propietario del proceso o el superusuario pueden terminar un proceso.
- Intentar terminar procesos del sistema críticos puede llevar a inestabilidad del sistema operativo.
- El proceso padre debe hacer `sys_wait` del proceso terminado para evitar que quede como zombie.

## Véase también
- [sys_exit](./sys_exit.md)
- [sys_run](./sys_run.md)
- [sys_wait](./sys_wait.md)
- [sys_clone](./sys_clone.md)
