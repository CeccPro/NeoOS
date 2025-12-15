# NeoOS - sys_clone
La syscall `sys_clone(char *const argv[], int flags, void *stack)` en NeoOS se utiliza para crear un nuevo proceso duplicando el proceso actual. A diferencia de `sys_run`, que ejecuta un nuevo programa, `sys_clone` crea una copia del proceso en ejecución, similar a fork en sistemas POSIX pero con mayor control sobre qué recursos se comparten.

## Prototipo
```c
int sys_clone(char *const argv[], int flags, void *stack);
```

## Parámetros
- `argv`: Un arreglo de cadenas de caracteres que representa los argumentos que se pasarán al nuevo proceso. Si es `NULL`, el proceso clonado heredará el contexto completo del proceso padre. El arreglo debe terminar con un puntero nulo (`NULL`).
- `flags`: Banderas que controlan el comportamiento de la clonación. Pueden especificar qué recursos compartir entre el proceso padre y el hijo (memoria, descriptores de archivos, etc.).
- `stack`: Un puntero a la región de memoria que se utilizará como pila para el nuevo proceso. Si es `NULL`, se asignará automáticamente una nueva pila.

## Comportamiento
Cuando un proceso llama a `sys_clone`, el sistema operativo realiza las siguientes acciones:
1. Crea una nueva entrada en la tabla de procesos para el proceso hijo.
2. Según las banderas especificadas, copia o comparte los recursos del proceso padre (espacio de direcciones, descriptores de archivos, manejadores de señales, etc.).
3. Si se proporciona un puntero de pila, configura el proceso hijo para usar esa pila; de lo contrario, asigna una nueva.
4. Copia el contexto de ejecución del proceso padre al hijo (registros, contador de programa, etc.).
5. Si se proporcionan argumentos en `argv`, inicializa el contexto del hijo con esos argumentos.
6. Devuelve el PID del hijo al proceso padre y 0 al proceso hijo.

## Valor de Retorno
- En el proceso padre: Devuelve el PID del proceso hijo creado si la syscall se ejecuta correctamente.
- En el proceso hijo: Devuelve 0.
- Devuelve un código de error (negativo) si ocurre un problema, como:
  - `E_NOMEM`: Si no hay suficiente memoria para crear el nuevo proceso.
  - `E_INVAL`: Si los parámetros proporcionados son inválidos.
  - Otros códigos de error definidos en `error.h`.

## Ejemplo de Uso
```c
#include <error.h>
#include <syscalls.h>
void main() {
    int pid = sys_clone(NULL, 0, NULL);
    
    if (pid < 0) {
        // Manejar error
    } else if (pid == 0) {
        // Código del proceso hijo
        // ...
        sys_exit(0);
    } else {
        // Código del proceso padre
        int status;
        sys_wait(&status);
    }
}
```

## Notas
- El uso de `sys_clone` permite mayor flexibilidad que `sys_run` cuando se necesita crear procesos que compartan recursos con el padre.
- Las banderas de clonación deben usarse con cuidado para evitar condiciones de carrera y problemas de sincronización.
- Es responsabilidad del programador asegurar que el proceso hijo termine correctamente llamando a `sys_exit`.

## Véase también
- [sys_exit](./sys_exit.md)
- [sys_run](./sys_run.md)
- [sys_wait](./sys_wait.md)
- [sys_kill](./sys_kill.md)
