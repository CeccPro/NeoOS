/**
 * NeoOS - Driver Interface
 * Interfaz común para drivers del kernel
 */

#ifndef _KERNEL_DRIVER_H
#define _KERNEL_DRIVER_H

#include "../../core/include/module.h"
#include "../../core/include/error.h"

// Tipos de dispositivos
typedef enum {
    DEVICE_TYPE_VIDEO = 1,
    DEVICE_TYPE_STORAGE,
    DEVICE_TYPE_NETWORK,
    DEVICE_TYPE_INPUT,
    // Agregar más según necesidad
} device_type_t;

// Estructura de dispositivo (usada por Device Manager)
typedef struct {
    uint32_t id;              // ID único del dispositivo
    device_type_t type;       // Tipo de dispositivo
    void* resources;          // Recursos asignados (e.g., puertos, memoria)
    void* private_data;       // Datos privados del driver
    bool active;              // Estado del dispositivo
} device_t;

// Interfaz extendida para drivers (hereda de module_entry_t)
typedef struct {
    module_entry_t base;      // Funciones base del módulo
    int (*probe)(device_t* dev);     // Detectar si el driver soporta el dispositivo
    int (*remove)(device_t* dev);    // Desconectar el dispositivo
    int (*suspend)(device_t* dev);   // Suspender el dispositivo
    int (*resume)(device_t* dev);    // Reanudar el dispositivo
} driver_entry_t;

// Función para registrar un driver en el Driver Manager
int driver_register(const char* name, driver_entry_t* entry, device_type_t supported_types[]);

#endif /* _KERNEL_DRIVER_H */