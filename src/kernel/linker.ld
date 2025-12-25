/* Linker script para el kernel de NeoOS */

ENTRY(_start)

SECTIONS
{
    /* El kernel comienza en 1 MiB */
    . = 1M;

    /* Sección de código */
    .text ALIGN(4K) : {
        *(.multiboot)
        *(.text)
    }

    /* Sección de datos de solo lectura */
    .rodata ALIGN(4K) : {
        *(.rodata)
    }

    /* Sección de datos inicializados */
    .data ALIGN(4K) : {
        *(.data)
    }

    /* Sección de datos no inicializados (BSS) */
    .bss ALIGN(4K) : {
        *(COMMON)
        *(.bss)
    }

    /* Descartar secciones innecesarias */
    /DISCARD/ : {
        *(.comment)
        *(.eh_frame)
    }
}
