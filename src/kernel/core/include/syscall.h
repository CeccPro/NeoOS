/**
 * NeoOS - Syscall Interface
 * Sistema de llamadas al sistema (microkernel)
 */

#ifndef _KERNEL_SYSCALL_H
#define _KERNEL_SYSCALL_H

#include "../../lib/include/types.h"

/**
 * Números de Syscall (15 syscalls en total)
 */

// === IPC / Comunicación (0-3) ===
#define SYS_SEND        0   // sys_send(pid_t dest, void *msg, size_t len, int flags)
#define SYS_RECV        1   // sys_recv(pid_t *src, void *buf, size_t len, int flags)
#define SYS_CALL        2   // sys_call(pid_t dest, void *req, void *resp, size_t len)
#define SYS_SIGNAL      3   // sys_signal(pid_t pid, int sig)

// === Scheduler / Threads (4-9) ===
#define SYS_THREAD_CREATE   4   // sys_thread_create(void *entry, void *stack, int flags)
#define SYS_THREAD_EXIT     5   // sys_thread_exit(int status)
#define SYS_YIELD           6   // sys_yield()
#define SYS_SETPRIORITY     7   // sys_setpriority(pid_t pid, int priority)
#define SYS_GETPRIORITY     8   // sys_getpriority(pid_t pid)
#define SYS_WAIT            9   // sys_wait(int *event_mask, timeout_t timeout)

// === Memory Management (10-12) ===
#define SYS_MAP         10  // sys_map(void *addr, size_t len, int prot, int flags)
#define SYS_UNMAP       11  // sys_unmap(void *addr, size_t len)
#define SYS_GRANT       12  // sys_grant(pid_t dest, void *addr, size_t len, int prot)

// === Sistema (13-14) ===
#define SYS_GETINFO     13  // sys_getinfo(int type, void *buf)
#define SYS_DEBUG       14  // sys_debug(const char *msg)

#define SYSCALL_COUNT   15  // Total de syscalls

/**
 * Tipos de información para sys_getinfo
 */
#define INFO_PID        0   // Obtener PID actual
#define INFO_UPTIME     1   // Tiempo desde el boot (en ticks)
#define INFO_TIME       2   // Tiempo actual (timestamp)
#define INFO_MEMORY     3   // Estadísticas de memoria

/**
 * Wrapper genérico para syscalls
 * Evita tener que escribir el inline assembly cada vez
 * 
 * @param num: Número de syscall
 * @param arg1-arg5: Argumentos de la syscall (hasta 5)
 * @return: Valor de retorno de la syscall
 * 
 * Ejemplo:
 *   int result = syscall(SYS_SEND, dest_pid, msg, size, flags, 0);
 */
static inline int syscall(int num, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5) {
    int ret;
    __asm__ volatile (
        "int $0x80"
        : "=a"(ret)
        : "a"(num), "b"(arg1), "c"(arg2), "d"(arg3), "S"(arg4), "D"(arg5)
        : "memory"
    );
    return ret;
}

/**
 * Wrappers específicos para cada syscall (más legibles)
 */

// === IPC ===
static inline int sys_send(pid_t dest, const void *msg, size_t len, int flags) {
    return syscall(SYS_SEND, (uint32_t)dest, (uint32_t)msg, (uint32_t)len, (uint32_t)flags, 0);
}

static inline int sys_recv(pid_t *src, void *buf, size_t len, int flags) {
    return syscall(SYS_RECV, (uint32_t)src, (uint32_t)buf, (uint32_t)len, (uint32_t)flags, 0);
}

static inline int sys_call(pid_t dest, void *req, void *resp, size_t len) {
    return syscall(SYS_CALL, (uint32_t)dest, (uint32_t)req, (uint32_t)resp, (uint32_t)len, 0);
}

static inline int sys_signal(pid_t pid, int sig) {
    return syscall(SYS_SIGNAL, (uint32_t)pid, (uint32_t)sig, 0, 0, 0);
}

// === Scheduler ===
static inline pid_t sys_thread_create(void *entry, void *stack, int flags) {
    return syscall(SYS_THREAD_CREATE, (uint32_t)entry, (uint32_t)stack, (uint32_t)flags, 0, 0);
}

static inline void sys_thread_exit(int status) {
    syscall(SYS_THREAD_EXIT, (uint32_t)status, 0, 0, 0, 0);
    // Nunca retorna, pero el compilador no lo sabe
    while(1) __asm__ volatile("hlt");
}

static inline void sys_yield(void) {
    syscall(SYS_YIELD, 0, 0, 0, 0, 0);
}

static inline int sys_setpriority(pid_t pid, int priority) {
    return syscall(SYS_SETPRIORITY, (uint32_t)pid, (uint32_t)priority, 0, 0, 0);
}

static inline int sys_getpriority(pid_t pid) {
    return syscall(SYS_GETPRIORITY, (uint32_t)pid, 0, 0, 0, 0);
}

static inline int sys_wait(int *event_mask, uint32_t timeout) {
    return syscall(SYS_WAIT, (uint32_t)event_mask, timeout, 0, 0, 0);
}

// === Memory ===
static inline int sys_map(void *addr, size_t len, int prot, int flags) {
    return syscall(SYS_MAP, (uint32_t)addr, (uint32_t)len, (uint32_t)prot, (uint32_t)flags, 0);
}

static inline int sys_unmap(void *addr, size_t len) {
    return syscall(SYS_UNMAP, (uint32_t)addr, (uint32_t)len, 0, 0, 0);
}

static inline int sys_grant(pid_t dest, void *addr, size_t len, int prot) {
    return syscall(SYS_GRANT, (uint32_t)dest, (uint32_t)addr, (uint32_t)len, (uint32_t)prot, 0);
}

// === Sistema ===
static inline int sys_getinfo(int type, void *buf) {
    return syscall(SYS_GETINFO, (uint32_t)type, (uint32_t)buf, 0, 0, 0);
}

static inline int sys_debug(const char *msg) {
    return syscall(SYS_DEBUG, (uint32_t)msg, 0, 0, 0, 0);
}

/**
 * Inicializar el sistema de syscalls
 * Configura el handler de int 0x80 en la IDT
 */
void syscall_init(bool verbose);

/**
 * Handler de syscalls (llamado por int 0x80)
 * Esta función es invocada desde assembly
 */
void syscall_handler(void);

#endif /* _KERNEL_SYSCALL_H */
