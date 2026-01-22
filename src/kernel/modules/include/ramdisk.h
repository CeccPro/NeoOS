/**
 * NeoOS - RAM Disk Module
 * Módulo de disco en memoria RAM
 */

#ifndef _MODULE_RAMDISK_H
#define _MODULE_RAMDISK_H

#include "../../lib/include/types.h"
#include "../../core/include/error.h"
#include "../../core/include/module.h"

/**
 * Tamaño del ramdisk (1MB por defecto)
 */
#define RAMDISK_SIZE (1024 * 1024)

/**
 * Tamaño de un sector (512 bytes)
 */
#define RAMDISK_SECTOR_SIZE 512

/**
 * Número de sectores en el ramdisk
 */
#define RAMDISK_SECTORS (RAMDISK_SIZE / RAMDISK_SECTOR_SIZE)

/**
 * Comandos IPC del ramdisk
 */
#define RAMDISK_CMD_READ    1   // Leer sectores
#define RAMDISK_CMD_WRITE   2   // Escribir sectores
#define RAMDISK_CMD_FORMAT  3   // Formatear
#define RAMDISK_CMD_GETINFO 4   // Obtener información

/**
 * Estructura de petición para lectura/escritura
 */
typedef struct {
    uint32_t command;       // Comando a ejecutar
    uint32_t sector;        // Sector inicial
    uint32_t count;         // Número de sectores
    uint8_t data[];         // Datos (para escritura)
} ramdisk_request_t;

/**
 * Estructura de respuesta
 */
typedef struct {
    int result;             // Código de resultado
    uint32_t data_size;     // Tamaño de los datos retornados
    uint8_t data[];         // Datos (para lectura)
} ramdisk_response_t;

/**
 * Inicializa el ramdisk
 * @return E_OK si éxito, código de error en caso contrario
 */
int ramdisk_init(void);

/**
 * Limpia el ramdisk
 * @return E_OK si éxito, código de error en caso contrario
 */
int ramdisk_cleanup(void);

/**
 * Lee datos del ramdisk
 * @param sector: Sector inicial
 * @param count: Número de sectores a leer
 * @param buffer: Buffer donde se almacenarán los datos
 * @return Número de sectores leídos, o código de error negativo
 */
int ramdisk_read(uint32_t sector, uint32_t count, void* buffer);

/**
 * Escribe datos en el ramdisk
 * @param sector: Sector inicial
 * @param count: Número de sectores a escribir
 * @param buffer: Buffer con los datos a escribir
 * @return Número de sectores escritos, o código de error negativo
 */
int ramdisk_write(uint32_t sector, uint32_t count, const void* buffer);

/**
 * Obtiene el tamaño del ramdisk en bytes
 * @return Tamaño del ramdisk
 */
uint32_t ramdisk_get_size(void);

/**
 * Obtiene el número de sectores del ramdisk
 * @return Número de sectores
 */
uint32_t ramdisk_get_sectors(void);

/**
 * Formatea el ramdisk (lo llena de ceros)
 * @return E_OK si éxito, código de error en caso contrario
 */
int ramdisk_format(void);

/**
 * Obtiene el entry point del módulo
 * Esta función es requerida por el Module Manager
 */
module_entry_t* ramdisk_get_entry(void);

#endif /* _MODULE_RAMDISK_H */
