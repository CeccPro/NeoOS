/**
 * NeoOS - Device Manager
 * Gestiona la detección y registro de dispositivos
 */

#ifndef _KERNEL_DEVICE_H
#define _KERNEL_DEVICE_H

#include "../../drivers/include/driver.h"
#include "error.h"

// Número máximo de dispositivos
#define MAX_DEVICES 32

/**
 * Inicializa el Device Manager
 * @param verbose: Modo verbose para debugging
 * @return E_OK si éxito, código de error en caso contrario
 */
int device_manager_init(bool verbose);

/**
 * Registra un dispositivo en el sistema
 * @param dev: Dispositivo a registrar
 * @return E_OK si éxito, código de error en caso contrario
 */
int device_register(device_t* dev);

/**
 * Desregistra un dispositivo
 * @param id: ID del dispositivo
 * @return E_OK si éxito, código de error en caso contrario
 */
int device_unregister(uint32_t id);

/**
 * Obtiene un dispositivo por ID
 * @param id: ID del dispositivo
 * @return Puntero al dispositivo, o NULL si no existe
 */
device_t* device_get(uint32_t id);

/**
 * Lista todos los dispositivos registrados
 * @param buffer: Buffer para la lista
 * @param buffer_size: Tamaño del buffer
 * @return Número de dispositivos listados
 */
int device_list(char* buffer, size_t buffer_size);

#endif /* _KERNEL_DEVICE_H */