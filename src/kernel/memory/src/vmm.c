/**
 * NeoOS - Virtual Memory Manager (VMM)
 * Gestión de memoria virtual mediante paginación
 * 
 * Implementa paginación de 2 niveles (x86 32-bit):
 * - Page Directory: 1024 entradas (cada una mapea 4MB)
 * - Page Tables: 1024 entradas cada una (cada una mapea 4KB)
 * 
 * Dirección virtual de 32 bits:
 * [31-22: Dir Index | 21-12: Table Index | 11-0: Offset]
 */

#include "../include/memory.h"
#include "../../core/include/kconfig.h"
#include "../../drivers/include/vga.h"
#include "../../lib/include/string.h"

// Directorio de páginas del kernel (identity mapping inicial)
// Usamos memoria estática para evitar problemas de acceso antes de habilitar paginación
static page_directory_t kernel_directory_data __attribute__((aligned(PAGE_SIZE)));
static page_directory_t* kernel_directory = NULL;
static page_directory_t* current_directory = NULL;

// Dirección física del directorio del kernel (para CR3)
static uint32_t kernel_directory_phys = 0;

// Tablas de páginas estáticas para los primeros 128MB (necesitamos 32 tablas)
// Cada tabla mapea 4MB, entonces 32 tablas = 128MB
static page_table_t kernel_tables[32] __attribute__((aligned(PAGE_SIZE)));

/*
 * Funciones auxiliares
 */

/**
 * Extrae el índice del directorio de una dirección virtual
 */
static inline uint32_t vmm_get_dir_index(uint32_t virt) {
    return virt >> 22;
}

/**
 * Extrae el índice de la tabla de una dirección virtual
 */
static inline uint32_t vmm_get_table_index(uint32_t virt) {
    return (virt >> 12) & 0x3FF;
}

/**
 * Carga un directorio de páginas en CR3
 */
static void vmm_load_directory(uint32_t phys_addr) {
    __asm__ volatile(
        "mov %0, %%cr3"
        :
        : "r"(phys_addr)
        : "memory"
    );
}

/**
 * Habilita la paginación
 */
static void vmm_enable_paging(void) {
    uint32_t cr0;
    __asm__ volatile(
        "mov %%cr0, %0\n"
        "or $0x80000000, %0\n"  // Establecer bit 31 (PG)
        "mov %0, %%cr0"
        : "=r"(cr0)
        :
        : "memory"
    );
}

/**
 * Invalida la entrada TLB para una dirección virtual
 */
static void vmm_invalidate_page(uint32_t virt) {
    __asm__ volatile(
        "invlpg (%0)"
        :
        : "r"(virt)
        : "memory"
    );
}

/**
 * Inicializa el Virtual Memory Manager
 */
int vmm_init(bool kdebug __attribute__((unused)), bool kverbose __attribute__((unused))) {
    if (is_kverbose()) {
        vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
        vga_write("Inicializando Virtual Memory Manager ===\n");
    }

    // Usar el directorio estático (está en la sección .bss del kernel)
    kernel_directory = &kernel_directory_data;
    kernel_directory_phys = (uint32_t)kernel_directory;

    if (is_kdebug()) {
        vga_write("[VMM] Directorio del kernel en: ");
        vga_write_hex(kernel_directory_phys);
        vga_write("\n");
    }

    // Limpiar el directorio
    memset(kernel_directory, 0, sizeof(page_directory_t));

    // Limpiar las tablas estáticas
    memset(kernel_tables, 0, sizeof(kernel_tables));

    if (is_kverbose()) {
        vga_write("[VMM] Creando identity mapping para los primeros 128MB...\n");
    }

    // Crear identity mapping para los primeros 128MB usando tablas estáticas
    // Cada tabla mapea 4MB, así que necesitamos 32 tablas para 128MB
    for (int table_idx = 0; table_idx < 32; table_idx++) {
        // Configurar la entrada del directorio para esta tabla
        uint32_t table_phys = (uint32_t)&kernel_tables[table_idx];
        kernel_directory->entries[table_idx] = table_phys | PAGE_PRESENT | PAGE_WRITE;

        // Llenar la tabla con identity mapping
        for (int page_idx = 0; page_idx < 1024; page_idx++) {
            uint32_t phys_addr = (table_idx * 1024 + page_idx) * PAGE_SIZE;
            kernel_tables[table_idx].entries[page_idx] = phys_addr | PAGE_PRESENT | PAGE_WRITE;
        }
    }

    if (is_kverbose()) {
        vga_write("[VMM] Identity mapping completado (0MB - 128MB)\n");
    }
    
    // También pre-mapear el rango del heap del kernel
    // El heap está en KERNEL_HEAP_START (4MB) y tiene KERNEL_HEAP_SIZE (4MB)
    // Esto ya está incluido en los primeros 128MB, así que no hace falta más

    // Activar el directorio de páginas del kernel
    current_directory = kernel_directory;
    vmm_load_directory(kernel_directory_phys);

    // Habilitar paginación
    vmm_enable_paging();

    if (is_kdebug()) {
        vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
        vga_write("[VMM] [OK] Paginacion habilitada\n");
    }

    return E_OK;
}

