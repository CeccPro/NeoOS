/**
 * NeoOS - ISR & IRQ Stubs
 * Código ensamblador para los handlers de interrupciones
 * 
 * Este archivo define los stubs para:
 * - 32 excepciones del CPU (ISR 0-31)
 * - 16 IRQs de hardware (IRQ 0-15, mapeadas a ISR 32-47)
 */

.section .text
.align 4

/**
 * Macro para crear un ISR sin código de error
 * El CPU no empuja un código de error, así que lo hacemos nosotros (0)
 * NOTA: El CPU ya deshabilita interrupciones automáticamente al entrar a una excepción
 */
.macro ISR_NOERRCODE num
.global isr\num
isr\num:
    pushl $0                # Empujar código de error dummy (0)
    pushl $\num             # Empujar número de interrupción
    jmp isr_common_stub     # Saltar al handler común
.endm

/**
 * Macro para crear un ISR con código de error
 * El CPU ya empuja el código de error automáticamente
 * NOTA: El CPU ya deshabilita interrupciones automáticamente al entrar a una excepción
 */
.macro ISR_ERRCODE num
.global isr\num
isr\num:
    pushl $\num             # Empujar número de interrupción
    jmp isr_common_stub     # Saltar al handler común
.endm

/**
 * Macro para crear un IRQ handler
 * Los IRQs se mapean a ISR 32-47
 */
.macro IRQ num, isr_num
.global irq\num
irq\num:
    cli                     # Deshabilitar interrupciones
    pushl $0                # Empujar código de error dummy (0)
    pushl $\isr_num         # Empujar número de interrupción (32 + num)
    jmp irq_common_stub     # Saltar al handler común de IRQ
.endm

# ==============================================================================
# ISR 0-31: Excepciones del CPU
# ==============================================================================

# 0-7: Excepciones sin código de error
ISR_NOERRCODE 0    # Division By Zero
ISR_NOERRCODE 1    # Debug
ISR_NOERRCODE 2    # Non Maskable Interrupt
ISR_NOERRCODE 3    # Breakpoint
ISR_NOERRCODE 4    # Overflow
ISR_NOERRCODE 5    # Bound Range Exceeded
ISR_NOERRCODE 6    # Invalid Opcode
ISR_NOERRCODE 7    # Device Not Available

# 8: Double Fault (con código de error)
ISR_ERRCODE 8

# 9: Coprocessor Segment Overrun (sin código de error)
ISR_NOERRCODE 9

# 10-14: Excepciones con código de error
ISR_ERRCODE 10     # Invalid TSS
ISR_ERRCODE 11     # Segment Not Present
ISR_ERRCODE 12     # Stack-Segment Fault
ISR_ERRCODE 13     # General Protection Fault
ISR_ERRCODE 14     # Page Fault

# 15-16: Sin código de error
ISR_NOERRCODE 15   # Reserved
ISR_NOERRCODE 16   # x87 FPU Error

# 17: Alignment Check (con código de error)
ISR_ERRCODE 17

# 18-20: Sin código de error
ISR_NOERRCODE 18   # Machine Check
ISR_NOERRCODE 19   # SIMD Floating-Point Exception
ISR_NOERRCODE 20   # Virtualization Exception

# 21-29: Reservadas (sin código de error)
ISR_NOERRCODE 21
ISR_NOERRCODE 22
ISR_NOERRCODE 23
ISR_NOERRCODE 24
ISR_NOERRCODE 25
ISR_NOERRCODE 26
ISR_NOERRCODE 27
ISR_NOERRCODE 28
ISR_NOERRCODE 29

# 30: Security Exception (con código de error)
ISR_ERRCODE 30

# 31: Reservada (sin código de error)
ISR_NOERRCODE 31

# ==============================================================================
# IRQ 0-15: Interrupciones de hardware (mapeadas a ISR 32-47)
# ==============================================================================

