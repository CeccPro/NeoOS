# NeoOS - sys_run
La syscall `sys_run(const char *path, char *const argv[])` en NeoOS se utiliza para crear y ejecutar un nuevo proceso. Esta syscall es esencial para la gestión de procesos, ya que permite a un proceso iniciar la ejecución de otro programa especificado por su ruta.

## Prototipo
```c
int sys_run(const char *path, char *const argv[]);
```

## Parámetros
- `path`: Una cadena de caracteres que representa la ruta al ejecutable del programa que se desea ejecutar. Esta ruta puede ser absoluta o relativa al directorio de trabajo actual del proceso.
- `argv`: Un arreglo de cadenas de caracteres que representa los argumentos que se pasarán al nuevo proceso. El primer elemento de este arreglo generalmente es el nombre del programa, seguido de los argumentos adicionales. El arreglo debe terminar con un puntero nulo (`NULL`).

## Comportamiento
Cuando un proceso llama a `sys_run`, el sistema operativo realiza las siguientes acciones:
1. Verifica que el archivo especificado por `path` exista y sea ejecutable.
2. Crea un nuevo proceso y asigna los recursos necesarios, como memoria y descriptores de archivos.
3. Carga el ejecutable en la memoria del nuevo proceso.
4. Inicializa el contexto de ejecución del nuevo proceso, incluyendo la pila y los registros.
5. Pasa los argumentos especificados en `argv` al nuevo proceso.
6. Inicia la ejecución del nuevo proceso y devuelve el ID del proceso (PID) al proceso padre.

## Valor de Retorno
- Devuelve el ID del proceso (PID) del nuevo proceso creado si la syscall se ejecuta correctamente.
- Devuelve un código de error si ocurre un problema, como:
  - `E_NOENT`: Si el archivo especificado no existe.
  - `E_PERM`: Si el archivo no tiene permisos de ejecución.
  - `E_NOMEM`: Si no hay suficiente memoria para crear el nuevo proceso.
  - Otros códigos de error definidos en `error.h`.

## Ejemplo de Uso
```c
#include <error.h>
#include <syscalls.h>
void main() {
    char *args[] = {"mi_programa", "arg1", "arg2", NULL};
    int pid = sys_run("/bin/mi_programa", args);
    if (pid < 0) {
        // Manejar error
    } else {
        // El proceso se ha creado exitosamente
    }
}
```

## Notas
- Es importante que el proceso padre maneje adecuadamente el valor de retorno de `sys_run` para detectar y gestionar errores.
- El nuevo proceso creado comenzará su ejecución de inmediato, y el proceso padre puede optar por esperar a que termine utilizando la syscall `sys_wait`.
- `sys_run` no reemplaza la imagen del proceso actual.
- Siempre crea un nuevo proceso independiente.

## Véase también
- [sys_exit](./sys_exit.md)
- [sys_wait](./sys_wait.md)
- [sys_clone](./sys_clone.md)