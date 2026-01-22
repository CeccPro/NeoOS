/**
 * NeoOS - Early NeoFS Module Implementation
 * Sistema de archivos simplificado para uso temprano en el kernel
 */

#include "../include/early_neofs.h"
#include "../../core/include/module.h"
#include "../../core/include/error.h"
#include "../../core/include/timer.h"
#include "../../memory/include/memory.h"
#include "../../lib/include/string.h"
#include "../../drivers/include/vga.h"

// Estado global del filesystem
static uint8_t* fs_buffer = NULL;
static early_neofs_superblock_t* superblock = NULL;
static early_neofs_inode_t* inode_table = NULL;
static uint8_t* block_bitmap = NULL;
static uint8_t* inode_bitmap = NULL;
static uint8_t* data_blocks = NULL;
static early_neofs_file_descriptor_t file_descriptors[EARLY_NEOFS_MAX_OPEN_FILES];
static bool fs_initialized = false;

// ========== Funciones auxiliares de bitmap ==========

/**
 * Verifica si un bit está establecido en el bitmap
 */
static bool bitmap_test(uint8_t* bitmap, uint32_t bit) {
    uint32_t byte = bit / 8;
    uint32_t offset = bit % 8;
    return (bitmap[byte] & (1 << offset)) != 0;
}

/**
 * Establece un bit en el bitmap
 */
static void bitmap_set(uint8_t* bitmap, uint32_t bit) {
    uint32_t byte = bit / 8;
    uint32_t offset = bit % 8;
    bitmap[byte] |= (1 << offset);
}

/**
 * Limpia un bit en el bitmap
 */
static void bitmap_clear(uint8_t* bitmap, uint32_t bit) {
    uint32_t byte = bit / 8;
    uint32_t offset = bit % 8;
    bitmap[byte] &= ~(1 << offset);
}

/**
 * Encuentra un bit libre en el bitmap
 * @return Índice del bit libre, o -1 si no hay bits libres
 */
static int bitmap_find_free(uint8_t* bitmap, uint32_t max_bits) {
    for (uint32_t i = 0; i < max_bits; i++) {
        if (!bitmap_test(bitmap, i)) {
            return i;
        }
    }
    return -1;
}

// ========== Funciones de gestión de bloques ==========

/**
 * Asigna un bloque libre
 * @return Número de bloque asignado, o -1 si no hay bloques libres
 */
static int alloc_block(void) {
    if (superblock->free_blocks == 0) {
        return -1;
    }
    
    int block = bitmap_find_free(block_bitmap, superblock->total_blocks);
    if (block < 0) {
        return -1;
    }
    
    bitmap_set(block_bitmap, block);
    superblock->free_blocks--;
    
    return block;
}

/**
 * Libera un bloque
 */
static void free_block(uint32_t block) {
    if (block >= superblock->total_blocks) {
        return;
    }
    
    bitmap_clear(block_bitmap, block);
    superblock->free_blocks++;
}

/**
 * Lee datos de un bloque
 */
static void read_block(uint32_t block, void* buffer) {
    if (block >= superblock->total_blocks || buffer == NULL) {
        return;
    }
    
    uint32_t offset = block * EARLY_NEOFS_BLOCK_SIZE;
    memcpy(buffer, data_blocks + offset, EARLY_NEOFS_BLOCK_SIZE);
}

/**
 * Escribe datos en un bloque
 */
static void write_block(uint32_t block, const void* buffer) {
    if (block >= superblock->total_blocks || buffer == NULL) {
        return;
    }
    
    uint32_t offset = block * EARLY_NEOFS_BLOCK_SIZE;
    memcpy(data_blocks + offset, buffer, EARLY_NEOFS_BLOCK_SIZE);
}

// ========== Funciones de gestión de inodos ==========

/**
 * Asigna un inodo libre
 * @return Número de inodo asignado, o -1 si no hay inodos libres
 */
static int alloc_inode(void) {
    if (superblock->free_inodes == 0) {
        return -1;
    }
    
    int inode = bitmap_find_free(inode_bitmap, superblock->total_inodes);
    if (inode < 0) {
        return -1;
    }
    
    bitmap_set(inode_bitmap, inode);
    superblock->free_inodes--;
    
    // Inicializar el inodo
    early_neofs_inode_t* in = &inode_table[inode];
    memset(in, 0, sizeof(early_neofs_inode_t));
    in->inode_number = inode;
    in->create_time = timer_get_ticks();
    in->modify_time = in->create_time;
    in->access_time = in->create_time;
    in->link_count = 1;
    
    return inode;
}

/**
 * Libera un inodo y sus bloques asociados
 */
static void free_inode(uint32_t inode) {
    if (inode >= superblock->total_inodes) {
        return;
    }
    
    early_neofs_inode_t* in = &inode_table[inode];
    
    // Liberar bloques directos
    for (uint32_t i = 0; i < EARLY_NEOFS_DIRECT_BLOCKS; i++) {
        if (in->direct_blocks[i] != 0) {
            free_block(in->direct_blocks[i]);
            in->direct_blocks[i] = 0;
        }
    }
    
    // Liberar bloque indirecto si existe
    if (in->indirect_block != 0) {
        // TODO: Implementar gestión de bloques indirectos
        free_block(in->indirect_block);
        in->indirect_block = 0;
    }
    
    // Limpiar el inodo
    memset(in, 0, sizeof(early_neofs_inode_t));
    
    bitmap_clear(inode_bitmap, inode);
    superblock->free_inodes++;
}

