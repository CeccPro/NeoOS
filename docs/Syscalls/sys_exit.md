# NeoOS - sys_exit
La syscall `sys_exit(int status)` en NeoOS se utiliza para terminar el proceso actual y devolver un estado de salida al sistema operativo. Esta syscall es fundamental para la gestión de procesos, ya que permite a un proceso indicar que ha finalizado su ejecución y proporcionar información sobre su resultado.

## Prototipo
```c
void sys_exit(int status);
```

## Parámetros
- `status`: Un entero que representa el estado de salida del proceso. Este valor puede ser utilizado por otros procesos (como el proceso padre) para determinar cómo terminó el proceso. Por convención, un valor de `0` generalmente indica una terminación exitosa, mientras que cualquier valor distinto de cero puede indicar un error o una condición específica.

## Comportamiento
Cuando un proceso llama a `sys_exit`, el sistema operativo realiza las siguientes acciones:
1. Marca el proceso como terminado y libera los recursos asociados, como memoria y descriptores de archivos.
2. Actualiza la tabla de procesos para reflejar el estado de salida del proceso.
3. Si el proceso tiene un proceso padre, notifica al proceso padre sobre la terminación del proceso hijo.
4. Si el proceso es el último proceso en ejecución, el sistema operativo puede iniciar procedimientos de apagado o reinicio, según la configuración del sistema.
## Ejemplo de Uso
```c
#include <syscalls.h>
void main() {
    // Realizar algunas operaciones
    // ...

    // Terminar el proceso con un estado de salida 0 (éxito)
    sys_exit(0);
}
```

## Notas
- Es importante que los procesos llamen a `sys_exit` para liberar correctamente los recursos del sistema. No llamar a esta syscall puede resultar en fugas de memoria y otros problemas de gestión de recursos.
- Después de llamar a `sys_exit`, el proceso no debe intentar realizar ninguna otra operación, ya que su ejecución se considera finalizada.
- Los desarrolladores deben manejar adecuadamente el estado de salida para facilitar la depuración y el monitoreo del comportamiento de los procesos en el sistema.

## Véase también
- [sys_run](./sys_run.md)
- [sys_wait](./sys_wait.md)
- [sys_kill](./sys_kill.md)