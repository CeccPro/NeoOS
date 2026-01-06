/**
 * NeoOS - Kernel Main
 * Entry point principal del kernel
 */

#include "../../core/include/kmain.h"
#include "../../core/include/kconfig.h"
#include "../../core/include/gdt.h"
#include "../../core/include/idt.h"
#include "../../core/include/interrupts.h"
#include "../../core/include/timer.h"
#include "../../core/include/scheduler.h"
#include "../../memory/include/memory.h"


// Test (Quitar después)
void test_process01(void) {
    for (int i = 0; i < 10; i++) {
        vga_write("[TEST01] Running iteration ");
        vga_write_dec(i);
        vga_write("\n");
        timer_wait_ms(100);
    }
}

void test_process02(void) {
    for (int i = 0; i < 10; i++) {
        vga_write("[TEST02] Running iteration ");
        vga_write_dec(i);
        vga_write("\n");
        timer_wait_ms(100);
    }
}

void test_process03(void) {
    for (int i = 0; i < 10; i++) {
        vga_write("[TEST03] Running iteration ");
        vga_write_dec(i);
        vga_write("\n");
        timer_wait_ms(100);
    }
}

// Símbolo proporcionado por el linker script
extern uint32_t __kernel_end;
uint32_t kernel_end = (uint32_t)&__kernel_end;

/**
 * Kernel Main - Entry point del kernel en C
 * @param magic: Magic number de Multiboot (debe ser 0x2BADB002)
 * @param mbi: Puntero a la estructura multiboot_info
 */
