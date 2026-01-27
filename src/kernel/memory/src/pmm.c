/**
 * NeoOS - Physical Memory Manager (PMM)
 * Gestión de páginas físicas mediante bitmap
 * 
 * El PMM mantiene un bitmap donde cada bit representa una página de 4KB:
 *   0 = página libre
 *   1 = página ocupada
 */

#include "../include/memory.h"
#include "../../core/include/kconfig.h"
#include "../../drivers/include/early_vga.h"
#include "../../lib/include/string.h"

// Bitmap de páginas físicas
static uint32_t* pmm_bitmap = NULL;
static uint32_t pmm_bitmap_size = 0;  // Tamaño del bitmap en DWORDs (uint32_t)
static uint32_t pmm_total_pages = 0;
static uint32_t pmm_free_pages = 0;
static uint32_t pmm_memory_size = 0;  // Tamaño total de memoria en bytes

// Dirección donde termina el kernel (se actualizará durante la inicialización)
extern uint32_t kernel_end;

/*
 * Funciones auxiliares para manipular el bitmap
 */

/**
 * Establece un bit en el bitmap (marca página como ocupada)
 */
static inline void pmm_bitmap_set(uint32_t page_num) {
    uint32_t index = page_num / 32;
    uint32_t bit = page_num % 32;
    pmm_bitmap[index] |= (1 << bit);
}

/**
 * Limpia un bit en el bitmap (marca página como libre)
 */
static inline void pmm_bitmap_clear(uint32_t page_num) {
    uint32_t index = page_num / 32;
    uint32_t bit = page_num % 32;
    pmm_bitmap[index] &= ~(1 << bit);
}

/**
 * Verifica si un bit está establecido en el bitmap
 */
static inline bool pmm_bitmap_test(uint32_t page_num) {
    uint32_t index = page_num / 32;
    uint32_t bit = page_num % 32;
    return (pmm_bitmap[index] & (1 << bit)) != 0;
}

/**
 * Encuentra la primera página libre en el bitmap
 * @return Número de página, o (uint32_t)-1 si no hay páginas libres
 */
static uint32_t pmm_find_free_page(void) {
    for (uint32_t i = 0; i < pmm_bitmap_size; i++) {
        if (pmm_bitmap[i] != 0xFFFFFFFF) {
            // Hay al menos un bit libre en este DWORD
            for (uint32_t bit = 0; bit < 32; bit++) {
                uint32_t page_num = i * 32 + bit;
                // Verificar que la página esté dentro del rango válido
                if (page_num >= pmm_total_pages) {
                    return (uint32_t)-1;
                }
                if (!(pmm_bitmap[i] & (1 << bit))) {
                    return page_num;
                }
            }
        }
    }
    return (uint32_t)-1;  // No hay páginas libres
}

/**
 * Inicializa el Physical Memory Manager
 */
