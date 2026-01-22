/**
 * NeoOS - RAM Disk Module Implementation
 * Módulo de disco en memoria RAM
 */

#include "../include/ramdisk.h"
#include "../../core/include/module.h"
#include "../../core/include/error.h"
#include "../../memory/include/memory.h"
#include "../../lib/include/string.h"
#include "../../drivers/include/vga.h"

// Buffer del ramdisk
static uint8_t* ramdisk_buffer = NULL;
static bool ramdisk_initialized = false;

/**
 * Inicializa el ramdisk
 */
int ramdisk_init(void) {
    if (ramdisk_initialized) {
        return E_EXISTS;
    }
    
    // Asignar memoria para el ramdisk
    ramdisk_buffer = (uint8_t*)kmalloc(RAMDISK_SIZE);
    if (ramdisk_buffer == NULL) {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_write("[RAMDISK] Error: No hay memoria suficiente\n");
        return E_NOMEM;
    }
    
    // Limpiar el ramdisk
    memset(ramdisk_buffer, 0, RAMDISK_SIZE);
    
    ramdisk_initialized = true;
    
    return E_OK;
}

/**
 * Limpia el ramdisk
 */
int ramdisk_cleanup(void) {
    if (!ramdisk_initialized) {
        return E_INVAL;
    }
    
    if (ramdisk_buffer != NULL) {
        kfree(ramdisk_buffer);
        ramdisk_buffer = NULL;
    }
    
    ramdisk_initialized = false;
    
    return E_OK;
}

/**
 * Lee datos del ramdisk
 */
int ramdisk_read(uint32_t sector, uint32_t count, void* buffer) {
    if (!ramdisk_initialized || ramdisk_buffer == NULL) {
        return E_INVAL;
    }
    
    if (buffer == NULL) {
        return E_INVAL;
    }
    
    // Verificar límites
    if (sector >= RAMDISK_SECTORS) {
        return E_INVAL;
    }
    
    // Ajustar count si excede el límite
    if (sector + count > RAMDISK_SECTORS) {
        count = RAMDISK_SECTORS - sector;
    }
    
    // Copiar datos
    uint32_t offset = sector * RAMDISK_SECTOR_SIZE;
    uint32_t size = count * RAMDISK_SECTOR_SIZE;
    memcpy(buffer, ramdisk_buffer + offset, size);
    
    return count;
}

/**
 * Escribe datos en el ramdisk
 */
int ramdisk_write(uint32_t sector, uint32_t count, const void* buffer) {
    if (!ramdisk_initialized || ramdisk_buffer == NULL) {
        return E_INVAL;
    }
    
    if (buffer == NULL) {
        return E_INVAL;
    }
    
    // Verificar límites
    if (sector >= RAMDISK_SECTORS) {
        return E_INVAL;
    }
    
    // Ajustar count si excede el límite
    if (sector + count > RAMDISK_SECTORS) {
        count = RAMDISK_SECTORS - sector;
    }
    
    // Copiar datos
    uint32_t offset = sector * RAMDISK_SECTOR_SIZE;
    uint32_t size = count * RAMDISK_SECTOR_SIZE;
    memcpy(ramdisk_buffer + offset, buffer, size);
    
    return count;
}

/**
 * Obtiene el tamaño del ramdisk en bytes
 */
uint32_t ramdisk_get_size(void) {
    return RAMDISK_SIZE;
}

/**
 * Obtiene el número de sectores del ramdisk
 */
uint32_t ramdisk_get_sectors(void) {
    return RAMDISK_SECTORS;
}

/**
 * Formatea el ramdisk
 */
int ramdisk_format(void) {
    if (!ramdisk_initialized || ramdisk_buffer == NULL) {
        return E_INVAL;
    }
    
    memset(ramdisk_buffer, 0, RAMDISK_SIZE);
    
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    
    return E_OK;
}

// ========== Implementación del Module Entry ==========

/**
 * Función de inicialización del módulo
 */
int module_ramdisk_init(void) {
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    return ramdisk_init();
}

/**
 * Función de limpieza del módulo
 */
