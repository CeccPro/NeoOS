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
#include "../../memory/include/memory.h"

// PIDs globales para el juego de Marco-Polo
static pid_t marco_pid = 0;
static pid_t polo_pid = 0;

/**
 * Proceso Marco - Inicia el juego enviando "Marco"
 * Espera recibir "Polo" y repite el ciclo
 */
void marco_process(void) {
    // Esperar a que el proceso Polo esté listo
    timer_wait_ms(100);
    
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_write("[MARCO] Proceso iniciado. Mi PID: ");
    vga_write_dec(scheduler_get_current_process()->pid);
    vga_write("\n");
    
    for (int i = 0; i < 5; i++) {
        // Enviar "Marco" al proceso Polo
        const char* msg = "Marco";
        vga_set_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
        vga_write("[MARCO] Enviando: ");
        vga_write(msg);
        vga_write(" -> Polo (PID ");
        vga_write_dec(polo_pid);
        vga_write(")\n");
        
        int result = ipc_send(polo_pid, msg, 6);  // 6 = strlen("Marco") + 1
        if (result != E_OK) {
            vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
            vga_write("[MARCO] Error al enviar mensaje: ");
            vga_write(error_to_string(result));
            vga_write("\n");
            break;
        }
        
        // Esperar respuesta "Polo"
        vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
        vga_write("[MARCO] Esperando respuesta...\n");
        
        ipc_message_t response;
        result = ipc_recv(&response, IPC_BLOCK);
        if (result == E_OK) {
            vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
            vga_write("[MARCO] Recibido de PID ");
            vga_write_dec(response.sender_pid);
            vga_write(": ");
            vga_write((char*)response.buffer);
            vga_write("\n");
            
            // Liberar el mensaje
            ipc_free(&response);
        } else {
            vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
            vga_write("[MARCO] Error al recibir: ");
            vga_write(error_to_string(result));
            vga_write("\n");
        }
        
        // Pequeña pausa antes del próximo round
        timer_wait_ms(500);
    }
    
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_write("[MARCO] Juego terminado!\n");
}

/**
 * Proceso Polo - Espera recibir "Marco" y responde con "Polo"
 */
void polo_process(void) {
    vga_set_color(VGA_COLOR_LIGHT_MAGENTA, VGA_COLOR_BLACK);
    vga_write("[POLO] Proceso iniciado. Mi PID: ");
    vga_write_dec(scheduler_get_current_process()->pid);
    vga_write("\n");
    vga_write("[POLO] Esperando mensajes...\n");
    
    for (int i = 0; i < 5; i++) {
        // Esperar mensaje "Marco"
        ipc_message_t msg;
        int result = ipc_recv(&msg, IPC_BLOCK);
        
        if (result == E_OK) {
            vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
            vga_write("[POLO] Recibido de PID ");
            vga_write_dec(msg.sender_pid);
            vga_write(": ");
            vga_write((char*)msg.buffer);
            vga_write("\n");
            
            // Responder con "Polo"
            const char* response = "Polo";
            vga_set_color(VGA_COLOR_LIGHT_MAGENTA, VGA_COLOR_BLACK);
            vga_write("[POLO] Respondiendo: ");
            vga_write(response);
            vga_write(" -> Marco (PID ");
            vga_write_dec(msg.sender_pid);
            vga_write(")\n");
            
            int send_result = ipc_send(msg.sender_pid, response, 5);  // 5 = strlen("Polo") + 1
            if (send_result != E_OK) {
                vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
                vga_write("[POLO] Error al enviar respuesta: ");
                vga_write(error_to_string(send_result));
                vga_write("\n");
            }
            
            // Liberar el mensaje recibido
            ipc_free(&msg);
        } else {
            vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
            vga_write("[POLO] Error al recibir: ");
            vga_write(error_to_string(result));
            vga_write("\n");
        }
    }
    
    vga_set_color(VGA_COLOR_LIGHT_MAGENTA, VGA_COLOR_BLACK);
    vga_write("[POLO] Juego terminado!\n");
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
        vga_write("== Scheduler inicializado ==\n\n");
    }

    // Inicializar IPC
    if (kverbose) {
        vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
        vga_write("== Inicializando IPC ==\n");
    }
    int ipc_result = ipc_init();
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

    vga_write("\n");
    
    // === Demo IPC: Juego de Marco-Polo ===
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_write("== Creando procesos para demo IPC (Marco-Polo) ==\n");
    
    // Crear proceso Polo primero
    polo_pid = scheduler_create_process("Polo", polo_process, PROCESS_PRIORITY_NORMAL);
    vga_write("[KMAIN] Proceso Polo creado con PID: ");
    vga_write_dec(polo_pid);
    vga_write("\n");
    
    // Crear proceso Marco
    marco_pid = scheduler_create_process("Marco", marco_process, PROCESS_PRIORITY_NORMAL);
    vga_write("[KMAIN] Proceso Marco creado con PID: ");
    vga_write_dec(marco_pid);
    vga_write("\n");
    
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_write("Iniciando juego de Marco-Polo usando IPC...\n\n");

    // TODO: Inicializar subsistemas adicionales del kernel:
    // - Syscalls
    // - RAM Disk
    // - Sistema de archivos
    // - Module Manager

    vga_write("\nSubsistemas del kernel iniciados correctamente\n");

    // Cargar la partición de NeoOS
    if (kverbose) {
        // vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
        // vga_write("Cargando particion de NeoOS...\n");
        // TODO: Implementar carga de partición NeoOS
    }

    // Transferir el control al scheduler (nunca retorna)
    scheduler_switch();
    
    // Nunca debemos llegar aquí
    vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
    vga_write("ERROR: scheduler_switch retorno!\n");
    while (1) {
        __asm__ volatile("hlt");
    }

    // Detener el kernel si llega aquí (no debería, pero
    // con lo que he vivido estas últimas dos semanas debuggeando
    // comienzo a creer que el compilador conspira contra mí
    // Y que todo es posible)
    vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
    vga_write("ERROR: Por alguna razón el kernel se pasó por los huevos\n");
    vga_write("todos los hlt anteriores y llegó aquí. Deteniendo el kernel...\n");
    while (1) {
        __asm__ volatile("hlt");
    }
}