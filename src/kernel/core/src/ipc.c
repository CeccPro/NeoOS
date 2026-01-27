/**
 * NeoOS - IPC Implementation
 * Implementación del sistema de comunicación entre procesos
 */

#include "../../core/include/ipc.h"
#include "../../core/include/scheduler.h"
#include "../../core/include/error.h"
#include "../../memory/include/memory.h"
#include "../../lib/include/string.h"
#include "../../drivers/include/early_vga.h"

// Flag de inicialización
static bool ipc_initialized = false;

/**
 * Inicializa el sistema IPC
 */
int ipc_init(bool kverbose) {
    if (ipc_initialized) {
        return E_OK;
    }

    ipc_initialized = true;
    if (kverbose) {
        vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
        vga_write("[IPC] Sistema IPC inicializado\n");
    }
    return E_OK;
}

/**
 * Inicializa una cola de mensajes
 */
void ipc_queue_init(ipc_queue_t* queue) {
    if (queue == NULL) {
        return;
    }

    queue->head = NULL;
    queue->tail = NULL;
    queue->count = 0;
}

/**
 * Envía un mensaje a otro proceso
 */
int ipc_send(pid_t dest_pid, const void* msg, size_t size) {
    // Validar parámetros
    if (msg == NULL) {
        return E_INVAL;
    }

    if (size == 0 || size > IPC_MAX_MESSAGE_SIZE) {
        return E_INVAL;
    }

    // Obtener proceso actual y destino
    process_t* current = scheduler_get_current_process();
    if (current == NULL) {
        return E_PERM;  // No hay proceso actual
    }

    process_t* dest = scheduler_get_process(dest_pid);
    if (dest == NULL) {
        return E_NOENT;  // Proceso destino no existe
    }

    // Verificar que la cola del destino no esté llena
    if (dest->ipc_queue.count >= IPC_MAX_QUEUE_SIZE) {
        return E_BUSY;  // Cola llena
    }

    // Asignar memoria para el mensaje en cola
    ipc_queue_message_t* queue_msg = (ipc_queue_message_t*)kmalloc(sizeof(ipc_queue_message_t));
    if (queue_msg == NULL) {
        return E_NOMEM;
    }

    // Copiar datos del mensaje
    queue_msg->sender_pid = current->pid;
    queue_msg->size = size;
    memcpy(queue_msg->data, msg, size);
    queue_msg->next = NULL;

    // Agregar el mensaje al final de la cola (FIFO)
    if (dest->ipc_queue.tail == NULL) {
        // Cola vacía
        dest->ipc_queue.head = queue_msg;
        dest->ipc_queue.tail = queue_msg;
    } else {
        // Agregar al final
        dest->ipc_queue.tail->next = queue_msg;
        dest->ipc_queue.tail = queue_msg;
    }

    dest->ipc_queue.count++;

    // Si el proceso destino está bloqueado esperando mensajes, desbloquearlo
    if (dest->state == PROCESS_STATE_BLOCKED) {
        scheduler_unblock_process(dest->pid);
    }

    return E_OK;
}

/**
 * Recibe un mensaje de la cola del proceso actual
 */
int ipc_recv(ipc_message_t* msg, int flags) {
    // Validar parámetros
    if (msg == NULL) {
        return E_INVAL;
    }

    // Obtener proceso actual
    process_t* current = scheduler_get_current_process();
    if (current == NULL) {
        return E_PERM;  // No hay proceso actual
    }

    // Verificar si hay mensajes en la cola
    if (current->ipc_queue.head == NULL) {
        // No hay mensajes
        if (flags & IPC_NONBLOCKING) {
            // Modo no bloqueante: retornar inmediatamente
            return E_BUSY;
        } else {
            // Modo bloqueante: bloquear el proceso hasta que llegue un mensaje
            // Nota: el scheduler debe verificar si hay mensajes al desbloquear
            scheduler_block_process(current->pid);
            
            // Cuando el proceso se desbloquee, verificar nuevamente
            if (current->ipc_queue.head == NULL) {
                return E_BUSY;  // No debería pasar, pero por seguridad
            }
        }
    }

    // Extraer el primer mensaje de la cola (FIFO)
    ipc_queue_message_t* queue_msg = current->ipc_queue.head;
    current->ipc_queue.head = queue_msg->next;
    
    if (current->ipc_queue.head == NULL) {
        // Era el último mensaje
        current->ipc_queue.tail = NULL;
    }

    current->ipc_queue.count--;

    // Asignar memoria para el buffer del mensaje
    void* buffer = kmalloc(queue_msg->size);
    if (buffer == NULL) {
        // Error de memoria: devolver el mensaje a la cola
        queue_msg->next = current->ipc_queue.head;
        current->ipc_queue.head = queue_msg;
        if (current->ipc_queue.tail == NULL) {
            current->ipc_queue.tail = queue_msg;
        }
        current->ipc_queue.count++;
        return E_NOMEM;
    }

    // Copiar los datos al buffer
    memcpy(buffer, queue_msg->data, queue_msg->size);

    // Llenar la estructura del mensaje
    msg->sender_pid = queue_msg->sender_pid;
    msg->size = queue_msg->size;
    msg->buffer = buffer;

    // Liberar el mensaje de la cola
    kfree(queue_msg);

    return E_OK;
}

/**
 * Libera los recursos de un mensaje recibido
 */
int ipc_free(ipc_message_t* msg) {
    // Validar parámetros
    if (msg == NULL) {
        return E_INVAL;
    }

    if (msg->buffer == NULL) {
        return E_INVAL;  // Ya fue liberado o nunca asignado
    }

    // Liberar el buffer
    kfree(msg->buffer);

    // Invalidar el mensaje para prevenir doble liberación
    msg->buffer = NULL;
    msg->size = 0;
    msg->sender_pid = 0;

    return E_OK;
}

/**
 * Limpia la cola de mensajes de un proceso
 * Se llama cuando un proceso termina
 */
void ipc_cleanup_queue(ipc_queue_t* queue) {
    if (queue == NULL) {
        return;
    }

    ipc_queue_message_t* current = queue->head;
    while (current != NULL) {
        ipc_queue_message_t* next = current->next;
        kfree(current);
        current = next;
    }

    queue->head = NULL;
    queue->tail = NULL;
    queue->count = 0;
}
