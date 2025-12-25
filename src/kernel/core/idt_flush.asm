; idt_flush.asm
; Función para cargar la IDT
; Copyright (C) 2025 CeccPro

[BITS 32]

global idt_flush

idt_flush:
    mov eax, [esp + 4]   ; Obtener el puntero a la IDT del stack
    lidt [eax]           ; Cargar la IDT
    sti                  ; Habilitar interrupciones
    ret

; Marcar la stack como no ejecutable
section .note.GNU-stack noalloc noexec nowrite progbits
