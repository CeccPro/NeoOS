/**
 * @file kernel_main.c
 * @brief Punto de entrada principal del kernel NeoOS
 * 
 * Este archivo contiene la función principal del kernel que se ejecuta
 * después del arranque inicial. Inicializa todos los subsistemas del kernel.
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

#include "../include/kernel.h"
#include "../include/vga.h"
#include "../include/multiboot.h"
#include "../include/config.h"
#include "../include/gdt.h"
#include "../include/idt.h"
#include "../include/memory.h"
#include "../include/timer.h"

/**
 * @brief Punto de entrada principal del kernel
 * 
 * @param magic Número mágico de multiboot para verificar que el bootloader es compatible
 * @param mbi Puntero a la estructura de información de multiboot
 */
void kernel_main(uint32_t magic, multiboot_info_t* mbi) {
    // Inicializar el terminal VGA
    vga_init();
    
    vga_puts("NeoOS Kernel" KERNEL_VERSION " (" BUILD_DATE ")\n");
    vga_puts("===================\n\n");
    
    // Verificar que el bootloader es compatible con multiboot
    if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        vga_puts("ERROR: Invalid multiboot magic number!\n");
        vga_puts("Expected: 0x2BADB002\n");
        goto halt;
    }

    if (mbi->flags & (1 << 2)) { // Bit 2 = cmdline presente
        const char* cmdline = (const char*) mbi->cmdline;
        vga_puts("[CMDLINE] ");
        vga_puts(cmdline);
        vga_puts("\n");
    }
    
    vga_puts("[OK] Multiboot header verified\n");
    
    // Inicializar GDT (Global Descriptor Table)
    vga_puts("[...] Initializing GDT\n");
    gdt_init();
    vga_puts("[OK] GDT initialized\n");
    
    // Inicializar IDT (Interrupt Descriptor Table)
    vga_puts("[...] Initializing IDT\n");
    idt_init();
    vga_puts("[OK] IDT initialized\n");
    
    // Inicializar Memory Manager
    vga_puts("[...] Initializing Memory Manager\n");
    error_t mem_err = memory_init(mbi);
    if (mem_err != E_OK) {
        vga_puts("[FAIL] Memory initialization failed!\n");
        goto halt;
    }
    
    // Mostrar información de memoria
    size_t total_mem = pmm_get_total_memory();
    size_t free_mem = pmm_get_free_memory();
    vga_puts("[OK] Memory Manager initialized - Total: ");
    vga_put_dec(total_mem / 1024 / 1024);
    vga_puts(" MB, Free: ");
    vga_put_dec(free_mem / 1024 / 1024);
    vga_puts(" MB\n");
    
    // Inicializar el timer
    vga_puts("[...] Initializing Timer\n");
    timer_init(TIMER_FREQUENCY);
    vga_puts("[OK] Timer initialized at ");
    vga_put_dec(TIMER_FREQUENCY);
    vga_puts(" Hz\n");
    
    // Probar el timer con una pequeña espera
    vga_puts("[TEST] Waiting 1 second...\n");
    timer_sleep(1000);
    vga_puts("[OK] Timer test passed - Uptime: ");
    vga_put_dec(timer_get_seconds());
    vga_puts(" seconds\n");
    
    // TODO: Inicializar el teclado
    vga_puts("[...] Initializing Keyboard\n");
    
    // TODO: Inicializar IPC Manager
    vga_puts("[...] Initializing IPC Manager\n");
    
    // TODO: Inicializar Process Scheduler
    vga_puts("[...] Initializing Process Scheduler\n");
    
    // TODO: Inicializar Module Manager
    vga_puts("[...] Initializing Module Manager\n");
    
    // TODO: Cargar InitImage desde multiboot
    vga_puts("[...] Loading InitImage\n");
    
    vga_puts("\n[OK] Kernel initialization complete!\n");
    vga_puts("System ready.\n");
    
halt:
    vga_puts("\nSystem halted.\n");
    
    // Loop infinito
    while(1) {
        asm volatile("hlt");
    }
}

// This is for u, Maia <3
// - CeccPro