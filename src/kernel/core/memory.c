/**
 * @file memory.c
 * @brief Implementación del gestor de memoria de NeoOS
 * 
 * Este módulo implementa el Physical Memory Manager (PMM) y el
 * Virtual Memory Manager (VMM) con paginación.
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

#include "../include/memory.h"
#include "../include/kernel.h"
#include "../include/vga.h"
#include <stdbool.h>

// Símbolos del linker para el final del kernel
extern uint32_t end;

// Variables del PMM
static uint32_t* pmm_bitmap = NULL;     // Bitmap de páginas físicas
static uint32_t pmm_bitmap_size = 0;    // Tamaño del bitmap en DWORDs
static uint32_t pmm_total_pages = 0;    // Total de páginas físicas
static uint32_t pmm_free_pages = 0;     // Páginas libres

// Variables del VMM
static uint32_t* kernel_page_directory = NULL;
static uint32_t kernel_heap_current = 0;
static uint32_t kernel_heap_end = 0;

#define KERNEL_HEAP_START 0xC0000000
#define KERNEL_HEAP_SIZE  0x10000000  // 256 MB

// Macros para el bitmap
#define PMM_BLOCKS_PER_BYTE 8
#define PMM_BLOCK_SIZE PAGE_SIZE
#define PMM_BITS_PER_INDEX 32

static inline void pmm_bitmap_set(uint32_t bit) {
    pmm_bitmap[bit / PMM_BITS_PER_INDEX] |= (1 << (bit % PMM_BITS_PER_INDEX));
}

static inline void pmm_bitmap_clear(uint32_t bit) {
    pmm_bitmap[bit / PMM_BITS_PER_INDEX] &= ~(1 << (bit % PMM_BITS_PER_INDEX));
}

static inline bool pmm_bitmap_test(uint32_t bit) {
    return pmm_bitmap[bit / PMM_BITS_PER_INDEX] & (1 << (bit % PMM_BITS_PER_INDEX));
}

/**
 * @brief Encuentra el primer bit libre en el bitmap
 */
static int32_t pmm_find_first_free(void) {
    for (uint32_t i = 0; i < pmm_bitmap_size; i++) {
        if (pmm_bitmap[i] != 0xFFFFFFFF) {
            for (uint32_t j = 0; j < PMM_BITS_PER_INDEX; j++) {
                if (!(pmm_bitmap[i] & (1 << j))) {
                    return i * PMM_BITS_PER_INDEX + j;
                }
            }
        }
    }
    return -1;
}

/**
 * @brief Asigna una página física
 */
void* pmm_alloc_page(void) {
    if (pmm_free_pages == 0) {
        return NULL;
    }

    int32_t page = pmm_find_first_free();
    if (page < 0) {
        return NULL;
    }

    pmm_bitmap_set(page);
    pmm_free_pages--;

    return (void*)(page * PMM_BLOCK_SIZE);
}

/**
 * @brief Libera una página física
 */
void pmm_free_page(void* page) {
    uint32_t addr = (uint32_t)page;
    uint32_t block = addr / PMM_BLOCK_SIZE;

    if (pmm_bitmap_test(block)) {
        pmm_bitmap_clear(block);
        pmm_free_pages++;
    }
}

/**
 * @brief Obtiene la cantidad total de memoria
 */
size_t pmm_get_total_memory(void) {
    return pmm_total_pages * PAGE_SIZE;
}

/**
 * @brief Obtiene la cantidad de memoria libre
 */
size_t pmm_get_free_memory(void) {
    return pmm_free_pages * PAGE_SIZE;
}

/**
 * @brief Inicializa el Physical Memory Manager
 */
