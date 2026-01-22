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
#include "../../memory/include/memory.h"
#include "../../drivers/include/vga.h"
#include "../../lib/include/string.h"
#include "../../modules/include/ramdisk.h"
#include "../../modules/include/early_neofs.h"   

// Símbolo proporcionado por el linker script
extern uint32_t __kernel_end;
uint32_t kernel_end = (uint32_t)&__kernel_end;

/**
 * Test de Early NeoFS usando PMIC
 */
void test_early_neofs(mid_t neofs_mid, bool verbose) {
    if (verbose) {
        vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
        vga_write("\n== Test de Early NeoFS (via PMIC) ==\n");
    }
    
    uint8_t request_buffer[2048];
    uint8_t response_buffer[4096];  // Aumentado para acomodar múltiples entradas de directorio
    early_neofs_ipc_request_t* req = (early_neofs_ipc_request_t*)request_buffer;
    early_neofs_ipc_response_t* resp = (early_neofs_ipc_response_t*)response_buffer;
    
    // Test 1: Crear un directorio
    if (verbose) {
        vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
        vga_write("[TEST] Creando directorio /test via PMIC...\n");
    }
    memset(request_buffer, 0, sizeof(request_buffer));
    req->command = EARLY_NEOFS_CMD_MKDIR;
    strcpy(req->path, "/test");
    req->permissions = 0755;
    
    size_t resp_size = sizeof(response_buffer);
    int ipc_result = module_call(neofs_mid, request_buffer, sizeof(early_neofs_ipc_request_t), 
                                        response_buffer, &resp_size);
    if (ipc_result == E_OK && resp->result == E_OK) {
        if (verbose) {
            vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
            vga_write("[OK] Directorio /test creado\n");
        }
    } else {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_write("[FAIL] Error al crear /test (IPC: ");
        vga_write(error_to_string(ipc_result));
        vga_write(", FS: ");
        vga_write(error_to_string(resp->result));
        vga_write(")\n");
    }
    
    // Test 2: Crear un archivo
    if (verbose) {
        vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
        vga_write("[TEST] Creando archivo /test/hello.txt via PMIC...\n");
    }
    memset(request_buffer, 0, sizeof(request_buffer));
    req->command = EARLY_NEOFS_CMD_CREATE;
    strcpy(req->path, "/test/hello.txt");
    req->permissions = 0644;
    
    resp_size = sizeof(response_buffer);
    ipc_result = module_call(neofs_mid, request_buffer, sizeof(early_neofs_ipc_request_t), 
                                    response_buffer, &resp_size);
    if (ipc_result == E_OK && resp->result == E_OK) {
        if (verbose) {
            vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
            vga_write("[OK] Archivo /test/hello.txt creado\n");
        }
    } else {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_write("[FAIL] Error al crear archivo\n");
    }
    
    // Test 3: Abrir archivo para escritura
    if (verbose) {
        vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
        vga_write("[TEST] Abriendo archivo para escritura via PMIC...\n");
    }
    memset(request_buffer, 0, sizeof(request_buffer));
    req->command = EARLY_NEOFS_CMD_OPEN;
    strcpy(req->path, "/test/hello.txt");
    req->flags = EARLY_NEOFS_O_WRONLY;
    
    resp_size = sizeof(response_buffer);
    ipc_result = module_call(neofs_mid, request_buffer, sizeof(early_neofs_ipc_request_t), 
                                    response_buffer, &resp_size);
    int fd = -1;
    if (ipc_result == E_OK && resp->result >= 0) {
        fd = resp->result;
        if (verbose) {
            vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
            vga_write("[OK] Archivo abierto con fd: ");
            vga_write_dec(fd);
            vga_write("\n");
        }
        
        // Test 4: Escribir datos
        const char* mensaje = "Hola desde NeoOS Early FS via PMIC!";
        if (verbose) {
            vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
            vga_write("[TEST] Escribiendo: '");
            vga_write(mensaje);
            vga_write("'\n");
        }
        
        memset(request_buffer, 0, sizeof(request_buffer));
        req->command = EARLY_NEOFS_CMD_WRITE;
        req->fd = fd;
        req->count = strlen(mensaje);
        memcpy(req->data, mensaje, req->count);
        
        resp_size = sizeof(response_buffer);
        ipc_result = module_call(neofs_mid, request_buffer, 
                                        sizeof(early_neofs_ipc_request_t) + req->count,
                                        response_buffer, &resp_size);
        if (ipc_result == E_OK && resp->result > 0) {
            if (verbose) {
                vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
                vga_write("[OK] ");
                vga_write_dec(resp->result);
                vga_write(" bytes escritos\n");
            }
        } else {
            vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
            vga_write("[FAIL] Error al escribir\n");
        }
        
        // Cerrar archivo
        memset(request_buffer, 0, sizeof(request_buffer));
        req->command = EARLY_NEOFS_CMD_CLOSE;
        req->fd = fd;
        resp_size = sizeof(response_buffer);
        module_call(neofs_mid, request_buffer, sizeof(early_neofs_ipc_request_t), 
                          response_buffer, &resp_size);
    } else {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_write("[FAIL] Error al abrir archivo para escritura\n");
    }
    
    // Test 5: Abrir y leer archivo
    if (verbose) {
        vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
        vga_write("[TEST] Abriendo archivo para lectura via PMIC...\n");
    }
    memset(request_buffer, 0, sizeof(request_buffer));
    req->command = EARLY_NEOFS_CMD_OPEN;
    strcpy(req->path, "/test/hello.txt");
    req->flags = EARLY_NEOFS_O_RDONLY;
    
    resp_size = sizeof(response_buffer);
    ipc_result = module_call(neofs_mid, request_buffer, sizeof(early_neofs_ipc_request_t), 
                                    response_buffer, &resp_size);
    if (ipc_result == E_OK && resp->result >= 0) {
        fd = resp->result;
        
        // Leer datos
        memset(request_buffer, 0, sizeof(request_buffer));
        req->command = EARLY_NEOFS_CMD_READ;
        req->fd = fd;
        req->count = 128;
        
        resp_size = sizeof(response_buffer);
        ipc_result = module_call(neofs_mid, request_buffer, sizeof(early_neofs_ipc_request_t),
                                        response_buffer, &resp_size);
        if (ipc_result == E_OK && resp->result > 0) {
            if (verbose) {
                vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
                vga_write("[OK] ");
                vga_write_dec(resp->result);
                vga_write(" bytes leidos: '");
                // Asegurar terminación nula
                resp->data[resp->result] = '\0';
                vga_write((char*)resp->data);
                vga_write("'\n");
            }
        } else {
            vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
            vga_write("[FAIL] Error al leer\n");
        }
        
        // Cerrar archivo
        memset(request_buffer, 0, sizeof(request_buffer));
        req->command = EARLY_NEOFS_CMD_CLOSE;
        req->fd = fd;
        resp_size = sizeof(response_buffer);
        module_call(neofs_mid, request_buffer, sizeof(early_neofs_ipc_request_t), 
                          response_buffer, &resp_size);
    } else {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_write("[FAIL] Error al abrir archivo para lectura\n");
    }
    
    // Test 6: Obtener información del archivo
    if (verbose) {
        vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
        vga_write("[TEST] Obteniendo informacion del archivo via PMIC...\n");
    }
    memset(request_buffer, 0, sizeof(request_buffer));
    req->command = EARLY_NEOFS_CMD_STAT;
    strcpy(req->path, "/test/hello.txt");
    
    resp_size = sizeof(response_buffer);
    ipc_result = module_call(neofs_mid, request_buffer, sizeof(early_neofs_ipc_request_t),
                                    response_buffer, &resp_size);
    if (ipc_result == E_OK && resp->result == E_OK) {
        early_neofs_stat_t* stat = (early_neofs_stat_t*)resp->data;
        if (verbose) {
            vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
            vga_write("[OK] Stat exitoso:\n");
            vga_write("  - Inode: ");
            vga_write_dec(stat->inode_number);
            vga_write("\n  - Tamano: ");
            vga_write_dec(stat->size);
            vga_write(" bytes\n");
            vga_write("  - Tipo: ");
            vga_write(stat->type == INODE_TYPE_FILE ? "Archivo" : "Directorio");
            vga_write("\n");
        }
    } else {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_write("[FAIL] Error en stat\n");
    }
    
    // Test 7: Crear más archivos
    if (verbose) {
        vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
        vga_write("[TEST] Creando archivos adicionales via PMIC...\n");
    }
    
    memset(request_buffer, 0, sizeof(request_buffer));
    req->command = EARLY_NEOFS_CMD_CREATE;
    strcpy(req->path, "/test/file1.txt");
    req->permissions = 0644;
    resp_size = sizeof(response_buffer);
    module_call(neofs_mid, request_buffer, sizeof(early_neofs_ipc_request_t), 
                       response_buffer, &resp_size);
    
    memset(request_buffer, 0, sizeof(request_buffer));
    req->command = EARLY_NEOFS_CMD_CREATE;
    strcpy(req->path, "/test/file2.txt");
    req->permissions = 0644;
    resp_size = sizeof(response_buffer);
    module_call(neofs_mid, request_buffer, sizeof(early_neofs_ipc_request_t), 
                       response_buffer, &resp_size);
    
    memset(request_buffer, 0, sizeof(request_buffer));
    req->command = EARLY_NEOFS_CMD_MKDIR;
    strcpy(req->path, "/test/subdir");
    req->permissions = 0755;
    resp_size = sizeof(response_buffer);
    module_call(neofs_mid, request_buffer, sizeof(early_neofs_ipc_request_t), 
                       response_buffer, &resp_size);
    
    // Test 8: Listar directorio
    if (verbose) {
        vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
        vga_write("[TEST] Listando directorio /test via PMIC...\n");
    }
    memset(request_buffer, 0, sizeof(request_buffer));
    req->command = EARLY_NEOFS_CMD_READDIR;
    strcpy(req->path, "/test");
    req->max_entries = 32;
    
    resp_size = sizeof(response_buffer);
    ipc_result = module_call(neofs_mid, request_buffer, sizeof(early_neofs_ipc_request_t),
                                    response_buffer, &resp_size);
    if (ipc_result == E_OK && resp->result > 0) {
        early_neofs_dir_entry_t* entries = (early_neofs_dir_entry_t*)resp->data;
        int count = resp->result;
        if (verbose) {
            vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
            vga_write("[OK] ");
            vga_write_dec(count);
            vga_write(" entradas encontradas:\n");
            for (int i = 0; i < count; i++) {
                vga_write("  - ");
                vga_write(entries[i].name);
                vga_write(" (");
                vga_write(entries[i].type == INODE_TYPE_DIR ? "DIR" : "FILE");
                vga_write(")\n");
            }
        }
    } else {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_write("[FAIL] Error al listar directorio (PMIC: ");
        vga_write(error_to_string(ipc_result));
        vga_write(", FS: ");
        vga_write_dec(resp->result);
        vga_write(")\n");
    }
    
    // Test 9: Eliminar un archivo
    if (verbose) {
        vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
        vga_write("[TEST] Eliminando /test/file1.txt via PMIC...\n");
    }
    memset(request_buffer, 0, sizeof(request_buffer));
    req->command = EARLY_NEOFS_CMD_UNLINK;
    strcpy(req->path, "/test/file1.txt");
    
    resp_size = sizeof(response_buffer);
    ipc_result = module_call(neofs_mid, request_buffer, sizeof(early_neofs_ipc_request_t),
                                    response_buffer, &resp_size);
    if (ipc_result == E_OK && resp->result == E_OK) {
        if (verbose) {
            vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
            vga_write("[OK] Archivo eliminado\n");
        }
    } else {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_write("[FAIL] Error al eliminar archivo\n");
    }
    
    if (verbose) {
        vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
        vga_write("== Test de Early NeoFS completado ==\n");
    }
}

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

    // Test de Early NeoFS usando PMIC
    if (early_neofs_mid > 0) {
        test_early_neofs(early_neofs_mid, kverbose);
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
    scheduler_switch();

    // Detener el kernel si llega aquí (no debería, pero
    // con lo que he vivido estas últimas dos semanas debuggeando
    // comienzo a creer que el compilador conspira contra mí
    // Y que todo es posible, y que el sched se muera es posible, tbh)
    panic("scheduler_switch retorno inesperadamente");
}   