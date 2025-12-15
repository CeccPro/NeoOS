# NeoOS - Syscalls
El sistema de llamadas al sistema (syscalls) en NeoOS es un componente fundamental que permite a los procesos en modo usuario interactuar con el núcleo del sistema operativo. A través de las syscalls, los procesos pueden solicitar servicios del kernel, como la gestión de memoria, la comunicación entre procesos, la gestión de archivos y dispositivos, entre otros. Las syscalls de NeoOS no son syscalls standard POSIX, pero ofrecen funcionalidades similares adaptadas a las necesidades específicas del sistema operativo.

## Mecanismo de Syscalls
Cuando un proceso en modo usuario necesita realizar una operación que requiere privilegios del kernel, realiza una syscall. Este proceso implica los siguientes pasos:
1. El proceso coloca los parámetros necesarios para la syscall en registros específicos.
2. El proceso ejecuta una instrucción especial que provoca una interrupción o excepción, transfiriendo el control al kernel.
3. El kernel identifica la syscall solicitada y ejecuta la función correspondiente.
4. Una vez completada la operación, el kernel devuelve el control al proceso en modo usuario, junto con cualquier valor de retorno necesario.

## Lista de Syscalls Disponibles
A continuación se presenta una lista de las syscalls más comunes disponibles en NeoOS:
01. `sys_exit(int status)`: Termina el proceso actual con el estado especificado.
02. `sys_run(const char *path, char *const argv[])`: Crea y ejecuta un nuevo proceso.
03. `sys_wait(int *status)`: Espera a que un proceso hijo termine y obtiene su estado de salida.
04. `sys_clone(char *const argv[], int flags, void *stack)`: Crea un nuevo proceso duplicando el proceso actual.
05. `sys_kill(int pid)`: Envía una señal para terminar el proceso con el ID especificado.
06. `sys_getpid()`: Devuelve el ID del proceso actual.
07. `sys_yield()`: Cede el control del CPU al scheduler para permitir que otros procesos se ejecuten.
08. `sys_sleep(int ticks)`: Suspende la ejecución del proceso actual durante un número especificado de ticks del reloj.
09. `sys_uptime()`: Devuelve el tiempo total que el sistema ha estado en funcionamiento desde el último arranque.
10. `sys_open(const char *filename, int mode)`: Abre un archivo con el nombre y modo especificados.
11. `sys_read(int fd, void *buf, int count)`: Lee datos desde un archivo abierto en el descriptor de archivo especificado.
12. `sys_write(int fd, const void *buf, int count)`: Escribe datos en un archivo abierto en el descriptor de archivo especificado.
13. `sys_close(int fd)`: Cierra un archivo abierto en el descriptor de archivo especificado.
14. `sys_mmap(int fd, int offset, int size)`: Mapea un archivo en memoria virtual del proceso.
15. `sys_munmap(void *addr, int size)`: Desmapea una región de memoria previamente mapeada.
16. `sys_sbrk(int increment)`: Extiende o contrae el heap del proceso.
17. `sys_chdir(const char *path)`: Cambia el directorio de trabajo actual del proceso.
18. `sys_mkdir(const char *path)`: Crea un nuevo directorio.
19. `sys_rmdir(const char *path)`: Elimina un directorio existente.
20. `sys_unlink(const char *path)`: Elimina un archivo.
21. `sys_stat(const char *path, struct stat *buf)`: Obtiene información sobre un archivo o directorio.
22. `sys_signal(int sig, void (*handler)(int))`: Registra un manejador para una señal específica.
23. `sys_gettime(struct timespec *ts)`: Obtiene la hora actual del sistema con precisión.
24. `sys_setpriority(int pid, int priority)`: Establece la prioridad de un proceso.
25. `sys_getpriority(int pid)`: Obtiene la prioridad de un proceso.
26. `sys_ipc_send(int pid, const char *msg, int size)`: Envía un mensaje a otro proceso.
27. `sys_ipc_recv(ipc_message_t *msg, int flags)`: Recibe el siguiente mensaje disponible en la cola IPC del proceso.
28. `sys_ipc_free(ipc_message_t *msg)`: Libera los recursos asociados a un mensaje IPC.

Para más detalles sobre cada syscall, incluyendo sus parámetros y valores de retorno, consulte la documentación específica de cada syscall en /docs/Syscalls/[Syscall_Name].md.

## Cómo Usar las Syscalls
Para utilizar las syscalls en NeoOS, los desarrolladores deben incluir el archivo de encabezado correspondiente y llamar a las funciones de syscall con los parámetros adecuados. Es importante manejar correctamente los valores de retorno para detectar y gestionar errores. Aquí hay un ejemplo básico de cómo utilizar una syscall para abrir un archivo:
```c
#include <syscalls.h>
int fd = sys_open("mi_archivo.txt", O_RDONLY);
if (fd < 0) {
    // Manejar error
} else {
    // Leer o escribir en el archivo
    sys_close(fd);
}
```

## Conclusión
Las syscalls de NeoOS proporcionan una interfaz esencial para que los procesos en modo usuario interactúen con el kernel del sistema operativo. Al comprender y utilizar estas syscalls de manera efectiva, los desarrolladores pueden crear aplicaciones robustas y eficientes que aprovechen al máximo las capacidades del sistema operativo NeoOS.