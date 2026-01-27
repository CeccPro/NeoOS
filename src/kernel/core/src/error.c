/*
 * NeoOS - Error Codes
 * Definiciones de códigos de error para el kernel
 */

#include "../include/error.h"
#include "../../drivers/include/early_vga.h"

/**
 * Convierte un código de error a su nombre como string
 * @param error Código de error
 * @return String con el nombre del error
 */
 const char* error_to_string(int error) {
    switch (error) {
        case E_OK:            return "E_OK";
        case E_UNKNOWN:       return "E_UNKNOWN";
        case E_NOMEM:         return "E_NOMEM";
        case E_INVAL:         return "E_INVAL";
        case E_NOENT:         return "E_NOENT";
        case E_EXISTS:        return "E_EXISTS";
        case E_BUSY:          return "E_BUSY";
        case E_IO:            return "E_IO";
        case E_PERM:          return "E_PERM";
        case E_TIMEOUT:       return "E_TIMEOUT";
        case E_MODULE_ERR:    return "E_MODULE_ERR";
        case E_NOT_IMPL:      return "E_NOT_IMPL";
        case E_NOT_SUPPORTED: return "E_NOT_SUPPORTED";
        default:              return "E_UNKNOWN";
    }
}

/**
 * Genera un kernel panic con un mensaje de error
 * @param msg Mensaje de error
 */
void panic(const char* msg) {
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_RED);
    vga_write("\n\n\n                             !!! KERNEL PANIC !!!\n");

    // Calcular espacios para centrar el mensaje
    int len = 0;
    const char* p = msg;
    while (p && *p) {
        len++;
        p++;
    }

    int spaces = (48 - len + 17) / 2;

    for (int i = 0; i < spaces; i++) {
        vga_write(" ");
    }

    vga_write("Unhandled Error: ");
    
    if (msg) {
        vga_write(msg);
    } else {
        vga_write("Unknown");
    }
    for (int i = 0; i < 2 ; i++) {
        vga_write("\n");
    }
    // Detener el kernel
    __asm__ volatile("cli; hlt");
    while(1);
}