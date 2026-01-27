/**
 * NeoOS - VGA Driver Module Header
 * Header para el módulo del driver VGA
 */

#ifndef _KERNEL_VGA_DRIVER_H
#define _KERNEL_VGA_DRIVER_H

#include "../include/driver.h"

// Mensajes PMIC para VGA
#define VGA_MSG_WRITE       1
#define VGA_MSG_SET_COLOR   2
#define VGA_MSG_CLEAR       3
#define VGA_MSG_WRITE_HEX   4
#define VGA_MSG_WRITE_DEC   5

// Estructura para mensaje de escritura
typedef struct {
    uint32_t type;
    char data[256];  // Para strings
} vga_message_t;

// Estructura para mensaje de color
typedef struct {
    uint32_t type;
    uint8_t fg;
    uint8_t bg;
} vga_color_message_t;

// Estructura para mensaje numérico
typedef struct {
    uint32_t type;
    uint32_t value;
} vga_number_message_t;

/**
 * Obtiene el punto de entrada del driver VGA
 * @return Puntero al driver_entry_t del VGA
 */
driver_entry_t* vga_driver_get_entry(void);

#endif /* _KERNEL_VGA_DRIVER_H */