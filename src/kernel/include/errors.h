/**
 * @file errors.h
 * @brief Códigos de error del sistema NeoOS
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

#ifndef ERRORS_H
#define ERRORS_H

typedef enum {
    E_OK = 0,              // Operación exitosa
    E_NOMEM = 1,           // Sin memoria disponible
    E_INVAL = 2,           // Argumento inválido
    E_PERM = 3,            // Operación no permitida
    E_NOENT = 4,           // No existe el archivo/recurso
    E_IO = 5,              // Error de entrada/salida
    E_BUSY = 6,            // Recurso ocupado
    E_EXISTS = 7,          // Ya existe
    E_TIMEOUT = 8,         // Tiempo de espera agotado
    E_NOT_IMPL = 9,        // No implementado
    E_NOT_SUPPORTED = 10,  // No soportado
    E_MODULE_ERR = 11,     // Error de módulo
    E_UNKNOWN = 255        // Error desconocido
} error_t;

// Convierte un código de error a una cadena de texto
const char* error_to_string(error_t err);

#endif // ERRORS_H
