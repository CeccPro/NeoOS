/**
 * NeoOS - VGA Driver Module
 * Driver VGA como módulo del kernel
 */

#include "../include/driver.h"
#include "../include/early_vga.h"
#include "../../core/include/error.h"
#include "../../lib/include/string.h"

// Mensajes PMIC para VGA
#define VGA_MSG_WRITE       1
#define VGA_MSG_SET_COLOR   2
#define VGA_MSG_CLEAR       3
#define VGA_MSG_WRITE_HEX   4
#define VGA_MSG_WRITE_DEC   5

// Estructura para mensaje de escritura
typedef struct {
    uint32_t type;
    char data[256];  // Para strings
} vga_message_t;

// Estructura para mensaje de color
typedef struct {
    uint32_t type;
    uint8_t fg;
    uint8_t bg;
} vga_color_message_t;

// Estructura para mensaje numérico
typedef struct {
    uint32_t type;
    uint32_t value;
} vga_number_message_t;

// Instancia del dispositivo VGA
static device_t* vga_device = NULL;

/**
 * Inicialización del módulo VGA
 */
static int vga_driver_init(void) {
    vga_init();
    return E_OK;
}

/**
 * Limpieza del módulo VGA
 */
static int vga_driver_cleanup(void) {
    // Nada especial por ahora
    return E_OK;
}

/**
 * Inicio del módulo VGA
 */
static int vga_driver_start(void) {
    return E_OK;
}

/**
 * Detención del módulo VGA
 */
static int vga_driver_stop(void) {
    return E_OK;
}

/**
 * Actualización del módulo VGA
 */
static int vga_driver_update(void) {
    // Nada por ahora
    return E_OK;
}

/**
 * Handler de mensajes PMIC para VGA
 */
static int vga_driver_handle_message(const void* msg, size_t size, void* response, size_t* response_size) {
    if (msg == NULL || size < sizeof(uint32_t)) {
        return E_INVAL;
    }

    uint32_t msg_type = *(uint32_t*)msg;

    switch (msg_type) {
        case VGA_MSG_WRITE: {
            vga_message_t* vmsg = (vga_message_t*)msg;
            if (size >= sizeof(vga_message_t)) {
                vga_write(vmsg->data);
            }
            break;
        }
        case VGA_MSG_SET_COLOR: {
            vga_color_message_t* vmsg = (vga_color_message_t*)msg;
            if (size >= sizeof(vga_color_message_t)) {
                vga_set_color(vmsg->fg, vmsg->bg);
            }
            break;
        }
        case VGA_MSG_CLEAR: {
            vga_clear();
            break;
        }
        case VGA_MSG_WRITE_HEX: {
            vga_number_message_t* vmsg = (vga_number_message_t*)msg;
            if (size >= sizeof(vga_number_message_t)) {
                vga_write_hex(vmsg->value);
            }
            break;
        }
        case VGA_MSG_WRITE_DEC: {
            vga_number_message_t* vmsg = (vga_number_message_t*)msg;
            if (size >= sizeof(vga_number_message_t)) {
                vga_write_dec(vmsg->value);
            }
            break;
        }
        default:
            return E_NOT_SUPPORTED;
    }

    return E_OK;
}

/**
 * Probe del driver VGA
 */
static int vga_driver_probe(device_t* dev) {
    if (dev->type == DEVICE_TYPE_VIDEO && dev->resources == (void*)0xB8000) {
        vga_device = dev;
        return E_OK;
    }
    return E_NOT_SUPPORTED;
}

/**
 * Remove del driver VGA
 */
static int vga_driver_remove(device_t* dev) {
    if (dev == vga_device) {
        vga_device = NULL;
    }
    return E_OK;
}

/**
 * Suspend del driver VGA
 */
static int vga_driver_suspend(device_t* dev) {
    // Implementar si es necesario
    return E_OK;
}

/**
 * Resume del driver VGA
 */
static int vga_driver_resume(device_t* dev) {
    // Implementar si es necesario
    return E_OK;
}

// Definición del driver entry
static driver_entry_t vga_driver_entry = {
    .base = {
        .init = vga_driver_init,
        .cleanup = vga_driver_cleanup,
        .start = vga_driver_start,
        .stop = vga_driver_stop,
        .update = vga_driver_update,
        .handle_message = vga_driver_handle_message
    },
    .probe = vga_driver_probe,
    .remove = vga_driver_remove,
    .suspend = vga_driver_suspend,
    .resume = vga_driver_resume
};

/**
 * Función para obtener el entry del driver VGA
 */
driver_entry_t* vga_driver_get_entry(void) {
    return &vga_driver_entry;
}