/**
 * NeoOS - Early NeoFS Module
 * Sistema de archivos simplificado para uso temprano en el kernel
 */

#ifndef _MODULE_EARLY_NEOFS_H
#define _MODULE_EARLY_NEOFS_H

#include "../../lib/include/types.h"
#include "../../core/include/error.h"
#include "../../core/include/module.h"

/**
 * Tamaño del early NeoFS (2MB por defecto)
 */
#define EARLY_NEOFS_SIZE (2 * 1024 * 1024)

/**
 * Tamaño de un bloque (1KB)
 */
#define EARLY_NEOFS_BLOCK_SIZE 1024

/**
 * Número de bloques en el filesystem
 */
#define EARLY_NEOFS_BLOCKS (EARLY_NEOFS_SIZE / EARLY_NEOFS_BLOCK_SIZE)

/**
 * Número máximo de inodos
 */
#define EARLY_NEOFS_MAX_INODES 128

/**
 * Número máximo de archivos abiertos simultáneamente
 */
#define EARLY_NEOFS_MAX_OPEN_FILES 32

/**
 * Longitud máxima del nombre de archivo
 */
#define EARLY_NEOFS_MAX_FILENAME 56

/**
 * Número máximo de entradas por directorio
 */
#define EARLY_NEOFS_MAX_DIR_ENTRIES 64

/**
 * Número de bloques directos por inodo
 */
#define EARLY_NEOFS_DIRECT_BLOCKS 10

/**
 * Magic number del superblock
 */
#define EARLY_NEOFS_MAGIC 0x4E454F46  // "NEOF" en ASCII

/**
 * Tipos de inodo
 */
typedef enum {
    INODE_TYPE_FREE = 0,
    INODE_TYPE_FILE = 1,
    INODE_TYPE_DIR = 2
} inode_type_t;

/**
 * Superblock del filesystem
 * Contiene información general sobre el filesystem
 */
typedef struct {
    uint32_t magic;                     // Magic number (EARLY_NEOFS_MAGIC)
    uint32_t version;                   // Versión del filesystem
    uint32_t block_size;                // Tamaño de un bloque
    uint32_t total_blocks;              // Número total de bloques
    uint32_t free_blocks;               // Número de bloques libres
    uint32_t total_inodes;              // Número total de inodos
    uint32_t free_inodes;               // Número de inodos libres
    uint32_t root_inode;                // Número de inodo raíz
    uint32_t first_data_block;          // Primer bloque de datos
    uint32_t inode_table_block;         // Bloque donde comienza la tabla de inodos
    uint32_t block_bitmap_block;        // Bloque del bitmap de bloques
    uint32_t inode_bitmap_block;        // Bloque del bitmap de inodos
    uint32_t mount_time;                // Tiempo de montaje (tick del sistema)
    uint32_t mount_count;               // Número de veces montado
    char label[32];                     // Etiqueta del volumen
    uint8_t padding[EARLY_NEOFS_BLOCK_SIZE - 96];  // Padding para llenar el bloque
} __attribute__((packed)) early_neofs_superblock_t;

/**
 * Inodo del filesystem
 * Representa un archivo o directorio
 */
typedef struct {
    uint32_t inode_number;              // Número de inodo
    inode_type_t type;                  // Tipo de inodo (archivo o directorio)
    uint32_t size;                      // Tamaño en bytes
    uint32_t blocks_used;               // Número de bloques usados
    uint32_t create_time;               // Tiempo de creación (tick del sistema)
    uint32_t modify_time;               // Tiempo de modificación
    uint32_t access_time;               // Tiempo de último acceso
    uint32_t permissions;               // Permisos (simplificado)
    uint32_t direct_blocks[EARLY_NEOFS_DIRECT_BLOCKS];  // Bloques directos
    uint32_t indirect_block;            // Bloque indirecto (para archivos grandes)
    uint32_t parent_inode;              // Inodo padre (para directorios)
    uint32_t link_count;                // Número de enlaces al inodo
} __attribute__((packed)) early_neofs_inode_t;

/**
 * Entrada de directorio
 */
typedef struct {
    uint32_t inode_number;              // Número de inodo del archivo/directorio
    char name[EARLY_NEOFS_MAX_FILENAME]; // Nombre del archivo/directorio
    inode_type_t type;                  // Tipo (archivo o directorio)
} __attribute__((packed)) early_neofs_dir_entry_t;

/**
 * Descriptor de archivo abierto
 */
typedef struct {
    bool in_use;                        // Si el descriptor está en uso
    uint32_t inode_number;              // Número de inodo
    uint32_t position;                  // Posición actual en el archivo
    uint32_t flags;                     // Flags de apertura (lectura, escritura, etc.)
} early_neofs_file_descriptor_t;

/**
 * Estructura de información de archivo (para stat)
 */
typedef struct {
    uint32_t inode_number;              // Número de inodo
    inode_type_t type;                  // Tipo de inodo
    uint32_t size;                      // Tamaño en bytes
    uint32_t blocks_used;               // Número de bloques usados
    uint32_t create_time;               // Tiempo de creación
    uint32_t modify_time;               // Tiempo de modificación
    uint32_t access_time;               // Tiempo de último acceso
    uint32_t permissions;               // Permisos
} early_neofs_stat_t;

/**
 * Flags de apertura de archivo
 */
