; gdt_flush.asm
; Carga la GDT y actualiza los registros de segmento
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

global gdt_flush

gdt_flush:
    mov eax, [esp + 4]      ; Obtener el puntero a la GDT
    lgdt [eax]              ; Cargar la GDT

    ; Recargar los selectores de segmento de datos
    mov ax, 0x10            ; 0x10 es el offset del segmento de datos del kernel
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Far jump para recargar CS (segmento de código)
    ; Usamos la sintaxis correcta de NASM para far jump
    jmp 0x08:flush_cs

[BITS 32]
flush_cs:
    ret

; Marcar la stack como no ejecutable
section .note.GNU-stack noalloc noexec nowrite progbits
