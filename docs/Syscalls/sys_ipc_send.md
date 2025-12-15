# NeoOS - sys_ipc_send
La syscall `sys_ipc_send(int pid, const char *msg, int size)` en NeoOS se utiliza para enviar un mensaje a otro proceso utilizando el mecanismo de comunicación entre procesos (IPC - Inter-Process Communication). Esta syscall permite que los procesos intercambien datos de manera asíncrona.

## Prototipo
```c
int sys_ipc_send(int pid, const char *msg, int size);
```

## Parámetros
- `pid`: El ID del proceso destinatario al cual se enviará el mensaje.
- `msg`: Un puntero al buffer que contiene el mensaje a enviar. Debe ser una dirección de memoria válida accesible por el proceso.
- `size`: El tamaño del mensaje en bytes. Debe ser mayor que 0 y no exceder el tamaño máximo de mensaje permitido por el sistema.

## Comportamiento
Cuando un proceso llama a `sys_ipc_send`, el sistema operativo realiza las siguientes acciones:
1. Verifica que el PID del destinatario sea válido y corresponda a un proceso existente.
2. Verifica que el puntero del mensaje y el tamaño sean válidos.
3. Verifica que el proceso emisor tenga permisos para enviar mensajes al proceso destinatario.
4. Asigna memoria para almacenar una copia del mensaje.
5. Copia los datos del mensaje desde el espacio de direcciones del emisor.
6. Crea una estructura de mensaje IPC con metadatos (PID del emisor, tamaño, timestamp, etc.).
7. Coloca el mensaje en la cola IPC del proceso destinatario.
8. Si el proceso destinatario está bloqueado esperando mensajes, lo despierta.
9. Devuelve el control al proceso emisor.

## Valor de Retorno
- Devuelve `E_OK` (0) si la syscall se ejecuta correctamente y el mensaje fue enviado.
- Devuelve un código de error (negativo) si ocurre un problema, como:
  - `E_NOENT`: Si el PID del destinatario no corresponde a ningún proceso existente.
  - `E_INVAL`: Si el puntero del mensaje es NULL o el tamaño es inválido (0 o negativo).
  - `E_NOMEM`: Si no hay suficiente memoria para almacenar el mensaje.
  - `E_PERM`: Si el proceso no tiene permisos para enviar mensajes al destinatario.
  - `E_BUSY`: Si la cola IPC del destinatario está llena.
  - Otros códigos de error definidos en `error.h`.

## Ejemplo de Uso
```c
#include <error.h>
#include <syscalls.h>
void main() {
    const char *mensaje = "Hola desde el proceso emisor!";
    int pid_destinatario = 42;  // PID del proceso destinatario
    
    int result = sys_ipc_send(pid_destinatario, mensaje, strlen(mensaje) + 1);
    
    if (result == E_OK) {
        sys_write(STDOUT, "Mensaje enviado exitosamente\n", 30);
    } else if (result == E_NOENT) {
        sys_write(STDERR, "El proceso destinatario no existe\n", 35);
    } else if (result == E_BUSY) {
        sys_write(STDERR, "Cola IPC del destinatario llena\n", 33);
    } else {
        sys_write(STDERR, "Error al enviar mensaje\n", 24);
    }
}
```

## Ejemplo de Comunicación Bidireccional
```c
#include <syscalls.h>
void comunicacion_con_servidor(int pid_servidor) {
    // Enviar solicitud al servidor
    const char *solicitud = "GET_DATA";
    int result = sys_ipc_send(pid_servidor, solicitud, strlen(solicitud) + 1);
    
    if (result == E_OK) {
        sys_write(STDOUT, "Solicitud enviada, esperando respuesta...\n", 43);
        
        // Esperar respuesta del servidor
        ipc_message_t respuesta;
        if (sys_ipc_recv(&respuesta, 0) == E_OK) {
            sys_write(STDOUT, "Respuesta recibida: ", 20);
            sys_write(STDOUT, respuesta.data, respuesta.size);
            sys_write(STDOUT, "\n", 1);
            
            sys_ipc_free(&respuesta);
        }
    }
}
```

## Ejemplo de Envío de Estructura de Datos
```c
#include <syscalls.h>
typedef struct {
    int comando;
    int param1;
    int param2;
    char datos[64];
} comando_t;

void enviar_comando(int pid_destino, int cmd, int p1, int p2, const char *datos) {
    comando_t comando;
    comando.comando = cmd;
    comando.param1 = p1;
    comando.param2 = p2;
    strncpy(comando.datos, datos, sizeof(comando.datos) - 1);
    comando.datos[sizeof(comando.datos) - 1] = '\0';
    
    int result = sys_ipc_send(pid_destino, (const char *)&comando, sizeof(comando));
    
    if (result == E_OK) {
        sys_write(STDOUT, "Comando enviado\n", 16);
    } else {
        sys_write(STDERR, "Error al enviar comando\n", 24);
    }
}

void main() {
    int pid_worker = 100;
    enviar_comando(pid_worker, 1, 42, 99, "procesar datos");
}
```

## Ejemplo de Broadcast a Múltiples Procesos
```c
#include <syscalls.h>
void enviar_broadcast(int *pids, int num_procesos, const char *mensaje) {
    int enviados = 0;
    int fallidos = 0;
    
    for (int i = 0; i < num_procesos; i++) {
        int result = sys_ipc_send(pids[i], mensaje, strlen(mensaje) + 1);
        
        if (result == E_OK) {
            enviados++;
        } else {
            fallidos++;
        }
    }
    
    char msg[100];
    sprintf(msg, "Broadcast: %d enviados, %d fallidos\n", enviados, fallidos);
    sys_write(STDOUT, msg, strlen(msg));
}
```

## Notas
- `sys_ipc_send` es asíncrono: el emisor no espera a que el destinatario reciba el mensaje.
- El mensaje se copia, por lo que el emisor puede reutilizar o liberar el buffer después de la llamada.
- Cada proceso tiene una cola IPC limitada; si está llena, la syscall falla con `E_BUSY`.
- Los mensajes se entregan en orden FIFO (primero en entrar, primero en salir).
- Es responsabilidad de la aplicación establecer protocolos de comunicación (formato de mensajes, sincronización, etc.).
- Para comunicación confiable, considere implementar mecanismos de confirmación en el nivel de aplicación.

## Véase también
- [sys_ipc_recv](./sys_ipc_recv.md)
- [sys_ipc_free](./sys_ipc_free.md)
- [sys_getpid](./sys_getpid.md)
- [IPC](../IPC.md)
