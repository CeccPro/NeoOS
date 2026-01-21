/*
 * NeoOS - Error Codes Header
 * Definiciones de códigos de error para el kernel
 */

#ifndef _KERNEL_ERROR_H
#define _KERNEL_ERROR_H

// Códigos de error generales
#define E_OK            0   // Operación exitosa
#define E_UNKNOWN      -1   // Error desconocido
#define E_NOMEM        -2   // Memoria insuficiente
#define E_INVAL        -3   // Argumento inválido
#define E_NOENT        -4   // Entrada no encontrada
#define E_EXISTS       -5   // Entrada ya existe
#define E_BUSY         -6   // Recurso ocupado
#define E_IO           -7   // Error de entrada/salida
#define E_PERM         -8   // Permiso denegado
#define E_TIMEOUT      -9   // Operación agotó el tiempo
#define E_MODULE_ERR   -10  // Error de módulo
#define E_NOT_IMPL     -11  // Funcionalidad no implementada
#define E_NOT_SUPPORTED -12 // Funcionalidad no soportada

/**
 * Convierte un código de error a su nombre como string
 * @param error Código de error
 * @return String con el nombre del error
 */
 const char* error_to_string(int error);

/**
 * Genera un kernel panic con un mensaje de error
 * @param msg Mensaje de error
 */
void panic(const char* msg);

#endif /* _KERNEL_ERROR_H */