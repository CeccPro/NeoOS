/**
 * @file idt.c
 * @brief Implementación de la Interrupt Descriptor Table
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

#include "../include/idt.h"
#include "../include/gdt.h"
#include "../include/kernel.h"
#include "../include/vga.h"

// Array de entradas IDT
static idt_entry_t idt_entries[IDT_ENTRIES];
static idt_ptr_t idt_ptr;

// Array de handlers personalizados
static interrupt_handler_t interrupt_handlers[IDT_ENTRIES];

// Función externa en ensamblador que carga la IDT
extern void idt_flush(uint32_t);

// ISRs (Interrupt Service Routines) declaradas en interrupt_handlers.asm
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

/**
 * @brief Establece una entrada en la IDT
 */
void idt_set_gate(uint8_t num, uint32_t base, uint16_t selector, uint8_t flags) {
    idt_entries[num].base_low = base & 0xFFFF;
    idt_entries[num].base_high = (base >> 16) & 0xFFFF;
    idt_entries[num].selector = selector;
    idt_entries[num].zero = 0;
    idt_entries[num].flags = flags;
}

/**
 * @brief Inicializa la Interrupt Descriptor Table
 */
error_t idt_init(void) {
    idt_ptr.limit = (sizeof(idt_entry_t) * IDT_ENTRIES) - 1;
    idt_ptr.base = (uint32_t)&idt_entries;

    // Limpiar la IDT
    for (int i = 0; i < IDT_ENTRIES; i++) {
        idt_entries[i].base_low = 0;
        idt_entries[i].base_high = 0;
        idt_entries[i].selector = 0;
        idt_entries[i].zero = 0;
        idt_entries[i].flags = 0;
        interrupt_handlers[i] = NULL;
    }

    // Configurar los ISRs de excepciones del CPU (0-31)
    idt_set_gate(0, (uint32_t)isr0, GDT_KERNEL_CODE, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32BIT);
    idt_set_gate(1, (uint32_t)isr1, GDT_KERNEL_CODE, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32BIT);
    idt_set_gate(2, (uint32_t)isr2, GDT_KERNEL_CODE, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32BIT);
    idt_set_gate(3, (uint32_t)isr3, GDT_KERNEL_CODE, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32BIT);
    idt_set_gate(4, (uint32_t)isr4, GDT_KERNEL_CODE, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32BIT);
    idt_set_gate(5, (uint32_t)isr5, GDT_KERNEL_CODE, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32BIT);
    idt_set_gate(6, (uint32_t)isr6, GDT_KERNEL_CODE, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32BIT);
    idt_set_gate(7, (uint32_t)isr7, GDT_KERNEL_CODE, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32BIT);
    idt_set_gate(8, (uint32_t)isr8, GDT_KERNEL_CODE, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32BIT);
    idt_set_gate(9, (uint32_t)isr9, GDT_KERNEL_CODE, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32BIT);
    idt_set_gate(10, (uint32_t)isr10, GDT_KERNEL_CODE, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32BIT);
    idt_set_gate(11, (uint32_t)isr11, GDT_KERNEL_CODE, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32BIT);
    idt_set_gate(12, (uint32_t)isr12, GDT_KERNEL_CODE, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32BIT);
    idt_set_gate(13, (uint32_t)isr13, GDT_KERNEL_CODE, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32BIT);
    idt_set_gate(14, (uint32_t)isr14, GDT_KERNEL_CODE, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32BIT);
    idt_set_gate(15, (uint32_t)isr15, GDT_KERNEL_CODE, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32BIT);
    idt_set_gate(16, (uint32_t)isr16, GDT_KERNEL_CODE, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32BIT);
    idt_set_gate(17, (uint32_t)isr17, GDT_KERNEL_CODE, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32BIT);
    idt_set_gate(18, (uint32_t)isr18, GDT_KERNEL_CODE, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32BIT);
    idt_set_gate(19, (uint32_t)isr19, GDT_KERNEL_CODE, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32BIT);
    idt_set_gate(20, (uint32_t)isr20, GDT_KERNEL_CODE, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32BIT);
    idt_set_gate(21, (uint32_t)isr21, GDT_KERNEL_CODE, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32BIT);
    idt_set_gate(22, (uint32_t)isr22, GDT_KERNEL_CODE, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32BIT);
    idt_set_gate(23, (uint32_t)isr23, GDT_KERNEL_CODE, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32BIT);
    idt_set_gate(24, (uint32_t)isr24, GDT_KERNEL_CODE, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32BIT);
    idt_set_gate(25, (uint32_t)isr25, GDT_KERNEL_CODE, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32BIT);
    idt_set_gate(26, (uint32_t)isr26, GDT_KERNEL_CODE, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32BIT);
    idt_set_gate(27, (uint32_t)isr27, GDT_KERNEL_CODE, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32BIT);
    idt_set_gate(28, (uint32_t)isr28, GDT_KERNEL_CODE, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32BIT);
    idt_set_gate(29, (uint32_t)isr29, GDT_KERNEL_CODE, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32BIT);
    idt_set_gate(30, (uint32_t)isr30, GDT_KERNEL_CODE, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32BIT);
    idt_set_gate(31, (uint32_t)isr31, GDT_KERNEL_CODE, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32BIT);

    // Cargar la IDT
    idt_flush((uint32_t)&idt_ptr);

    return E_OK;
}

/**
 * @brief Registra un handler personalizado para una interrupción
 */
void idt_register_handler(uint8_t num, interrupt_handler_t handler) {
    interrupt_handlers[num] = handler;
}

/**
 * @brief Handler común para todas las interrupciones
 * Esta función es llamada desde interrupt_handlers.asm
 */
void isr_handler(registers_t* regs) {
    // Si hay un handler personalizado registrado, llamarlo
    if (interrupt_handlers[regs->int_no] != NULL) {
        interrupt_handlers[regs->int_no](regs);
    } else {
        // Si no hay handler, mostrar información de la excepción
        vga_puts("\n!!! EXCEPTION !!!\n");
        vga_puts("Exception: ");
        vga_put_dec(regs->int_no);
        vga_puts(" (");
        
        // Nombres de las excepciones
        const char* exception_messages[] = {
            "Division By Zero",
            "Debug",
            "Non Maskable Interrupt",
            "Breakpoint",
            "Into Detected Overflow",
            "Out of Bounds",
            "Invalid Opcode",
            "No Coprocessor",
            "Double Fault",
            "Coprocessor Segment Overrun",
            "Bad TSS",
            "Segment Not Present",
            "Stack Fault",
            "General Protection Fault",
            "Page Fault",
            "Unknown Interrupt",
            "Coprocessor Fault",
            "Alignment Check",
            "Machine Check",
            "Reserved",
            "Reserved",
            "Reserved",
            "Reserved",
            "Reserved",
            "Reserved",
            "Reserved",
            "Reserved",
            "Reserved",
            "Reserved",
            "Reserved",
            "Reserved",
            "Reserved"
        };
        
        if (regs->int_no < 32) {
            vga_puts(exception_messages[regs->int_no]);
        }
        
        vga_puts(")\nError code: ");
        vga_put_hex(regs->err_code);
        vga_puts("\nEIP: ");
        vga_put_hex(regs->eip);
        vga_puts("\n");
        
        // Detener el sistema
        vga_puts("System halted.\n");
        while(1) {
            asm volatile("hlt");
        }
    }
}

/**
 * @brief Habilita las interrupciones
 */
void interrupts_enable(void) {
    asm volatile("sti");
}

/**
 * @brief Deshabilita las interrupciones
 */
void interrupts_disable(void) {
    asm volatile("cli");
}
