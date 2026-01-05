/**
 * NeoOS - GDT (Global Descriptor Table)
 * Configuración de la tabla de descriptores globales
 */

#ifndef _KERNEL_GDT_H
#define _KERNEL_GDT_H

// Definiciones de selectores de segmento (Para mejor legibilidad)
#define KERNEL_CS GDT_KERNEL_CODE_SEGMENT
#define KERNEL_DS GDT_KERNEL_DATA_SEGMENT
#define USER_CS   GDT_USER_CODE_SELECTOR
#define USER_DS   GDT_USER_DATA_SELECTOR

#include "../../lib/include/types.h"

// Número de entradas en la GDT
#define GDT_ENTRIES 5

/**
 * Estructura de una entrada GDT
 * Empaquetada para evitar padding del compilador
 */
struct gdt_entry {
    uint16_t limit_low;    // Límite bajo (bits 0-15)
    uint16_t base_low;     // Base baja (bits 0-15)
    uint8_t  base_middle;  // Base media (bits 16-23)
    uint8_t  access;       // Flags de acceso
    uint8_t  granularity;  // Granularidad y límite alto (bits 16-19)
    uint8_t  base_high;    // Base alta (bits 24-31)
} __attribute__((packed));

typedef struct gdt_entry gdt_entry_t;

/**
 * Puntero a la GDT (GDTR)
 * Esta estructura se carga con la instrucción LGDT
 */
struct gdt_ptr {
    uint16_t limit;   // Tamaño de la GDT - 1
    uint32_t base;    // Dirección base de la GDT
} __attribute__((packed));

typedef struct gdt_ptr gdt_ptr_t;

/**
 * Valores de los selectores de segmento
 * Estos son los offsets en la GDT
 */
#define GDT_KERNEL_CODE_SEGMENT 0x08  // Segmento de código del kernel (índice 1)
#define GDT_KERNEL_DATA_SEGMENT 0x10  // Segmento de datos del kernel (índice 2)
#define GDT_USER_CODE_SEGMENT   0x18  // Segmento de código de usuario (índice 3)
#define GDT_USER_DATA_SEGMENT   0x20  // Segmento de datos de usuario (índice 4)

/**
 * Selectores de segmento con RPL (Requested Privilege Level) para ring 3
 * Los últimos 2 bits del selector indican el nivel de privilegio (RPL).
 * Para acceder a segmentos de usuario desde ring 3, hay que ORear con 0x03.
 * Usar estos cuando se implemente el salto a modo usuario.
 */
#define GDT_USER_CODE_SELECTOR  (GDT_USER_CODE_SEGMENT | 0x03)  // 0x1B - Ring 3
#define GDT_USER_DATA_SELECTOR  (GDT_USER_DATA_SEGMENT | 0x03)  // 0x23 - Ring 3

/**
 * Bits de acceso para entradas GDT
 */
#define GDT_ACCESS_PRESENT    0x80  // Segmento presente
#define GDT_ACCESS_PRIV_0     0x00  // Ring 0 (kernel)
#define GDT_ACCESS_PRIV_3     0x60  // Ring 3 (usuario)
#define GDT_ACCESS_CODE_SEG   0x18  // Segmento de código ejecutable
#define GDT_ACCESS_DATA_SEG   0x10  // Segmento de datos
#define GDT_ACCESS_READ_WRITE 0x02  // Lectura/escritura permitida

/**
 * Bits de granularidad
 */
#define GDT_GRAN_4K           0x80  // Límite en páginas de 4KB
#define GDT_GRAN_32BIT        0x40  // Segmento de 32 bits

/**
 * Inicializar la GDT
 * Configura 5 entradas: null, kernel code, kernel data, user code, user data
 */
void gdt_init(void);

/**
 * Función en assembly para cargar la GDT
 * Definida en gdt_load.S
 */
extern void gdt_flush(uint32_t gdt_ptr);

#endif /* _KERNEL_GDT_H */
