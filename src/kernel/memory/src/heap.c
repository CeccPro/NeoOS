/**
 * NeoOS - Kernel Heap
 * Asignación dinámica de memoria para el kernel
 * 
 * Implementa un heap simple con kmalloc/kfree usando una lista enlazada
 * de bloques libres y ocupados.
 * 
 * Estructura de cada bloque:
 * [ header | datos... ]
 * 
 * El header contiene:
 * - magic: número mágico para detección de corrupción
 * - size: tamaño del bloque (sin incluir el header)
 * - is_free: indica si el bloque está libre
 */

#include "../include/memory.h"
#include "../../core/include/kconfig.h"
#include "../../drivers/include/vga.h"
#include "../../lib/include/string.h"

#define HEAP_MAGIC 0x12345678
#define HEAP_HEADER_SIZE sizeof(heap_block_t)

// Alineación mínima de bloques (16 bytes para mejor rendimiento)
#define HEAP_MIN_ALIGN 16

// Tamaño mínimo de bloque
#define HEAP_MIN_BLOCK_SIZE 32

// Estructura de un bloque del heap
typedef struct heap_block {
    uint32_t magic;                 // Número mágico para verificación
    size_t size;                    // Tamaño del bloque (sin header)
    bool is_free;                   // ¿Está libre?
    struct heap_block* next;        // Siguiente bloque
    struct heap_block* prev;        // Bloque anterior
} heap_block_t;

// Variables globales del heap
static uint32_t heap_start = 0;
static uint32_t heap_end = 0;
static uint32_t heap_current = 0;  // Puntero para asignación simple antes del heap completo
static heap_block_t* heap_first_block = NULL;
static bool heap_initialized = false;

/*
 * Funciones auxiliares
 */

/**
 * Alinea un valor hacia arriba al siguiente múltiplo de align
 */
static inline uint32_t align_up(uint32_t value, uint32_t align) {
    return (value + align - 1) & ~(align - 1);
}

/**
 * Encuentra un bloque libre que sea lo suficientemente grande
 */
static heap_block_t* heap_find_free_block(size_t size) {
    heap_block_t* current = heap_first_block;
    
    while (current != NULL) {
        if (current->magic != HEAP_MAGIC) {
            vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
            vga_write("[HEAP] [ERROR] Corrupcion detectada en bloque ");
            vga_write_hex((uint32_t)current);
            vga_write("\n");
            return NULL;
        }
        
        if (current->is_free && current->size >= size) {
            return current;
        }
        
        current = current->next;
    }
    
    return NULL;
}

/**
 * Divide un bloque si es lo suficientemente grande
 */
static void heap_split_block(heap_block_t* block, size_t size) {
    // Solo dividir si el espacio restante es útil
    if (block->size >= size + HEAP_HEADER_SIZE + HEAP_MIN_BLOCK_SIZE) {
        heap_block_t* new_block = (heap_block_t*)((uint32_t)block + HEAP_HEADER_SIZE + size);
        
        new_block->magic = HEAP_MAGIC;
        new_block->size = block->size - size - HEAP_HEADER_SIZE;
        new_block->is_free = true;
        new_block->next = block->next;
        new_block->prev = block;
        
        if (block->next != NULL) {
            block->next->prev = new_block;
        }
        
        block->next = new_block;
        block->size = size;
    }
}

/**
 * Intenta fusionar un bloque con sus vecinos libres
 */
static void heap_merge_blocks(heap_block_t* block) {
    // Fusionar con el siguiente si está libre
    if (block->next != NULL && block->next->is_free) {
        block->size += HEAP_HEADER_SIZE + block->next->size;
        block->next = block->next->next;
        if (block->next != NULL) {
            block->next->prev = block;
        }
    }
    
    // Fusionar con el anterior si está libre
    if (block->prev != NULL && block->prev->is_free) {
        block->prev->size += HEAP_HEADER_SIZE + block->size;
        block->prev->next = block->next;
        if (block->next != NULL) {
            block->next->prev = block->prev;
        }
    }
}

/**
 * Expande el heap asignando más páginas
 * 
 * NOTA: Con identity mapping, solo podemos usar páginas que ya estén mapeadas.
 * Por ahora, usamos el espacio virtual pre-mapeado y asignamos páginas físicas
 * de la región baja de memoria.
 */
static bool heap_expand(size_t needed_size) {
    size_t total_needed = needed_size + HEAP_HEADER_SIZE;
    size_t pages_needed = (total_needed + PAGE_SIZE - 1) / PAGE_SIZE;
    
    // Verificar que no excedamos el límite del heap
    if (heap_current + pages_needed * PAGE_SIZE > heap_end) {
        if (is_kdebug()) {
            vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
            vga_write("[HEAP] [DEBUG] heap_current + needed > heap_end\n");
        }
        return false;
    }
    
    // Con identity mapping de los primeros 128MB, podemos usar directamente
    // el espacio virtual si asignamos físicamente páginas en ese rango.
    // Por simplicidad, como ya tenemos identity mapping, solo avanzamos heap_current
    // Las páginas ya están mapeadas virtualmente por el identity mapping del VMM.
    
    if (is_kdebug()) {
        vga_write("[HEAP] Expandiendo heap: ");
        vga_write_dec(pages_needed);
        vga_write(" paginas\n");
    }
    
    // Crear un nuevo bloque con el espacio expandido
    heap_block_t* new_block = (heap_block_t*)heap_current;
    new_block->magic = HEAP_MAGIC;
    new_block->size = pages_needed * PAGE_SIZE - HEAP_HEADER_SIZE;
    new_block->is_free = true;
    new_block->next = NULL;
    new_block->prev = NULL;
    
    // Agregar a la lista
    if (heap_first_block == NULL) {
        heap_first_block = new_block;
    } else {
        heap_block_t* last = heap_first_block;
        while (last->next != NULL) {
            last = last->next;
        }
        last->next = new_block;
        new_block->prev = last;
        
        // Intentar fusionar con el bloque anterior
        if (last->is_free) {
            heap_merge_blocks(last);
        }
    }
    
    heap_current += pages_needed * PAGE_SIZE;
    return true;
}

