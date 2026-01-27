/**
 * NeoOS - Interrupts Implementation
 * Sistema de manejo de interrupciones (ISR e IRQ)
 */

#include "../../core/include/interrupts.h"
#include "../../core/include/idt.h"
#include "../../core/include/gdt.h"
#include "../../drivers/include/early_vga.h"
#include "../../lib/include/string.h"

// Array de handlers de interrupciones
static isr_handler_t interrupt_handlers[256];

// Nombres de las excepciones del CPU
static const char* exception_messages[] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack-Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "Reserved",
    "x87 FPU Error",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Security Exception",
    "Reserved"
};

/**
 * Remapear el PIC (Programmable Interrupt Controller)
 * Por defecto, el PIC mapea IRQ 0-7 a interrupciones 8-15,
 * que se superponen con las excepciones del CPU (0-31).
 * Las remapeamos a 32-47.
 */
static void pic_remap(void) {
    // Guardar las máscaras actuales
    uint8_t mask1, mask2;
    __asm__ volatile("inb %1, %0" : "=a"(mask1) : "Nd"(PIC1_DATA));
    __asm__ volatile("inb %1, %0" : "=a"(mask2) : "Nd"(PIC2_DATA));

    // Iniciar secuencia de inicialización (ICW1)
    __asm__ volatile("outb %0, %1" : : "a"((uint8_t)(ICW1_INIT | ICW1_ICW4)), "Nd"(PIC1_COMMAND));
    __asm__ volatile("outb %0, %1" : : "a"((uint8_t)(ICW1_INIT | ICW1_ICW4)), "Nd"(PIC2_COMMAND));

    // Establecer vectores de offset (ICW2)
    __asm__ volatile("outb %0, %1" : : "a"((uint8_t)32), "Nd"(PIC1_DATA)); // PIC1 offset: 32
    __asm__ volatile("outb %0, %1" : : "a"((uint8_t)40), "Nd"(PIC2_DATA)); // PIC2 offset: 40

    // Configurar cascada (ICW3)
    __asm__ volatile("outb %0, %1" : : "a"((uint8_t)4), "Nd"(PIC1_DATA)); // IRQ2 es cascada
    __asm__ volatile("outb %0, %1" : : "a"((uint8_t)2), "Nd"(PIC2_DATA)); // Identidad de cascada

    // Modo 8086 (ICW4)
    __asm__ volatile("outb %0, %1" : : "a"((uint8_t)ICW4_8086), "Nd"(PIC1_DATA));
    __asm__ volatile("outb %0, %1" : : "a"((uint8_t)ICW4_8086), "Nd"(PIC2_DATA));

    // Restaurar las máscaras
    __asm__ volatile("outb %0, %1" : : "a"(mask1), "Nd"(PIC1_DATA));
    __asm__ volatile("outb %0, %1" : : "a"(mask2), "Nd"(PIC2_DATA));
}

/**
 * Enviar End Of Interrupt (EOI) al PIC
 * @param irq_num: Número de la IRQ (32-47)
 */
static void pic_send_eoi(uint8_t irq_num) {
    // Si la IRQ vino del PIC esclavo, enviar EOI a ambos
    if (irq_num >= 40) {
        __asm__ volatile("outb %0, %1" : : "a"((uint8_t)PIC_EOI), "Nd"(PIC2_COMMAND));
    }
    
    // Enviar EOI al PIC maestro
    __asm__ volatile("outb %0, %1" : : "a"((uint8_t)PIC_EOI), "Nd"(PIC1_COMMAND));
}

/**
 * Handler común de ISR (Interrupt Service Routine)
 * Llamado desde el código ensamblador en isr.S
 * @param regs: Estructura con los registros guardados
 */
void isr_handler(registers_t* regs) {
    // Si hay un handler registrado para esta interrupción, llamarlo
    if (interrupt_handlers[regs->int_no] != 0) {
        isr_handler_t handler = interrupt_handlers[regs->int_no];
        handler(regs);
    } else {
        // Si no hay handler, mostrar mensaje de error
        vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_RED);
        vga_write("\n\n!!! KERNEL PANIC !!!\n");
        vga_write("Unhandled Exception: ");
        
        if (regs->int_no < 32) {
            vga_write(exception_messages[regs->int_no]);
        } else {
            vga_write("Unknown");
        }
        
        vga_write("\nException #");
        vga_write_dec(regs->int_no);
        vga_write(" Error Code: ");
        vga_write_hex(regs->err_code);
        vga_write("\n");
        vga_write("EIP: ");
        vga_write_hex(regs->eip);
        vga_write(" CS: ");
        vga_write_hex(regs->cs);
        vga_write(" EFLAGS: ");
        vga_write_hex(regs->eflags);
        vga_write("\n");
        vga_write("EAX: ");
        vga_write_hex(regs->eax);
        vga_write(" EBX: ");
        vga_write_hex(regs->ebx);
        vga_write(" ECX: ");
        vga_write_hex(regs->ecx);
        vga_write(" EDX: ");
        vga_write_hex(regs->edx);
        vga_write("\n");
        vga_write("ESI: ");
        vga_write_hex(regs->esi);
        vga_write(" EDI: ");
        vga_write_hex(regs->edi);
        vga_write(" EBP: ");
        vga_write_hex(regs->ebp);
        vga_write(" ESP: ");
        vga_write_hex(regs->esp);
        vga_write("\n");
        
        // Detener el kernel
        __asm__ volatile("cli; hlt");
        while(1);
    }
}

