/**
 * @file gdt.h
 * @brief Global Descriptor Table (GDT) para NeoOS
 * 
 * Implementación de la tabla de descriptores globales para la gestión
 * de segmentación de memoria en arquitectura x86.
 *
 * Copyright (C) 2025 CeccPro
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef GDT_H
#define GDT_H

#include <stdint.h>

// Estructura de una entrada de la GDT
struct gdt_entry {
    uint16_t limit_low;    // Los primeros 16 bits del límite
    uint16_t base_low;     // Los primeros 16 bits de la base
    uint8_t  base_middle;  // Los siguientes 8 bits de la base
    uint8_t  access;       // Byte de acceso
    uint8_t  granularity;  // Granularidad y límite superior
    uint8_t  base_high;    // Los últimos 8 bits de la base
} __attribute__((packed));

// Puntero a la GDT
struct gdt_ptr {
    uint16_t limit;        // Tamaño de la tabla - 1
    uint32_t base;         // Dirección de la primera entrada
} __attribute__((packed));

// Bits de acceso para la GDT
#define GDT_ACCESS_PRESENT      0x80  // Segmento presente
#define GDT_ACCESS_RING0        0x00  // Ring 0 (kernel)
#define GDT_ACCESS_RING3        0x60  // Ring 3 (userspace)
#define GDT_ACCESS_CODE_DATA    0x10  // Código/Datos (no sistema)
#define GDT_ACCESS_EXECUTABLE   0x08  // Segmento ejecutable
#define GDT_ACCESS_DIRECTION    0x04  // Dirección (0=crece hacia arriba)
#define GDT_ACCESS_RW           0x02  // Lectura/Escritura permitida
#define GDT_ACCESS_ACCESSED     0x01  // Bit de accedido (CPU lo pone a 1)

// Bits de granularidad
#define GDT_GRAN_4K             0x80  // Granularidad de 4KB
#define GDT_GRAN_32BIT          0x40  // Modo de 32 bits
#define GDT_GRAN_LIMIT_MASK     0x0F  // Máscara para los 4 bits superiores del límite

/**
 * @brief Inicializa la GDT con los segmentos básicos
 * 
 * Configura la GDT con:
 * - Segmento nulo (obligatorio)
 * - Segmento de código del kernel (Ring 0)
 * - Segmento de datos del kernel (Ring 0)
 * - Segmento de código de usuario (Ring 3)
 * - Segmento de datos de usuario (Ring 3)
 */
void gdt_init(void);

/**
 * @brief Establece una entrada en la GDT
 * 
 * @param num Número de entrada (0-5)
 * @param base Dirección base del segmento
 * @param limit Límite del segmento
 * @param access Byte de acceso
 * @param gran Byte de granularidad
 */
void gdt_set_gate(int32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran);

// Función en ensamblador para cargar la GDT
extern void gdt_flush(uint32_t gdt_ptr);

#endif // GDT_H
