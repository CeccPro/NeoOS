/**
 * @file vga.c
 * @brief Implementación del driver de terminal VGA en modo texto
 *
 * Copyright (C) 2025 CeccPro
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "../include/vga.h"
#include "../include/kernel.h"

// Buffer de video VGA
static uint16_t* vga_buffer = (uint16_t*)0xB8000;

// Estado del terminal
static size_t terminal_row = 0;
static size_t terminal_column = 0;
static uint8_t terminal_color = 0;

/**
 * @brief Crea un color VGA combinando foreground y background
 */
static inline uint8_t vga_entry_color(vga_color_t fg, vga_color_t bg) {
    return fg | bg << 4;
}

/**
 * @brief Crea una entrada VGA combinando carácter y color
 */
static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t)uc | (uint16_t)color << 8;
}

/**
 * @brief Calcula la longitud de una cadena
 */
static size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len])
        len++;
    return len;
}

/**
 * @brief Inicializa el terminal VGA
 */
void vga_init(void) {
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_clear();
}

/**
 * @brief Limpia la pantalla
 */
void vga_clear(void) {
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            vga_buffer[index] = vga_entry(' ', terminal_color);
        }
    }
    terminal_row = 0;
    terminal_column = 0;
}

/**
 * @brief Establece el color de texto
 */
void vga_set_color(vga_color_t fg, vga_color_t bg) {
    terminal_color = vga_entry_color(fg, bg);
}

/**
 * @brief Escribe un carácter en una posición específica
 */
static void vga_putentryat(char c, uint8_t color, size_t x, size_t y) {
    const size_t index = y * VGA_WIDTH + x;
    vga_buffer[index] = vga_entry(c, color);
}

/**
 * @brief Desplaza la pantalla una línea hacia arriba
 */
static void vga_scroll(void) {
    // Mover todas las líneas una posición arriba
    for (size_t y = 0; y < VGA_HEIGHT - 1; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            vga_buffer[y * VGA_WIDTH + x] = vga_buffer[(y + 1) * VGA_WIDTH + x];
        }
    }
    
    // Limpiar la última línea
    for (size_t x = 0; x < VGA_WIDTH; x++) {
        vga_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = vga_entry(' ', terminal_color);
    }
    
    terminal_row = VGA_HEIGHT - 1;
    terminal_column = 0;
}

/**
 * @brief Escribe un carácter en la posición actual del cursor
 */
void vga_putchar(char c) {
    // Manejar caracteres especiales
    if (c == '\n') {
        terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT) {
            vga_scroll();
        }
        return;
    }
    
    if (c == '\t') {
        terminal_column = (terminal_column + 4) & ~(4 - 1);
        if (terminal_column >= VGA_WIDTH) {
            terminal_column = 0;
            if (++terminal_row == VGA_HEIGHT) {
                vga_scroll();
            }
        }
        return;
    }
    
    // Escribir el carácter
    vga_putentryat(c, terminal_color, terminal_column, terminal_row);
    
    if (++terminal_column == VGA_WIDTH) {
        terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT) {
            vga_scroll();
        }
    }
}

/**
 * @brief Escribe una cadena de texto
 */
void vga_puts(const char* str) {
    size_t len = strlen(str);
    for (size_t i = 0; i < len; i++) {
        vga_putchar(str[i]);
    }
}

/**
 * @brief Escribe un número en hexadecimal
 */
void vga_put_hex(uint32_t num) {
    const char hex_chars[] = "0123456789ABCDEF";
    char buffer[11]; // "0x" + 8 dígitos + '\0'
    buffer[0] = '0';
    buffer[1] = 'x';
    
    for (int i = 7; i >= 0; i--) {
        buffer[2 + i] = hex_chars[num & 0xF];
        num >>= 4;
    }
    
    buffer[10] = '\0';
    vga_puts(buffer);
}

/**
 * @brief Escribe un número en decimal
 */
void vga_put_dec(uint32_t num) {
    if (num == 0) {
        vga_putchar('0');
        return;
    }
    
    char buffer[11]; // Máximo 10 dígitos + '\0'
    int i = 0;
    
    while (num > 0) {
        buffer[i++] = '0' + (num % 10);
        num /= 10;
    }
    
    // Imprimir en orden inverso
    for (int j = i - 1; j >= 0; j--) {
        vga_putchar(buffer[j]);
    }
}

/**
 * @brief Alias para vga_put_hex (compatibilidad)
 */
void vga_puthex(uint32_t num) {
    vga_put_hex(num);
}

/**
 * @brief Establece el color directamente con un byte
 */
void vga_setcolor(uint8_t color) {
    terminal_color = color;
}
