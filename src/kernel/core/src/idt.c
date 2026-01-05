/**
 * NeoOS - IDT Implementation
 * Configuración de la tabla de descriptores de interrupciones
 */

#include "../../core/include/idt.h"
#include "../../core/include/gdt.h"
#include "../../lib/include/string.h"

// Array de entradas IDT
static idt_entry_t idt_entries[IDT_ENTRIES];

// Puntero a la IDT que se carga con LIDT
static idt_ptr_t idt_ptr;

/**
 * Establecer una entrada de la IDT
 * @param num: Número de la interrupción (0-255)
 * @param base: Dirección del handler
 * @param selector: Selector de segmento (generalmente GDT_KERNEL_CODE_SEGMENT)
 * @param flags: Flags de tipo y atributos
 */
void idt_set_gate(uint8_t num, uint32_t base, uint16_t selector, uint8_t flags) {
    idt_entries[num].base_low  = base & 0xFFFF;
    idt_entries[num].base_high = (base >> 16) & 0xFFFF;
    
    idt_entries[num].selector = selector;
    idt_entries[num].always0  = 0;
    idt_entries[num].flags    = flags;
}

/**
 * Inicializar la IDT
 * Configura las 256 entradas de la tabla de interrupciones
 */
void idt_init(void) {
    // Configurar el puntero a la IDT
    idt_ptr.limit = (sizeof(idt_entry_t) * IDT_ENTRIES) - 1;
    idt_ptr.base  = (uint32_t)&idt_entries;

    // Inicializar todas las entradas a 0
    memset(&idt_entries, 0, sizeof(idt_entry_t) * IDT_ENTRIES);

    // Las entradas se configurarán en interrupts_init()
    // que llamará a idt_set_gate() para cada ISR e IRQ

    // Cargar la IDT en el CPU
    idt_flush((uint32_t)&idt_ptr);
}
