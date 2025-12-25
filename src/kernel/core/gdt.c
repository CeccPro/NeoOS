/**
 * @file gdt.c
 * @brief Implementación de la Global Descriptor Table (GDT)
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

// Array de entradas de la GDT (5 entradas)
static struct gdt_entry gdt_entries[5];
static struct gdt_ptr gdt_pointer;

/**
 * @brief Establece una entrada en la GDT
 */
void gdt_set_gate(int32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt_entries[num].base_low    = (base & 0xFFFF);
    gdt_entries[num].base_middle = (base >> 16) & 0xFF;
    gdt_entries[num].base_high   = (base >> 24) & 0xFF;

    gdt_entries[num].limit_low   = (limit & 0xFFFF);
    gdt_entries[num].granularity = (limit >> 16) & 0x0F;

    gdt_entries[num].granularity |= gran & 0xF0;
    gdt_entries[num].access      = access;
}

/**
 * @brief Inicializa la GDT
 */
void gdt_init(void) {
    gdt_pointer.limit = (sizeof(struct gdt_entry) * 5) - 1;
    gdt_pointer.base  = (uint32_t)&gdt_entries;

    // Entrada nula (obligatoria)
    gdt_set_gate(0, 0, 0, 0, 0);

    // Segmento de código del kernel
    // Base = 0x00000000, Limite = 0xFFFFFFFF
    // Acceso: Presente, Ring 0, Código/Datos, Ejecutable, Lectura/Escritura
    // Granularidad: 4KB, 32-bit
    gdt_set_gate(1, 0, 0xFFFFFFFF, 
                 GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_CODE_DATA | 
                 GDT_ACCESS_EXECUTABLE | GDT_ACCESS_RW,
                 GDT_GRAN_4K | GDT_GRAN_32BIT | 0x0F);

    // Segmento de datos del kernel
    // Base = 0x00000000, Limite = 0xFFFFFFFF
    // Acceso: Presente, Ring 0, Código/Datos, No ejecutable, Lectura/Escritura
    // Granularidad: 4KB, 32-bit
    gdt_set_gate(2, 0, 0xFFFFFFFF,
                 GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_CODE_DATA | 
                 GDT_ACCESS_RW,
                 GDT_GRAN_4K | GDT_GRAN_32BIT | 0x0F);

    // Segmento de código de usuario
    // Base = 0x00000000, Limite = 0xFFFFFFFF
    // Acceso: Presente, Ring 3, Código/Datos, Ejecutable, Lectura/Escritura
    // Granularidad: 4KB, 32-bit
    gdt_set_gate(3, 0, 0xFFFFFFFF,
                 GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_CODE_DATA | 
                 GDT_ACCESS_EXECUTABLE | GDT_ACCESS_RW,
                 GDT_GRAN_4K | GDT_GRAN_32BIT | 0x0F);

    // Segmento de datos de usuario
    // Base = 0x00000000, Limite = 0xFFFFFFFF
    // Acceso: Presente, Ring 3, Código/Datos, No ejecutable, Lectura/Escritura
    // Granularidad: 4KB, 32-bit
    gdt_set_gate(4, 0, 0xFFFFFFFF,
                 GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_CODE_DATA | 
                 GDT_ACCESS_RW,
                 GDT_GRAN_4K | GDT_GRAN_32BIT | 0x0F);

    // Cargar la nueva GDT
    gdt_flush((uint32_t)&gdt_pointer);
}