/**
 * Inicializa el heap del kernel
 */
int heap_init(uint32_t start, uint32_t size, bool kdebug __attribute__((unused)), bool kverbose __attribute__((unused))) {
    if (is_kverbose()) {
        vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
        vga_write("[HEAP] Inicializando heap del kernel...\n");
    }
    
    heap_start = start;
    heap_end = start + size;
    heap_current = start;
    heap_first_block = NULL;
    heap_initialized = true;
    
    if (is_kdebug()) {
        vga_write("[HEAP] Rango: ");
        vga_write_hex(heap_start);
        vga_write(" - ");
        vga_write_hex(heap_end);
        vga_write(" (");
        vga_write_dec(size / (1024 * 1024));
        vga_write(" MB)\n");
    }
    
    // Asignar el primer bloque
    if (!heap_expand(PAGE_SIZE)) {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_write("[HEAP] [FAIL] No se pudo expandir el heap inicial\n");
        return E_NOMEM;
    }
    
    if (is_kverbose()) {
        vga_write("[HEAP] Heap inicializado\n");
    }
    
    return E_OK;
}

/**
 * Asignación simple antes de que el heap esté completamente inicializado
 */
static void* kmalloc_early(size_t size, bool align, uint32_t* phys) {
    if (align) {
        heap_current = align_up(heap_current, PAGE_SIZE);
    }
    
    if (heap_current + size > heap_end) {
        return NULL;
    }
    
    void* ret = (void*)heap_current;
    
    if (phys != NULL) {
        *phys = heap_current;
    }
    
    heap_current += size;
    return ret;
}

/**
 * Asigna memoria del heap del kernel
 */
void* kmalloc(size_t size) {
    if (size == 0) {
        return NULL;
    }
    
    // Si el heap no está inicializado, usar asignación simple
    if (!heap_initialized) {
        return kmalloc_early(size, false, NULL);
    }
    
    // Alinear el tamaño
    size = align_up(size, HEAP_MIN_ALIGN);
    
    // Buscar un bloque libre
    heap_block_t* block = heap_find_free_block(size);
    
    // Si no hay bloque, expandir el heap
    if (block == NULL) {
        if (!heap_expand(size)) {
            return NULL;
        }
        block = heap_find_free_block(size);
        if (block == NULL) {
            return NULL;
        }
    }
    
    // Marcar como ocupado
    block->is_free = false;
    
    // Dividir si es necesario
    heap_split_block(block, size);
    
    // Retornar puntero a los datos (después del header)
    return (void*)((uint32_t)block + HEAP_HEADER_SIZE);
}

/**
 * Asigna memoria alineada a página
 */
void* kmalloc_a(size_t size) {
    if (!heap_initialized) {
        return kmalloc_early(size, true, NULL);
    }
    
    // Por simplicidad, asignar tamaño múltiplo de página
    size = align_up(size, PAGE_SIZE);
    return kmalloc(size);
}

/**
 * Asigna memoria y devuelve la dirección física
 */
void* kmalloc_p(size_t size, uint32_t* phys) {
    if (!heap_initialized) {
        return kmalloc_early(size, false, phys);
    }
    
    void* ptr = kmalloc(size);
    if (ptr != NULL && phys != NULL) {
        *phys = vmm_get_physical(vmm_get_kernel_directory(), (uint32_t)ptr);
    }
    return ptr;
}

/**
 * Asigna memoria alineada y devuelve la dirección física
 */
void* kmalloc_ap(size_t size, uint32_t* phys) {
    if (!heap_initialized) {
        return kmalloc_early(size, true, phys);
    }
    
    void* ptr = kmalloc_a(size);
    if (ptr != NULL && phys != NULL) {
        *phys = vmm_get_physical(vmm_get_kernel_directory(), (uint32_t)ptr);
    }
    return ptr;
}

/**
 * Libera memoria del heap
 */
void kfree(void* ptr) {
    if (ptr == NULL || !heap_initialized) {
        return;
    }
    
    // Obtener el bloque a partir del puntero
    heap_block_t* block = (heap_block_t*)((uint32_t)ptr - HEAP_HEADER_SIZE);
    
    // Verificar magic
    if (block->magic != HEAP_MAGIC) {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_write("[HEAP] [ERROR] kfree con puntero invalido: ");
        vga_write_hex((uint32_t)ptr);
        vga_write("\n");
        return;
    }
    
    // Marcar como libre
    block->is_free = true;
    
    // Fusionar con bloques vecinos si están libres
    heap_merge_blocks(block);
}
