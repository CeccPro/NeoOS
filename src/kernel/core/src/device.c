/**
 * NeoOS - Device Manager Implementation
 * Gestiona la detección y registro de dispositivos
 */

#include "../include/device.h"
#include "../include/driver_manager.h"
#include "../include/error.h"
#include "../../drivers/include/early_vga.h"  // Para verbose
#include "../../lib/include/string.h"

// Lista de dispositivos registrados
static device_t devices[MAX_DEVICES];
static int device_count = 0;
static uint32_t next_device_id = 1;

/**
 * Inicializa el Device Manager
 */
int device_manager_init(bool verbose) {
    device_count = 0;
    next_device_id = 1;

    if (verbose) {
        vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
        vga_write("[DEVICE] Device Manager inicializado\n");
    }

    // Detección inicial de dispositivos (hardcodeada por ahora)
    // Registrar dispositivo VGA
    device_t vga_dev = {
        .id = next_device_id++,
        .type = DEVICE_TYPE_VIDEO,
        .resources = (void*)0xB8000,  // Dirección VGA
        .private_data = NULL,
        .active = false
    };
    device_register(&vga_dev);

    return E_OK;
}

/**
 * Registra un dispositivo en el sistema
 */
int device_register(device_t* dev) {
    if (device_count >= MAX_DEVICES) {
        return E_NOMEM;
    }

    devices[device_count] = *dev;
    device_count++;

    // Intentar asignar un driver
    int result = driver_bind(dev);
    if (result == E_OK) {
        dev->active = true;
    }

    return E_OK;
}

/**
 * Desregistra un dispositivo
 */
int device_unregister(uint32_t id) {
    for (int i = 0; i < device_count; i++) {
        if (devices[i].id == id) {
            driver_unbind(&devices[i]);
            // Mover el último al lugar del eliminado
            devices[i] = devices[--device_count];
            return E_OK;
        }
    }
    return E_NOENT;
}

/**
 * Obtiene un dispositivo por ID
 */
device_t* device_get(uint32_t id) {
    for (int i = 0; i < device_count; i++) {
        if (devices[i].id == id) {
            return &devices[i];
        }
    }
    return NULL;
}

/**
 * Lista todos los dispositivos registrados
 */
int device_list(char* buffer, size_t buffer_size) {
    // Simple implementation
    strcpy(buffer, "Device list not implemented\n");
    return device_count;
}