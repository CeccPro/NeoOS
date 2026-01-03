/**
 * NeoOS - VGA Driver Header
 * Driver básico para modo texto VGA 80x25
 */

#ifndef _KERNEL_VGA_H
#define _KERNEL_VGA_H

#include "../../lib/include/types.h"

// Dimensiones de la pantalla VGA en modo texto
#define VGA_WIDTH  80
#define VGA_HEIGHT 25

// Colores VGA
enum vga_color {
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE = 1,
    VGA_COLOR_GREEN = 2,
    VGA_COLOR_CYAN = 3,
    VGA_COLOR_RED = 4,
    VGA_COLOR_MAGENTA = 5,
    VGA_COLOR_BROWN = 6,
    VGA_COLOR_LIGHT_GREY = 7,
    VGA_COLOR_DARK_GREY = 8,
    VGA_COLOR_LIGHT_BLUE = 9,
    VGA_COLOR_LIGHT_GREEN = 10,
    VGA_COLOR_LIGHT_CYAN = 11,
    VGA_COLOR_LIGHT_RED = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_LIGHT_BROWN = 14,
    VGA_COLOR_WHITE = 15,
};

/**
 * Inicializa el driver VGA
 */
void vga_init(void);

/**
 * Limpia la pantalla
 */
void vga_clear(void);

/**
 * Establece el color de texto
 */
void vga_set_color(enum vga_color fg, enum vga_color bg);

/**
 * Escribe un carácter en la posición actual
 */
void vga_putchar(char c);

/**
 * Escribe una cadena de texto
 */
void vga_write(const char* str);

/**
 * Escribe una cadena con longitud específica
 */
void vga_write_len(const char* str, size_t len);

/**
 * Escribe un número en formato hexadecimal
 */
void vga_write_hex(uint32_t value);

/**
 * Escribe un número en formato decimal
 */
void vga_write_dec(uint32_t value);

#endif /* _KERNEL_VGA_H */
