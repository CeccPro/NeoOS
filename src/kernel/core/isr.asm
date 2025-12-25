; isr.asm
; Interrupt Service Routines (ISRs)
; Copyright (C) 2025 CeccPro

[BITS 32]

; Macro para ISRs que NO empujan un código de error
%macro ISR_NOERRCODE 1
    global isr%1
    isr%1:
        cli                  ; Deshabilitar interrupciones
        push byte 0          ; Empujar un código de error dummy
        push byte %1         ; Empujar el número de interrupción
        jmp isr_common_stub
%endmacro

; Macro para ISRs que SÍ empujan un código de error
%macro ISR_ERRCODE 1
    global isr%1
    isr%1:
        cli                  ; Deshabilitar interrupciones
        push byte %1         ; Empujar el número de interrupción
        jmp isr_common_stub
%endmacro

; Macro para IRQs
%macro IRQ 2
    global irq%1
    irq%1:
        cli
        push byte 0          ; Código de error dummy
        push byte %2         ; Número de interrupción
        jmp isr_common_stub
%endmacro

; Excepciones del CPU (0-31)
ISR_NOERRCODE 0     ; Division By Zero
ISR_NOERRCODE 1     ; Debug
ISR_NOERRCODE 2     ; Non Maskable Interrupt
ISR_NOERRCODE 3     ; Breakpoint
ISR_NOERRCODE 4     ; Into Detected Overflow
ISR_NOERRCODE 5     ; Out of Bounds
ISR_NOERRCODE 6     ; Invalid Opcode
ISR_NOERRCODE 7     ; No Coprocessor
ISR_ERRCODE   8     ; Double Fault (con código de error)
ISR_NOERRCODE 9     ; Coprocessor Segment Overrun
ISR_ERRCODE   10    ; Bad TSS (con código de error)
ISR_ERRCODE   11    ; Segment Not Present (con código de error)
ISR_ERRCODE   12    ; Stack Fault (con código de error)
ISR_ERRCODE   13    ; General Protection Fault (con código de error)
ISR_ERRCODE   14    ; Page Fault (con código de error)
ISR_NOERRCODE 15    ; Reserved
ISR_NOERRCODE 16    ; Floating Point Exception
ISR_ERRCODE   17    ; Alignment Check (con código de error)
ISR_NOERRCODE 18    ; Machine Check
ISR_NOERRCODE 19    ; SIMD Floating-Point Exception
ISR_NOERRCODE 20    ; Reserved
ISR_NOERRCODE 21    ; Reserved
ISR_NOERRCODE 22    ; Reserved
ISR_NOERRCODE 23    ; Reserved
ISR_NOERRCODE 24    ; Reserved
ISR_NOERRCODE 25    ; Reserved
ISR_NOERRCODE 26    ; Reserved
ISR_NOERRCODE 27    ; Reserved
ISR_NOERRCODE 28    ; Reserved
ISR_NOERRCODE 29    ; Reserved
ISR_NOERRCODE 30    ; Reserved
ISR_NOERRCODE 31    ; Reserved

; IRQs (32-47)
IRQ 0, 32    ; Timer
IRQ 1, 33    ; Keyboard
IRQ 2, 34    ; Cascade
IRQ 3, 35    ; COM2
IRQ 4, 36    ; COM1
IRQ 5, 37    ; LPT2
IRQ 6, 38    ; Floppy
IRQ 7, 39    ; LPT1
IRQ 8, 40    ; RTC
IRQ 9, 41    ; Free
IRQ 10, 42   ; Free
IRQ 11, 43   ; Free
IRQ 12, 44   ; PS2 Mouse
IRQ 13, 45   ; FPU
IRQ 14, 46   ; Primary ATA
IRQ 15, 47   ; Secondary ATA

; Punto común para todos los ISRs
extern interrupt_handler

isr_common_stub:
    ; Guardar todos los registros
    pusha                ; Empuja edi, esi, ebp, esp, ebx, edx, ecx, eax
    
    mov ax, ds           ; Guardar el segmento de datos
    push eax
    
    mov ax, 0x10         ; Cargar el segmento de datos del kernel
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    push esp             ; Pasar el puntero a la estructura de registros como parámetro
    call interrupt_handler
    add esp, 4           ; Limpiar el parámetro del stack
    
    pop eax              ; Restaurar el segmento de datos original
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    popa                 ; Restaurar todos los registros
    add esp, 8           ; Limpiar el número de interrupción y el código de error
    sti                  ; Rehabilitar interrupciones
    iret                 ; Retornar de la interrupción

; Marcar la stack como no ejecutable
section .note.GNU-stack noalloc noexec nowrite progbits
