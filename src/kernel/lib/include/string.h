/**
 * NeoOS - String Library Header
 * Funciones básicas de manipulación de cadenas
 */

#ifndef _KERNEL_STRING_H
#define _KERNEL_STRING_H

#include "types.h"

/**
 * Copia n bytes de src a dest
 */
void* memcpy(void* dest, const void* src, size_t n);

/**
 * Establece n bytes de s al valor c
 */
void* memset(void* s, int c, size_t n);

/**
 * Compara n bytes de s1 y s2
 * Retorna 0 si son iguales, <0 si s1 < s2, >0 si s1 > s2
 */
int memcmp(const void* s1, const void* s2, size_t n);

/**
 * Mueve n bytes de src a dest (maneja overlapping)
 */
void* memmove(void* dest, const void* src, size_t n);

/**
 * Calcula la longitud de una cadena
 */
size_t strlen(const char* s);

/**
 * Copia la cadena src a dest
 */
char* strcpy(char* dest, const char* src);

/**
 * Copia hasta n caracteres de src a dest
 */
char* strncpy(char* dest, const char* src, size_t n);

/**
 * Concatena src al final de dest
 */
char* strcat(char* dest, const char* src);

/**
 * Concatena hasta n caracteres de src a dest
 */
char* strncat(char* dest, const char* src, size_t n);

/**
 * Compara dos cadenas
 */
int strcmp(const char* s1, const char* s2);

/**
 * Compara hasta n caracteres de dos cadenas
 */
int strncmp(const char* s1, const char* s2, size_t n);

/**
 * Busca el carácter c en la cadena s
 */
char* strchr(const char* s, int c);

/**
 * Busca la última ocurrencia del carácter c en s
 */
char* strrchr(const char* s, int c);

/**
 * Busca la primera ocurrencia de la subcadena needle en haystack
 */
char* strstr(const char* haystack, const char* needle);

#endif /* _KERNEL_STRING_H */
