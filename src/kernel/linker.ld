/**
 * NeoOS Linker Script
 * Define el layout de memoria del kernel
 */

/* Formato de salida */
OUTPUT_FORMAT(elf32-i386)
OUTPUT_ARCH(i386)

/* Entry point del kernel */
ENTRY(_start)

/* Definir las secciones de memoria */
SECTIONS
{
    /* El kernel será cargado en 1MB de memoria física */
    . = 0x00100000;

    /* Inicio del kernel */
    __kernel_start = .;

    /* Sección .multiboot - DEBE estar al inicio del binario */
    .boot : {
        *(.multiboot)
        *(.text)
    }

    /* Sección .rodata - Datos de solo lectura */
    .rodata ALIGN(4096) : {
        *(.rodata)
        *(.rodata.*)
    }

    /* Sección .data - Datos inicializados */
    .data ALIGN(4096) : {
        *(.data)
        *(.data.*)
    }

    /* Sección .bss - Datos no inicializados */
    .bss ALIGN(4096) : {
        *(COMMON)
        *(.bss)
        *(.bss.*)
    }

    /* Fin del kernel */
    __kernel_end = .;
    __kernel_size = __kernel_end - __kernel_start;
}