int pmm_init(multiboot_info_t* mbi, __attribute__((unused)) bool kdebug, __attribute__((unused)) bool kverbose) {
    if (is_kverbose()) {
        vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
        vga_write("[PMM] Inicializando Physical Memory Manager...\n");
    }

    // Verificar que tenemos información de memoria
    if (!(mbi->flags & MULTIBOOT_INFO_MEMORY)) {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_write("[PMM] [FAIL] No hay informacion de memoria de Multiboot\n");
        return E_INVAL;
    }

    // Calcular memoria total (inferior + superior)
    // mem_lower está en KB y va desde 0 hasta 640KB (memoria convencional)
    // mem_upper está en KB y empieza desde 1MB
    uint64_t total_memory = ((uint64_t)(mbi->mem_lower) + (uint64_t)(mbi->mem_upper) + 1024ULL) * 1024ULL;  // Convertir a bytes
    // Limitar a 4GB para sistemas de 32 bits
    if (total_memory > 0xFFFFFFFFULL) {
        pmm_memory_size = 0xFFFFFFFFU;
    } else {
        pmm_memory_size = (uint32_t)total_memory;
    }
    pmm_total_pages = (uint32_t)(pmm_memory_size / PAGE_SIZE);

    if (is_kdebug()) {
        vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
        vga_write("[PMM] Memoria total: ");
        vga_write_dec(pmm_memory_size / (1024 * 1024));
        vga_write(" MB (");
        vga_write_dec(pmm_total_pages);
        vga_write(" paginas)\n");
    }

    // Calcular el tamaño del bitmap (1 bit por página)
    // Necesitamos pmm_total_pages bits, o pmm_total_pages/8 bytes
    // Redondeamos hacia arriba y alineamos a DWORD (4 bytes)
    uint32_t bitmap_bytes = (pmm_total_pages + 7) / 8;
    pmm_bitmap_size = (bitmap_bytes + 3) / 4;  // Tamaño en DWORDs

    // Colocar el bitmap después del kernel
    // El símbolo `kernel_end` declarado en C contiene la dirección final
    // del kernel; debemos usar su valor, no la dirección de la variable.
    pmm_bitmap = (uint32_t*)kernel_end;

    if (is_kdebug()) {
        vga_write("[PMM] Bitmap ubicado en: ");
        vga_write_hex((uint32_t)pmm_bitmap);
        vga_write(" (");
        vga_write_dec(pmm_bitmap_size * 4);
        vga_write(" bytes)\n");
    }

    // Inicializar el bitmap: todas las páginas ocupadas inicialmente
    // Luego marcaremos las libres según el mapa de memoria
    for (uint32_t i = 0; i < pmm_bitmap_size; i++) {
        pmm_bitmap[i] = 0xFFFFFFFF;
    }
    pmm_free_pages = 0;

    // Marcar como ocupadas las páginas del kernel y el bitmap ANTES de parsear
    // Esto asegura que no se marquen como libres accidentalmente
    uint32_t kernel_start_page = KERNEL_START / PAGE_SIZE;
    uint32_t bitmap_end = (uint32_t)pmm_bitmap + pmm_bitmap_size * 4;
    uint32_t kernel_end_page = (bitmap_end + PAGE_SIZE - 1) / PAGE_SIZE;

    if (is_kdebug()) {
        vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
        vga_write("[PMM] Reservando paginas para el kernel y bitmap...\n");
        vga_write("[PMM] Kernel ocupa desde pagina ");
        vga_write_dec(kernel_start_page);
        vga_write(" hasta pagina ");
        vga_write_dec(kernel_end_page);
        vga_write("\n");
    }

    // Parsear el mapa de memoria de Multiboot para encontrar regiones libres
    if (mbi->flags & MULTIBOOT_INFO_MEM_MAP) {
        multiboot_mmap_entry_t* mmap = (multiboot_mmap_entry_t*)mbi->mmap_addr;
        multiboot_mmap_entry_t* mmap_end = (multiboot_mmap_entry_t*)(mbi->mmap_addr + mbi->mmap_length);

        if (is_kverbose()) {
            vga_write("[PMM] Parseando mapa de memoria de Multiboot...\n");
        }
        
        uint32_t entry_count = 0;
        while (mmap < mmap_end) {
            entry_count++;
            if (is_kdebug()) {
                vga_write("[PMM] Entrada ");
                vga_write_dec(entry_count);
                vga_write(": addr=");
                vga_write_hex((uint32_t)mmap->addr);
                vga_write(" len=");
                vga_write_hex((uint32_t)mmap->len);
                vga_write(" type=");
                vga_write_dec(mmap->type);
                vga_write("\n");
            }
            
            // Type 1 = memoria disponible
            if (mmap->type == MULTIBOOT_MEMORY_AVAILABLE) {
                uint64_t region_start = mmap->addr;
                uint64_t region_end = mmap->addr + mmap->len;

                // Solo procesar regiones en los primeros 4GB (limitación de 32 bits)
                if (region_start < 0x100000000ULL) {
                    if (region_end > 0x100000000ULL) {
                        region_end = 0x100000000ULL;
                    }

                    // Calcular páginas de inicio y fin
                    uint32_t start_page = (region_start + PAGE_SIZE - 1) / PAGE_SIZE;  // Redondear hacia arriba
                    uint32_t end_page = region_end / PAGE_SIZE;  // Redondear hacia abajo

                    if (is_kdebug()) {
                        vga_write("[PMM]   -> Marcando paginas ");
                        vga_write_dec(start_page);
                        vga_write(" a ");
                        vga_write_dec(end_page);
                        vga_write(" como libres\n");
                    }

                    // Marcar páginas como libres, excepto las del kernel
                    uint32_t freed_count = 0;
                    for (uint32_t page = start_page; page < end_page && page < pmm_total_pages; page++) {
                        // No marcar como libre si está en el rango del kernel
                        if (page < kernel_start_page || page >= kernel_end_page) {
                            pmm_bitmap_clear(page);
                            pmm_free_pages++;
                            freed_count++;
                        }
                    }
                    if (is_kdebug()) {
                        vga_write("[PMM]   -> Liberadas ");
                        vga_write_dec(freed_count);
                        vga_write(" paginas\n");
                    }
                }
            }

            // Avanzar al siguiente entry (el tamaño incluye el campo size)
            mmap = (multiboot_mmap_entry_t*)((uint32_t)mmap + mmap->size + sizeof(mmap->size));
        }
    } else {
        // Si no hay mapa de memoria detallado, usar la información básica
        // Marcar como libre la memoria superior (desde 1MB), excepto el kernel
        uint32_t start_page = 0x100000 / PAGE_SIZE;  // 1MB
        uint32_t end_page = pmm_total_pages;

        for (uint32_t page = start_page; page < end_page; page++) {
            // No marcar como libre si está en el rango del kernel
            if (page < kernel_start_page || page >= kernel_end_page) {
                pmm_bitmap_clear(page);
                pmm_free_pages++;
            }
        }
    }

    if (is_kdebug()) {
        vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
        vga_write("[PMM] [OK] Inicializacion completada\n");
        vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
        vga_write("[PMM] Paginas libres: ");
        vga_write_dec(pmm_free_pages);
        vga_write(" (");
        vga_write_dec((pmm_free_pages * PAGE_SIZE) / (1024 * 1024));
        vga_write(" MB)\n");
    }

    return E_OK;
}

