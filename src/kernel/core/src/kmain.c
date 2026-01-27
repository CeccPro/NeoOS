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
#include "../../core/include/ipc.h"
#include "../../core/include/syscall.h"
#include "../../core/include/module.h"
#include "../../core/include/device.h"
#include "../../core/include/driver_manager.h"
#include "../../memory/include/memory.h"
#include "../../drivers/include/early_vga.h"
#include "../../drivers/include/vga_driver.h"
#include "../../lib/include/string.h"
#include "../../modules/include/ramdisk.h"
#include "../../modules/include/early_neofs.h"   

// Funciones wrapper para VGA via PMIC
void vga_write_pmic(const char* str) {
    vga_message_t msg = { .type = VGA_MSG_WRITE };
    strncpy(msg.data, str, sizeof(msg.data) - 1);
    module_send_by_name("vga", &msg, sizeof(msg));
}

void vga_set_color_pmic(enum vga_color fg, enum vga_color bg) {
    vga_color_message_t msg = { .type = VGA_MSG_SET_COLOR, .fg = fg, .bg = bg };
    module_send_by_name("vga", &msg, sizeof(msg));
}

void vga_clear_pmic() {
    uint32_t msg = VGA_MSG_CLEAR;
    module_send_by_name("vga", &msg, sizeof(msg));
}

void vga_write_hex_pmic(uint32_t value) {
    vga_number_message_t msg = { .type = VGA_MSG_WRITE_HEX, .value = value };
    module_send_by_name("vga", &msg, sizeof(msg));
}