/**
 * Obtiene un puntero al inodo
 */
static early_neofs_inode_t* get_inode(uint32_t inode) {
    if (inode >= superblock->total_inodes) {
        return NULL;
    }
    
    return &inode_table[inode];
}

// ========== Funciones de paths y búsqueda ==========

/**
 * Busca un archivo/directorio por su ruta
 * @return Número de inodo, o -1 si no se encuentra
 */
static int find_path(const char* path) {
    if (path == NULL || path[0] != '/') {
        return -1;
    }
    
    // Si es la raíz
    if (strcmp(path, "/") == 0) {
        return superblock->root_inode;
    }
    
    // Copiar el path para poder modificarlo
    char path_copy[256];
    strncpy(path_copy, path + 1, 255);  // Saltar el '/' inicial
    path_copy[255] = '\0';
    
    uint32_t current_inode = superblock->root_inode;
    char* token = path_copy;
    char* next_token;
    
    while (token != NULL && *token != '\0') {
        // Buscar el siguiente '/'
        next_token = strchr(token, '/');
        if (next_token != NULL) {
            *next_token = '\0';
            next_token++;
        }
        
        // Buscar el token en el directorio actual
        early_neofs_inode_t* dir_inode = get_inode(current_inode);
        if (dir_inode == NULL || dir_inode->type != INODE_TYPE_DIR) {
            return -1;
        }
        
        // Leer el contenido del directorio
        uint8_t block_buffer[EARLY_NEOFS_BLOCK_SIZE];
        bool found = false;
        
        for (uint32_t i = 0; i < EARLY_NEOFS_DIRECT_BLOCKS && !found; i++) {
            if (dir_inode->direct_blocks[i] == 0) {
                break;
            }
            
            read_block(dir_inode->direct_blocks[i], block_buffer);
            early_neofs_dir_entry_t* entries = (early_neofs_dir_entry_t*)block_buffer;
            
            uint32_t entries_per_block = EARLY_NEOFS_BLOCK_SIZE / sizeof(early_neofs_dir_entry_t);
            for (uint32_t j = 0; j < entries_per_block; j++) {
                if (entries[j].inode_number != 0 && strcmp(entries[j].name, token) == 0) {
                    current_inode = entries[j].inode_number;
                    found = true;
                    break;
                }
            }
        }
        
        if (!found) {
            return -1;
        }
        
        token = next_token;
    }
    
    return current_inode;
}

/**
 * Añade una entrada a un directorio
 */
static int add_dir_entry(uint32_t dir_inode, uint32_t child_inode, const char* name, inode_type_t type) {
    early_neofs_inode_t* dir = get_inode(dir_inode);
    if (dir == NULL || dir->type != INODE_TYPE_DIR) {
        return E_INVAL;
    }
    
    // Buscar un espacio libre en el directorio
    uint8_t block_buffer[EARLY_NEOFS_BLOCK_SIZE];
    
    for (uint32_t i = 0; i < EARLY_NEOFS_DIRECT_BLOCKS; i++) {
        // Si el bloque no está asignado, asignarlo
        if (dir->direct_blocks[i] == 0) {
            int new_block = alloc_block();
            if (new_block < 0) {
                return E_NOMEM;
            }
            dir->direct_blocks[i] = new_block;
            memset(block_buffer, 0, EARLY_NEOFS_BLOCK_SIZE);
            write_block(new_block, block_buffer);
        }
        
        read_block(dir->direct_blocks[i], block_buffer);
        early_neofs_dir_entry_t* entries = (early_neofs_dir_entry_t*)block_buffer;
        
        uint32_t entries_per_block = EARLY_NEOFS_BLOCK_SIZE / sizeof(early_neofs_dir_entry_t);
        for (uint32_t j = 0; j < entries_per_block; j++) {
            if (entries[j].inode_number == 0) {
                // Encontramos un espacio libre
                entries[j].inode_number = child_inode;
                strncpy(entries[j].name, name, EARLY_NEOFS_MAX_FILENAME - 1);
                entries[j].name[EARLY_NEOFS_MAX_FILENAME - 1] = '\0';
                entries[j].type = type;
                
                write_block(dir->direct_blocks[i], block_buffer);
                dir->modify_time = timer_get_ticks();
                return E_OK;
            }
        }
    }
    
    return E_NOMEM;
}

/**
 * Elimina una entrada de un directorio
 */
static int remove_dir_entry(uint32_t dir_inode, const char* name) {
    early_neofs_inode_t* dir = get_inode(dir_inode);
    if (dir == NULL || dir->type != INODE_TYPE_DIR) {
        return E_INVAL;
    }
    
    uint8_t block_buffer[EARLY_NEOFS_BLOCK_SIZE];
    
    for (uint32_t i = 0; i < EARLY_NEOFS_DIRECT_BLOCKS; i++) {
        if (dir->direct_blocks[i] == 0) {
            continue;
        }
        
        read_block(dir->direct_blocks[i], block_buffer);
        early_neofs_dir_entry_t* entries = (early_neofs_dir_entry_t*)block_buffer;
        
        uint32_t entries_per_block = EARLY_NEOFS_BLOCK_SIZE / sizeof(early_neofs_dir_entry_t);
        for (uint32_t j = 0; j < entries_per_block; j++) {
            if (entries[j].inode_number != 0 && strcmp(entries[j].name, name) == 0) {
                // Encontramos la entrada, la eliminamos
                memset(&entries[j], 0, sizeof(early_neofs_dir_entry_t));
                write_block(dir->direct_blocks[i], block_buffer);
                dir->modify_time = timer_get_ticks();
                return E_OK;
            }
        }
    }
    
    return E_NOENT;
}

