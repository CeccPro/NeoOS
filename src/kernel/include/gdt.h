/**
 * @file gdt.h
 * @brief Global Descriptor Table (GDT) para NeoOS
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
#include "errors.h"

// Selectores de segmento
#define GDT_KERNEL_CODE 0x08  // Offset del segmento de código del kernel
#define GDT_KERNEL_DATA 0x10  // Offset del segmento de datos del kernel
#define GDT_USER_CODE   0x18  // Offset del segmento de código de usuario
#define GDT_USER_DATA   0x20  // Offset del segmento de datos de usuario
#define GDT_TSS         0x28  // Offset del Task State Segment

// Flags de acceso
#define GDT_ACCESS_PRESENT     0x80  // Presente en memoria
#define GDT_ACCESS_RING0       0x00  // Ring 0 (kernel)
#define GDT_ACCESS_RING3       0x60  // Ring 3 (usuario)
#define GDT_ACCESS_EXECUTABLE  0x08  // Segmento ejecutable (código)
#define GDT_ACCESS_RW          0x02  // Lectura/Escritura

// Flags de granularidad
#define GDT_FLAG_GRANULARITY   0x80  // Granularidad de 4KB
#define GDT_FLAG_32BIT         0x40  // Segmento de 32 bits

// Estructura de una entrada GDT
typedef struct {
    uint16_t limit_low;      // Límite 0-15
    uint16_t base_low;       // Base 0-15
    uint8_t  base_middle;    // Base 16-23
    uint8_t  access;         // Flags de acceso
    uint8_t  granularity;    // Flags de granularidad + límite 16-19
    uint8_t  base_high;      // Base 24-31
} __attribute__((packed)) gdt_entry_t;

// Estructura del puntero GDT que se carga en el registro GDTR
typedef struct {
    uint16_t limit;          // Tamaño de la GDT - 1
    uint32_t base;           // Dirección base de la GDT
} __attribute__((packed)) gdt_ptr_t;

/**
 * @brief Inicializa la Global Descriptor Table
 * @return E_OK si fue exitoso, código de error en caso contrario
 */
error_t gdt_init(void);

/**
 * @brief Establece una entrada en la GDT
 * @param num Número de entrada (0-5)
 * @param base Dirección base del segmento
 * @param limit Límite del segmento
 * @param access Flags de acceso
 * @param granularity Flags de granularidad
 */
void gdt_set_gate(int32_t num, uint32_t base, uint32_t limit, 
                  uint8_t access, uint8_t granularity);

#endif // GDT_H