IRQ 0,  32   # System Timer (PIT)
IRQ 1,  33   # Keyboard
IRQ 2,  34   # Cascade (used internally by PICs)
IRQ 3,  35   # COM2
IRQ 4,  36   # COM1
IRQ 5,  37   # LPT2
IRQ 6,  38   # Floppy Disk
IRQ 7,  39   # LPT1
IRQ 8,  40   # CMOS Real-Time Clock
IRQ 9,  41   # Free for peripherals / legacy SCSI / NIC
IRQ 10, 42   # Free for peripherals / SCSI / NIC
IRQ 11, 43   # Free for peripherals / SCSI / NIC
IRQ 12, 44   # PS2 Mouse
IRQ 13, 45   # FPU / Coprocessor / Inter-processor
IRQ 14, 46   # Primary ATA Hard Disk
IRQ 15, 47   # Secondary ATA Hard Disk

# ==============================================================================
# Stub común para ISR
# ==============================================================================
isr_common_stub:
    # Guardar el estado del CPU
    pushal                  # Empuja EAX, ECX, EDX, EBX, ESP (original), EBP, ESI, EDI
    
    # Guardar el segmento de datos
    movw %ds, %ax
    pushl %eax
    
    # Cargar el segmento de datos del kernel
    movw $0x10, %ax         # GDT_KERNEL_DATA_SEGMENT = 0x10
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    
    # Llamar al handler de C
    # El puntero a la estructura registers_t está en ESP
    pushl %esp
    call isr_handler
    addl $4, %esp
    
    # Restaurar el segmento de datos original
    popl %eax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    
    # Restaurar el estado del CPU
    popal                   # Pop EDI, ESI, EBP, ESP, EBX, EDX, ECX, EAX
    
    # Limpiar el número de interrupción y código de error empujados
    addl $8, %esp
    
    # Retornar de la interrupción
    # NOTA: iret restaura CS, EIP y EFLAGS (incluyendo IF) automáticamente
    iret

# ==============================================================================
# Stub común para IRQ
# ==============================================================================
irq_common_stub:
    # Guardar el estado del CPU
    pushal                  # Empuja EAX, ECX, EDX, EBX, ESP (original), EBP, ESI, EDI
    
    # Guardar el segmento de datos
    movw %ds, %ax
    pushl %eax
    
    # Cargar el segmento de datos del kernel
    movw $0x10, %ax         # GDT_KERNEL_DATA_SEGMENT = 0x10
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    
    # Llamar al handler de C
    # El puntero a la estructura registers_t está en ESP
    pushl %esp
    call irq_handler
    addl $4, %esp
    
    # Restaurar el segmento de datos original
    popl %eax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    
    # Restaurar el estado del CPU
    popal                   # Pop EDI, ESI, EBP, ESP, EBX, EDX, ECX, EAX
    
    # Limpiar el número de interrupción y código de error empujados
    addl $8, %esp
    
    # Retornar de la interrupción
    # NOTA: EOI ya fue enviado en irq_handler()
    # NOTA: iret restaura CS, EIP y EFLAGS (incluyendo IF) automáticamente
    iret

# ==============================================================================
# Funciones para cargar GDT e IDT
# ==============================================================================

.global gdt_flush
gdt_flush:
    movl 4(%esp), %eax      # Obtener el puntero a gdt_ptr desde el stack
    lgdt (%eax)             # Cargar la GDT
    
    # Recargar los selectores de segmento
    movw $0x10, %ax         # 0x10 es el offset del segmento de datos del kernel
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss
    
    # Far jump para recargar CS (segmento de código)
    # 0x08 es el offset del segmento de código del kernel
    ljmp $0x08, $.gdt_flush_done
    
.gdt_flush_done:
    ret

.global idt_flush
idt_flush:
    movl 4(%esp), %eax      # Obtener el puntero a idt_ptr desde el stack
    lidt (%eax)             # Cargar la IDT
    ret