// ========== API Pública ==========

/**
 * Formatea el filesystem
 */
int early_neofs_format(const char* label) {
    if (!fs_initialized || fs_buffer == NULL) {
        return E_INVAL;
    }
    
    // Limpiar todo el buffer
    memset(fs_buffer, 0, EARLY_NEOFS_SIZE);
    
    // Inicializar punteros a las estructuras
    superblock = (early_neofs_superblock_t*)fs_buffer;
    
    uint32_t offset = EARLY_NEOFS_BLOCK_SIZE;  // El superblock ocupa el bloque 0
    
    // Bitmap de inodos (1 bloque)
    inode_bitmap = fs_buffer + offset;
    offset += EARLY_NEOFS_BLOCK_SIZE;
    
    // Bitmap de bloques (1 bloque)
    block_bitmap = fs_buffer + offset;
    offset += EARLY_NEOFS_BLOCK_SIZE;
    
    // Tabla de inodos (suficientes bloques para MAX_INODES)
    uint32_t inode_table_size = EARLY_NEOFS_MAX_INODES * sizeof(early_neofs_inode_t);
    uint32_t inode_table_blocks = (inode_table_size + EARLY_NEOFS_BLOCK_SIZE - 1) / EARLY_NEOFS_BLOCK_SIZE;
    inode_table = (early_neofs_inode_t*)(fs_buffer + offset);
    offset += inode_table_blocks * EARLY_NEOFS_BLOCK_SIZE;
    
    // Bloques de datos
    data_blocks = fs_buffer + offset;
    uint32_t data_blocks_count = (EARLY_NEOFS_SIZE - offset) / EARLY_NEOFS_BLOCK_SIZE;
    
    // Inicializar superblock
    superblock->magic = EARLY_NEOFS_MAGIC;
    superblock->version = 1;
    superblock->block_size = EARLY_NEOFS_BLOCK_SIZE;
    superblock->total_blocks = data_blocks_count;
    superblock->free_blocks = data_blocks_count;
    superblock->total_inodes = EARLY_NEOFS_MAX_INODES;
    superblock->free_inodes = EARLY_NEOFS_MAX_INODES;
    superblock->root_inode = 0;
    superblock->first_data_block = offset / EARLY_NEOFS_BLOCK_SIZE;
    superblock->inode_table_block = 3;
    superblock->block_bitmap_block = 2;
    superblock->inode_bitmap_block = 1;
    superblock->mount_time = timer_get_ticks();
    superblock->mount_count = 1;
    
    if (label != NULL) {
        strncpy(superblock->label, label, 31);
        superblock->label[31] = '\0';
    } else {
        strcpy(superblock->label, "NeoOS Early FS");
    }
    
    // Crear directorio raíz
    int root_inode = alloc_inode();
    if (root_inode < 0) {
        return E_NOMEM;
    }
    
    superblock->root_inode = root_inode;
    early_neofs_inode_t* root = get_inode(root_inode);
    root->type = INODE_TYPE_DIR;
    root->permissions = 0755;
    root->parent_inode = root_inode;  // La raíz es su propio padre
    
    // Añadir entradas "." y ".." al directorio raíz
    add_dir_entry(root_inode, root_inode, ".", INODE_TYPE_DIR);
    add_dir_entry(root_inode, root_inode, "..", INODE_TYPE_DIR);
    return E_OK;
}

/**
 * Inicializa el early NeoFS
 */
int early_neofs_init(void) {
    if (fs_initialized) {
        return E_EXISTS;
    }
    
    // Asignar memoria para el filesystem
    fs_buffer = (uint8_t*)kmalloc(EARLY_NEOFS_SIZE);
    if (fs_buffer == NULL) {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_write("[EARLY_NEOFS] Error: No hay memoria suficiente\n");
        return E_NOMEM;
    }
    
    // Inicializar descriptores de archivo
    for (uint32_t i = 0; i < EARLY_NEOFS_MAX_OPEN_FILES; i++) {
        file_descriptors[i].in_use = false;
    }
    
    fs_initialized = true;
    
    // Formatear el filesystem
    int result = early_neofs_format("NeoOS Early FS");
    if (result != E_OK) {
        kfree(fs_buffer);
        fs_buffer = NULL;
        fs_initialized = false;
        return result;
    }
    
    return E_OK;
}

/**
 * Limpia el early NeoFS
 */
int early_neofs_cleanup(void) {
    if (!fs_initialized) {
        return E_INVAL;
    }
    
    // Cerrar todos los archivos abiertos
    for (uint32_t i = 0; i < EARLY_NEOFS_MAX_OPEN_FILES; i++) {
        if (file_descriptors[i].in_use) {
            file_descriptors[i].in_use = false;
        }
    }
    
    if (fs_buffer != NULL) {
        kfree(fs_buffer);
        fs_buffer = NULL;
    }
    
    fs_initialized = false;
    
    return E_OK;
}

