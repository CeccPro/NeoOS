/**
 * @file idt.h
 * @brief Interrupt Descriptor Table (IDT) para NeoOS
 * 
 * Implementación de la tabla de descriptores de interrupciones para
 * manejar interrupciones y excepciones en arquitectura x86.
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

// Estructura de una entrada de la IDT
struct idt_entry {
    uint16_t base_low;     // Los primeros 16 bits de la dirección del handler
    uint16_t selector;     // Selector de segmento de código
    uint8_t  always0;      // Siempre 0
    uint8_t  flags;        // Flags de tipo y atributos
    uint16_t base_high;    // Los últimos 16 bits de la dirección del handler
} __attribute__((packed));

// Puntero a la IDT
struct idt_ptr {
    uint16_t limit;        // Tamaño de la tabla - 1
    uint32_t base;         // Dirección de la primera entrada
} __attribute__((packed));

// Estructura de registros durante una interrupción
struct registers {
    uint32_t ds;                                      // Segmento de datos
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;  // Registros generales
    uint32_t int_no, err_code;                        // Número de interrupción y código de error
    uint32_t eip, cs, eflags, useresp, ss;            // Registros de la CPU
};

// Flags de la IDT
#define IDT_FLAG_PRESENT    0x80  // Presente en memoria
#define IDT_FLAG_RING0      0x00  // Ring 0 (kernel)
#define IDT_FLAG_RING3      0x60  // Ring 3 (userspace)
#define IDT_FLAG_GATE32     0x0E  // Puerta de interrupción de 32 bits
#define IDT_FLAG_TRAP32     0x0F  // Puerta de trampa de 32 bits

// Número total de entradas en la IDT
#define IDT_ENTRIES 256

// Excepciones del CPU (0-31)
#define EXC_DIVIDE_ERROR           0
#define EXC_DEBUG                  1
#define EXC_NMI                    2
#define EXC_BREAKPOINT             3
#define EXC_OVERFLOW               4
#define EXC_BOUND_RANGE            5
#define EXC_INVALID_OPCODE         6
#define EXC_DEVICE_NOT_AVAILABLE   7
#define EXC_DOUBLE_FAULT           8
#define EXC_COPROCESSOR_SEGMENT    9
#define EXC_INVALID_TSS           10
#define EXC_SEGMENT_NOT_PRESENT   11
#define EXC_STACK_FAULT           12
#define EXC_GENERAL_PROTECTION    13
#define EXC_PAGE_FAULT            14
#define EXC_RESERVED              15
#define EXC_FPU_ERROR             16
#define EXC_ALIGNMENT_CHECK       17
#define EXC_MACHINE_CHECK         18
#define EXC_SIMD_FP_EXCEPTION     19

// IRQs (32-47)
#define IRQ0  32
#define IRQ1  33
#define IRQ2  34
#define IRQ3  35
#define IRQ4  36
#define IRQ5  37
#define IRQ6  38
#define IRQ7  39
#define IRQ8  40
#define IRQ9  41
#define IRQ10 42
#define IRQ11 43
#define IRQ12 44
#define IRQ13 45
#define IRQ14 46
#define IRQ15 47

/**
 * @brief Inicializa la IDT con las entradas de excepciones e IRQs
 */
void idt_init(void);

/**
 * @brief Establece una entrada en la IDT
 * 
 * @param num Número de entrada (0-255)
 * @param base Dirección del handler de interrupción
 * @param selector Selector de segmento (normalmente 0x08 para código del kernel)
 * @param flags Flags de tipo y atributos
 */
void idt_set_gate(uint8_t num, uint32_t base, uint16_t selector, uint8_t flags);

/**
 * @brief Handler genérico de interrupciones
 * 
 * @param regs Estructura con los registros en el momento de la interrupción
 */
void interrupt_handler(struct registers* regs);

// Función en ensamblador para cargar la IDT
extern void idt_flush(uint32_t idt_ptr);

// Declaraciones de los ISRs (Interrupt Service Routines) en ensamblador
extern void isr0(void);
extern void isr1(void);
extern void isr2(void);
extern void isr3(void);
extern void isr4(void);
extern void isr5(void);
extern void isr6(void);
extern void isr7(void);
extern void isr8(void);
extern void isr9(void);
extern void isr10(void);
extern void isr11(void);
extern void isr12(void);
extern void isr13(void);
extern void isr14(void);
extern void isr15(void);
extern void isr16(void);
extern void isr17(void);
extern void isr18(void);
extern void isr19(void);
extern void isr20(void);
extern void isr21(void);
extern void isr22(void);
extern void isr23(void);
extern void isr24(void);
extern void isr25(void);
extern void isr26(void);
extern void isr27(void);
extern void isr28(void);
extern void isr29(void);
extern void isr30(void);
extern void isr31(void);

// IRQs
extern void irq0(void);
extern void irq1(void);
extern void irq2(void);
extern void irq3(void);
extern void irq4(void);
extern void irq5(void);
extern void irq6(void);
extern void irq7(void);
extern void irq8(void);
extern void irq9(void);
extern void irq10(void);
extern void irq11(void);
extern void irq12(void);
extern void irq13(void);
extern void irq14(void);
extern void irq15(void);

#endif // IDT_H
