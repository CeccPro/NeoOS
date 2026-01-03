/**
 * NeoOS - Types Header
 * Definiciones de tipos básicos del sistema
 */

#ifndef _KERNEL_TYPES_H
#define _KERNEL_TYPES_H

// Tipos básicos de tamaño fijo
typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;

typedef signed char        int8_t;
typedef signed short       int16_t;
typedef signed int         int32_t;
typedef signed long long   int64_t;

// Tipos de tamaño de palabra
typedef unsigned long      size_t;
typedef signed long        ssize_t;
typedef unsigned long      uintptr_t;
typedef signed long        intptr_t;

// Tipos booleanos
typedef int bool;
#define true  1
#define false 0

// Macros útiles
#define NULL ((void*)0)

#endif /* _KERNEL_TYPES_H */
