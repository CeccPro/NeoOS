/**
 * @file module.h
 * @brief Gestor de módulos dinámicos de NeoOS
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

#ifndef MODULE_H
#define MODULE_H

#include <stdint.h>
#include <stddef.h>
#include "errors.h"

// ID de módulo (MID)
typedef uint32_t mid_t;

// Estado del módulo
typedef enum {
    MODULE_STATE_LOADED,      // Cargado en memoria
    MODULE_STATE_INITIALIZED, // Inicializado y listo
    MODULE_STATE_RUNNING,     // En ejecución
    MODULE_STATE_UNLOADING    // En proceso de descarga
} module_state_t;

// Tipo de módulo
typedef enum {
    MODULE_TYPE_DRIVER,       // Controlador de dispositivo
    MODULE_TYPE_FILESYSTEM,   // Sistema de archivos
    MODULE_TYPE_SERVICE,      // Servicio del sistema
    MODULE_TYPE_OTHER         // Otro tipo
} module_type_t;

// Estructura de información del módulo
typedef struct {
    char name[64];            // Nombre del módulo
    char version[16];         // Versión
    module_type_t type;       // Tipo de módulo
    char description[256];    // Descripción
} module_info_t;

// Función de inicialización del módulo
typedef error_t (*module_init_fn)(void);

// Función de limpieza del módulo
typedef void (*module_cleanup_fn)(void);

// Estructura del módulo
typedef struct module {
    mid_t mid;                    // ID del módulo
    module_info_t info;           // Información del módulo
    module_state_t state;         // Estado actual
    
    void* base_addr;              // Dirección base en memoria
    size_t size;                  // Tamaño del módulo
    
    module_init_fn init;          // Función de inicialización
    module_cleanup_fn cleanup;    // Función de limpieza
    
    uint32_t ref_count;           // Contador de referencias
    struct module* next;          // Siguiente módulo en la lista
} module_t;

/**
 * @brief Inicializa el gestor de módulos
 * @return E_OK si fue exitoso, código de error en caso contrario
 */
error_t module_manager_init(void);

/**
 * @brief Carga un módulo en memoria
 * @param data Puntero a los datos del módulo
 * @param size Tamaño del módulo
 * @param mid_out Puntero donde se almacenará el MID asignado
 * @return E_OK si fue exitoso, código de error en caso contrario
 */
error_t module_load(const void* data, size_t size, mid_t* mid_out);

/**
 * @brief Inicializa un módulo cargado
 * @param mid ID del módulo
 * @return E_OK si fue exitoso, código de error en caso contrario
 */
error_t module_init(mid_t mid);

/**
 * @brief Descarga un módulo de memoria
 * @param mid ID del módulo a descargar
 * @return E_OK si fue exitoso, código de error en caso contrario
 */
error_t module_unload(mid_t mid);

/**
 * @brief Obtiene información de un módulo
 * @param mid ID del módulo
 * @param info_out Puntero donde se almacenará la información
 * @return E_OK si fue exitoso, código de error en caso contrario
 */
error_t module_get_info(mid_t mid, module_info_t* info_out);

/**
 * @brief Obtiene un módulo por su nombre
 * @param name Nombre del módulo
 * @return Puntero al módulo o NULL si no existe
 */
module_t* module_find_by_name(const char* name);

/**
 * @brief Lista todos los módulos cargados
 * @param modules Array donde se almacenarán los MIDs
 * @param max_count Tamaño máximo del array
 * @return Número de módulos listados
 */
size_t module_list_all(mid_t* modules, size_t max_count);

#endif // MODULE_H