/**
 * Crea un directorio
 */
int early_neofs_mkdir(const char* path, uint32_t permissions) {
    if (!fs_initialized || path == NULL) {
        return E_INVAL;
    }
    
    // Separar el path en directorio padre y nombre
    char parent_path[256];
    char dir_name[EARLY_NEOFS_MAX_FILENAME];
    
    const char* last_slash = strrchr(path, '/');
    if (last_slash == NULL) {
        return E_INVAL;
    }
    
    if (last_slash == path) {
        // El padre es la raíz
        strcpy(parent_path, "/");
    } else {
        uint32_t len = last_slash - path;
        if (len >= 256) {
            return E_INVAL;
        }
        memcpy(parent_path, path, len);
        parent_path[len] = '\0';
    }
    
    strncpy(dir_name, last_slash + 1, EARLY_NEOFS_MAX_FILENAME - 1);
    dir_name[EARLY_NEOFS_MAX_FILENAME - 1] = '\0';
    
    // Buscar el inodo del directorio padre
    int parent_inode = find_path(parent_path);
    if (parent_inode < 0) {
        return E_NOENT;
    }
    
    // Verificar que el padre sea un directorio
    early_neofs_inode_t* parent = get_inode(parent_inode);
    if (parent == NULL || parent->type != INODE_TYPE_DIR) {
        return E_INVAL;
    }
    
    // Verificar que no exista ya
    if (find_path(path) >= 0) {
        return E_EXISTS;
    }
    
    // Crear el nuevo inodo
    int new_inode = alloc_inode();
    if (new_inode < 0) {
        return E_NOMEM;
    }
    
    early_neofs_inode_t* dir = get_inode(new_inode);
    dir->type = INODE_TYPE_DIR;
    dir->permissions = permissions;
    dir->parent_inode = parent_inode;
    
    // Añadir entradas "." y ".."
    add_dir_entry(new_inode, new_inode, ".", INODE_TYPE_DIR);
    add_dir_entry(new_inode, parent_inode, "..", INODE_TYPE_DIR);
    
    // Añadir la entrada al directorio padre
    int result = add_dir_entry(parent_inode, new_inode, dir_name, INODE_TYPE_DIR);
    if (result != E_OK) {
        free_inode(new_inode);
        return result;
    }
    
    return E_OK;
}

/**
 * Crea un archivo
 */
int early_neofs_create(const char* path, uint32_t permissions) {
    if (!fs_initialized || path == NULL) {
        return E_INVAL;
    }
    
    // Separar el path en directorio padre y nombre
    char parent_path[256];
    char file_name[EARLY_NEOFS_MAX_FILENAME];
    
    const char* last_slash = strrchr(path, '/');
    if (last_slash == NULL) {
        return E_INVAL;
    }
    
    if (last_slash == path) {
        strcpy(parent_path, "/");
    } else {
        uint32_t len = last_slash - path;
        if (len >= 256) {
            return E_INVAL;
        }
        memcpy(parent_path, path, len);
        parent_path[len] = '\0';
    }
    
    strncpy(file_name, last_slash + 1, EARLY_NEOFS_MAX_FILENAME - 1);
    file_name[EARLY_NEOFS_MAX_FILENAME - 1] = '\0';
    
    // Buscar el inodo del directorio padre
    int parent_inode = find_path(parent_path);
    if (parent_inode < 0) {
        return E_NOENT;
    }
    
    // Verificar que el padre sea un directorio
    early_neofs_inode_t* parent = get_inode(parent_inode);
    if (parent == NULL || parent->type != INODE_TYPE_DIR) {
        return E_INVAL;
    }
    
    // Verificar que no exista ya
    if (find_path(path) >= 0) {
        return E_EXISTS;
    }
    
    // Crear el nuevo inodo
    int new_inode = alloc_inode();
    if (new_inode < 0) {
        return E_NOMEM;
    }
    
    early_neofs_inode_t* file = get_inode(new_inode);
    file->type = INODE_TYPE_FILE;
    file->permissions = permissions;
    file->parent_inode = parent_inode;
    file->size = 0;
    
    // Añadir la entrada al directorio padre
    int result = add_dir_entry(parent_inode, new_inode, file_name, INODE_TYPE_FILE);
    if (result != E_OK) {
        free_inode(new_inode);
        return result;
    }
    
    return E_OK;
}

/**
 * Abre un archivo
 */
