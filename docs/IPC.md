# NeoOS - IPC (Inter-Process Communication)
La Comunicación entre Procesos (IPC) en NeoOS es un conjunto de mecanismos que permiten a los procesos intercambiar datos y señales de manera eficiente y segura. Estos mecanismos son esenciales para la coordinación de tareas y la gestión de recursos en el sistema operativo.

## Mecanismo del IPC de NeoOS
NeoOS implementa un sistema de IPC basado en PID (Proces ID), MID (Module ID) y cola de mensajes. Cada proceso en NeoOS tiene un PID único, y los módulos del sistema tienen un MID que los identifica (Al ser un semi-microkernel, los módulos son procesos independientes, por lo que se deben de comunicar a través de IPC). Los procesos y módulos pueden enviar y recibir mensajes a través de colas de mensajes, que actúan como canales de comunicación entre ellos.

## Envío y Recepción de Mensajes
Los procesos pueden enviar mensajes a otros procesos o módulos utilizando sus PID o MID. Los mensajes se colocan en una cola de mensajes asociada al proceso o módulo receptor. El receptor puede entonces leer los mensajes de su cola en el orden en que fueron recibidos.

### Ejemplo de Envío de Mensajes
Para enviar un mensaje a un proceso con PID 1234, un proceso puede utilizar la siguiente sintaxis:
```c
#include <ipc.h>
int pid = 1234;
char message[] = "Hola, este es un mensaje.";
sys_ipc_send(pid, message, sizeof(message));
```

### Ejemplo de Recepción de Mensajes
Para recibir la cola mensajes, un proceso puede utilizar la siguiente sintaxis:
```c
#include <ipc.h>
ipc_message_t msg;
int ret = sys_ipc_recv(&msg, IPC_BLOCK);

if (ret == E_OK) {
    printf("Mensaje de %d: %s\n", msg.sender_pid, (char *)msg.buffer);
    sys_ipc_free(&msg);
}
```

Internamente cada mensaje es un struct con la siguiente forma:
```c
typedef struct {
    pid_t sender_pid;
    size_t size;
    void *buffer;
} ipc_message_t;
```

## Sincronización de Procesos
Además del intercambio de mensajes, NeoOS proporciona mecanismos de sincronización para coordinar la ejecución de procesos. Esto incluye semáforos y mutexes que permiten a los procesos gestionar el acceso a recursos compartidos de manera segura.
### Semáforos
Los semáforos en NeoOS permiten a los procesos señalizar eventos y coordinar acciones. Un proceso puede esperar a que un semáforo sea liberado antes de continuar su ejecución, asegurando que los recursos compartidos no sean accedidos simultáneamente por múltiples procesos.
### Mutexes
Los mutexes (mutual exclusions) son utilizados para proteger secciones críticas del código, garantizando que solo un proceso pueda ejecutar esa sección a la vez. Esto es crucial para evitar condiciones de carrera y asegurar la integridad de los datos.

## Conclusión
El sistema de Comunicación entre Procesos (IPC) de NeoOS es una parte fundamental del sistema operativo, proporcionando mecanismos robustos para el intercambio de datos y la sincronización entre procesos. Estos mecanismos aseguran que los procesos puedan trabajar juntos de manera eficiente y segura, facilitando la gestión de recursos y la coordinación de tareas en el sistema operativo.