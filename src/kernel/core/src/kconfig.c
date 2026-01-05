/**
 * NeoOS - Kernel Configuration
 * Implementación de configuración global del kernel
 */

#include "../include/kconfig.h"

// Variables globales de configuración
bool kernel_debug_mode = false;
bool kernel_verbose_mode = false;

/**
 * Inicializa la configuración del kernel
 */
void kconfig_init(bool debug, bool verbose) {
    kernel_debug_mode = debug;
    kernel_verbose_mode = verbose;
}
