# NeoOS - sys_ipc_recv
La syscall `sys_ipc_recv(ipc_message_t *msg, int flags)` en NeoOS se utiliza para recibir el siguiente mensaje disponible en la cola IPC del proceso. Esta syscall permite que los procesos lean mensajes enviados por otros procesos.

## Prototipo
```c
int sys_ipc_recv(ipc_message_t *msg, int flags);
```

## Parámetros
- `msg`: Un puntero a una estructura `ipc_message_t` donde se almacenará el mensaje recibido. La estructura típicamente contiene:
  - `sender_pid`: PID del proceso que envió el mensaje.
  - `size`: Tamaño del mensaje en bytes.
  - `data`: Puntero a los datos del mensaje.
  - `timestamp`: Momento en que se recibió el mensaje (opcional).
- `flags`: Banderas que controlan el comportamiento de la recepción:
  - `0` o `IPC_BLOCKING`: Bloquear el proceso hasta que llegue un mensaje.
  - `IPC_NONBLOCKING`: No bloquear; devolver inmediatamente si no hay mensajes.
  - Otras banderas específicas del sistema.

## Comportamiento
Cuando un proceso llama a `sys_ipc_recv`, el sistema operativo realiza las siguientes acciones:
1. Verifica que el puntero a la estructura de mensaje sea válido.
2. Revisa la cola IPC del proceso para ver si hay mensajes disponibles.
3. Si hay un mensaje disponible:
   - Extrae el primer mensaje de la cola (FIFO).
   - Llena la estructura `ipc_message_t` con los datos del mensaje.
   - Devuelve `E_OK` al proceso.
4. Si no hay mensajes disponibles:
   - Si `flags` indica operación bloqueante: bloquea el proceso hasta que llegue un mensaje.
   - Si `flags` indica operación no bloqueante: devuelve inmediatamente con código de error.

## Valor de Retorno
- Devuelve `E_OK` (0) si la syscall se ejecuta correctamente y un mensaje fue recibido.
- Devuelve un código de error (negativo) si ocurre un problema, como:
  - `E_INVAL`: Si el puntero del mensaje es NULL o las banderas son inválidas.
  - `E_BUSY`: Si no hay mensajes disponibles y se especificó operación no bloqueante.
  - Otros códigos de error definidos en `error.h`.

## Ejemplo de Uso
```c
#include <error.h>
#include <syscalls.h>
void main() {
    sys_write(STDOUT, "Esperando mensajes...\n", 22);
    
    while (1) {
        ipc_message_t mensaje;
        int result = sys_ipc_recv(&mensaje, 0);  // Bloqueante
        
        if (result == E_OK) {
            char info[100];
            sprintf(info, "Mensaje de PID %d (%d bytes): ", 
                    mensaje.sender_pid, mensaje.size);
            sys_write(STDOUT, info, strlen(info));
            sys_write(STDOUT, mensaje.data, mensaje.size);
            sys_write(STDOUT, "\n", 1);
            
            // Liberar recursos del mensaje
            sys_ipc_free(&mensaje);
        } else {
            sys_write(STDERR, "Error al recibir mensaje\n", 26);
            break;
        }
    }
}
```

## Ejemplo de Recepción No Bloqueante
```c
#include <syscalls.h>
void procesar_mensajes_pendientes() {
    ipc_message_t mensaje;
    int mensajes_procesados = 0;
    
    // Procesar todos los mensajes disponibles sin bloquear
    while (sys_ipc_recv(&mensaje, IPC_NONBLOCKING) == E_OK) {
        // Procesar mensaje
        procesar_mensaje(&mensaje);
        sys_ipc_free(&mensaje);
        mensajes_procesados++;
    }
    
    if (mensajes_procesados > 0) {
        char info[50];
        sprintf(info, "Procesados %d mensajes\n", mensajes_procesados);
        sys_write(STDOUT, info, strlen(info));
    }
}

void main() {
    while (1) {
        // Hacer trabajo normal
        realizar_trabajo();
        
        // Verificar si hay mensajes pendientes
        procesar_mensajes_pendientes();
        
        sys_yield();
    }
}
```

## Ejemplo de Servidor de Comandos
```c
#include <syscalls.h>
typedef struct {
    int comando;
    int param1;
    int param2;
} comando_t;

void servidor_comandos() {
    sys_write(STDOUT, "Servidor iniciado\n", 18);
    
    while (1) {
        ipc_message_t mensaje;
        
        if (sys_ipc_recv(&mensaje, 0) == E_OK) {
            if (mensaje.size >= sizeof(comando_t)) {
                comando_t *cmd = (comando_t *)mensaje.data;
                
                char info[100];
                sprintf(info, "Comando %d recibido de PID %d\n", 
                        cmd->comando, mensaje.sender_pid);
                sys_write(STDOUT, info, strlen(info));
                
                // Ejecutar comando
                ejecutar_comando(cmd);
                
                // Enviar respuesta al emisor
                const char *respuesta = "Comando ejecutado";
                sys_ipc_send(mensaje.sender_pid, respuesta, strlen(respuesta) + 1);
            }
            
            sys_ipc_free(&mensaje);
        }
    }
}
```

## Ejemplo con Timeout (simulado)
```c
#include <syscalls.h>
int recibir_con_timeout(ipc_message_t *mensaje, int timeout_ticks) {
    int tiempo_inicio = sys_uptime();
    
    while (1) {
        // Intentar recibir sin bloquear
        int result = sys_ipc_recv(mensaje, IPC_NONBLOCKING);
        
        if (result == E_OK) {
            return E_OK;  // Mensaje recibido
        }
        
        // Verificar timeout
        if (sys_uptime() - tiempo_inicio > timeout_ticks) {
            return E_TIMEOUT;  // Timeout alcanzado
        }
        
        // Pequeña espera antes de reintentar
        sys_sleep(10);
    }
}

void main() {
    ipc_message_t mensaje;
    int result = recibir_con_timeout(&mensaje, 500);  // 5 segundos
    
    if (result == E_OK) {
        sys_write(STDOUT, "Mensaje recibido\n", 17);
        sys_ipc_free(&mensaje);
    } else if (result == E_TIMEOUT) {
        sys_write(STDOUT, "Timeout: no se recibió mensaje\n", 32);
    }
}
```

## Notas
- Es fundamental llamar a `sys_ipc_free` después de procesar cada mensaje para liberar la memoria.
- La operación bloqueante suspende el proceso hasta que llegue un mensaje, liberando CPU para otros procesos.
- Los mensajes se reciben en orden FIFO (primero en entrar, primero en salir).
- El puntero `data` en la estructura del mensaje apunta a memoria del kernel; debe copiarse si se necesita retener después de `sys_ipc_free`.
- En modo no bloqueante, es común implementar un bucle de polling o combinar con `sys_yield`.
- El proceso debe manejar correctamente los mensajes de diferentes emisores si espera comunicación de múltiples fuentes.

## Véase también
- [sys_ipc_send](./sys_ipc_send.md)
- [sys_ipc_free](./sys_ipc_free.md)
- [sys_sleep](./sys_sleep.md)
- [IPC](../IPC.md)
