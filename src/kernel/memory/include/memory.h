/**
 * NeoOS - Memory Manager Header
 * API del gestor de memoria del kernel
 * 
 * El Memory Manager de NeoOS gestiona la memoria del sistema mediante:
 * - PMM (Physical Memory Manager): Gestión de páginas físicas usando bitmap
 * - VMM (Virtual Memory Manager): Paginación y memoria virtual
 * - Heap: Asignación dinámica de memoria (kmalloc/kfree)
 */

#ifndef _KERNEL_MEMORY_H
#define _KERNEL_MEMORY_H

#include "../../lib/include/types.h"
#include "../../lib/include/multiboot.h"
#include "../../core/include/error.h"

// Tamaño de página: 4KB (4096 bytes)
#define PAGE_SIZE 4096

// Máscaras y offsets para paginación
#define PAGE_ALIGN_MASK 0xFFFFF000
#define PAGE_OFFSET_MASK 0x00000FFF

// Flags para páginas
#define PAGE_PRESENT    (1 << 0)  // Página presente en memoria
#define PAGE_WRITE      (1 << 1)  // Página escribible
#define PAGE_USER       (1 << 2)  // Página accesible desde modo usuario
#define PAGE_ACCESSED   (1 << 5)  // Página accedida
#define PAGE_DIRTY      (1 << 6)  // Página modificada

// Direcciones del kernel
#define KERNEL_START    0x00100000  // 1MB - Inicio del kernel
#define KERNEL_HEAP_START 0x00400000  // 4MB - Inicio del heap del kernel
#define KERNEL_HEAP_SIZE  0x00400000  // 4MB - Tamaño del heap

/*
 * ============================================================================
 * PHYSICAL MEMORY MANAGER (PMM)
 * ============================================================================
 * Gestiona las páginas físicas de memoria usando un bitmap.
 * Cada bit representa una página de 4KB:
 *   0 = página libre
 *   1 = página ocupada
 */

/**
 * Inicializa el Physical Memory Manager
 * 
 * @param mbi Información de Multiboot con el mapa de memoria
 * @return E_OK si fue exitoso, código de error en caso contrario
 */
int pmm_init(multiboot_info_t* mbi, bool kdebug __attribute__((unused)), bool kverbose __attribute__((unused)));

/**
 * Asigna una página física
 * 
 * @return Dirección física de la página asignada, 0 si no hay memoria
 */
uint32_t pmm_alloc_page(void);

/**
 * Libera una página física
 * 
 * @param page Dirección física de la página a liberar
 */
void pmm_free_page(uint32_t page);

/**
 * Obtiene la cantidad de páginas libres
 * 
 * @return Número de páginas de 4KB disponibles
 */
uint32_t pmm_get_free_pages(void);

/**
 * Obtiene la cantidad total de páginas
 * 
 * @return Número total de páginas de 4KB
 */
uint32_t pmm_get_total_pages(void);

/*
 * ============================================================================
 * VIRTUAL MEMORY MANAGER (VMM)
 * ============================================================================
 * Gestiona la memoria virtual usando paginación de 2 niveles:
 *   - Page Directory (1024 entradas)
 *   - Page Tables (1024 entradas cada una)
 * 
 * Cada entrada mapea 4KB, permitiendo direccionar 4GB de memoria virtual.
 */

// Tipos de entradas de paginación
typedef uint32_t page_directory_entry_t;
typedef uint32_t page_table_entry_t;

// Estructuras de paginación
typedef struct {
    page_table_entry_t entries[1024];
} __attribute__((aligned(PAGE_SIZE))) page_table_t;

typedef struct {
    page_directory_entry_t entries[1024];
} __attribute__((aligned(PAGE_SIZE))) page_directory_t;

/**
 * Inicializa el Virtual Memory Manager
 * Configura la paginación básica del kernel
 * 
 * @return E_OK si fue exitoso, código de error en caso contrario
 */
int vmm_init(bool kdebug __attribute__((unused)), bool kverbose __attribute__((unused)));

/**
 * Mapea una página virtual a una página física
 * 
 * @param page_dir Directorio de páginas
 * @param virt Dirección virtual
 * @param phys Dirección física
 * @param flags Flags de la página (PAGE_PRESENT, PAGE_WRITE, etc.)
 * @return E_OK si fue exitoso, código de error en caso contrario
 */
