/**
 * NeoOS - IDT (Interrupt Descriptor Table)
 * Configuración de la tabla de descriptores de interrupciones
 */

#ifndef _KERNEL_IDT_H
#define _KERNEL_IDT_H

#include "../../lib/include/types.h"

// Número de entradas en la IDT (256 vectores de interrupción)
#define IDT_ENTRIES 256

/**
 * Estructura de una entrada IDT
 * Empaquetada para evitar padding del compilador
 */
struct idt_entry {
    uint16_t base_low;     // Base baja (bits 0-15) de la dirección del handler
    uint16_t selector;     // Selector de segmento de código
    uint8_t  always0;      // Siempre 0
    uint8_t  flags;        // Flags de tipo y atributos
    uint16_t base_high;    // Base alta (bits 16-31) de la dirección del handler
} __attribute__((packed));

typedef struct idt_entry idt_entry_t;

/**
 * Puntero a la IDT (IDTR)
 * Esta estructura se carga con la instrucción LIDT
 */
struct idt_ptr {
    uint16_t limit;   // Tamaño de la IDT - 1
    uint32_t base;    // Dirección base de la IDT
} __attribute__((packed));

typedef struct idt_ptr idt_ptr_t;

/**
 * Flags para entradas IDT
 */
#define IDT_FLAG_PRESENT     0x80  // Segmento presente
#define IDT_FLAG_RING_0      0x00  // Ring 0 (kernel)
#define IDT_FLAG_RING_3      0x60  // Ring 3 (usuario)
#define IDT_FLAG_GATE_32_INT 0x0E  // Puerta de interrupción de 32 bits
#define IDT_FLAG_GATE_32_TRAP 0x0F // Puerta de trampa de 32 bits

/**
 * Tipos de interrupciones
 */
#define IDT_TYPE_INTERRUPT   (IDT_FLAG_PRESENT | IDT_FLAG_RING_0 | IDT_FLAG_GATE_32_INT)
#define IDT_TYPE_TRAP        (IDT_FLAG_PRESENT | IDT_FLAG_RING_0 | IDT_FLAG_GATE_32_TRAP)

/**
 * Inicializar la IDT
 * Configura las 256 entradas de la tabla de interrupciones
 */
void idt_init(void);

/**
 * Establecer una entrada de la IDT
 * @param num: Número de la interrupción (0-255)
 * @param base: Dirección del handler
 * @param selector: Selector de segmento (generalmente GDT_KERNEL_CODE_SEGMENT)
 * @param flags: Flags de tipo y atributos
 */
void idt_set_gate(uint8_t num, uint32_t base, uint16_t selector, uint8_t flags);

/**
 * Función en assembly para cargar la IDT
 * Definida en idt_load.S
 */
extern void idt_flush(uint32_t idt_ptr);

#endif /* _KERNEL_IDT_H */