int early_neofs_open(const char* path, uint32_t flags) {
    if (!fs_initialized || path == NULL) {
        return E_INVAL;
    }
    
    // Buscar un descriptor de archivo libre
    int fd = -1;
    for (uint32_t i = 0; i < EARLY_NEOFS_MAX_OPEN_FILES; i++) {
        if (!file_descriptors[i].in_use) {
            fd = i;
            break;
        }
    }
    
    if (fd < 0) {
        return E_NOMEM;
    }
    
    // Buscar el archivo
    int inode = find_path(path);
    
    // Si no existe y se especificó O_CREAT, crearlo
    if (inode < 0 && (flags & EARLY_NEOFS_O_CREAT)) {
        int result = early_neofs_create(path, 0644);
        if (result != E_OK) {
            return result;
        }
        inode = find_path(path);
    }
    
    if (inode < 0) {
        return E_NOENT;
    }
    
    // Verificar que sea un archivo
    early_neofs_inode_t* file = get_inode(inode);
    if (file == NULL || file->type != INODE_TYPE_FILE) {
        return E_INVAL;
    }
    
    // Si se especificó O_TRUNC, truncar el archivo
    if (flags & EARLY_NEOFS_O_TRUNC) {
        file->size = 0;
        file->blocks_used = 0;
        // Liberar bloques (simplificado)
        for (uint32_t i = 0; i < EARLY_NEOFS_DIRECT_BLOCKS; i++) {
            if (file->direct_blocks[i] != 0) {
                free_block(file->direct_blocks[i]);
                file->direct_blocks[i] = 0;
            }
        }
    }
    
    // Configurar el descriptor
    file_descriptors[fd].in_use = true;
    file_descriptors[fd].inode_number = inode;
    file_descriptors[fd].position = 0;
    file_descriptors[fd].flags = flags;
    
    file->access_time = timer_get_ticks();
    
    return fd;
}

/**
 * Cierra un archivo
 */
int early_neofs_close(int fd) {
    if (!fs_initialized || fd < 0 || fd >= EARLY_NEOFS_MAX_OPEN_FILES) {
        return E_INVAL;
    }
    
    if (!file_descriptors[fd].in_use) {
        return E_INVAL;
    }
    
    file_descriptors[fd].in_use = false;
    
    return E_OK;
}

/**
 * Lee datos de un archivo
 */
int early_neofs_read(int fd, void* buffer, uint32_t count) {
    if (!fs_initialized || fd < 0 || fd >= EARLY_NEOFS_MAX_OPEN_FILES || buffer == NULL) {
        return E_INVAL;
    }
    
    if (!file_descriptors[fd].in_use) {
        return E_INVAL;
    }
    
    uint32_t inode_num = file_descriptors[fd].inode_number;
    early_neofs_inode_t* file = get_inode(inode_num);
    if (file == NULL || file->type != INODE_TYPE_FILE) {
        return E_INVAL;
    }
    
    uint32_t pos = file_descriptors[fd].position;
    
    // Si estamos al final del archivo, no hay nada que leer
    if (pos >= file->size) {
        return 0;
    }
    
    // Ajustar count si excede el tamaño del archivo
    if (pos + count > file->size) {
        count = file->size - pos;
    }
    
    uint32_t bytes_read = 0;
    uint8_t block_buffer[EARLY_NEOFS_BLOCK_SIZE];
    
    while (bytes_read < count) {
        uint32_t block_index = pos / EARLY_NEOFS_BLOCK_SIZE;
        uint32_t block_offset = pos % EARLY_NEOFS_BLOCK_SIZE;
        uint32_t bytes_in_block = EARLY_NEOFS_BLOCK_SIZE - block_offset;
        uint32_t bytes_to_read = (bytes_in_block < (count - bytes_read)) ? bytes_in_block : (count - bytes_read);
        
        // Solo soportamos bloques directos por ahora
        if (block_index >= EARLY_NEOFS_DIRECT_BLOCKS) {
            break;
        }
        
        if (file->direct_blocks[block_index] == 0) {
            // El bloque no está asignado, devolver ceros
            memset((uint8_t*)buffer + bytes_read, 0, bytes_to_read);
        } else {
            read_block(file->direct_blocks[block_index], block_buffer);
            memcpy((uint8_t*)buffer + bytes_read, block_buffer + block_offset, bytes_to_read);
        }
        
        bytes_read += bytes_to_read;
        pos += bytes_to_read;
    }
    
    file_descriptors[fd].position = pos;
    file->access_time = timer_get_ticks();
    
    return bytes_read;
}

/**
 * Escribe datos en un archivo
 */
int early_neofs_write(int fd, const void* buffer, uint32_t count) {
    if (!fs_initialized || fd < 0 || fd >= EARLY_NEOFS_MAX_OPEN_FILES || buffer == NULL) {
        return E_INVAL;
    }
    
    if (!file_descriptors[fd].in_use) {
        return E_INVAL;
    }
    
    uint32_t inode_num = file_descriptors[fd].inode_number;
    early_neofs_inode_t* file = get_inode(inode_num);
    if (file == NULL || file->type != INODE_TYPE_FILE) {
        return E_INVAL;
    }
    
    uint32_t pos = file_descriptors[fd].position;
    uint32_t bytes_written = 0;
    uint8_t block_buffer[EARLY_NEOFS_BLOCK_SIZE];
    
    while (bytes_written < count) {
        uint32_t block_index = pos / EARLY_NEOFS_BLOCK_SIZE;
        uint32_t block_offset = pos % EARLY_NEOFS_BLOCK_SIZE;
        uint32_t bytes_in_block = EARLY_NEOFS_BLOCK_SIZE - block_offset;
        uint32_t bytes_to_write = (bytes_in_block < (count - bytes_written)) ? bytes_in_block : (count - bytes_written);
        
        // Solo soportamos bloques directos por ahora
        if (block_index >= EARLY_NEOFS_DIRECT_BLOCKS) {
            break;
        }
        
        // Asignar el bloque si no está asignado
        if (file->direct_blocks[block_index] == 0) {
            int new_block = alloc_block();
            if (new_block < 0) {
                break;  // No hay más bloques disponibles
            }
            file->direct_blocks[block_index] = new_block;
            file->blocks_used++;
            memset(block_buffer, 0, EARLY_NEOFS_BLOCK_SIZE);
        } else {
            read_block(file->direct_blocks[block_index], block_buffer);
        }
        
        memcpy(block_buffer + block_offset, (const uint8_t*)buffer + bytes_written, bytes_to_write);
        write_block(file->direct_blocks[block_index], block_buffer);
        
        bytes_written += bytes_to_write;
        pos += bytes_to_write;
    }
    
    // Actualizar el tamaño del archivo si es necesario
    if (pos > file->size) {
        file->size = pos;
    }
    
    file_descriptors[fd].position = pos;
    file->modify_time = timer_get_ticks();
    file->access_time = file->modify_time;
    
    return bytes_written;
}