/**
 * Handler común de IRQ (Hardware Interrupt Request)
 * Llamado desde el código ensamblador en isr.S
 * @param regs: Estructura con los registros guardados
 */
void irq_handler(registers_t* regs) {
    // Enviar EOI al PIC
    pic_send_eoi(regs->int_no);

    // Si hay un handler registrado para esta IRQ, llamarlo
    if (interrupt_handlers[regs->int_no] != 0) {
        isr_handler_t handler = interrupt_handlers[regs->int_no];
        handler(regs);
    }
}

/**
 * Inicializar el sistema de interrupciones
 * @param verbose: Si es true, muestra mensajes de debug
 */
void interrupts_init(bool verbose) {
    if (verbose) {
        vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
        vga_write("[INT] Inicializando sistema de interrupciones...\n");
    }

    // Inicializar el array de handlers
    memset(&interrupt_handlers, 0, sizeof(isr_handler_t) * 256);

    // Remapear el PIC
    pic_remap();

    // Configurar las entradas IDT para las excepciones del CPU (ISR 0-31)
    idt_set_gate(0,  (uint32_t)isr0,  GDT_KERNEL_CODE_SEGMENT, IDT_TYPE_INTERRUPT);
    idt_set_gate(1,  (uint32_t)isr1,  GDT_KERNEL_CODE_SEGMENT, IDT_TYPE_INTERRUPT);
    idt_set_gate(2,  (uint32_t)isr2,  GDT_KERNEL_CODE_SEGMENT, IDT_TYPE_INTERRUPT);
    idt_set_gate(3,  (uint32_t)isr3,  GDT_KERNEL_CODE_SEGMENT, IDT_TYPE_INTERRUPT);
    idt_set_gate(4,  (uint32_t)isr4,  GDT_KERNEL_CODE_SEGMENT, IDT_TYPE_INTERRUPT);
    idt_set_gate(5,  (uint32_t)isr5,  GDT_KERNEL_CODE_SEGMENT, IDT_TYPE_INTERRUPT);
    idt_set_gate(6,  (uint32_t)isr6,  GDT_KERNEL_CODE_SEGMENT, IDT_TYPE_INTERRUPT);
    idt_set_gate(7,  (uint32_t)isr7,  GDT_KERNEL_CODE_SEGMENT, IDT_TYPE_INTERRUPT);
    idt_set_gate(8,  (uint32_t)isr8,  GDT_KERNEL_CODE_SEGMENT, IDT_TYPE_INTERRUPT);
    idt_set_gate(9,  (uint32_t)isr9,  GDT_KERNEL_CODE_SEGMENT, IDT_TYPE_INTERRUPT);
    idt_set_gate(10, (uint32_t)isr10, GDT_KERNEL_CODE_SEGMENT, IDT_TYPE_INTERRUPT);
    idt_set_gate(11, (uint32_t)isr11, GDT_KERNEL_CODE_SEGMENT, IDT_TYPE_INTERRUPT);
    idt_set_gate(12, (uint32_t)isr12, GDT_KERNEL_CODE_SEGMENT, IDT_TYPE_INTERRUPT);
    idt_set_gate(13, (uint32_t)isr13, GDT_KERNEL_CODE_SEGMENT, IDT_TYPE_INTERRUPT);
    idt_set_gate(14, (uint32_t)isr14, GDT_KERNEL_CODE_SEGMENT, IDT_TYPE_INTERRUPT);
    idt_set_gate(15, (uint32_t)isr15, GDT_KERNEL_CODE_SEGMENT, IDT_TYPE_INTERRUPT);
    idt_set_gate(16, (uint32_t)isr16, GDT_KERNEL_CODE_SEGMENT, IDT_TYPE_INTERRUPT);
    idt_set_gate(17, (uint32_t)isr17, GDT_KERNEL_CODE_SEGMENT, IDT_TYPE_INTERRUPT);
    idt_set_gate(18, (uint32_t)isr18, GDT_KERNEL_CODE_SEGMENT, IDT_TYPE_INTERRUPT);
    idt_set_gate(19, (uint32_t)isr19, GDT_KERNEL_CODE_SEGMENT, IDT_TYPE_INTERRUPT);
    idt_set_gate(20, (uint32_t)isr20, GDT_KERNEL_CODE_SEGMENT, IDT_TYPE_INTERRUPT);
    idt_set_gate(21, (uint32_t)isr21, GDT_KERNEL_CODE_SEGMENT, IDT_TYPE_INTERRUPT);
    idt_set_gate(22, (uint32_t)isr22, GDT_KERNEL_CODE_SEGMENT, IDT_TYPE_INTERRUPT);
    idt_set_gate(23, (uint32_t)isr23, GDT_KERNEL_CODE_SEGMENT, IDT_TYPE_INTERRUPT);
    idt_set_gate(24, (uint32_t)isr24, GDT_KERNEL_CODE_SEGMENT, IDT_TYPE_INTERRUPT);
    idt_set_gate(25, (uint32_t)isr25, GDT_KERNEL_CODE_SEGMENT, IDT_TYPE_INTERRUPT);
    idt_set_gate(26, (uint32_t)isr26, GDT_KERNEL_CODE_SEGMENT, IDT_TYPE_INTERRUPT);
    idt_set_gate(27, (uint32_t)isr27, GDT_KERNEL_CODE_SEGMENT, IDT_TYPE_INTERRUPT);
    idt_set_gate(28, (uint32_t)isr28, GDT_KERNEL_CODE_SEGMENT, IDT_TYPE_INTERRUPT);
    idt_set_gate(29, (uint32_t)isr29, GDT_KERNEL_CODE_SEGMENT, IDT_TYPE_INTERRUPT);
    idt_set_gate(30, (uint32_t)isr30, GDT_KERNEL_CODE_SEGMENT, IDT_TYPE_INTERRUPT);
    idt_set_gate(31, (uint32_t)isr31, GDT_KERNEL_CODE_SEGMENT, IDT_TYPE_INTERRUPT);

    // Configurar las entradas IDT para las IRQs de hardware (IRQ 0-15)
    idt_set_gate(32, (uint32_t)irq0,  GDT_KERNEL_CODE_SEGMENT, IDT_TYPE_INTERRUPT);
    idt_set_gate(33, (uint32_t)irq1,  GDT_KERNEL_CODE_SEGMENT, IDT_TYPE_INTERRUPT);
    idt_set_gate(34, (uint32_t)irq2,  GDT_KERNEL_CODE_SEGMENT, IDT_TYPE_INTERRUPT);
    idt_set_gate(35, (uint32_t)irq3,  GDT_KERNEL_CODE_SEGMENT, IDT_TYPE_INTERRUPT);
    idt_set_gate(36, (uint32_t)irq4,  GDT_KERNEL_CODE_SEGMENT, IDT_TYPE_INTERRUPT);
    idt_set_gate(37, (uint32_t)irq5,  GDT_KERNEL_CODE_SEGMENT, IDT_TYPE_INTERRUPT);
    idt_set_gate(38, (uint32_t)irq6,  GDT_KERNEL_CODE_SEGMENT, IDT_TYPE_INTERRUPT);
    idt_set_gate(39, (uint32_t)irq7,  GDT_KERNEL_CODE_SEGMENT, IDT_TYPE_INTERRUPT);
    idt_set_gate(40, (uint32_t)irq8,  GDT_KERNEL_CODE_SEGMENT, IDT_TYPE_INTERRUPT);
    idt_set_gate(41, (uint32_t)irq9,  GDT_KERNEL_CODE_SEGMENT, IDT_TYPE_INTERRUPT);
    idt_set_gate(42, (uint32_t)irq10, GDT_KERNEL_CODE_SEGMENT, IDT_TYPE_INTERRUPT);
    idt_set_gate(43, (uint32_t)irq11, GDT_KERNEL_CODE_SEGMENT, IDT_TYPE_INTERRUPT);
    idt_set_gate(44, (uint32_t)irq12, GDT_KERNEL_CODE_SEGMENT, IDT_TYPE_INTERRUPT);
    idt_set_gate(45, (uint32_t)irq13, GDT_KERNEL_CODE_SEGMENT, IDT_TYPE_INTERRUPT);
    idt_set_gate(46, (uint32_t)irq14, GDT_KERNEL_CODE_SEGMENT, IDT_TYPE_INTERRUPT);
    idt_set_gate(47, (uint32_t)irq15, GDT_KERNEL_CODE_SEGMENT, IDT_TYPE_INTERRUPT);

    if (verbose) {
        vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
        vga_write("[INT] Sistema de interrupciones inicializado\n");
    }

    // Habilitar interrupciones
    __asm__ volatile("sti");
}

/**
 * Registrar un handler para una interrupción específica
 * @param num: Número de la interrupción (0-255)
 * @param handler: Función handler a llamar
 */
void interrupts_register_handler(uint8_t num, isr_handler_t handler) {
    interrupt_handlers[num] = handler;
}
