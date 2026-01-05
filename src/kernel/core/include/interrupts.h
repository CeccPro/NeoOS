/**
 * NeoOS - Interrupts
 * Sistema de manejo de interrupciones (ISR e IRQ)
 */

#ifndef _KERNEL_INTERRUPTS_H
#define _KERNEL_INTERRUPTS_H

#include "../../lib/include/types.h"

/**
 * Estructura de los registros guardados durante una interrupción
 * Esta estructura se empuja en el stack antes de llamar al handler
 */
struct registers {
    uint32_t ds;                                     // Segmento de datos
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // Registros generales (pusha)
    uint32_t int_no, err_code;                       // Número de interrupción y código de error
    uint32_t eip, cs, eflags, useresp, ss;          // Empujado automáticamente por el CPU
} __attribute__((packed));

typedef struct registers registers_t;

/**
 * Tipo de función handler de interrupciones
 * Los handlers reciben un puntero a los registros guardados
 */
typedef void (*isr_handler_t)(registers_t*);

/**
 * Números de las excepciones del CPU (ISR 0-31)
 */
#define ISR_DIVIDE_BY_ZERO          0   // División por cero
#define ISR_DEBUG                   1   // Debug
#define ISR_NON_MASKABLE_INT        2   // Interrupción no enmascarable
#define ISR_BREAKPOINT              3   // Breakpoint
#define ISR_OVERFLOW                4   // Overflow
#define ISR_BOUND_RANGE_EXCEEDED    5   // Rango de límites excedido
#define ISR_INVALID_OPCODE          6   // Opcode inválido
#define ISR_DEVICE_NOT_AVAILABLE    7   // Dispositivo no disponible
#define ISR_DOUBLE_FAULT            8   // Fallo doble
#define ISR_COPROCESSOR_SEGMENT     9   // Segmento de coprocesador (obsoleto)
#define ISR_INVALID_TSS             10  // TSS inválido
#define ISR_SEGMENT_NOT_PRESENT     11  // Segmento no presente
#define ISR_STACK_SEGMENT_FAULT     12  // Fallo de segmento de pila
#define ISR_GENERAL_PROTECTION      13  // Fallo de protección general
#define ISR_PAGE_FAULT              14  // Fallo de página
#define ISR_RESERVED_15             15  // Reservado
#define ISR_FPU_EXCEPTION           16  // Excepción de punto flotante
#define ISR_ALIGNMENT_CHECK         17  // Verificación de alineación
#define ISR_MACHINE_CHECK           18  // Verificación de máquina
#define ISR_SIMD_EXCEPTION          19  // Excepción SIMD
#define ISR_VIRTUALIZATION          20  // Virtualización
#define ISR_RESERVED_21             21  // Reservado
#define ISR_RESERVED_22             22  // Reservado
#define ISR_RESERVED_23             23  // Reservado
#define ISR_RESERVED_24             24  // Reservado
#define ISR_RESERVED_25             25  // Reservado
#define ISR_RESERVED_26             26  // Reservado
#define ISR_RESERVED_27             27  // Reservado
#define ISR_RESERVED_28             28  // Reservado
#define ISR_RESERVED_29             29  // Reservado
#define ISR_SECURITY_EXCEPTION      30  // Excepción de seguridad
#define ISR_RESERVED_31             31  // Reservado

/**
 * Números de las IRQs de hardware (IRQ 0-15)
 * Mapeadas a ISR 32-47 después de remapear el PIC
 */
#define IRQ0  32  // Timer del sistema (PIT)
#define IRQ1  33  // Teclado
#define IRQ2  34  // Cascada para el PIC esclavo
#define IRQ3  35  // Puerto serial 2 (COM2)
#define IRQ4  36  // Puerto serial 1 (COM1)
#define IRQ5  37  // Puerto paralelo 2 o tarjeta de sonido
#define IRQ6  38  // Controlador de disquete
#define IRQ7  39  // Puerto paralelo 1
#define IRQ8  40  // Reloj de tiempo real (RTC)
#define IRQ9  41  // Periféricos ACPI
#define IRQ10 42  // Disponible
#define IRQ11 43  // Disponible
#define IRQ12 44  // Mouse PS/2
#define IRQ13 45  // Coprocesador matemático
#define IRQ14 46  // Controlador IDE primario
#define IRQ15 47  // Controlador IDE secundario

/**
 * Puertos del PIC (Programmable Interrupt Controller)
 */
#define PIC1_COMMAND 0x20  // Puerto de comandos del PIC maestro
#define PIC1_DATA    0x21  // Puerto de datos del PIC maestro
#define PIC2_COMMAND 0xA0  // Puerto de comandos del PIC esclavo
#define PIC2_DATA    0xA1  // Puerto de datos del PIC esclavo

/**
 * Comandos del PIC
 */
#define PIC_EOI      0x20  // End of Interrupt
#define ICW1_ICW4    0x01  // ICW4 necesario
#define ICW1_INIT    0x10  // Comando de inicialización
#define ICW4_8086    0x01  // Modo 8086

/**
 * Inicializar el sistema de interrupciones
 * Configura los ISR, remapea el PIC e inicializa los handlers
 * @param verbose: Si es true, muestra mensajes de debug
 */
void interrupts_init(bool verbose);

/**
 * Registrar un handler para una interrupción específica
 * @param num: Número de la interrupción (0-255)
 * @param handler: Función handler a llamar
 */
void interrupts_register_handler(uint8_t num, isr_handler_t handler);

/**
 * Handlers de excepciones (ISR 0-31)
 * Definidos en arch/x86/isr.S
 */
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
 * Handlers de IRQs (IRQ 0-15)
 * Definidos en arch/x86/isr.S
 */
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

#endif /* _KERNEL_INTERRUPTS_H */
