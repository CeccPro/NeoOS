/**
 * @file idt.c
 * @brief Implementación de la Interrupt Descriptor Table (IDT)
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
#include "../include/kernel.h"
#include "../include/vga.h"
#include "../include/timer.h"

// Array de entradas de la IDT
static struct idt_entry idt_entries[IDT_ENTRIES];
static struct idt_ptr idt_pointer;

// Nombres de las excepciones
static const char* exception_messages[] = {
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
    "SIMD Floating-Point Exception",
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

/**
 * @brief Establece una entrada en la IDT
 */
void idt_set_gate(uint8_t num, uint32_t base, uint16_t selector, uint8_t flags) {
    idt_entries[num].base_low  = base & 0xFFFF;
    idt_entries[num].base_high = (base >> 16) & 0xFFFF;

    idt_entries[num].selector = selector;
    idt_entries[num].always0  = 0;
    idt_entries[num].flags    = flags;
}

/**
 * @brief Remapea el PIC (Programmable Interrupt Controller)
 * 
 * Por defecto, el PIC mapea IRQ 0-7 a interrupciones 8-15 e IRQ 8-15 a 0x70-0x77.
 * Esto interfiere con las excepciones del CPU, por lo que remapeamos:
 * - IRQ 0-7  a interrupciones 32-39
 * - IRQ 8-15 a interrupciones 40-47
 */
static void pic_remap(void) {
    // Guardar las máscaras actuales
    uint8_t mask1 = inb(0x21);
    uint8_t mask2 = inb(0xA1);

    // Iniciar secuencia de inicialización del PIC
    outb(0x20, 0x11);  // ICW1: Inicializar PIC1
    outb(0xA0, 0x11);  // ICW1: Inicializar PIC2

    // ICW2: Offset de vectores de interrupción
    outb(0x21, 0x20);  // PIC1: IRQ 0-7  -> interrupciones 32-39
    outb(0xA1, 0x28);  // PIC2: IRQ 8-15 -> interrupciones 40-47

    // ICW3: Configurar cascada
    outb(0x21, 0x04);  // PIC1: hay un esclavo en IRQ2
    outb(0xA1, 0x02);  // PIC2: identificador de cascada

    // ICW4: Modo 8086
    outb(0x21, 0x01);
    outb(0xA1, 0x01);

    // Restaurar máscaras
    outb(0x21, mask1);
    outb(0xA1, mask2);
}

/**
 * @brief Inicializa la IDT
 */
