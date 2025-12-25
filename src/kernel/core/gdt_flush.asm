; gdt_flush.asm
; Función para cargar la GDT
; Copyright (C) 2025 CeccPro

[BITS 32]

global gdt_flush

gdt_flush:
    mov eax, [esp + 4]   ; Obtener el puntero a la GDT del stack
    lgdt [eax]           ; Cargar la GDT
    
    mov ax, 0x10         ; 0x10 es el offset del segmento de datos del kernel en la GDT
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    
    jmp 0x08:.flush      ; 0x08 es el offset del segmento de código del kernel
                         ; Far jump para recargar CS
.flush:
    ret

; Marcar la stack como no ejecutable
section .note.GNU-stack noalloc noexec nowrite progbits