/**
 * Mapea una página virtual a una página física
 */
int vmm_map_page(page_directory_t* page_dir, uint32_t virt, uint32_t phys, uint32_t flags) {
    uint32_t dir_index = vmm_get_dir_index(virt);
    uint32_t table_index = vmm_get_table_index(virt);

    // Verificar si la tabla de páginas existe
    if (!(page_dir->entries[dir_index] & PAGE_PRESENT)) {
        // Necesitamos crear una nueva tabla de páginas
        uint32_t table_phys = pmm_alloc_page();
        if (table_phys == 0) {
            return E_NOMEM;
        }

        // Limpiar la tabla
        page_table_t* table = (page_table_t*)table_phys;
        memset(table, 0, sizeof(page_table_t));

        // Agregar la tabla al directorio
        page_dir->entries[dir_index] = table_phys | PAGE_PRESENT | PAGE_WRITE | (flags & PAGE_USER);
    }

    // Obtener la tabla de páginas
    uint32_t table_phys = page_dir->entries[dir_index] & PAGE_ALIGN_MASK;
    page_table_t* table = (page_table_t*)table_phys;

    // Mapear la página
    table->entries[table_index] = (phys & PAGE_ALIGN_MASK) | (flags & 0xFFF) | PAGE_PRESENT;

    // Invalidar TLB si este es el directorio actual
    if (page_dir == current_directory) {
        vmm_invalidate_page(virt);
    }

    return E_OK;
}

/**
 * Desmapea una página virtual
 */
void vmm_unmap_page(page_directory_t* page_dir, uint32_t virt) {
    uint32_t dir_index = vmm_get_dir_index(virt);
    uint32_t table_index = vmm_get_table_index(virt);

    // Verificar si la tabla existe
    if (!(page_dir->entries[dir_index] & PAGE_PRESENT)) {
        return;  // La página ya no está mapeada
    }

    // Obtener la tabla de páginas
    uint32_t table_phys = page_dir->entries[dir_index] & PAGE_ALIGN_MASK;
    page_table_t* table = (page_table_t*)table_phys;

    // Desmarcar la página
    table->entries[table_index] = 0;

    // Invalidar TLB si este es el directorio actual
    if (page_dir == current_directory) {
        vmm_invalidate_page(virt);
    }
}

/**
 * Obtiene la dirección física de una dirección virtual
 */
uint32_t vmm_get_physical(page_directory_t* page_dir, uint32_t virt) {
    uint32_t dir_index = vmm_get_dir_index(virt);
    uint32_t table_index = vmm_get_table_index(virt);
    uint32_t offset = virt & PAGE_OFFSET_MASK;

    // Verificar si la tabla existe
    if (!(page_dir->entries[dir_index] & PAGE_PRESENT)) {
        return 0;  // No mapeado
    }

    // Obtener la tabla de páginas
    uint32_t table_phys = page_dir->entries[dir_index] & PAGE_ALIGN_MASK;
    page_table_t* table = (page_table_t*)table_phys;

    // Verificar si la página está presente
    if (!(table->entries[table_index] & PAGE_PRESENT)) {
        return 0;  // No mapeado
    }

    // Retornar dirección física
    uint32_t page_phys = table->entries[table_index] & PAGE_ALIGN_MASK;
    return page_phys + offset;
}

/**
 * Cambia el directorio de páginas activo
 */
void vmm_switch_directory(page_directory_t* page_dir) {
    current_directory = page_dir;
    
    // Obtener la dirección física del directorio
    // Por ahora asumimos identity mapping
    uint32_t phys = (uint32_t)page_dir;
    vmm_load_directory(phys);
}

/**
 * Obtiene el directorio de páginas del kernel
 */
page_directory_t* vmm_get_kernel_directory(void) {
    return kernel_directory;
}
