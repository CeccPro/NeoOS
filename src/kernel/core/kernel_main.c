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
    
    // TODO: Inicializar Memory Manager
    vga_puts("[...] Initializing Memory Manager\n");
    
    // TODO: Inicializar IDT (Interrupt Descriptor Table)
    vga_puts("[...] Initializing IDT\n");
    
    // TODO: Inicializar GDT (Global Descriptor Table)
    vga_puts("[...] Initializing GDT\n");
    
    // TODO: Inicializar PIC (Programmable Interrupt Controller)
    vga_puts("[...] Initializing PIC\n");
    
    // TODO: Inicializar el timer
    vga_puts("[...] Initializing Timer\n");
    
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