/**
 * Elimina un archivo
 */
int early_neofs_unlink(const char* path) {
    if (!fs_initialized || path == NULL) {
        return E_INVAL;
    }
    
    // Buscar el archivo
    int inode = find_path(path);
    if (inode < 0) {
        return E_NOENT;
    }
    
    early_neofs_inode_t* file = get_inode(inode);
    if (file == NULL || file->type != INODE_TYPE_FILE) {
        return E_INVAL;
    }
    
    // Verificar que no esté abierto
    for (uint32_t i = 0; i < EARLY_NEOFS_MAX_OPEN_FILES; i++) {
        if (file_descriptors[i].in_use && file_descriptors[i].inode_number == (uint32_t)inode) {
            return E_BUSY;
        }
    }
    
    // Separar el path en directorio padre y nombre
    char parent_path[256];
    char file_name[EARLY_NEOFS_MAX_FILENAME];
    
    const char* last_slash = strrchr(path, '/');
    if (last_slash == NULL) {
        return E_INVAL;
    }
    
    if (last_slash == path) {
        strcpy(parent_path, "/");
    } else {
        uint32_t len = last_slash - path;
        memcpy(parent_path, path, len);
        parent_path[len] = '\0';
    }
    
    strncpy(file_name, last_slash + 1, EARLY_NEOFS_MAX_FILENAME - 1);
    file_name[EARLY_NEOFS_MAX_FILENAME - 1] = '\0';
    
    int parent_inode = find_path(parent_path);
    if (parent_inode < 0) {
        return E_NOENT;
    }
    
    // Eliminar la entrada del directorio padre
    int result = remove_dir_entry(parent_inode, file_name);
    if (result != E_OK) {
        return result;
    }
    
    // Liberar el inodo
    free_inode(inode);
    
    return E_OK;
}

/**
 * Elimina un directorio
 */
int early_neofs_rmdir(const char* path) {
    if (!fs_initialized || path == NULL) {
        return E_INVAL;
    }
    
    // No se puede eliminar la raíz
    if (strcmp(path, "/") == 0) {
        return E_INVAL;
    }
    
    // Buscar el directorio
    int inode = find_path(path);
    if (inode < 0) {
        return E_NOENT;
    }
    
    early_neofs_inode_t* dir = get_inode(inode);
    if (dir == NULL || dir->type != INODE_TYPE_DIR) {
        return E_INVAL;
    }
    
    // Verificar que esté vacío (solo debe tener "." y "..")
    uint8_t block_buffer[EARLY_NEOFS_BLOCK_SIZE];
    uint32_t entry_count = 0;
    
    for (uint32_t i = 0; i < EARLY_NEOFS_DIRECT_BLOCKS; i++) {
        if (dir->direct_blocks[i] == 0) {
            continue;
        }
        
        read_block(dir->direct_blocks[i], block_buffer);
        early_neofs_dir_entry_t* entries = (early_neofs_dir_entry_t*)block_buffer;
        
        uint32_t entries_per_block = EARLY_NEOFS_BLOCK_SIZE / sizeof(early_neofs_dir_entry_t);
        for (uint32_t j = 0; j < entries_per_block; j++) {
            if (entries[j].inode_number != 0) {
                entry_count++;
            }
        }
    }
    
    // Si tiene más de 2 entradas (. y ..), no está vacío
    if (entry_count > 2) {
        return E_INVAL;
    }
    
    // Separar el path en directorio padre y nombre
    char parent_path[256];
    char dir_name[EARLY_NEOFS_MAX_FILENAME];
    
    const char* last_slash = strrchr(path, '/');
    if (last_slash == NULL) {
        return E_INVAL;
    }
    
    if (last_slash == path) {
        strcpy(parent_path, "/");
    } else {
        uint32_t len = last_slash - path;
        memcpy(parent_path, path, len);
        parent_path[len] = '\0';
    }
    
    strncpy(dir_name, last_slash + 1, EARLY_NEOFS_MAX_FILENAME - 1);
    dir_name[EARLY_NEOFS_MAX_FILENAME - 1] = '\0';
    
    int parent_inode = find_path(parent_path);
    if (parent_inode < 0) {
        return E_NOENT;
    }
    
    // Eliminar la entrada del directorio padre
    int result = remove_dir_entry(parent_inode, dir_name);
    if (result != E_OK) {
        return result;
    }
    
    // Liberar el inodo
    free_inode(inode);
    
    return E_OK;
}

