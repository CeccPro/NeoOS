/**
 * @file ipc.h
 * @brief Sistema de comunicación entre procesos (IPC) de NeoOS
 *
 * Copyright (C) 2025 CeccPro
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef IPC_H
#define IPC_H

#include <stdint.h>
#include <stddef.h>
#include "process.h"
#include "errors.h"

// ID de mensaje
typedef uint32_t msg_id_t;

// Tamaño máximo de un mensaje IPC
#define IPC_MAX_MSG_SIZE 4096

// Tipos de mensaje
typedef enum {
    IPC_MSG_NORMAL = 0,    // Mensaje normal
    IPC_MSG_SIGNAL = 1,    // Señal
    IPC_MSG_REQUEST = 2,   // Petición
    IPC_MSG_RESPONSE = 3   // Respuesta
} ipc_msg_type_t;

// Estructura de mensaje IPC
typedef struct {
    msg_id_t id;               // ID del mensaje
    pid_t sender;              // PID del emisor
    pid_t receiver;            // PID del receptor
    ipc_msg_type_t type;       // Tipo de mensaje
    size_t size;               // Tamaño de los datos
    uint8_t data[IPC_MAX_MSG_SIZE]; // Datos del mensaje
} ipc_message_t;

/**
 * @brief Inicializa el sistema IPC
 * @return E_OK si fue exitoso, código de error en caso contrario
 */
error_t ipc_init(void);

/**
 * @brief Envía un mensaje a otro proceso
 * @param receiver PID del proceso receptor
 * @param type Tipo de mensaje
 * @param data Puntero a los datos a enviar
 * @param size Tamaño de los datos
 * @param msg_id_out Puntero donde se almacenará el ID del mensaje (puede ser NULL)
 * @return E_OK si fue exitoso, código de error en caso contrario
 */
error_t ipc_send(pid_t receiver, ipc_msg_type_t type, 
                 const void* data, size_t size, msg_id_t* msg_id_out);

/**
 * @brief Recibe un mensaje
 * @param msg Puntero donde se almacenará el mensaje recibido
 * @param timeout_ms Tiempo de espera en milisegundos (0 = no bloqueante, -1 = infinito)
 * @return E_OK si fue exitoso, código de error en caso contrario
 */
error_t ipc_receive(ipc_message_t* msg, int timeout_ms);

/**
 * @brief Responde a un mensaje recibido
 * @param original_msg Mensaje original al que se responde
 * @param data Puntero a los datos de respuesta
 * @param size Tamaño de los datos
 * @return E_OK si fue exitoso, código de error en caso contrario
 */
error_t ipc_reply(const ipc_message_t* original_msg, 
                  const void* data, size_t size);

/**
 * @brief Libera los recursos asociados a un mensaje
 * @param msg_id ID del mensaje a liberar
 * @return E_OK si fue exitoso, código de error en caso contrario
 */
error_t ipc_free(msg_id_t msg_id);

/**
 * @brief Verifica si hay mensajes pendientes para el proceso actual
 * @return Número de mensajes pendientes
 */
size_t ipc_pending_messages(void);

#endif // IPC_H
