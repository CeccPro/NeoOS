/**
 * @file kernel.h
 * @brief Definiciones principales del kernel NeoOS
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

#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// Versión del kernel
#define KERNEL_VERSION_MAJOR 0
#define KERNEL_VERSION_MINOR 1
#define KERNEL_VERSION_PATCH 0

// Tipos básicos
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;

// NULL pointer
#ifndef NULL
#define NULL ((void*)0)
#endif

// Macros útiles
#define UNUSED(x) (void)(x)
#define ALIGN(x, a) (((x) + (a) - 1) & ~((a) - 1))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

// Funciones de entrada/salida de puertos
extern void outb(uint16_t port, uint8_t value);
extern uint8_t inb(uint16_t port);

// Forward declaration
struct multiboot_info;

// Prototipo de la función principal del kernel
void kernel_main(uint32_t magic, struct multiboot_info* mbi);

#endif // KERNEL_H
