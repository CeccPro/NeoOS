# sys_send - Enviar Mensaje IPC

## Sinopsis
```c
int sys_send(pid_t dest_pid, const void *msg, size_t size, int flags);
```

## Descripción
Envía un mensaje a otro proceso mediante IPC (Inter-Process Communication). El mensaje se copia al buffer del proceso destino y se encola en su cola de mensajes.

## Parámetros
- **dest_pid**: PID del proceso destino
- **msg**: Puntero al buffer del mensaje a enviar
- **size**: Tamaño del mensaje en bytes (máximo 4096)
- **flags**: Flags de comportamiento (actualmente sin uso, pasar 0)

## Valor de Retorno
- **E_OK** (0): Mensaje enviado exitosamente
- **E_INVAL**: Parámetros inválidos (size > IPC_MAX_MESSAGE_SIZE, msg == NULL)
- **E_NOENT**: Proceso destino no existe
- **E_NOMEM**: Sin memoria para encolar el mensaje
- **E_BUSY**: Cola del destino llena (IPC_MAX_QUEUE_SIZE alcanzado)

## Ejemplo
```c
#include <neoos/ipc.h>

pid_t server_pid = 5;
const char *request = "GET /data";

int result = sys_send(server_pid, request, strlen(request) + 1, 0);
if (result != E_OK) {
    printf("Error al enviar mensaje: %s\n", error_to_string(result));
}
```

## Notas
- El mensaje se **copia** al espacio del kernel, por lo que el buffer puede reutilizarse inmediatamente
- Si la cola del destino está llena, el envío falla (no es bloqueante)
- Esta syscall es **asíncrona**: retorna inmediatamente sin esperar respuesta
- Para RPC síncrono, usar `sys_call` en su lugar

## Ver También
- [sys_recv](sys_recv.md) - Recibir mensajes
- [sys_call](sys_call.md) - RPC: send + recv atómico
- [IPC.md](../IPC.md) - Sistema de mensajería completo
