/**
 * @file idt.h
 * @brief Interrupt Descriptor Table (IDT) para NeoOS
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

#ifndef IDT_H
#define IDT_H

#include <stdint.h>
#include "errors.h"

// Flags de tipo de gate
#define IDT_FLAG_PRESENT    0x80  // Presente en memoria
#define IDT_FLAG_RING0      0x00  // Ring 0 (kernel)
#define IDT_FLAG_RING3      0x60  // Ring 3 (usuario)
#define IDT_FLAG_GATE_32BIT 0x0E  // Gate de 32 bits
#define IDT_FLAG_TRAP       0x0F  // Trap gate (no deshabilita interrupciones)

// Número total de entradas en la IDT
#define IDT_ENTRIES 256

// Estructura de una entrada IDT
typedef struct {
    uint16_t base_low;       // Base 0-15
    uint16_t selector;       // Selector de segmento de código
    uint8_t  zero;           // Siempre 0
    uint8_t  flags;          // Flags de tipo y atributos
    uint16_t base_high;      // Base 16-31
} __attribute__((packed)) idt_entry_t;

// Estructura del puntero IDT que se carga en el registro IDTR
typedef struct {
    uint16_t limit;          // Tamaño de la IDT - 1
    uint32_t base;           // Dirección base de la IDT
} __attribute__((packed)) idt_ptr_t;

// Estructura de los registros cuando ocurre una interrupción
typedef struct {
    uint32_t ds;                                      // Segmento de datos
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;  // Registros guardados por pusha
    uint32_t int_no, err_code;                        // Número de interrupción y código de error
    uint32_t eip, cs, eflags, useresp, ss;            // Guardados automáticamente por el CPU
} __attribute__((packed)) registers_t;

// Tipo de handler de interrupción
typedef void (*interrupt_handler_t)(registers_t* regs);

/**
 * @brief Inicializa la Interrupt Descriptor Table
 * @return E_OK si fue exitoso, código de error en caso contrario
 */
error_t idt_init(void);

/**
 * @brief Establece una entrada en la IDT
 * @param num Número de interrupción (0-255)
 * @param base Dirección de la función handler
 * @param selector Selector de segmento de código (normalmente 0x08)
 * @param flags Flags de tipo y atributos
 */
void idt_set_gate(uint8_t num, uint32_t base, uint16_t selector, uint8_t flags);

/**
 * @brief Registra un handler personalizado para una interrupción
 * @param num Número de interrupción
 * @param handler Función handler
 */
void idt_register_handler(uint8_t num, interrupt_handler_t handler);

/**
 * @brief Habilita las interrupciones (STI)
 */
void interrupts_enable(void);

/**
 * @brief Deshabilita las interrupciones (CLI)
 */
void interrupts_disable(void);

#endif // IDT_H
