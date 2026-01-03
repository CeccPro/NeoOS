/**
 * NeoOS - VGA Driver
 * Driver básico para modo texto VGA 80x25
 */

#include "../include/vga.h"

// Dirección de memoria del buffer VGA
static uint16_t* const VGA_MEMORY = (uint16_t*)0xB8000;

// Estado del driver
static size_t vga_row;
static size_t vga_column;
static uint8_t vga_color;

/**
 * Crea un valor de color VGA
 */
static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
    return fg | bg << 4;
}

/**
 * Crea una entrada VGA (carácter + color)
 */
static inline uint16_t vga_entry(unsigned char c, uint8_t color) {
    return (uint16_t)c | (uint16_t)color << 8;
}

/**
 * Desplaza la pantalla una línea hacia arriba
 */
static void vga_scroll(void) {
    // Copiar todas las líneas una posición arriba
    for (size_t y = 0; y < VGA_HEIGHT - 1; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t src_index = (y + 1) * VGA_WIDTH + x;
            const size_t dst_index = y * VGA_WIDTH + x;
            VGA_MEMORY[dst_index] = VGA_MEMORY[src_index];
        }
    }
    
    // Limpiar la última línea
    for (size_t x = 0; x < VGA_WIDTH; x++) {
        const size_t index = (VGA_HEIGHT - 1) * VGA_WIDTH + x;
        VGA_MEMORY[index] = vga_entry(' ', vga_color);
    }
}

/**
 * Inicializa el driver VGA
 */
void vga_init(void) {
    vga_row = 0;
    vga_column = 0;
    vga_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_clear();
}

/**
 * Limpia la pantalla
 */
void vga_clear(void) {
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            VGA_MEMORY[index] = vga_entry(' ', vga_color);
        }
    }
    vga_row = 0;
    vga_column = 0;
}

/**
 * Establece el color de texto
 */
void vga_set_color(enum vga_color fg, enum vga_color bg) {
    vga_color = vga_entry_color(fg, bg);
}

/**
 * Escribe un carácter en la posición actual
 */
void vga_putchar(char c) {
    // Manejo de caracteres especiales
    if (c == '\n') {
        vga_column = 0;
        if (++vga_row >= VGA_HEIGHT) {
            vga_row = VGA_HEIGHT - 1;
            vga_scroll();
        }
        return;
    }
    
    if (c == '\r') {
        vga_column = 0;
        return;
    }
    
    if (c == '\t') {
        // Tab de 4 espacios
        vga_column = (vga_column + 4) & ~3;
        if (vga_column >= VGA_WIDTH) {
            vga_column = 0;
            if (++vga_row >= VGA_HEIGHT) {
                vga_row = VGA_HEIGHT - 1;
                vga_scroll();
            }
        }
        return;
    }
    
    // Escribir carácter normal
    const size_t index = vga_row * VGA_WIDTH + vga_column;
    VGA_MEMORY[index] = vga_entry(c, vga_color);
    
    // Avanzar cursor
    if (++vga_column >= VGA_WIDTH) {
        vga_column = 0;
        if (++vga_row >= VGA_HEIGHT) {
            vga_row = VGA_HEIGHT - 1;
            vga_scroll();
        }
    }
}

/**
 * Escribe una cadena de texto
 */
void vga_write(const char* str) {
    while (*str) {
        vga_putchar(*str++);
    }
}

/**
 * Escribe una cadena con longitud específica
 */
void vga_write_len(const char* str, size_t len) {
    for (size_t i = 0; i < len; i++) {
        vga_putchar(str[i]);
    }
}

/**
 * Escribe un número en formato hexadecimal
 */
void vga_write_hex(uint32_t value) {
    const char hex_chars[] = "0123456789ABCDEF";
    char buffer[11];  // "0x" + 8 dígitos hex + '\0'
    
    buffer[0] = '0';
    buffer[1] = 'x';
    
    for (int i = 7; i >= 0; i--) {
        buffer[2 + i] = hex_chars[value & 0xF];
        value >>= 4;
    }
    
    buffer[10] = '\0';
    vga_write(buffer);
}

void vga_write_dec(uint32_t value) {
    char buffer[12]; // Máximo 10 dígitos + signo + '\0'
    int i = 0;
    
    if (value == 0) {
        vga_putchar('0');
        return;
    }
    
    while (value > 0) {
        buffer[i++] = '0' + (value % 10);
        value /= 10;
    }
    
    // Invertir la cadena
    for (int j = i - 1; j >= 0; j--) {
        vga_putchar(buffer[j]);
    }
}