void idt_init(void) {
    idt_pointer.limit = sizeof(struct idt_entry) * IDT_ENTRIES - 1;
    idt_pointer.base  = (uint32_t)&idt_entries;

    // Inicializar todas las entradas a 0
    for (int i = 0; i < IDT_ENTRIES; i++) {
        idt_entries[i].base_low  = 0;
        idt_entries[i].base_high = 0;
        idt_entries[i].selector  = 0;
        idt_entries[i].always0   = 0;
        idt_entries[i].flags     = 0;
    }

    // Remapear el PIC
    pic_remap();

    // Instalar los ISRs (Interrupt Service Routines) para las excepciones del CPU
    idt_set_gate(0,  (uint32_t)isr0,  0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE32);
    idt_set_gate(1,  (uint32_t)isr1,  0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE32);
    idt_set_gate(2,  (uint32_t)isr2,  0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE32);
    idt_set_gate(3,  (uint32_t)isr3,  0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE32);
    idt_set_gate(4,  (uint32_t)isr4,  0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE32);
    idt_set_gate(5,  (uint32_t)isr5,  0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE32);
    idt_set_gate(6,  (uint32_t)isr6,  0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE32);
    idt_set_gate(7,  (uint32_t)isr7,  0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE32);
    idt_set_gate(8,  (uint32_t)isr8,  0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE32);
    idt_set_gate(9,  (uint32_t)isr9,  0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE32);
    idt_set_gate(10, (uint32_t)isr10, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE32);
    idt_set_gate(11, (uint32_t)isr11, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE32);
    idt_set_gate(12, (uint32_t)isr12, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE32);
    idt_set_gate(13, (uint32_t)isr13, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE32);
    idt_set_gate(14, (uint32_t)isr14, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE32);
    idt_set_gate(15, (uint32_t)isr15, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE32);
    idt_set_gate(16, (uint32_t)isr16, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE32);
    idt_set_gate(17, (uint32_t)isr17, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE32);
    idt_set_gate(18, (uint32_t)isr18, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE32);
    idt_set_gate(19, (uint32_t)isr19, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE32);
    idt_set_gate(20, (uint32_t)isr20, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE32);
    idt_set_gate(21, (uint32_t)isr21, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE32);
    idt_set_gate(22, (uint32_t)isr22, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE32);
    idt_set_gate(23, (uint32_t)isr23, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE32);
    idt_set_gate(24, (uint32_t)isr24, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE32);
    idt_set_gate(25, (uint32_t)isr25, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE32);
    idt_set_gate(26, (uint32_t)isr26, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE32);
    idt_set_gate(27, (uint32_t)isr27, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE32);
    idt_set_gate(28, (uint32_t)isr28, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE32);
    idt_set_gate(29, (uint32_t)isr29, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE32);
    idt_set_gate(30, (uint32_t)isr30, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE32);
    idt_set_gate(31, (uint32_t)isr31, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE32);

    // Instalar los IRQs
    idt_set_gate(32, (uint32_t)irq0,  0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE32);
    idt_set_gate(33, (uint32_t)irq1,  0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE32);
    idt_set_gate(34, (uint32_t)irq2,  0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE32);
    idt_set_gate(35, (uint32_t)irq3,  0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE32);
    idt_set_gate(36, (uint32_t)irq4,  0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE32);
    idt_set_gate(37, (uint32_t)irq5,  0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE32);
    idt_set_gate(38, (uint32_t)irq6,  0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE32);
    idt_set_gate(39, (uint32_t)irq7,  0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE32);
    idt_set_gate(40, (uint32_t)irq8,  0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE32);
    idt_set_gate(41, (uint32_t)irq9,  0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE32);
    idt_set_gate(42, (uint32_t)irq10, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE32);
    idt_set_gate(43, (uint32_t)irq11, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE32);
    idt_set_gate(44, (uint32_t)irq12, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE32);
    idt_set_gate(45, (uint32_t)irq13, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE32);
    idt_set_gate(46, (uint32_t)irq14, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE32);
    idt_set_gate(47, (uint32_t)irq15, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE32);

    // Cargar la nueva IDT
    idt_flush((uint32_t)&idt_pointer);
}

/**
 * @brief Handler genérico de interrupciones
 */
void interrupt_handler(struct registers* regs) {
    // Manejar excepciones del CPU (0-31)
    if (regs->int_no < 32) {
        vga_setcolor(VGA_COLOR_LIGHT_RED);
        vga_puts("\n\n*** EXCEPTION: ");
        vga_puts(exception_messages[regs->int_no]);
        vga_puts(" ***\n");
        
        vga_puts("Error code: ");
        vga_puthex(regs->err_code);
        vga_puts("\n");
        
        vga_puts("EIP: ");
        vga_puthex(regs->eip);
        vga_puts("\n");
        
        vga_setcolor(VGA_COLOR_LIGHT_GREY);
        
        // Detener el sistema en caso de excepción
        for (;;) {
            asm volatile("cli; hlt");
        }
    }
    
    // Manejar IRQs (32-47)
    if (regs->int_no >= 32 && regs->int_no <= 47) {
        // Manejar IRQ específicos
        if (regs->int_no == 32) {
            // IRQ0 - Timer
            timer_handler();
        }
        
        // Enviar EOI (End Of Interrupt) al PIC
        if (regs->int_no >= 40) {
            // Si el IRQ vino del PIC esclavo, enviar EOI a ambos
            outb(0xA0, 0x20);
        }
        outb(0x20, 0x20);
    }
}