int vmm_map_page(page_directory_t* page_dir, uint32_t virt, uint32_t phys, uint32_t flags);

/**
 * Desmapea una página virtual
 * 
 * @param page_dir Directorio de páginas
 * @param virt Dirección virtual
 */
void vmm_unmap_page(page_directory_t* page_dir, uint32_t virt);

/**
 * Obtiene la dirección física de una dirección virtual
 * 
 * @param page_dir Directorio de páginas
 * @param virt Dirección virtual
 * @return Dirección física correspondiente, 0 si no está mapeada
 */
uint32_t vmm_get_physical(page_directory_t* page_dir, uint32_t virt);

/**
 * Cambia el directorio de páginas activo
 * 
 * @param page_dir Directorio de páginas a activar
 */
void vmm_switch_directory(page_directory_t* page_dir);

/**
 * Obtiene el directorio de páginas del kernel
 * 
 * @return Puntero al directorio de páginas del kernel
 */
page_directory_t* vmm_get_kernel_directory(void);

/*
 * ============================================================================
 * KERNEL HEAP
 * ============================================================================
 * Proporciona asignación dinámica de memoria para el kernel.
 * Implementa kmalloc() y kfree() similares a malloc() y free().
 */

/**
 * Inicializa el heap del kernel
 * 
 * @param start Dirección de inicio del heap
 * @param size Tamaño del heap en bytes
 * @return E_OK si fue exitoso, código de error en caso contrario
 */
int heap_init(uint32_t start, uint32_t size, bool kdebug __attribute__((unused)), bool kverbose __attribute__((unused)));

/**
 * Asigna memoria del heap del kernel
 * 
 * @param size Tamaño en bytes a asignar
 * @return Puntero a la memoria asignada, NULL si no hay memoria
 */
void* kmalloc(size_t size);

/**
 * Asigna memoria alineada a página del heap del kernel
 * 
 * @param size Tamaño en bytes a asignar
 * @return Puntero a la memoria asignada (alineada a 4KB), NULL si no hay memoria
 */
void* kmalloc_a(size_t size);

/**
 * Asigna memoria del heap del kernel y devuelve también la dirección física
 * 
 * @param size Tamaño en bytes a asignar
 * @param phys Puntero donde se almacenará la dirección física
 * @return Puntero a la memoria asignada, NULL si no hay memoria
 */
void* kmalloc_p(size_t size, uint32_t* phys);

/**
 * Asigna memoria alineada del heap del kernel y devuelve la dirección física
 * 
 * @param size Tamaño en bytes a asignar
 * @param phys Puntero donde se almacenará la dirección física
 * @return Puntero alineado a la memoria asignada, NULL si no hay memoria
 */
void* kmalloc_ap(size_t size, uint32_t* phys);

/**
 * Libera memoria del heap del kernel
 * 
 * @param ptr Puntero a la memoria a liberar
 */
void kfree(void* ptr);

/*
 * ============================================================================
 * INICIALIZACIÓN DEL MEMORY MANAGER
 * ============================================================================
 */

/**
 * Inicializa el Memory Manager completo
 * Debe ser llamado desde kernel_main() después de inicializar VGA
 * 
 * Inicializa en orden:
 * 1. PMM (Physical Memory Manager)
 * 2. VMM (Virtual Memory Manager)
 * 3. Heap del kernel
 * 
 * @param mbi Información de Multiboot con el mapa de memoria
 * @return E_OK si fue exitoso, código de error en caso contrario
 */
int memory_init(multiboot_info_t* mbi, bool kdebug __attribute__((unused)), bool kverbose __attribute__((unused)));

/**
 * Obtiene información sobre el estado de la memoria
 * 
 * @param total_kb Puntero donde se almacenará la memoria total en KB (puede ser NULL)
 * @param used_kb Puntero donde se almacenará la memoria usada en KB (puede ser NULL)
 * @param free_kb Puntero donde se almacenará la memoria libre en KB (puede ser NULL)
 */
void memory_get_info(uint32_t* total_kb, uint32_t* used_kb, uint32_t* free_kb);

#endif /* _KERNEL_MEMORY_H */
