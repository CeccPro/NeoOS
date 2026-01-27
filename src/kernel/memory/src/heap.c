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
#include "../../drivers/include/early_vga.h"
#include "../../lib/include/string.h"

#define HEAP_MAGIC 0x12345678
#define HEAP_HEADER_SIZE sizeof(heap_block_t)

// Alineación mínima de bloques (16 bytes para mejor rendimiento)
#define HEAP_MIN_ALIGN 16

// Tamaño mínimo de bloque
#define HEAP_MIN_BLOCK_SIZE 64

// Estructura de un bloque del heap
typedef struct heap_block {
    uint32_t magic;                 // Número mágico para verificación
    size_t size;                    // Tamaño del bloque (sin header)
    bool is_free;                   // ¿Está libre?
    struct heap_block* next;        // Siguiente bloque
    struct heap_block* prev;        // Bloque anterior
    uint32_t padding[3];            // Padding para alinear a 32 bytes (múltiplo de 16)
} heap_block_t;

// Variables globales del heap
static uint32_t heap_start = 0;
static uint32_t heap_end = 0;
static uint32_t heap_current = 0;  // Puntero para asignación simple antes del heap completo
static heap_block_t* heap_first_block = NULL;
static bool heap_initialized = false;

// Verificación estática en tiempo de compilación: el header debe ser múltiplo de HEAP_MIN_ALIGN
_Static_assert(sizeof(heap_block_t) % HEAP_MIN_ALIGN == 0, 
               "heap_block_t debe ser múltiplo de HEAP_MIN_ALIGN para garantizar alineación");

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
        // Calcular la dirección del nuevo bloque (alineada)
        uintptr_t new_block_addr = (uintptr_t)block + HEAP_HEADER_SIZE + size;
        
        // Alinear la dirección del nuevo bloque
        new_block_addr = align_up(new_block_addr, HEAP_MIN_ALIGN);
        
        // Recalcular el tamaño del nuevo bloque
        size_t actual_size = (uintptr_t)new_block_addr - ((uintptr_t)block + HEAP_HEADER_SIZE);
        
        // Validar que no se salga del heap y que haya espacio suficiente
        if (new_block_addr + HEAP_HEADER_SIZE > heap_end ||
            block->size < actual_size + HEAP_HEADER_SIZE + HEAP_MIN_BLOCK_SIZE) {
            return; // No dividir si se sale del heap o no hay espacio
        }
        
        heap_block_t* new_block = (heap_block_t*)new_block_addr;
        
        // Inicializar el nuevo bloque con ceros primero
        memset(new_block, 0, sizeof(heap_block_t));
        
        new_block->magic = HEAP_MAGIC;
        new_block->size = block->size - actual_size - HEAP_HEADER_SIZE;
        new_block->is_free = true;
        new_block->next = block->next;
        new_block->prev = block;
        
        if (block->next != NULL) {
            block->next->prev = new_block;
        }
        
        block->next = new_block;
        block->size = actual_size;
    }
}

// Verifica si dos bloques son contiguos en memoria
static bool are_contiguous(heap_block_t* a, heap_block_t* b) {
    return ((uint32_t)a + HEAP_HEADER_SIZE + a->size) == (uint32_t)b;
}


/**
 * Intenta fusionar un bloque con sus vecinos libres
 */