/**
 * Obtiene información de un archivo o directorio
 */
int early_neofs_stat(const char* path, early_neofs_stat_t* stat) {
    if (!fs_initialized || path == NULL || stat == NULL) {
        return E_INVAL;
    }
    
    int inode = find_path(path);
    if (inode < 0) {
        return E_NOENT;
    }
    
    early_neofs_inode_t* in = get_inode(inode);
    if (in == NULL) {
        return E_INVAL;
    }
    
    stat->inode_number = inode;
    stat->type = in->type;
    stat->size = in->size;
    stat->blocks_used = in->blocks_used;
    stat->create_time = in->create_time;
    stat->modify_time = in->modify_time;
    stat->access_time = in->access_time;
    stat->permissions = in->permissions;
    
    return E_OK;
}

/**
 * Lee las entradas de un directorio
 */
int early_neofs_readdir(const char* path, early_neofs_dir_entry_t* entries, uint32_t max_entries) {
    if (!fs_initialized || path == NULL || entries == NULL || max_entries == 0) {
        return E_INVAL;
    }
    
    int inode = find_path(path);
    if (inode < 0) {
        return E_NOENT;
    }
    
    early_neofs_inode_t* dir = get_inode(inode);
    if (dir == NULL || dir->type != INODE_TYPE_DIR) {
        return E_INVAL;
    }
    
    uint32_t entry_count = 0;
    uint8_t block_buffer[EARLY_NEOFS_BLOCK_SIZE];
    
    for (uint32_t i = 0; i < EARLY_NEOFS_DIRECT_BLOCKS && entry_count < max_entries; i++) {
        if (dir->direct_blocks[i] == 0) {
            continue;
        }
        
        read_block(dir->direct_blocks[i], block_buffer);
        early_neofs_dir_entry_t* block_entries = (early_neofs_dir_entry_t*)block_buffer;
        
        uint32_t entries_per_block = EARLY_NEOFS_BLOCK_SIZE / sizeof(early_neofs_dir_entry_t);
        for (uint32_t j = 0; j < entries_per_block && entry_count < max_entries; j++) {
            if (block_entries[j].inode_number != 0) {
                memcpy(&entries[entry_count], &block_entries[j], sizeof(early_neofs_dir_entry_t));
                entry_count++;
            }
        }
    }
    
    dir->access_time = timer_get_ticks();
    
    return entry_count;
}

// ========== Implementación del Module Entry ==========

/**
 * Función de inicialización del módulo
 */
int module_early_neofs_init(void) {
    return early_neofs_init();
}

/**
 * Función de limpieza del módulo
 */
int module_early_neofs_cleanup(void) {
    return early_neofs_cleanup();
}

/**
 * Función de inicio del módulo
 */
int module_early_neofs_start(void) {
    return E_OK;
}

/**
 * Función de detención del módulo
 */
int module_early_neofs_stop(void) {
    return E_OK;
}

/**
 * Función de actualización del módulo
 */
int module_early_neofs_update(void) {
    // El filesystem no necesita actualizaciones periódicas
    return E_OK;
}

/**
 * Handler de mensajes IPC
 */
