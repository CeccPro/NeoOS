/**
 * @file memory.h
 * @brief Gestor de memoria de NeoOS
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

#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include <stddef.h>
#include "errors.h"
#include "multiboot.h"

// Tamaño de página
#define PAGE_SIZE 4096
#define PAGE_ALIGN(addr) (((addr) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))

// Flags de página
#define PAGE_PRESENT    (1 << 0)  // Página presente en memoria
#define PAGE_WRITE      (1 << 1)  // Página de escritura
#define PAGE_USER       (1 << 2)  // Página accesible desde modo usuario
#define PAGE_ACCESSED   (1 << 5)  // Página accedida
#define PAGE_DIRTY      (1 << 6)  // Página modificada

/**
 * @brief Inicializa el gestor de memoria
 * @param mbi Información de multiboot con el mapa de memoria
 * @return E_OK si fue exitoso, código de error en caso contrario
 */
error_t memory_init(multiboot_info_t* mbi);

/**
 * @brief Asigna un bloque de memoria física
 * @return Dirección física del bloque asignado o 0 si no hay memoria disponible
 */
void* pmm_alloc_page(void);

/**
 * @brief Libera un bloque de memoria física
 * @param page Dirección física de la página a liberar
 */
void pmm_free_page(void* page);

/**
 * @brief Obtiene la cantidad total de memoria física
 * @return Cantidad de memoria en bytes
 */
size_t pmm_get_total_memory(void);

/**
 * @brief Obtiene la cantidad de memoria física libre
 * @return Cantidad de memoria libre en bytes
 */
size_t pmm_get_free_memory(void);

/**
 * @brief Mapea una página virtual a una física
 * @param virtual Dirección virtual
 * @param physical Dirección física
 * @param flags Flags de la página (PAGE_PRESENT, PAGE_WRITE, etc.)
 * @return E_OK si fue exitoso, código de error en caso contrario
 */
error_t vmm_map_page(void* virtual, void* physical, uint32_t flags);

/**
 * @brief Desmapea una página virtual
 * @param virtual Dirección virtual a desmapear
 * @return E_OK si fue exitoso, código de error en caso contrario
 */
error_t vmm_unmap_page(void* virtual);

/**
 * @brief Obtiene la dirección física de una dirección virtual
 * @param virtual Dirección virtual
 * @return Dirección física o 0 si no está mapeada
 */
void* vmm_get_physical(void* virtual);

/**
 * @brief Crea un nuevo directorio de páginas (espacio de direcciones)
 * @return Puntero al nuevo directorio de páginas o NULL en caso de error
 */
void* vmm_create_address_space(void);

/**
 * @brief Destruye un directorio de páginas
 * @param page_dir Puntero al directorio de páginas
 */
void vmm_destroy_address_space(void* page_dir);

/**
 * @brief Cambia al espacio de direcciones especificado
 * @param page_dir Puntero al directorio de páginas
 */
void vmm_switch_address_space(void* page_dir);

/**
 * @brief Asigna memoria dinámica del heap del kernel
 * @param size Tamaño en bytes
 * @return Puntero a la memoria asignada o NULL si falla
 */
void* kmalloc(size_t size);

/**
 * @brief Libera memoria del heap del kernel
 * @param ptr Puntero a la memoria a liberar
 */
void kfree(void* ptr);

#endif // MEMORY_H
