# sys_recv - Recibir Mensaje IPC

## Sinopsis
```c
int sys_recv(pid_t *src_pid, void *buffer, size_t size, int flags);
```

## Descripción
Recibe el siguiente mensaje disponible en la cola IPC del proceso actual. Puede operar en modo bloqueante o no bloqueante.

## Parámetros
- **src_pid**: Puntero donde se almacenará el PID del remitente
- **buffer**: Buffer donde se copiará el mensaje recibido
- **size**: Tamaño máximo del buffer
- **flags**: Flags de comportamiento
  - `IPC_BLOCK` (0x00): Bloquea hasta recibir un mensaje
  - `IPC_NONBLOCKING` (0x01): Retorna inmediatamente si no hay mensajes

## Valor de Retorno
- **> 0**: Tamaño del mensaje recibido en bytes
- **E_OK** (0): Sin mensajes (solo en modo no bloqueante)
- **E_INVAL**: Parámetros inválidos
- **E_NOMEM**: Buffer demasiado pequeño

## Ejemplo

### Modo Bloqueante
```c
#include <neoos/ipc.h>

char buffer[1024];
pid_t sender;

int result = sys_recv(&sender, buffer, sizeof(buffer), IPC_BLOCK);
if (result > 0) {
    printf("Mensaje de PID %d: %s\n", sender, buffer);
}
```

### Modo No Bloqueante
```c
int result = sys_recv(&sender, buffer, sizeof(buffer), IPC_NONBLOCKING);
if (result > 0) {
    printf("Mensaje recibido\n");
} else if (result == E_OK) {
    printf("Sin mensajes disponibles\n");
}
```

## Notas
- En modo bloqueante, el proceso pasa a estado `PROCESS_STATE_BLOCKED` hasta recibir un mensaje
- Los mensajes se entregan en orden FIFO
- El mensaje se **copia** del kernel al buffer del proceso
- Si el buffer es demasiado pequeño, el mensaje se trunca (considerar E_NOMEM)

## Ver También
- [sys_send](sys_send.md) - Enviar mensajes
- [sys_call](sys_call.md) - RPC síncrono
- [IPC.md](../IPC.md) - Sistema de mensajería
