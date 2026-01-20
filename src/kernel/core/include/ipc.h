/**
 * NeoOS - IPC (Inter-Process Communication)
 * Sistema de mensajería entre procesos
 */

#ifndef _KERNEL_IPC_H
#define _KERNEL_IPC_H

#include "../../lib/include/types.h"

/**
 * Configuración del sistema IPC
 */
#define IPC_MAX_MESSAGE_SIZE  4096    // Tamaño máximo de un mensaje (4KB)
#define IPC_MAX_QUEUE_SIZE    32      // Máximo de mensajes en cola por proceso

/**
 * Flags para sys_ipc_recv
 */
#define IPC_BLOCK        0x00   // Modo bloqueante (espera hasta recibir mensaje)
#define IPC_NONBLOCKING  0x01   // Modo no bloqueante (retorna inmediatamente)

/**
 * Estructura de un mensaje IPC
 * Esta es la estructura que se pasa a las syscalls
 */
typedef struct {
    pid_t sender_pid;     // PID del proceso que envió el mensaje
    size_t size;          // Tamaño del mensaje en bytes
    void* buffer;         // Buffer con los datos del mensaje
} ipc_message_t;

/**
 * Estructura interna de un mensaje en cola
 * Usado internamente por el kernel para la cola de mensajes
 */
typedef struct ipc_queue_message {
    pid_t sender_pid;                     // PID del remitente
    size_t size;                          // Tamaño del mensaje
    char data[IPC_MAX_MESSAGE_SIZE];      // Buffer de datos
    struct ipc_queue_message* next;       // Siguiente mensaje en la cola
} ipc_queue_message_t;

/**
 * Cola de mensajes IPC de un proceso
 */
typedef struct {
    ipc_queue_message_t* head;    // Primer mensaje en la cola (FIFO)
    ipc_queue_message_t* tail;    // Último mensaje en la cola
    uint32_t count;               // Número de mensajes en cola
} ipc_queue_t;

/**
 * Inicializa el sistema IPC
 * @return E_OK en caso de éxito, código de error en caso contrario
 */
int ipc_init(void);

/**
 * Inicializa la cola de mensajes de un proceso
 * @param queue Cola a inicializar
 */
void ipc_queue_init(ipc_queue_t* queue);

/**
 * Envía un mensaje a otro proceso
 * @param dest_pid PID del proceso destinatario
 * @param msg Buffer con el mensaje a enviar
 * @param size Tamaño del mensaje en bytes
 * @return E_OK en caso de éxito, código de error en caso contrario
 */
int ipc_send(pid_t dest_pid, const void* msg, size_t size);

/**
 * Recibe un mensaje de la cola del proceso actual
 * @param msg Estructura donde se almacenará el mensaje recibido
 * @param flags Flags de comportamiento (IPC_BLOCK o IPC_NONBLOCKING)
 * @return E_OK en caso de éxito, código de error en caso contrario
 */
int ipc_recv(ipc_message_t* msg, int flags);

/**
 * Libera los recursos de un mensaje recibido
 * @param msg Mensaje a liberar
 * @return E_OK en caso de éxito, código de error en caso contrario
 */
int ipc_free(ipc_message_t* msg);

/**
 * Libera todos los mensajes de la cola de un proceso
 * Se llama cuando un proceso termina
 * @param queue Cola de mensajes a limpiar
 */
void ipc_cleanup_queue(ipc_queue_t* queue);

#endif /* _KERNEL_IPC_H */
