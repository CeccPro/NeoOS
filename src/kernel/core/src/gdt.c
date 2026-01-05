/**
 * NeoOS - GDT Implementation
 * Configuración de la tabla de descriptores globales
 */

#include "../../core/include/gdt.h"
#include "../../lib/include/string.h"

// Array de entradas GDT
static gdt_entry_t gdt_entries[GDT_ENTRIES];

// Puntero a la GDT que se carga con LGDT
static gdt_ptr_t gdt_ptr;

/**
 * Establecer una entrada de la GDT
 * @param num: Índice de la entrada (0-4)
 * @param base: Dirección base del segmento
 * @param limit: Límite del segmento
 * @param access: Byte de acceso
 * @param gran: Byte de granularidad
 */
static void gdt_set_gate(int32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    // Configurar la dirección base
    gdt_entries[num].base_low    = (base & 0xFFFF);
    gdt_entries[num].base_middle = (base >> 16) & 0xFF;
    gdt_entries[num].base_high   = (base >> 24) & 0xFF;

    // Configurar el límite
    gdt_entries[num].limit_low   = (limit & 0xFFFF);
    gdt_entries[num].granularity = (limit >> 16) & 0x0F;

    // Configurar la granularidad y flags
    gdt_entries[num].granularity |= gran & 0xF0;
    
    // Configurar el byte de acceso
    gdt_entries[num].access = access;
}

/**
 * Inicializar la GDT
 * Configura las 5 entradas básicas: null, kernel code, kernel data, user code, user data
 */
void gdt_init(void) {
    // Configurar el puntero a la GDT
    gdt_ptr.limit = (sizeof(gdt_entry_t) * GDT_ENTRIES) - 1;
    gdt_ptr.base  = (uint32_t)&gdt_entries;

    // Inicializar todas las entradas a 0
    memset(&gdt_entries, 0, sizeof(gdt_entry_t) * GDT_ENTRIES);

    // Entrada 0: Descriptor nulo (requerido por la arquitectura x86)
    gdt_set_gate(0, 0, 0, 0, 0);

    // Entrada 1: Segmento de código del kernel
    // Base: 0x00000000, Límite: 0xFFFFFFFF (4GB)
    // Acceso: Present | Ring 0 | Code | Read/Write
    // Granularidad: 4KB pages | 32-bit
    gdt_set_gate(1, 0, 0xFFFFFFFF, 
                 GDT_ACCESS_PRESENT | GDT_ACCESS_PRIV_0 | GDT_ACCESS_CODE_SEG | GDT_ACCESS_READ_WRITE,
                 GDT_GRAN_4K | GDT_GRAN_32BIT | 0x0F);

    // Entrada 2: Segmento de datos del kernel
    // Base: 0x00000000, Límite: 0xFFFFFFFF (4GB)
    // Acceso: Present | Ring 0 | Data | Read/Write
    // Granularidad: 4KB pages | 32-bit
    gdt_set_gate(2, 0, 0xFFFFFFFF,
                 GDT_ACCESS_PRESENT | GDT_ACCESS_PRIV_0 | GDT_ACCESS_DATA_SEG | GDT_ACCESS_READ_WRITE,
                 GDT_GRAN_4K | GDT_GRAN_32BIT | 0x0F);

    // Entrada 3: Segmento de código de usuario
    // Base: 0x00000000, Límite: 0xFFFFFFFF (4GB)
    // Acceso: Present | Ring 3 | Code | Read/Write
    // Granularidad: 4KB pages | 32-bit
    gdt_set_gate(3, 0, 0xFFFFFFFF,
                 GDT_ACCESS_PRESENT | GDT_ACCESS_PRIV_3 | GDT_ACCESS_CODE_SEG | GDT_ACCESS_READ_WRITE,
                 GDT_GRAN_4K | GDT_GRAN_32BIT | 0x0F);

    // Entrada 4: Segmento de datos de usuario
    // Base: 0x00000000, Límite: 0xFFFFFFFF (4GB)
    // Acceso: Present | Ring 3 | Data | Read/Write
    // Granularidad: 4KB pages | 32-bit
    gdt_set_gate(4, 0, 0xFFFFFFFF,
                 GDT_ACCESS_PRESENT | GDT_ACCESS_PRIV_3 | GDT_ACCESS_DATA_SEG | GDT_ACCESS_READ_WRITE,
                 GDT_GRAN_4K | GDT_GRAN_32BIT | 0x0F);

    // Cargar la GDT en el CPU
    gdt_flush((uint32_t)&gdt_ptr);
}
