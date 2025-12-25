/**
 * @file gdt.c
 * @brief Implementación de la Global Descriptor Table
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

#include "../include/gdt.h"
#include "../include/kernel.h"

// Array de entradas GDT (6 entradas: null, código kernel, datos kernel, código usuario, datos usuario, TSS)
static gdt_entry_t gdt_entries[6];
static gdt_ptr_t gdt_ptr;

// Función externa en ensamblador que carga la GDT
extern void gdt_flush(uint32_t);

/**
 * @brief Establece una entrada en la GDT
 */
void gdt_set_gate(int32_t num, uint32_t base, uint32_t limit, 
                  uint8_t access, uint8_t granularity) {
    gdt_entries[num].base_low = (base & 0xFFFF);
    gdt_entries[num].base_middle = (base >> 16) & 0xFF;
    gdt_entries[num].base_high = (base >> 24) & 0xFF;

    gdt_entries[num].limit_low = (limit & 0xFFFF);
    gdt_entries[num].granularity = (limit >> 16) & 0x0F;
    
    gdt_entries[num].granularity |= granularity & 0xF0;
    gdt_entries[num].access = access;
}

/**
 * @brief Inicializa la Global Descriptor Table
 */
error_t gdt_init(void) {
    gdt_ptr.limit = (sizeof(gdt_entry_t) * 6) - 1;
    gdt_ptr.base = (uint32_t)&gdt_entries;

    // Entrada nula (requerida)
    gdt_set_gate(0, 0, 0, 0, 0);

    // Segmento de código del kernel (0x08)
    // Base = 0x00000000, Límite = 0xFFFFFFFF
    // Presente, Ring 0, Ejecutable, Lectura/Escritura
    gdt_set_gate(1, 0, 0xFFFFFFFF,
                 GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_EXECUTABLE | GDT_ACCESS_RW,
                 GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);

    // Segmento de datos del kernel (0x10)
    // Base = 0x00000000, Límite = 0xFFFFFFFF
    // Presente, Ring 0, No ejecutable, Lectura/Escritura
    gdt_set_gate(2, 0, 0xFFFFFFFF,
                 GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_RW,
                 GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);

    // Segmento de código de usuario (0x18)
    // Base = 0x00000000, Límite = 0xFFFFFFFF
    // Presente, Ring 3, Ejecutable, Lectura/Escritura
    gdt_set_gate(3, 0, 0xFFFFFFFF,
                 GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_EXECUTABLE | GDT_ACCESS_RW,
                 GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);

    // Segmento de datos de usuario (0x20)
    // Base = 0x00000000, Límite = 0xFFFFFFFF
    // Presente, Ring 3, No ejecutable, Lectura/Escritura
    gdt_set_gate(4, 0, 0xFFFFFFFF,
                 GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_RW,
                 GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);

    // TSS (Task State Segment) - por ahora vacío
    gdt_set_gate(5, 0, 0, 0, 0);

    // Cargar la GDT
    gdt_flush((uint32_t)&gdt_ptr);

    return E_OK;
}
