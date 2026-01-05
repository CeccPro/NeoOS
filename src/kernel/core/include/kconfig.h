/**
 * NeoOS - Kernel Configuration
 * Variables de configuración global del kernel
 */

#ifndef _KERNEL_KCONFIG_H
#define _KERNEL_KCONFIG_H

#include "../../lib/include/types.h"

// Variables globales de configuración del kernel
extern bool kernel_debug_mode;
extern bool kernel_verbose_mode;

/**
 * Inicializa la configuración del kernel
 * @param debug Activar modo debug
 * @param verbose Activar modo verbose
 */
void kconfig_init(bool debug, bool verbose);

/**
 * Verifica si el modo debug está activo
 * @return true si debug está activo
 */
static inline bool is_kdebug(void) {
    return kernel_debug_mode;
}

/**
 * Verifica si el modo verbose está activo
 * @return true si verbose está activo
 */
static inline bool is_kverbose(void) {
    return kernel_verbose_mode;
}

#endif /* _KERNEL_KCONFIG_H */
