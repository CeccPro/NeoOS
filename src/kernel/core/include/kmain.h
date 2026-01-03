/*
 * Kernel Main Header File
 * Definiciones para la función kernel_main
 */

#ifndef KMAIN_H
#define KMAIN_H

#include "../../lib/include/types.h"
#include "../../lib/include/string.h"
#include "../../lib/include/multiboot.h"
#include "../../drivers/include/vga.h"

// Definiciones de configuración
#define KERNEL_VERSION "0.1.0"
#define BUILD_DATE __DATE__ " " __TIME__

// Entrada principal del kernel
void kernel_main(uint32_t magic, multiboot_info_t* mbi);

#endif // KMAIN_H