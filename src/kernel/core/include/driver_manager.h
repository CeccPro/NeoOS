/**
 * NeoOS - Driver Manager
 * Gestiona el registro y asignación de drivers
 */

#ifndef _KERNEL_DRIVER_MANAGER_H
#define _KERNEL_DRIVER_MANAGER_H

#include "../../drivers/include/driver.h"
#include "error.h"

// Número máximo de drivers registrados
#define MAX_DRIVERS 16

/**
 * Inicializa el Driver Manager
 * @param verbose: Modo verbose para debugging
 * @return E_OK si éxito, código de error en caso contrario
 */
int driver_manager_init(bool verbose);

/**
 * Registra un driver en el sistema
 * @param name: Nombre del driver
 * @param entry: Punto de entrada del driver
 * @param supported_types: Tipos de dispositivos soportados (terminado en 0)
 * @return E_OK si éxito, código de error en caso contrario
 */
int driver_register(const char* name, driver_entry_t* entry, device_type_t supported_types[]);

/**
 * Asigna un driver a un dispositivo
 * @param dev: Dispositivo a asignar
 * @return E_OK si éxito, código de error en caso contrario
 */
int driver_bind(device_t* dev);

/**
 * Desasigna un driver de un dispositivo
 * @param dev: Dispositivo a desasignar
 * @return E_OK si éxito, código de error en caso contrario
 */
int driver_unbind(device_t* dev);

#endif /* _KERNEL_DRIVER_MANAGER_H */