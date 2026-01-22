/**
 * NeoOS - Syscall Implementation
 * Dispatcher de syscalls y handlers
 */

#include "../include/syscall.h"
#include "../include/error.h"
#include "../include/idt.h"
#include "../include/interrupts.h"
#include "../include/scheduler.h"
#include "../include/ipc.h"
#include "../include/module.h"
#include "../../memory/include/memory.h"
#include "../../drivers/include/vga.h"

/**
 * Handler de syscalls en C
 * Llamado desde el ISR de int 0x80
 */
int syscall_dispatch(uint32_t syscall_num, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5 __attribute__((unused))) {
    switch (syscall_num) {
        
        // ===== IPC / Comunicación =====
        case SYS_SEND:
            return ipc_send((pid_t)arg1, (const void*)arg2, (size_t)arg3);
        
        case SYS_RECV: {
            ipc_message_t msg;
            int result = ipc_recv(&msg, (int)arg4);
            if (result == E_OK && msg.size > 0) {
                // Copiar el mensaje al buffer del usuario
                pid_t *src = (pid_t*)arg1;
                void *buf = (void*)arg2;
                size_t len = (size_t)arg3;
                
                if (src) *src = msg.sender_pid;
                if (buf && len > 0) {
                    size_t copy_size = msg.size < len ? msg.size : len;
                    // TODO: usar memcpy cuando esté disponible
                    char *dst = (char*)buf;
                    char *src_buf = (char*)msg.buffer;
                    for (size_t i = 0; i < copy_size; i++) {
                        dst[i] = src_buf[i];
                    }
                }
                ipc_free(&msg);
                return msg.size;  // Retornar tamaño del mensaje
            }
            return result;
        }
        
        case SYS_CALL:
            // RPC: send + recv atómico
            // TODO: Implementar cuando sea necesario
            return E_NOT_IMPL;
        
        case SYS_SIGNAL:
            // TODO: Implementar sistema de señales
            return E_NOT_IMPL;
        
        // ===== Scheduler / Threads =====
        case SYS_THREAD_CREATE: {
            // arg1 = entry, arg2 = stack, arg3 = flags
            void (*entry)(void) = (void(*)(void))arg1;
            
            // Por ahora, prioridad normal por defecto
            // TODO: Parsear flags para determinar prioridad
            uint32_t pid = scheduler_create_process("userthread", entry, PROCESS_PRIORITY_NORMAL);
            return (int)pid;
        }
        
        case SYS_THREAD_EXIT:
            // arg1 = status
            scheduler_terminate_process(scheduler_get_current_process()->pid);
            // Nunca retorna, pero por si acaso:
            scheduler_yield();
            return E_OK;
        
        case SYS_YIELD:
            scheduler_yield();
            return E_OK;
        
        case SYS_SETPRIORITY:
            return scheduler_set_priority((uint32_t)arg1, (process_priority_t)arg2);
        
        case SYS_GETPRIORITY:
            return scheduler_get_priority((uint32_t)arg1);
        
        case SYS_WAIT:
            // TODO: Implementar wait con event mask y timeout
            return E_NOT_IMPL;
        
        // ===== Memory Management =====
        case SYS_MAP:
            // TODO: Implementar mapeo de memoria para userspace
            // Por ahora solo funciona en kernel space
            return E_NOT_IMPL;
        
        case SYS_UNMAP:
            // TODO: Implementar unmapping
            return E_NOT_IMPL;
        
        case SYS_GRANT:
            // TODO: Implementar compartir memoria entre procesos
            return E_NOT_IMPL;
        
        // ===== Sistema =====
        case SYS_GETINFO: {
            int type = (int)arg1;
            void *buf = (void*)arg2;
            
            if (!buf) return E_INVAL;
            
            switch (type) {
                case INFO_PID: {
                    pid_t *pid_ptr = (pid_t*)buf;
                    *pid_ptr = scheduler_get_current_process()->pid;
                    return E_OK;
                }
                
                case INFO_UPTIME: {
                    // TODO: Obtener uptime del timer
                    uint32_t *uptime = (uint32_t*)buf;
                    *uptime = 0;  // Placeholder
                    return E_NOT_IMPL;
                }
                
                case INFO_TIME:
                    // TODO: Implementar RTC
                    return E_NOT_IMPL;
                
                case INFO_MEMORY: {
                    // Obtener info de memoria
                    uint32_t *mem_info = (uint32_t*)buf;
                    uint32_t total_kb, used_kb, free_kb;
                    memory_get_info(&total_kb, &used_kb, &free_kb);
                    mem_info[0] = total_kb;
                    mem_info[1] = used_kb;
                    mem_info[2] = free_kb;
                    return E_OK;
                }
                
                default:
                    return E_INVAL;
            }
        }
        
        case SYS_DEBUG: {
            const char *msg = (const char*)arg1;
            if (!msg) return E_INVAL;
            
            #ifdef DEBUG
            vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
            vga_write("[SYSCALL DEBUG] ");
            vga_write(msg);
            vga_write("\n");
            #endif
            
            return E_OK;
        }
        
        // ===== Module Manager =====
        case SYS_MODLOAD: {
            const char *name = (const char*)arg1;
            if (!name) return E_INVAL;
            return module_load(name);
        }
        
        case SYS_MODUNLOAD: {
            mid_t mid = (mid_t)arg1;
            return module_unload(mid);
        }
        
        case SYS_MODSTART: {
            mid_t mid = (mid_t)arg1;
            return module_start(mid);
        }
        
        case SYS_MODSTOP: {
            mid_t mid = (mid_t)arg1;
            return module_stop(mid);
        }
        
        case SYS_MODSTATUS: {
            mid_t mid = (mid_t)arg1;
            return module_get_state(mid);
        }
        
        // ===== Module IPC =====
        case SYS_MODSEND: {
            mid_t mid = (mid_t)arg1;
            const void* msg = (const void*)arg2;
            size_t size = (size_t)arg3;
            
            if (!msg || size == 0) return E_INVAL;
            
            return module_send(mid, msg, size);
        }
        
        case SYS_MODSEND_NAME: {
            const char* name = (const char*)arg1;
            const void* msg = (const void*)arg2;
            size_t size = (size_t)arg3;
            
            if (!name || !msg || size == 0) return E_INVAL;
            
            return module_send_by_name(name, msg, size);
        }
        
        case SYS_MODCALL: {
            mid_t mid = (mid_t)arg1;
            const void* request = (const void*)arg2;
            size_t request_size = (size_t)arg3;
            void* response = (void*)arg4;
            size_t* response_size = (size_t*)arg5;
            
            if (!request || request_size == 0) return E_INVAL;
            if (!response || !response_size) return E_INVAL;
            
            return module_call(mid, request, request_size, response, response_size);
        }
        
        case SYS_MODGETID: {
            const char* name = (const char*)arg1;
            if (!name) return E_INVAL;
            
            return module_get_id(name);
        }
        
        default:
            return E_INVAL;
    }
}