void kernel_main(uint32_t magic, multiboot_info_t* mbi) {
    // Inicializar el driver VGA
    vga_init();
    
    bool kdebug = false;
    bool kverbose = false;
    bool ksubsystems = true;

    // Parsear CMDLINE
    if (mbi->flags & MULTIBOOT_INFO_CMDLINE) {
        if (strstr((const char*)mbi->cmdline, "--debug")) {
            kdebug = true;
        }

        if (strstr((const char*)mbi->cmdline, "--verbose")) {
            kverbose = true;
        }

        // Esto no debería usarse en producción, solo para pruebas
        // (Y también para poder apreciar el Banner xd)
        if (strstr((const char*)mbi->cmdline, "--no-subsystems")) {
            ksubsystems = false;
        }
    }

    // Inicializar configuración global del kernel
    kconfig_init(kdebug, kverbose);

    // Verificar si el bootloader es compatible con Multiboot
    if (magic != MULTIBOOT_MAGIC) {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_write("[FAIL]\n");
        vga_write("Error: Bootloader no compatible con Multiboot!\n");
        vga_write("Magic esperado: ");
        vga_write_hex(MULTIBOOT_MAGIC);
        vga_write("\n");
        vga_write("Magic recibido: ");
        vga_write_hex(magic);
        vga_write("\n");

        // Detener el kernel
        while(1) {
            __asm__ volatile("hlt");
        }
    }

    // Mostrar banner de bienvenida
    if (kverbose) {
        vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
        vga_write("================================================\n");
        vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        vga_write("          _   _             ___  ____  \n");
        vga_write("         | \\ | | ___  ___  / _ \\/ ___| \n");
        vga_write("         |  \\| |/ _ \\/ _ \\| | \\___ \\_\n");
        vga_write("         | |\\  |  __/ (_) | |_| ___) |\n");
        vga_write("         |_| \\_|\\___|\\___/ \\___/____/ \n");
        vga_write("              NeoOS Kernel v");
        vga_write(KERNEL_VERSION);
        vga_write("\n         (Build: ");
        vga_write(BUILD_DATE);
        vga_write(")\n");
        vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
        vga_write("================================================\n\n");
    }

    if (kdebug) {
        vga_set_color(VGA_COLOR_CYAN, VGA_COLOR_BLACK);
        vga_write("[Modo debug activado]\n\n");
    }

    // Mostrar información de memoria
    if (mbi->flags & MULTIBOOT_INFO_MEMORY && kdebug == true && kverbose == true) {
        vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
        vga_write("Memoria inferior: ");
        vga_write_dec(mbi->mem_lower);
        vga_write("KB\n");
        vga_write("Memoria superior: ");
        vga_write_dec(mbi->mem_upper);
        vga_write("KB\n");
    }

    // Mostrar cantidad de memoria total
    if (mbi->flags & MULTIBOOT_INFO_MEMORY && kverbose == true) {
        uint32_t total_mem_kb = mbi->mem_lower + mbi->mem_upper;
        vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
        vga_write("Memoria total: ");
        uint32_t total_mem_mb = total_mem_kb / 1024;
        // Si la memoria es mayor a 1024 MB, mostrar en GB
        if (total_mem_mb > 1024) {
            // Calcular la memoria en GB
            int total_mem_gb = total_mem_mb / 1024;

            vga_write_dec(total_mem_gb);
            vga_write("GB\n");
            } else {
            vga_write_dec(total_mem_mb);
            vga_write("MB\n");
        }
    }

    // Mostrar mensaje de inicialización completada
    if (kverbose) {vga_write("\n");}
    vga_set_color(VGA_COLOR_LIGHT_BROWN, VGA_COLOR_BLACK);
    vga_write("Inicializacion del kernel completada.\n");

    if (!ksubsystems) {
        vga_set_color(VGA_COLOR_LIGHT_BROWN, VGA_COLOR_BLACK);
        vga_write("Modo sin subsistemas activado. El kernel se detendra aqui.\n");
        while (1) {
            __asm__ volatile("hlt");
        }
    }

    vga_write("\n== Iniciando subsistemas del kernel ==\n");

    // Inicializar el Memory Manager
    int mm_result = memory_init(mbi, kdebug, kverbose);
    if (mm_result != E_OK) {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_write("[FAIL] Error al inicializar el Memory Manager\n");
        vga_write("Codigo de error: ");
        vga_write(error_to_string(mm_result));
        vga_write("\n");
        
        // Detener el kernel
        while(1) {
            __asm__ volatile("hlt");
        }
    }

    if (kverbose) {
        vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
        vga_write("== Inicializando sistema de interrupciones ==\n");
    }

    // Inicializar GDT (Global Descriptor Table)
    if (kverbose) {
        vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
        vga_write("[GDT] Inicializando GDT...\n");
    }
    gdt_init();
    if (kverbose) {
        vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
        vga_write("[GDT] GDT inicializada\n");
    }

    // Inicializar IDT (Interrupt Descriptor Table)
    if (kverbose) {
        vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
        vga_write("[IDT] Inicializando IDT...\n");
    }
    idt_init();
    if (kverbose) {
        vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
        vga_write("[IDT] IDT inicializada\n");
    }

    // Inicializar el sistema de interrupciones (ISR, IRQ, PIC)
    interrupts_init(kverbose);

    if (kverbose) {
        vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
        vga_write("== Sistema de interrupciones inicializado ==\n\n");
    }

    // Inicializar el PIT (Programmable Interval Timer)
    if (kverbose) {
        vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
        vga_write("== Inicializando PIT (Timer) ==\n");
    }
    timer_init(TIMER_DEFAULT_FREQUENCY, kverbose);
    if (kverbose) {
        vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
        vga_write("== PIT inicializado ==\n\n");
    }

    // Inicializar el Scheduler
    if (kverbose) {
        vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
        vga_write("== Inicializando Scheduler ==\n");
    }
    scheduler_init(kverbose);
    if (kverbose) {
        vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
        vga_write("== Scheduler inicializado ==\n");
    }

    // Crear procesos de prueba (Quitar después)
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_write("\nCreando procesos de prueba...\n");
    scheduler_create_process("test01", test_process01, PROCESS_PRIORITY_NORMAL);
    scheduler_create_process("test02", test_process02, PROCESS_PRIORITY_NORMAL);
    scheduler_create_process("test03", test_process03, PROCESS_PRIORITY_NORMAL);

    // TODO: Inicializar subsistemas adicionales del kernel:
    // - IPC
    // - Sistema de archivos
    // - Module Manager

    vga_write("Subsistemas del kernel iniciados correctamente\n");

    // Cargar la partición de NeoOS (Por ahora solo verificar el MAGIC al inicio de la partición [Primeros 512B])
    if (kverbose) {
        vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
        vga_write("Cargando particion de NeoOS...\n");
        // TODO: Implementar carga de partición NeoOS
    }
    
    // Loop infinito - el kernel no debe terminar
    while (1) {
        __asm__ volatile("hlt");
    }
}