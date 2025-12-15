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

### Gestión de Procesos
01. [`sys_exit(int status)`](./Syscalls/sys_exit.md): Termina el proceso actual con el estado especificado.
02. [`sys_run(const char *path, char *const argv[])`](./Syscalls/sys_run.md): Crea y ejecuta un nuevo proceso.
03. [`sys_wait(int *status)`](./Syscalls/sys_wait.md): Espera a que un proceso hijo termine y obtiene su estado de salida.
04. [`sys_clone(char *const argv[], int flags, void *stack)`](./Syscalls/sys_clone.md): Crea un nuevo proceso duplicando el proceso actual.
05. [`sys_kill(int pid)`](./Syscalls/sys_kill.md): Envía una señal para terminar el proceso con el ID especificado.
06. [`sys_getpid()`](./Syscalls/sys_getpid.md): Devuelve el ID del proceso actual.
07. [`sys_yield()`](./Syscalls/sys_yield.md): Cede el control del CPU al scheduler para permitir que otros procesos se ejecuten.
08. [`sys_sleep(int ticks)`](./Syscalls/sys_sleep.md): Suspende la ejecución del proceso actual durante un número especificado de ticks del reloj.
09. [`sys_uptime()`](./Syscalls/sys_uptime.md): Devuelve el tiempo total que el sistema ha estado en funcionamiento desde el último arranque.

### Gestión de Archivos
10. [`sys_open(const char *filename, int mode)`](./Syscalls/sys_open.md): Abre un archivo con el nombre y modo especificados.
11. [`sys_read(int fd, void *buf, int count)`](./Syscalls/sys_read.md): Lee datos desde un archivo abierto en el descriptor de archivo especificado.
12. [`sys_write(int fd, const void *buf, int count)`](./Syscalls/sys_write.md): Escribe datos en un archivo abierto en el descriptor de archivo especificado.
13. [`sys_close(int fd)`](./Syscalls/sys_close.md): Cierra un archivo abierto en el descriptor de archivo especificado.

### Gestión de Memoria
14. [`sys_mmap(int fd, int offset, int size)`](./Syscalls/sys_mmap.md): Mapea un archivo en memoria virtual del proceso.
15. [`sys_munmap(void *addr, int size)`](./Syscalls/sys_munmap.md): Desmapea una región de memoria previamente mapeada.
16. [`sys_sbrk(int increment)`](./Syscalls/sys_sbrk.md): Extiende o contrae el heap del proceso.

### Sistema de Archivos
17. [`sys_chdir(const char *path)`](./Syscalls/sys_chdir.md): Cambia el directorio de trabajo actual del proceso.
18. [`sys_mkdir(const char *path)`](./Syscalls/sys_mkdir.md): Crea un nuevo directorio.
19. [`sys_rmdir(const char *path)`](./Syscalls/sys_rmdir.md): Elimina un directorio existente.
20. [`sys_unlink(const char *path)`](./Syscalls/sys_unlink.md): Elimina un archivo.
21. [`sys_stat(const char *path, struct stat *buf)`](./Syscalls/sys_stat.md): Obtiene información sobre un archivo o directorio.

### Señales y Tiempo
22. [`sys_signal(int sig, void (*handler)(int))`](./Syscalls/sys_signal.md): Registra un manejador para una señal específica.
23. [`sys_gettime(struct timespec *ts)`](./Syscalls/sys_gettime.md): Obtiene la hora actual del sistema con precisión.

### Prioridad de Procesos
24. [`sys_setpriority(int pid, int priority)`](./Syscalls/sys_setpriority.md): Establece la prioridad de un proceso.
25. [`sys_getpriority(int pid)`](./Syscalls/sys_getpriority.md): Obtiene la prioridad de un proceso.

### Comunicación Entre Procesos (IPC)
26. [`sys_ipc_send(int pid, const char *msg, int size)`](./Syscalls/sys_ipc_send.md): Envía un mensaje a otro proceso.
27. [`sys_ipc_recv(ipc_message_t *msg, int flags)`](./Syscalls/sys_ipc_recv.md): Recibe el siguiente mensaje disponible en la cola IPC del proceso.
28. [`sys_ipc_free(ipc_message_t *msg)`](./Syscalls/sys_ipc_free.md): Libera los recursos asociados a un mensaje IPC.

Para más detalles sobre cada syscall, incluyendo sus parámetros y valores de retorno, consulte la documentación específica haciendo clic en los enlaces anteriores.

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