static void heap_merge_blocks(heap_block_t* block) {
    if (!block) {
        return;
    }
    
    // Validar el magic del bloque actual
    if (block->magic != HEAP_MAGIC) {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_write("[HEAP] [ERROR] Intento de fusionar bloque corrupto\n");
        return;
    }
    
    // Fusionar con el siguiente si está libre Y es contiguo
    if (block->next && block->next->magic == HEAP_MAGIC && 
        block->next->is_free && are_contiguous(block, block->next)) {
        heap_block_t* next = block->next;
        block->size += HEAP_HEADER_SIZE + next->size;
        block->next = next->next;
        if (block->next) {
            block->next->prev = block;
        }
        // Invalidar el magic del bloque fusionado para detectar uso después de fusión
        next->magic = 0;
    }

    // Fusionar con el anterior si está libre Y es contiguo
    if (block->prev && block->prev->magic == HEAP_MAGIC && 
        block->prev->is_free && are_contiguous(block->prev, block)) {
        // Guardar referencias necesarias antes de fusionar
        heap_block_t* prev_block = block->prev;
        heap_block_t* next_block = block->next;
        
        prev_block->size += HEAP_HEADER_SIZE + block->size;
        prev_block->next = next_block;
        if (next_block) {
            next_block->prev = prev_block;
        }
        // Invalidar el magic del bloque fusionado
        block->magic = 0;
        // No usar 'block' después de este punto, ya fue fusionado
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
    // Protección contra integer overflow
    if (needed_size > (SIZE_MAX - HEAP_HEADER_SIZE)) {
        if (is_kdebug()) {
            vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
            vga_write("[HEAP] [DEBUG] needed_size causa overflow\n");
        }
        return false;
    }
    
    size_t total_needed = needed_size + HEAP_HEADER_SIZE;
    size_t pages_needed = (total_needed + PAGE_SIZE - 1) / PAGE_SIZE;
    size_t expand_size = pages_needed * PAGE_SIZE;

    // Validar overflow en la suma
    if (heap_current > heap_end || expand_size > (heap_end - heap_current)) {
        if (is_kdebug()) {
            vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
            vga_write("[HEAP] [DEBUG] heap overflow\n");
        }
        return false;
    }

    if (is_kdebug()) {
        vga_write("[HEAP] Expandiendo heap: ");
        vga_write_dec(pages_needed);
        vga_write(" paginas\n");
    }

    // Si no hay bloques todavía, crear el primero
    if (heap_first_block == NULL) {
        // Asegurar que heap_current esté alineado
        heap_current = align_up(heap_current, HEAP_MIN_ALIGN);
        
        // CRÍTICO: Limpiar toda la región de memoria antes de usarla
        memset((void*)heap_current, 0, expand_size);
        
        heap_block_t* block = (heap_block_t*)heap_current;
        block->magic = HEAP_MAGIC;
        block->size = expand_size - HEAP_HEADER_SIZE;
        block->is_free = true;
        block->next = NULL;
        block->prev = NULL;

        heap_first_block = block;
        heap_current += expand_size;
        return true;
    }

    // Buscar el último bloque
    heap_block_t* last = heap_first_block;
    while (last->next) {
        last = last->next;
    }

    // Si el último bloque es libre y está al final del heap, extenderlo
    uintptr_t last_end =
        (uintptr_t)last + HEAP_HEADER_SIZE + last->size;

    if (last->is_free && last_end == heap_current) {
        // CRÍTICO: Limpiar la nueva región de memoria que se va a agregar
        memset((void*)heap_current, 0, expand_size);
        
        last->size += expand_size;
        heap_current += expand_size;
        return true;
    }

    // Si no, crear un bloque nuevo
    // Asegurar que heap_current esté alineado
    heap_current = align_up(heap_current, HEAP_MIN_ALIGN);
    
    // CRÍTICO: Limpiar toda la región de memoria antes de usarla
    memset((void*)heap_current, 0, expand_size);
    
    heap_block_t* new_block = (heap_block_t*)heap_current;
    new_block->magic = HEAP_MAGIC;
    new_block->size = expand_size - HEAP_HEADER_SIZE;
    new_block->is_free = true;
    new_block->next = NULL;
    new_block->prev = last;

    last->next = new_block;
    heap_current += expand_size;
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
    
    // Asegurar que el heap comience alineado a 16 bytes
    heap_start = align_up(start, HEAP_MIN_ALIGN);
    heap_end = start + size;
    heap_current = heap_start;
    heap_first_block = NULL;
    
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
    
    heap_initialized = true;
    
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
    void* ptr = (void*)((uint32_t)block + HEAP_HEADER_SIZE);
    
    // NOTA: No necesitamos memset aquí porque:
    // 1. heap_expand ya limpia la memoria cuando se crean nuevos bloques
    // 2. kfree limpia la memoria cuando se libera
    // Por lo tanto, todos los bloques libres ya están limpios
    
    return ptr;
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
        uint32_t physical = vmm_get_physical(vmm_get_kernel_directory(), (uint32_t)ptr);
        if (physical == 0) {
            vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
            vga_write("[HEAP] [ERROR] No se pudo obtener direccion fisica\n");
            kfree(ptr);
            return NULL;
        }
        *phys = physical;
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
        uint32_t physical = vmm_get_physical(vmm_get_kernel_directory(), (uint32_t)ptr);
        if (physical == 0) {
            vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
            vga_write("[HEAP] [ERROR] No se pudo obtener direccion fisica\n");
            kfree(ptr);
            return NULL;
        }
        *phys = physical;
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
    
    // CRÍTICO: Limpiar la memoria antes de liberarla para evitar
    // que datos viejos contaminen futuras asignaciones
    memset(ptr, 0, block->size);
    
    // Marcar como libre
    block->is_free = true;
    
    // Fusionar con bloques vecinos si están libres
    heap_merge_blocks(block);
}