void vga_write_dec_pmic(uint32_t value) {
    vga_number_message_t msg = { .type = VGA_MSG_WRITE_DEC, .value = value };
    module_send_by_name("vga", &msg, sizeof(msg));
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
        vga_set_color_pmic(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
        vga_write_pmic("================================================\n");
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
    if (mbi->flags & MULTIBOOT_INFO_MEMORY) {
        uint32_t total_mem_kb = mbi->mem_lower + mbi->mem_upper;
        if (kverbose == true) {
            vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
            vga_write("Memoria total: ");
        }
        uint32_t total_mem_mb = total_mem_kb / 1024;

        // Si la memoria es mayor a 1024 MB, mostrar en GB
        if (kverbose == true) {
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

        if (total_mem_mb < MINIMAL_MEMORY_MB) {
            // Cambiar si se cambia el requisito mínimo de memoria
            panic("Memoria insuficiente para ejecutar el kernel. Se requieren al menos 6MB de RAM.");
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
            __asm__ volatile("cli"); // Deshabilitar interrupciones
            __asm__ volatile("hlt");
        }
    }

    if (kverbose) {
        vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
        vga_write("\n== Iniciando subsistemas del kernel ==\n\n");
    }

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
        vga_write("== Scheduler inicializado ==\n\n");
    }

    // Inicializar IPC
    if (kverbose) {
        vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
        vga_write("== Inicializando IPC ==\n");
    }
    int ipc_result = ipc_init(kverbose);
    if (ipc_result != E_OK) {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_write("[FAIL] Error al inicializar IPC\n");
        while(1) {
            __asm__ volatile("hlt");
        }
    }
    if (kverbose) {
        vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
        vga_write("== IPC inicializado ==\n");
    }

    // Inicializar Syscalls
    if (kverbose) {
        vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
        vga_write("\n== Inicializando Syscalls ==\n");
    }
    syscall_init(kverbose);
    if (kverbose) {
        vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
        vga_write("== Syscalls inicializados ==\n");
    }

    // Inicializar Module Manager
    if (kverbose) {
        vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
        vga_write("\n== Inicializando Module Manager ==\n");
    }
    int module_result = module_manager_init(kverbose);
    if (module_result != E_OK) {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_write("[FAIL] Error al inicializar Module Manager\n");
        while(1) {
            __asm__ volatile("hlt");
        }
    }
    if (kverbose) {
        vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
        vga_write("== Module Manager inicializado ==\n");
    }

    // Inicializar Driver Manager
    if (kverbose) {
        vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
        vga_write("\n== Inicializando Driver Manager ==\n");
    }
    int driver_result = driver_manager_init(kverbose);
    if (driver_result != E_OK) {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_write("[FAIL] Error al inicializar Driver Manager\n");
        while(1) {
            __asm__ volatile("hlt");
        }
    }
    if (kverbose) {
        vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
        vga_write("== Driver Manager inicializado ==\n");
    }

    // Registrar driver VGA
    device_type_t vga_types[] = {DEVICE_TYPE_VIDEO, 0};
    // Depuración: obtener entry y volcar antes de registrar
    driver_entry_t* dbg_vga_entry = vga_driver_get_entry();
    driver_register("vga", dbg_vga_entry, vga_types);

    // Inicializar Device Manager
    if (kverbose) {
        vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
        vga_write("\n== Inicializando Device Manager ==\n");
    }
    int device_result = device_manager_init(kverbose);
    if (device_result != E_OK) {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_write("[FAIL] Error al inicializar Device Manager\n");
        while(1) {
            __asm__ volatile("hlt");
        }
    }
    if (kverbose) {
        vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
        vga_write("== Device Manager inicializado ==\n");
    }

    // Cargar módulos del kernel
    if (kverbose) {
        vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
        vga_write("\n== Cargando modulos del kernel ==\n");
    }

    // Cargar módulo ramdisk
    module_entry_t* ramdisk_entry = ramdisk_get_entry();
    if (ramdisk_entry == NULL) {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_write("[FAIL] ramdisk_get_entry() retornó NULL\n");
    }
    
    mid_t ramdisk_mid = module_register_static("ramdisk", ramdisk_entry);
    if (ramdisk_mid > 0) {
        if (kverbose) {
            vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
            vga_write("[MODULE] Ramdisk registrado con MID: ");
            vga_write_dec(ramdisk_mid);
            vga_write("\n");
        }
        
        // Iniciar el módulo ramdisk
        int ramdisk_start_result = module_start(ramdisk_mid);

        if (ramdisk_start_result != E_OK) {
            vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
            vga_write("[FAIL] Error al iniciar modulo ramdisk\n");
        }
    } else {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_write("[FAIL] Error al registrar modulo ramdisk\n");
    }

    // Cargar módulo early_neofs
    module_entry_t* early_neofs_entry = early_neofs_get_entry();
    if (early_neofs_entry == NULL) {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_write("[FAIL] early_neofs_get_entry() retornó NULL\n");
    }
    
    mid_t early_neofs_mid = module_register_static("early_neofs", early_neofs_entry);
    if (early_neofs_mid > 0) {
        if (kverbose) {
            vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
            vga_write("[MODULE] Early NeoFS registrado con MID: ");
            vga_write_dec(early_neofs_mid);
            vga_write("\n");
        }
        
        // Iniciar el módulo early_neofs
        int early_neofs_start_result = module_start(early_neofs_mid);

        if (early_neofs_start_result != E_OK) {
            vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
            vga_write("[FAIL] Error al iniciar modulo early_neofs\n");
        }
    } else {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_write("[FAIL] Error al registrar modulo early_neofs\n");
    }

    if (kverbose) {
        vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
        vga_write("== Modulos del kernel cargados ==\n");
    }


    // TODO: Inicializar subsistemas adicionales del kernel:
    // - NEO (Formato de ejecutable)

    if (kverbose) {
        vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
        vga_write("\nSubsistemas del kernel iniciados correctamente\n\n");
    }

    // Cargar la partición de NeoOS
    if (kverbose) {
        // vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
        // vga_write("Cargando particion de NeoOS...\n");
        // TODO: Implementar carga de partición NeoOS
    }

    // Transferir el control al scheduler (nunca retorna)
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    scheduler_switch();

    // Detener el kernel si llega aquí (no debería, pero
    // con lo que he vivido estas últimas dos semanas debuggeando
    // comienzo a creer que el compilador conspira contra mí
    // Y que todo es posible, y que el sched se muera es posible, tbh)
    panic("scheduler_switch retorno inesperadamente");
}   