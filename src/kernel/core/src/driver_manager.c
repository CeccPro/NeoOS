/**
 * NeoOS - Driver Manager Implementation
 * Gestiona el registro y asignación de drivers
 */

#include "../include/driver_manager.h"
#include "../include/module.h"
#include "../include/error.h"
#include "../../lib/include/string.h"
#include "../../drivers/include/early_vga.h"  // Para verbose

// Lista de drivers registrados
static struct {
    const char* name;
    driver_entry_t* entry;
    device_type_t supported_types[MAX_DRIVERS];  // Simplificado, asumir pocos tipos por driver
} registered_drivers[MAX_DRIVERS];
static int driver_count = 0;

/**
 * Inicializa el Driver Manager
 */
int driver_manager_init(bool verbose) {
    driver_count = 0;
    if (verbose) {
        vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
        vga_write("[DRIVER] Driver Manager inicializado\n");
    }
    return E_OK;
}

/**
 * Registra un driver en el sistema
 */
int driver_register(const char* name, driver_entry_t* entry, device_type_t supported_types[]) {
    if (driver_count >= MAX_DRIVERS) {
        return E_NOMEM;
    }

    // Copiar tipos soportados
    int i = 0;
    while (supported_types[i] != 0 && i < MAX_DRIVERS) {
        registered_drivers[driver_count].supported_types[i] = supported_types[i];
        i++;
    }
    registered_drivers[driver_count].supported_types[i] = 0;  // Terminar

    registered_drivers[driver_count].name = name;
    registered_drivers[driver_count].entry = entry;
    driver_count++;

    // Registrar como módulo estático
    mid_t mid = module_register_static(name, &entry->base);
    if (mid <= 0) {
        return E_MODULE_ERR;
    }

    return E_OK;
}

/**
 * Verifica si un driver soporta un tipo de dispositivo
 */
static bool driver_supports_type(driver_entry_t* entry, device_type_t type) {
    for (int i = 0; i < driver_count; i++) {
        if (registered_drivers[i].entry == entry) {
            int j = 0;
            while (registered_drivers[i].supported_types[j] != 0) {
                if (registered_drivers[i].supported_types[j] == type) {
                    return true;
                }
                j++;
            }
            break;
        }
    }
    return false;
}

/**
 * Asigna un driver a un dispositivo
 */
int driver_bind(device_t* dev) {
    for (int i = 0; i < driver_count; i++) {
        driver_entry_t* entry = registered_drivers[i].entry;

        if (driver_supports_type(entry, dev->type) && entry->probe(dev) == E_OK) {
            dev->private_data = entry;
            // Iniciar el módulo si no está iniciado
            mid_t mid = module_get_id(registered_drivers[i].name);
            if (mid > 0 && module_get_state(mid) != MODULE_STATE_RUNNING) {
                module_start(mid);
            }
            return E_OK;
        }
    }
    return E_NOT_SUPPORTED;
}

/**
 * Desasigna un driver de un dispositivo
 */
int driver_unbind(device_t* dev) {
    if (dev->private_data) {
        driver_entry_t* entry = (driver_entry_t*)dev->private_data;
        entry->remove(dev);
        dev->private_data = NULL;
    }
    return E_OK;
}