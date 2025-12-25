; interrupt_handlers.asm
; Manejadores de interrupciones en ensamblador
;
; Copyright (C) 2025 CeccPro
;
; This program is free software: you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation, either version 3 of the License, or
; (at your option) any later version.
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with this program.  If not, see <https://www.gnu.org/licenses/>.

; Handler común en C
extern isr_handler

; Macro para ISRs que NO tienen código de error
%macro ISR_NOERRCODE 1
    global isr%1
    isr%1:
        cli                     ; Deshabilitar interrupciones
        push byte 0             ; Empujar un código de error dummy
        push byte %1            ; Empujar el número de interrupción
        jmp isr_common_stub     ; Saltar al handler común
%endmacro

; Macro para ISRs que SÍ tienen código de error
%macro ISR_ERRCODE 1
    global isr%1
    isr%1:
        cli                     ; Deshabilitar interrupciones
        push byte %1            ; Empujar el número de interrupción
        jmp isr_common_stub     ; Saltar al handler común
%endmacro

; Definir los ISRs (0-31 son excepciones del CPU)
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
ISR_NOERRCODE 16    ; Coprocessor Fault
ISR_ERRCODE   17    ; Alignment Check (con código de error)
ISR_NOERRCODE 18    ; Machine Check
ISR_NOERRCODE 19    ; Reserved
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

; Handler común que llama a la función en C
isr_common_stub:
    pusha               ; Empujar todos los registros de propósito general
    
    mov ax, ds          ; Guardar el segmento de datos
    push eax
    
    mov ax, 0x10        ; Cargar el segmento de datos del kernel
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    push esp            ; Empujar el puntero a la estructura de registros
    call isr_handler    ; Llamar al handler en C
    add esp, 4          ; Limpiar el stack
    
    pop eax             ; Restaurar el segmento de datos
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    popa                ; Restaurar los registros de propósito general
    add esp, 8          ; Limpiar el número de interrupción y código de error
    iret                ; Retornar de la interrupción

; Función para cargar la IDT
global idt_flush
idt_flush:
    mov eax, [esp + 4]  ; Obtener el puntero a la IDT
    lidt [eax]          ; Cargar la IDT
    ret

; Marcar la stack como no ejecutable
section .note.GNU-stack noalloc noexec nowrite progbits
