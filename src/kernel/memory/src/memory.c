/**
 * NeoOS - Memory Manager
 * Inicialización y funciones generales del gestor de memoria
 */

#include "../include/memory.h"
#include "../../core/include/kconfig.h"
#include "../../drivers/include/early_vga.h"

/**
 * Inicializa el Memory Manager completo
 */
int memory_init(multiboot_info_t* mbi, bool kdebug __attribute__((unused)), bool kverbose __attribute__((unused))) {
    int result;

    if (is_kverbose()) {
        vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
        vga_write("\n=== Inicializando Memory Manager ===\n");
    }

    // 1. Inicializar PMM (Physical Memory Manager)
    result = pmm_init(mbi, kdebug, kverbose);
    if (result != E_OK) {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_write("[MM] [FAIL] Error al inicializar PMM\n");
        return result;
    }

    // 2. Inicializar VMM (Virtual Memory Manager)
    result = vmm_init(kdebug, kverbose);
    if (result != E_OK) {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_write("[MM] [FAIL] Error al inicializar VMM: ");
        vga_write(error_to_string(result));
        vga_write("\n");
        return result;
    }

    // 3. Inicializar el Heap del kernel
    result = heap_init(KERNEL_HEAP_START, KERNEL_HEAP_SIZE, kdebug, kverbose);
    if (result != E_OK) {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_write("[MM] [FAIL] Error al inicializar Heap: ");
        vga_write(error_to_string(result));
        vga_write("\n");
        return result;
    }

    if (is_kverbose()) {
        vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
        vga_write("=== Memory Manager inicializado ===\n\n");
    }

    return E_OK;
}

/**
 * Obtiene información sobre el estado de la memoria
 */
void memory_get_info(uint32_t* total_kb, uint32_t* used_kb, uint32_t* free_kb) {
    uint32_t total_pages = pmm_get_total_pages();
    uint32_t free_pages = pmm_get_free_pages();
    uint32_t used_pages = total_pages - free_pages;

    if (total_kb != NULL) {
        *total_kb = (total_pages * PAGE_SIZE) / 1024;
    }

    if (used_kb != NULL) {
        *used_kb = (used_pages * PAGE_SIZE) / 1024;
    }

    if (free_kb != NULL) {
        *free_kb = (free_pages * PAGE_SIZE) / 1024;
    }
}