static error_t pmm_init(multiboot_info_t* mbi) {
    // Calcular la cantidad total de memoria
    uint32_t mem_size = 0;
    
    if (mbi->flags & MULTIBOOT_INFO_MEMORY) {
        mem_size = (mbi->mem_upper + 1024) * 1024;  // Convertir de KB a bytes
    } else {
        return E_INVAL;
    }

    // Calcular el número de páginas
    pmm_total_pages = mem_size / PAGE_SIZE;
    pmm_free_pages = pmm_total_pages;

    // Calcular el tamaño del bitmap
    pmm_bitmap_size = pmm_total_pages / PMM_BITS_PER_INDEX;
    if (pmm_total_pages % PMM_BITS_PER_INDEX) {
        pmm_bitmap_size++;
    }

    // Colocar el bitmap después del kernel
    pmm_bitmap = (uint32_t*)PAGE_ALIGN((uint32_t)&end);

    // Inicializar el bitmap (todo libre)
    for (uint32_t i = 0; i < pmm_bitmap_size; i++) {
        pmm_bitmap[i] = 0;
    }

    // Marcar como ocupada la memoria hasta el final del bitmap
    uint32_t bitmap_end = (uint32_t)pmm_bitmap + pmm_bitmap_size * sizeof(uint32_t);
    uint32_t kernel_pages = bitmap_end / PAGE_SIZE;
    if (bitmap_end % PAGE_SIZE) {
        kernel_pages++;
    }

    for (uint32_t i = 0; i < kernel_pages; i++) {
        pmm_bitmap_set(i);
        pmm_free_pages--;
    }

    // Marcar regiones reservadas según el mmap de multiboot
    if (mbi->flags & MULTIBOOT_INFO_MEM_MAP) {
        multiboot_mmap_t* mmap = (multiboot_mmap_t*)mbi->mmap_addr;
        uint32_t mmap_end = mbi->mmap_addr + mbi->mmap_length;

        while ((uint32_t)mmap < mmap_end) {
            if (mmap->type != 1) {  // No es RAM disponible
                // Marcar esta región como ocupada
                uint64_t start = mmap->addr;
                uint64_t end = mmap->addr + mmap->len;
                
                for (uint64_t addr = start; addr < end; addr += PAGE_SIZE) {
                    if (addr < (uint64_t)(pmm_total_pages * PAGE_SIZE)) {
                        uint32_t page = addr / PAGE_SIZE;
                        if (!pmm_bitmap_test(page)) {
                            pmm_bitmap_set(page);
                            pmm_free_pages--;
                        }
                    }
                }
            }
            
            mmap = (multiboot_mmap_t*)((uint32_t)mmap + mmap->size + sizeof(mmap->size));
        }
    }

    return E_OK;
}

/**
 * @brief Obtiene una entrada de tabla de páginas
 */
static uint32_t* vmm_get_page_table(uint32_t* page_dir, uint32_t vaddr, bool create) {
    uint32_t pd_index = vaddr >> 22;
    
    if (!(page_dir[pd_index] & PAGE_PRESENT)) {
        if (!create) {
            return NULL;
        }
        
        // Crear nueva tabla de páginas
        uint32_t* page_table = (uint32_t*)pmm_alloc_page();
        if (!page_table) {
            return NULL;
        }
        
        // Limpiar la tabla
        for (int i = 0; i < 1024; i++) {
            page_table[i] = 0;
        }
        
        // Añadir la tabla al directorio
        page_dir[pd_index] = (uint32_t)page_table | PAGE_PRESENT | PAGE_WRITE;
    }
    
    return (uint32_t*)(page_dir[pd_index] & ~0xFFF);
}

/**
 * @brief Mapea una página virtual a una física
 */
error_t vmm_map_page(void* virtual, void* physical, uint32_t flags) {
    uint32_t vaddr = (uint32_t)virtual;
    uint32_t paddr = (uint32_t)physical;
    
    // Obtener o crear la tabla de páginas
    uint32_t* page_table = vmm_get_page_table(kernel_page_directory, vaddr, true);
    if (!page_table) {
        return E_NOMEM;
    }
    
    uint32_t pt_index = (vaddr >> 12) & 0x3FF;
    page_table[pt_index] = paddr | flags;
    
    // Invalidar TLB
    asm volatile("invlpg (%0)" : : "r"(vaddr) : "memory");
    
    return E_OK;
}

/**
 * @brief Desmapea una página virtual
 */
error_t vmm_unmap_page(void* virtual) {
    uint32_t vaddr = (uint32_t)virtual;
    
    uint32_t* page_table = vmm_get_page_table(kernel_page_directory, vaddr, false);
    if (!page_table) {
        return E_NOENT;
    }
    
    uint32_t pt_index = (vaddr >> 12) & 0x3FF;
    page_table[pt_index] = 0;
    
    // Invalidar TLB
    asm volatile("invlpg (%0)" : : "r"(vaddr) : "memory");
    
    return E_OK;
}

/**
 * @brief Obtiene la dirección física de una dirección virtual
 */
void* vmm_get_physical(void* virtual) {
    uint32_t vaddr = (uint32_t)virtual;
    
    uint32_t* page_table = vmm_get_page_table(kernel_page_directory, vaddr, false);
    if (!page_table) {
        return NULL;
    }
    
    uint32_t pt_index = (vaddr >> 12) & 0x3FF;
    if (!(page_table[pt_index] & PAGE_PRESENT)) {
        return NULL;
    }
    
    return (void*)((page_table[pt_index] & ~0xFFF) | (vaddr & 0xFFF));
}

/**
 * @brief Habilita la paginación
 */
static void vmm_enable_paging(uint32_t* page_dir) {
    // Cargar el directorio de páginas en CR3
    asm volatile("mov %0, %%cr3" : : "r"(page_dir));
    
    // Habilitar paginación en CR0
    uint32_t cr0;
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;  // Bit 31 = PG (Paging)
    asm volatile("mov %0, %%cr0" : : "r"(cr0));
}