int module_early_neofs_handle_message(const void* msg, size_t size, void* response, size_t* response_size) {
    if (msg == NULL || size < sizeof(early_neofs_ipc_request_t)) {
        return E_INVAL;
    }
    
    const early_neofs_ipc_request_t* req = (const early_neofs_ipc_request_t*)msg;
    early_neofs_ipc_response_t* resp = (early_neofs_ipc_response_t*)response;
    
    switch (req->command) {
        case EARLY_NEOFS_CMD_FORMAT: {
            // Formatear filesystem
            int result = early_neofs_format(req->path[0] != '\0' ? req->path : NULL);
            
            if (response != NULL && response_size != NULL) {
                if (*response_size >= sizeof(early_neofs_ipc_response_t)) {
                    resp->result = result;
                    resp->data_size = 0;
                    *response_size = sizeof(early_neofs_ipc_response_t);
                }
            }
            
            return E_OK;
        }
        
        case EARLY_NEOFS_CMD_CREATE: {
            // Crear archivo
            int result = early_neofs_create(req->path, req->permissions);
            
            if (response != NULL && response_size != NULL) {
                if (*response_size >= sizeof(early_neofs_ipc_response_t)) {
                    resp->result = result;
                    resp->data_size = 0;
                    *response_size = sizeof(early_neofs_ipc_response_t);
                }
            }
            
            return E_OK;
        }
        
        case EARLY_NEOFS_CMD_MKDIR: {
            // Crear directorio
            int result = early_neofs_mkdir(req->path, req->permissions);
            
            if (response != NULL && response_size != NULL) {
                if (*response_size >= sizeof(early_neofs_ipc_response_t)) {
                    resp->result = result;
                    resp->data_size = 0;
                    *response_size = sizeof(early_neofs_ipc_response_t);
                }
            }
            
            return E_OK;
        }
        
        case EARLY_NEOFS_CMD_OPEN: {
            // Abrir archivo
            int result = early_neofs_open(req->path, req->flags);
            
            if (response != NULL && response_size != NULL) {
                if (*response_size >= sizeof(early_neofs_ipc_response_t)) {
                    resp->result = result;  // result es el fd o código de error
                    resp->data_size = 0;
                    *response_size = sizeof(early_neofs_ipc_response_t);
                }
            }
            
            return E_OK;
        }
        
        case EARLY_NEOFS_CMD_CLOSE: {
            // Cerrar archivo
            int result = early_neofs_close(req->fd);
            
            if (response != NULL && response_size != NULL) {
                if (*response_size >= sizeof(early_neofs_ipc_response_t)) {
                    resp->result = result;
                    resp->data_size = 0;
                    *response_size = sizeof(early_neofs_ipc_response_t);
                }
            }
            
            return E_OK;
        }
        
        case EARLY_NEOFS_CMD_READ: {
            // Leer de archivo
            if (response == NULL || response_size == NULL) {
                return E_INVAL;
            }
            
            if (*response_size < sizeof(early_neofs_ipc_response_t) + req->count) {
                return E_NOMEM;
            }
            
            int result = early_neofs_read(req->fd, resp->data, req->count);
            resp->result = result;
            resp->data_size = (result > 0) ? result : 0;
            *response_size = sizeof(early_neofs_ipc_response_t) + resp->data_size;
            
            return E_OK;
        }
        
        case EARLY_NEOFS_CMD_WRITE: {
            // Escribir en archivo
            size_t expected_size = sizeof(early_neofs_ipc_request_t) + req->count;
            if (size < expected_size) {
                return E_INVAL;
            }
            
            int result = early_neofs_write(req->fd, req->data, req->count);
            
            if (response != NULL && response_size != NULL) {
                if (*response_size >= sizeof(early_neofs_ipc_response_t)) {
                    resp->result = result;
                    resp->data_size = 0;
                    *response_size = sizeof(early_neofs_ipc_response_t);
                }
            }
            
            return E_OK;
        }
        
        case EARLY_NEOFS_CMD_UNLINK: {
            // Eliminar archivo
            int result = early_neofs_unlink(req->path);
            
            if (response != NULL && response_size != NULL) {
                if (*response_size >= sizeof(early_neofs_ipc_response_t)) {
                    resp->result = result;
                    resp->data_size = 0;
                    *response_size = sizeof(early_neofs_ipc_response_t);
                }
            }
            
            return E_OK;
        }
        
        case EARLY_NEOFS_CMD_RMDIR: {
            // Eliminar directorio
            int result = early_neofs_rmdir(req->path);
            
            if (response != NULL && response_size != NULL) {
                if (*response_size >= sizeof(early_neofs_ipc_response_t)) {
                    resp->result = result;
                    resp->data_size = 0;
                    *response_size = sizeof(early_neofs_ipc_response_t);
                }
            }
            
            return E_OK;
        }
        
        case EARLY_NEOFS_CMD_STAT: {
            // Obtener información de archivo
            if (response == NULL || response_size == NULL) {
                return E_INVAL;
            }
            
            if (*response_size < sizeof(early_neofs_ipc_response_t) + sizeof(early_neofs_stat_t)) {
                return E_NOMEM;
            }
            
            early_neofs_stat_t* stat = (early_neofs_stat_t*)resp->data;
            int result = early_neofs_stat(req->path, stat);
            
            resp->result = result;
            resp->data_size = (result == E_OK) ? sizeof(early_neofs_stat_t) : 0;
            *response_size = sizeof(early_neofs_ipc_response_t) + resp->data_size;
            
            return E_OK;
        }
        
        case EARLY_NEOFS_CMD_READDIR: {
            // Leer directorio
            if (response == NULL || response_size == NULL) {
                return E_INVAL;
            }
            
            uint32_t max_entries = req->max_entries;
            if (max_entries == 0 || max_entries > EARLY_NEOFS_MAX_DIR_ENTRIES) {
                max_entries = EARLY_NEOFS_MAX_DIR_ENTRIES;
            }
            
            size_t required_size = sizeof(early_neofs_ipc_response_t) + 
                                 (max_entries * sizeof(early_neofs_dir_entry_t));
            if (*response_size < required_size) {
                return E_NOMEM;
            }
            
            early_neofs_dir_entry_t* entries = (early_neofs_dir_entry_t*)resp->data;
            int result = early_neofs_readdir(req->path, entries, max_entries);
            
            resp->result = result;
            resp->data_size = (result > 0) ? (result * sizeof(early_neofs_dir_entry_t)) : 0;
            *response_size = sizeof(early_neofs_ipc_response_t) + resp->data_size;
            
            return E_OK;
        }
        
        default:
            return E_INVAL;
    }
}

/**
 * Entry point del módulo
 */
module_entry_t early_neofs_module_entry;

/**
 * Función para obtener el entry point del módulo
 */
module_entry_t* early_neofs_get_entry(void) {
    early_neofs_module_entry.init = module_early_neofs_init;
    early_neofs_module_entry.cleanup = module_early_neofs_cleanup;
    early_neofs_module_entry.start = module_early_neofs_start;
    early_neofs_module_entry.stop = module_early_neofs_stop;
    early_neofs_module_entry.update = module_early_neofs_update;
    early_neofs_module_entry.handle_message = module_early_neofs_handle_message;
    
    return &early_neofs_module_entry;
}