int module_ramdisk_cleanup(void) {
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    return ramdisk_cleanup();
}

/**
 * Función de inicio del módulo
 */
int module_ramdisk_start(void) {
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    return E_OK;
}

/**
 * Función de detención del módulo
 */
int module_ramdisk_stop(void) {
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    return E_OK;
}

/**
 * Función de actualización del módulo
 * No hace nada en el caso del ramdisk, ya que es pasivo
 */
int module_ramdisk_update(void) {
    // El ramdisk no necesita actualizaciones periódicas
    return E_OK;
}

/**
 * Handler de mensajes IPC del ramdisk
 */
int module_ramdisk_handle_message(const void* msg, size_t size, void* response, size_t* response_size) {
    if (msg == NULL || size < sizeof(ramdisk_request_t)) {
        return E_INVAL;
    }
    
    const ramdisk_request_t* req = (const ramdisk_request_t*)msg;
    ramdisk_response_t* resp = (ramdisk_response_t*)response;
    
    switch (req->command) {
        case RAMDISK_CMD_READ: {
            // Leer sectores
            if (response == NULL || response_size == NULL) {
                return E_INVAL;
            }
            
            size_t data_size = req->count * RAMDISK_SECTOR_SIZE;
            if (*response_size < sizeof(ramdisk_response_t) + data_size) {
                return E_NOMEM;
            }
            
            int result = ramdisk_read(req->sector, req->count, resp->data);
            resp->result = result;
            resp->data_size = (result > 0) ? (result * RAMDISK_SECTOR_SIZE) : 0;
            *response_size = sizeof(ramdisk_response_t) + resp->data_size;
            
            return E_OK;
        }
        
        case RAMDISK_CMD_WRITE: {
            // Escribir sectores
            size_t expected_size = sizeof(ramdisk_request_t) + (req->count * RAMDISK_SECTOR_SIZE);
            if (size < expected_size) {
                return E_INVAL;
            }
            
            int result = ramdisk_write(req->sector, req->count, req->data);
            
            if (response != NULL && response_size != NULL) {
                if (*response_size >= sizeof(ramdisk_response_t)) {
                    resp->result = result;
                    resp->data_size = 0;
                    *response_size = sizeof(ramdisk_response_t);
                }
            }
            
            return E_OK;
        }
        
        case RAMDISK_CMD_FORMAT: {
            // Formatear
            int result = ramdisk_format();
            
            if (response != NULL && response_size != NULL) {
                if (*response_size >= sizeof(ramdisk_response_t)) {
                    resp->result = result;
                    resp->data_size = 0;
                    *response_size = sizeof(ramdisk_response_t);
                }
            }
            
            return E_OK;
        }
        
        case RAMDISK_CMD_GETINFO: {
            // Obtener información
            if (response == NULL || response_size == NULL) {
                return E_INVAL;
            }
            
            if (*response_size < sizeof(ramdisk_response_t) + 8) {
                return E_NOMEM;
            }
            
            uint32_t* info = (uint32_t*)resp->data;
            info[0] = ramdisk_get_size();
            info[1] = ramdisk_get_sectors();
            
            resp->result = E_OK;
            resp->data_size = 8;
            *response_size = sizeof(ramdisk_response_t) + 8;
            
            return E_OK;
        }
        
        default:
            return E_INVAL;
    }
}

/**
 * Entry point del módulo
 * Estructura que será inicializada dinámicamente
 */
module_entry_t ramdisk_module_entry;

/**
 * Función para obtener el entry point del módulo
 * Esta función es llamada por el Module Manager
 */
module_entry_t* ramdisk_get_entry(void) {
    // Inicializar la estructura dinámicamente
    ramdisk_module_entry.init = module_ramdisk_init;
    ramdisk_module_entry.cleanup = module_ramdisk_cleanup;
    ramdisk_module_entry.start = module_ramdisk_start;
    ramdisk_module_entry.stop = module_ramdisk_stop;
    ramdisk_module_entry.update = module_ramdisk_update;
    ramdisk_module_entry.handle_message = module_ramdisk_handle_message;
    
    return &ramdisk_module_entry;
}