#define EARLY_NEOFS_O_RDONLY 0x01       // Solo lectura
#define EARLY_NEOFS_O_WRONLY 0x02       // Solo escritura
#define EARLY_NEOFS_O_RDWR   0x03       // Lectura y escritura
#define EARLY_NEOFS_O_CREAT  0x04       // Crear si no existe
#define EARLY_NEOFS_O_TRUNC  0x08       // Truncar a 0 bytes

/**
 * Comandos IPC del early NeoFS
 */
#define EARLY_NEOFS_CMD_OPEN    1       // Abrir archivo
#define EARLY_NEOFS_CMD_CLOSE   2       // Cerrar archivo
#define EARLY_NEOFS_CMD_READ    3       // Leer de archivo
#define EARLY_NEOFS_CMD_WRITE   4       // Escribir en archivo
#define EARLY_NEOFS_CMD_MKDIR   5       // Crear directorio
#define EARLY_NEOFS_CMD_RMDIR   6       // Eliminar directorio
#define EARLY_NEOFS_CMD_UNLINK  7       // Eliminar archivo
#define EARLY_NEOFS_CMD_STAT    8       // Obtener información de archivo
#define EARLY_NEOFS_CMD_READDIR 9       // Leer directorio
#define EARLY_NEOFS_CMD_FORMAT  10      // Formatear filesystem
#define EARLY_NEOFS_CMD_CREATE  11      // Crear archivo

/**
 * Estructura de petición IPC genérica
 */
typedef struct {
    uint32_t command;                       // Comando a ejecutar
    char path[256];                         // Path del archivo/directorio
    uint32_t flags;                         // Flags (para open, mkdir, etc)
    uint32_t permissions;                   // Permisos (para create, mkdir)
    int fd;                                 // File descriptor (para read, write, close)
    uint32_t count;                         // Número de bytes a leer/escribir
    uint32_t max_entries;                   // Máximo de entradas (para readdir)
    uint8_t data[];                         // Datos (para write)
} early_neofs_ipc_request_t;

/**
 * Estructura de respuesta IPC genérica
 */
typedef struct {
    int result;                             // Código de resultado
    uint32_t data_size;                     // Tamaño de los datos retornados
    uint8_t data[];                         // Datos (para read, stat, readdir, etc)
} early_neofs_ipc_response_t;

// ========== API Pública ==========

/**
 * Inicializa el early NeoFS
 * @return E_OK si éxito, código de error en caso contrario
 */
int early_neofs_init(void);

/**
 * Limpia el early NeoFS
 * @return E_OK si éxito, código de error en caso contrario
 */
int early_neofs_cleanup(void);

/**
 * Formatea el filesystem
 * @param label: Etiqueta del volumen
 * @return E_OK si éxito, código de error en caso contrario
 */
int early_neofs_format(const char* label);

/**
 * Crea un archivo
 * @param path: Ruta del archivo
 * @param permissions: Permisos del archivo
 * @return E_OK si éxito, código de error en caso contrario
 */
int early_neofs_create(const char* path, uint32_t permissions);

/**
 * Crea un directorio
 * @param path: Ruta del directorio
 * @param permissions: Permisos del directorio
 * @return E_OK si éxito, código de error en caso contrario
 */
int early_neofs_mkdir(const char* path, uint32_t permissions);

/**
 * Abre un archivo
 * @param path: Ruta del archivo
 * @param flags: Flags de apertura
 * @return File descriptor si éxito, código de error negativo en caso contrario
 */
int early_neofs_open(const char* path, uint32_t flags);

/**
 * Cierra un archivo
 * @param fd: File descriptor
 * @return E_OK si éxito, código de error en caso contrario
 */
int early_neofs_close(int fd);

/**
 * Lee datos de un archivo
 * @param fd: File descriptor
 * @param buffer: Buffer donde se almacenarán los datos
 * @param count: Número de bytes a leer
 * @return Número de bytes leídos, o código de error negativo
 */
int early_neofs_read(int fd, void* buffer, uint32_t count);

/**
 * Escribe datos en un archivo
 * @param fd: File descriptor
 * @param buffer: Buffer con los datos a escribir
 * @param count: Número de bytes a escribir
 * @return Número de bytes escritos, o código de error negativo
 */
int early_neofs_write(int fd, const void* buffer, uint32_t count);

/**
 * Elimina un archivo
 * @param path: Ruta del archivo
 * @return E_OK si éxito, código de error en caso contrario
 */
int early_neofs_unlink(const char* path);

/**
 * Elimina un directorio
 * @param path: Ruta del directorio
 * @return E_OK si éxito, código de error en caso contrario
 */
int early_neofs_rmdir(const char* path);

/**
 * Obtiene información de un archivo o directorio
 * @param path: Ruta del archivo/directorio
 * @param stat: Estructura donde se almacenará la información
 * @return E_OK si éxito, código de error en caso contrario
 */
int early_neofs_stat(const char* path, early_neofs_stat_t* stat);

/**
 * Lee las entradas de un directorio
 * @param path: Ruta del directorio
 * @param entries: Array donde se almacenarán las entradas
 * @param max_entries: Número máximo de entradas a leer
 * @return Número de entradas leídas, o código de error negativo
 */
int early_neofs_readdir(const char* path, early_neofs_dir_entry_t* entries, uint32_t max_entries);

/**
 * Obtiene el entry point del módulo
 * Esta función es llamada por el Module Manager
 */
module_entry_t* early_neofs_get_entry(void);

#endif // _MODULE_EARLY_NEOFS_H