/**
 * @brief Inicializa el Virtual Memory Manager
 */
static error_t vmm_init(void) {
    // Crear el directorio de páginas del kernel
    kernel_page_directory = (uint32_t*)pmm_alloc_page();
    if (!kernel_page_directory) {
        return E_NOMEM;
    }
    
    // Limpiar el directorio
    for (int i = 0; i < 1024; i++) {
        kernel_page_directory[i] = 0;
    }
    
    // Mapeo de identidad de los primeros 4 MB (donde está el kernel)
    for (uint32_t i = 0; i < 0x400000; i += PAGE_SIZE) {
        vmm_map_page((void*)i, (void*)i, PAGE_PRESENT | PAGE_WRITE);
    }
    
    // Habilitar paginación
    vmm_enable_paging(kernel_page_directory);
    
    // Inicializar el heap del kernel
    kernel_heap_current = KERNEL_HEAP_START;
    kernel_heap_end = KERNEL_HEAP_START + KERNEL_HEAP_SIZE;
    
    return E_OK;
}

/**
 * @brief Asigna memoria del heap del kernel
 */
void* kmalloc(size_t size) {
    if (size == 0) {
        return NULL;
    }
    
    // Alinear a 4 bytes
    size = (size + 3) & ~3;
    
    uint32_t addr = kernel_heap_current;
    kernel_heap_current += size;
    
    if (kernel_heap_current >= kernel_heap_end) {
        return NULL;
    }
    
    // Mapear páginas si es necesario
    uint32_t start_page = addr & ~(PAGE_SIZE - 1);
    uint32_t end_page = (kernel_heap_current - 1) & ~(PAGE_SIZE - 1);
    
    for (uint32_t page = start_page; page <= end_page; page += PAGE_SIZE) {
        if (!vmm_get_physical((void*)page)) {
            void* phys = pmm_alloc_page();
            if (!phys) {
                return NULL;
            }
            vmm_map_page((void*)page, phys, PAGE_PRESENT | PAGE_WRITE);
        }
    }
    
    return (void*)addr;
}

/**
 * @brief Libera memoria del heap del kernel
 * Nota: Esta es una implementación simple que no reutiliza memoria
 */
void kfree(void* ptr) {
    // En esta implementación simple, no hacemos nada
    // Una implementación completa requeriría un heap manager con free lists
    (void)ptr;
}

/**
 * @brief Inicializa el gestor de memoria completo
 */
error_t memory_init(multiboot_info_t* mbi) {
    error_t err;
    
    // Inicializar el Physical Memory Manager
    err = pmm_init(mbi);
    if (err != E_OK) {
        return err;
    }
    
    // Inicializar el Virtual Memory Manager
    err = vmm_init();
    if (err != E_OK) {
        return err;
    }
    
    return E_OK;
}

/**
 * @brief Crea un nuevo espacio de direcciones
 */
void* vmm_create_address_space(void) {
    uint32_t* page_dir = (uint32_t*)pmm_alloc_page();
    if (!page_dir) {
        return NULL;
    }
    
    // Limpiar el directorio
    for (int i = 0; i < 1024; i++) {
        page_dir[i] = 0;
    }
    
    // Copiar las entradas del kernel (mapeo superior)
    for (int i = 768; i < 1024; i++) {
        page_dir[i] = kernel_page_directory[i];
    }
    
    return page_dir;
}

/**
 * @brief Destruye un espacio de direcciones
 */
void vmm_destroy_address_space(void* page_dir) {
    if (!page_dir || page_dir == kernel_page_directory) {
        return;
    }
    
    uint32_t* pd = (uint32_t*)page_dir;
    
    // Liberar todas las tablas de páginas de usuario (0-767)
    for (int i = 0; i < 768; i++) {
        if (pd[i] & PAGE_PRESENT) {
            uint32_t* page_table = (uint32_t*)(pd[i] & ~0xFFF);
            
            // Liberar todas las páginas de esta tabla
            for (int j = 0; j < 1024; j++) {
                if (page_table[j] & PAGE_PRESENT) {
                    pmm_free_page((void*)(page_table[j] & ~0xFFF));
                }
            }
            
            // Liberar la tabla misma
            pmm_free_page(page_table);
        }
    }
    
    // Liberar el directorio
    pmm_free_page(page_dir);
}

/**
 * @brief Cambia al espacio de direcciones especificado
 */
void vmm_switch_address_space(void* page_dir) {
    if (!page_dir) {
        return;
    }
    
    asm volatile("mov %0, %%cr3" : : "r"(page_dir) : "memory");
}
