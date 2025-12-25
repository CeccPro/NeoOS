; NeoOS Bootloader
; Multiboot Header y punto de entrada del kernel
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

MAGIC       equ 0x1BADB002              ; Multiboot magic number
FLAGS       equ 0x00000003              ; Module-aligned + Memory info
CHECKSUM    equ -(MAGIC + FLAGS)        ; Checksum

section .multiboot
align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

section .bss
align 16
stack_bottom:
    resb 16384                          ; 16 KB stack
stack_top:

section .data
boot_msg:
    db "NeoOS Kernel Starting...", 0

section .text
global _start
extern kernel_main

_start:
    ; Configurar el stack
    mov esp, stack_top
    mov ebp, esp

    ; Guardar información de multiboot
    ; EBX contiene la dirección de la estructura multiboot_info
    ; EAX contiene el magic number de multiboot
    push ebx                            ; multiboot_info pointer
    push eax                            ; multiboot magic

    ; Llamar a la función principal del kernel en C
    call kernel_main

    ; Si kernel_main retorna (no debería), detener el sistema
    cli
.hang:
    hlt
    jmp .hang

global outb
outb:
    mov dx, [esp + 4]                   ; Puerto
    mov al, [esp + 8]                   ; Valor
    out dx, al
    ret

global inb
inb:
    mov dx, [esp + 4]                   ; Puerto
    in al, dx
    xor eax, eax
    mov al, al
    ret

; Marcar la stack como no ejecutable
section .note.GNU-stack noalloc noexec nowrite progbits