/**
 * Asigna una página física
 */
uint32_t pmm_alloc_page(void) {
    if (pmm_free_pages == 0) {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_write("[PMM] No hay paginas libres\n");
        return 0;  // No hay memoria disponible
    }

    uint32_t page_num = pmm_find_free_page();
    if (page_num == (uint32_t)-1) {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_write("[PMM] pmm_find_free_page retorno -1\n");
        return 0;  // No se encontró página libre (no debería pasar)
    }

    pmm_bitmap_set(page_num);
    pmm_free_pages--;

    return page_num * PAGE_SIZE;  // Retornar dirección física
}

/**
 * Libera una página física
 */
void pmm_free_page(uint32_t page) {
    uint32_t page_num = page / PAGE_SIZE;

    if (page_num >= pmm_total_pages) {
        return;  // Página inválida
    }

    // Proteger el rango del kernel y bitmap
    uint32_t kernel_start_page = KERNEL_START / PAGE_SIZE;
    extern uint32_t kernel_end;
    uint32_t bitmap_end = (uint32_t)pmm_bitmap + pmm_bitmap_size * 4;
    uint32_t protected_end_page = (bitmap_end + PAGE_SIZE - 1) / PAGE_SIZE;
    
    if (page_num >= kernel_start_page && page_num < protected_end_page) {
        if (is_kdebug()) {
            vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
            vga_write("[PMM] [WARN] Intento de liberar pagina protegida: ");
            vga_write_dec(page_num);
            vga_write("\n");
        }
        return;  // Página en rango protegido
    }

    if (!pmm_bitmap_test(page_num)) {
        if (is_kdebug()) {
            vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
            vga_write("[PMM] [WARN] Double free detectado en pagina: ");
            vga_write_dec(page_num);
            vga_write("\n");
        }
        return;  // Página ya está libre (double free)
    }

    pmm_bitmap_clear(page_num);
    pmm_free_pages++;
}

/**
 * Obtiene la cantidad de páginas libres
 */
uint32_t pmm_get_free_pages(void) {
    return pmm_free_pages;
}

/**
 * Obtiene la cantidad total de páginas
 */
uint32_t pmm_get_total_pages(void) {
    return pmm_total_pages;
}