/**
 * ISR para int 0x80 (definido en arch/x86/isr.S)
 * Este debe llamar a syscall_handler_wrapper
 */
extern void isr128(void);

/**
 * Wrapper del handler de syscalls
 * Extrae los argumentos del stack y llama al dispatcher
 */
void syscall_handler_wrapper(registers_t *regs) {
    // Argumentos pasados en registros:
    // EAX = syscall number
    // EBX = arg1, ECX = arg2, EDX = arg3, ESI = arg4, EDI = arg5
    
    int result = syscall_dispatch(
        regs->eax,  // Número de syscall
        regs->ebx,  // arg1
        regs->ecx,  // arg2
        regs->edx,  // arg3
        regs->esi,  // arg4
        regs->edi   // arg5
    );
    
    // Retornar resultado en EAX
    regs->eax = result;
}

/**
 * Inicializar el sistema de syscalls
 */
void syscall_init(bool verbose) {
    if (verbose) {
        vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
        vga_write("[SYSCALL] Inicializando sistema de syscalls...\n");
    }
    
    // Registrar el handler de int 0x80 en la IDT
    // 0x80 = 128, 0x08 = kernel code segment, 0x8E = present, ring 0, interrupt gate
    idt_set_gate(0x80, (uint32_t)isr128, 0x08, 0x8E);
    
    if (verbose) {
        vga_write("[SYSCALL] Registradas ");
        vga_write_dec(SYSCALL_COUNT);
        vga_write(" syscalls\n");
        vga_write("[SYSCALL] Handler configurado en int 0x80\n");
    }
}
