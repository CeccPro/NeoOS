/**
 * NeoOS - String Library Implementation
 * Implementación de funciones básicas de manipulación de cadenas
 */

#include "../include/string.h"

/**
 * Copia n bytes de src a dest
 */
void* memcpy(void* dest, const void* src, size_t n) {
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;
    
    for (size_t i = 0; i < n; i++) {
        d[i] = s[i];
    }
    
    return dest;
}

/**
 * Establece n bytes de s al valor c
 */
void* memset(void* s, int c, size_t n) {
    volatile unsigned char* p = (volatile unsigned char*)s;
    unsigned char value = (unsigned char)c;
    
    for (size_t i = 0; i < n; i++) {
        p[i] = value;
    }
    
    return s;
}

/**
 * Compara n bytes de s1 y s2
 */
int memcmp(const void* s1, const void* s2, size_t n) {
    const unsigned char* p1 = (const unsigned char*)s1;
    const unsigned char* p2 = (const unsigned char*)s2;
    
    for (size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return p1[i] - p2[i];
        }
    }
    
    return 0;
}

/**
 * Mueve n bytes de src a dest (maneja overlapping)
 */
void* memmove(void* dest, const void* src, size_t n) {
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;
    
    if (d < s) {
        // Copiar hacia adelante
        for (size_t i = 0; i < n; i++) {
            d[i] = s[i];
        }
    } else if (d > s) {
        // Copiar hacia atrás para evitar overlapping
        for (size_t i = n; i > 0; i--) {
            d[i - 1] = s[i - 1];
        }
    }
    
    return dest;
}

/**
 * Calcula la longitud de una cadena
 */
size_t strlen(const char* s) {
    size_t len = 0;
    while (s[len]) {
        len++;
    }
    return len;
}

/**
 * Copia la cadena src a dest
 */
char* strcpy(char* dest, const char* src) {
    char* d = dest;
    while ((*d++ = *src++));
    return dest;
}

/**
 * Copia hasta n caracteres de src a dest
 */
char* strncpy(char* dest, const char* src, size_t n) {
    size_t i;
    
    for (i = 0; i < n && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    
    for (; i < n; i++) {
        dest[i] = '\0';
    }
    
    return dest;
}

/**
 * Concatena src al final de dest
 */
char* strcat(char* dest, const char* src) {
    char* d = dest;
    
    while (*d) {
        d++;
    }
    
    while ((*d++ = *src++));
    
    return dest;
}

/**
 * Concatena hasta n caracteres de src a dest
 */
char* strncat(char* dest, const char* src, size_t n) {
    char* d = dest;
    
    while (*d) {
        d++;
    }
    
    size_t i;
    for (i = 0; i < n && src[i] != '\0'; i++) {
        d[i] = src[i];
    }
    d[i] = '\0';
    
    return dest;
}

/**
 * Compara dos cadenas
 */
int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

/**
 * Compara hasta n caracteres de dos cadenas
 */
int strncmp(const char* s1, const char* s2, size_t n) {
    for (size_t i = 0; i < n; i++) {
        if (s1[i] != s2[i] || s1[i] == '\0') {
            return (unsigned char)s1[i] - (unsigned char)s2[i];
        }
    }
    return 0;
}

/**
 * Busca el carácter c en la cadena s
 */
char* strchr(const char* s, int c) {
    while (*s) {
        if (*s == (char)c) {
            return (char*)s;
        }
        s++;
    }
    
    if (c == '\0') {
        return (char*)s;
    }
    
    return NULL;
}

/**
 * Busca la última ocurrencia del carácter c en s
 */
char* strrchr(const char* s, int c) {
    const char* last = NULL;
    
    while (*s) {
        if (*s == (char)c) {
            last = s;
        }
        s++;
    }
    
    if (c == '\0') {
        return (char*)s;
    }
    
    return (char*)last;
}

/**
 * Busca la subcadena needle en haystack
 */
char* strstr(const char* haystack, const char* needle) {
    if (*needle == '\0') {
        return (char*)haystack;
    }
    for (; *haystack; haystack++) {
        const char* h = haystack;
        const char* n = needle;
        
        while (*h && *n && (*h == *n)) {
            h++;
            n++;
        }
        
        if (*n == '\0') {
            return (char*)haystack;
        }
    }
    return NULL;
}

/**
 * Convierte un entero a cadena en la base especificada
 */
char* itoa(int value, char* str, int base) {
    if (base < 2 || base > 36) {
        str[0] = '\0';
        return str;
    }
    
    char* ptr = str;
    char* ptr1 = str;
    char tmp_char;
    int tmp_value;
    
    // Manejar valor negativo para base 10
    int is_negative = 0;
    if (value < 0 && base == 10) {
        is_negative = 1;
        value = -value;
    }
    
    // Convertir número a cadena (en reversa)
    do {
        tmp_value = value;
        value /= base;
        *ptr++ = "0123456789abcdefghijklmnopqrstuvwxyz"[tmp_value - value * base];
    } while (value);
    
    // Agregar signo negativo si es necesario
    if (is_negative) {
        *ptr++ = '-';
    }
    
    *ptr-- = '\0';
    
    // Revertir la cadena
    while (ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr-- = *ptr1;
        *ptr1++ = tmp_char;
    }
    
    return str;
}