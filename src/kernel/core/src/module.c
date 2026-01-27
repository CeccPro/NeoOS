/**
 * NeoOS - Module Manager Implementation
 * Sistema de gestión de módulos del kernel
 */

#include "../include/module.h"
#include "../include/error.h"
#include "../include/timer.h"
#include "../include/ipc.h"
#include "../../lib/include/string.h"
#include "../../memory/include/memory.h"
#include "../../drivers/include/early_vga.h"

// Lista de módulos cargados
static module_t* module_list_head = NULL;
static mid_t next_mid = 1;  // MID 0 está reservado
static bool initialized = false;
static uint32_t module_count = 0;

/**
 * Busca un módulo por su MID
 */
static module_t* module_find_by_mid(mid_t mid) {
    module_t* current = module_list_head;
    while (current != NULL) {
        if (current->mid == mid) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

/**
 * Busca un módulo por su nombre
 */
static module_t* module_find_by_name(const char* name) {
    if (name == NULL) return NULL;
    
    module_t* current = module_list_head;
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

/**
 * Añade un módulo a la lista
 */
static void module_add_to_list(module_t* module) {
    if (module == NULL) return;
    
    module->next = module_list_head;
    module_list_head = module;
    module_count++;
}

/**
 * Elimina un módulo de la lista
 */
static void module_remove_from_list(module_t* module) {
    if (module == NULL || module_list_head == NULL) return;
    
    // Si es el primer elemento
    if (module_list_head == module) {
        module_list_head = module->next;
        module_count--;
        return;
    }
    
    // Buscar el módulo en la lista
    module_t* current = module_list_head;
    while (current->next != NULL) {
        if (current->next == module) {
            current->next = module->next;
            module_count--;
            return;
        }
        current = current->next;
    }
}

/**
 * Inicializa el Module Manager
 */
int module_manager_init(bool verbose) {
    if (initialized) {
        return E_EXISTS;
    }
    
    if (verbose) {
        vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
        vga_write("[MODULE] Inicializando Module Manager...\n");
    }
    
    module_list_head = NULL;
    next_mid = 1;
    module_count = 0;
    initialized = true;
    
    if (verbose) {
        vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
        vga_write("[MODULE] Module Manager inicializado\n");
    }
    
    return E_OK;
}

/**
 * Carga un módulo en el sistema
 */
mid_t module_load(const char* name) {
    if (!initialized) {
        return E_NOT_IMPL;
    }
    
    if (name == NULL) {
        return E_INVAL;
    }
    
    // Verificar si el módulo ya está cargado
    module_t* existing = module_find_by_name(name);
    if (existing != NULL) {
        return E_EXISTS;
    }
    
    // Verificar que no excedamos el límite de módulos
    if (module_count >= MAX_MODULES) {
        return E_NOMEM;
    }
    
    // Asignar memoria para el nuevo módulo
    module_t* new_module = (module_t*)kmalloc(sizeof(module_t));
    if (new_module == NULL) {
        return E_NOMEM;
    }
    
    // Inicializar el módulo
    new_module->mid = next_mid++;
    strncpy(new_module->name, name, MODULE_NAME_MAX - 1);
    new_module->name[MODULE_NAME_MAX - 1] = '\0';
    new_module->state = MODULE_STATE_LOADING;
    new_module->entry = NULL;
    new_module->dependency_count = 0;
    new_module->load_time = timer_get_ticks();
    new_module->update_count = 0;
    new_module->next = NULL;
    
    // TODO: Aquí se debería cargar el módulo desde el sistema de archivos
    // y obtener su punto de entrada (module_entry_t)
    // Por ahora, simplemente lo marcamos como cargado pero sin entry point
    
    // Añadir el módulo a la lista
    module_add_to_list(new_module);
    
    new_module->state = MODULE_STATE_LOADED;
    
    return new_module->mid;
}

/**
 * Descarga un módulo del sistema
 */
int module_unload(mid_t mid) {
    if (!initialized) {
        return E_NOT_IMPL;
    }
    
    // Buscar el módulo
    module_t* module = module_find_by_mid(mid);
    if (module == NULL) {
        return E_NOENT;
    }
    
    // Verificar que el módulo no esté en ejecución
    if (module->state == MODULE_STATE_RUNNING) {
        return E_BUSY;
    }
    
    module->state = MODULE_STATE_UNLOADING;
    
    // Llamar a la función de limpieza si existe
    if (module->entry != NULL && module->entry->cleanup != NULL) {
        int result = module->entry->cleanup();
        if (result != E_OK) {
            module->state = MODULE_STATE_STOPPED;
            return result;
        }
    }
    
    // Eliminar el módulo de la lista
    module_remove_from_list(module);
    
    // Liberar memoria
    kfree(module);
    
    return E_OK;
}

/**
 * Inicia un módulo cargado
 */
int module_start(mid_t mid) {
    if (!initialized) {
        return E_NOT_IMPL;
    }
    
    // Buscar el módulo
    module_t* module = module_find_by_mid(mid);
    if (module == NULL) {
        return E_NOENT;
    }
    
    // Verificar que el módulo esté cargado
    if (module->state != MODULE_STATE_LOADED && module->state != MODULE_STATE_STOPPED) {
        return E_INVAL;
    }
    
    // Verificar dependencias
    int dep_result = module_check_dependencies(mid);
    if (dep_result != E_OK) {
        return dep_result;
    }
    
    // Llamar a la función de inicio si existe
    if (module->entry != NULL) {
        // Primero inicializar si no se ha hecho
        if (module->entry->init != NULL) {
            int result = module->entry->init();
            if (result != E_OK) {
                return result;
            }
        }
        
        // Luego iniciar
        if (module->entry->start != NULL) {
            int result = module->entry->start();
            if (result != E_OK) {
                return result;
            }
        }
    }
    
    module->state = MODULE_STATE_RUNNING;
    return E_OK;
}

/**
 * Detiene un módulo en ejecución
 */
int module_stop(mid_t mid) {
    if (!initialized) {
        return E_NOT_IMPL;
    }
    
    // Buscar el módulo
    module_t* module = module_find_by_mid(mid);
    if (module == NULL) {
        return E_NOENT;
    }
    
    // Verificar que el módulo esté en ejecución
    if (module->state != MODULE_STATE_RUNNING) {
        return E_INVAL;
    }
    
    // Llamar a la función de detención si existe
    if (module->entry != NULL && module->entry->stop != NULL) {
        int result = module->entry->stop();
        if (result != E_OK) {
            return result;
        }
    }
    
    module->state = MODULE_STATE_STOPPED;
    return E_OK;
}

/**
 * Actualiza todos los módulos en ejecución
 */
int module_update_all(void) {
    if (!initialized) {
        return E_NOT_IMPL;
    }
    
    module_t* current = module_list_head;
    while (current != NULL) {
        // Solo actualizar módulos en ejecución
        if (current->state == MODULE_STATE_RUNNING) {
            if (current->entry != NULL && current->entry->update != NULL) {
                int result = current->entry->update();
                if (result != E_OK) {
                    // Si falla la actualización, detener el módulo
                    current->state = MODULE_STATE_STOPPED;
                }
                current->update_count++;
            }
        }
        current = current->next;
    }
    
    return E_OK;
}

/**
 * Obtiene el estado de un módulo
 */
module_state_t module_get_state(mid_t mid) {
    if (!initialized) {
        return MODULE_STATE_UNLOADED;
    }
    
    module_t* module = module_find_by_mid(mid);
    if (module == NULL) {
        return MODULE_STATE_UNLOADED;
    }
    
    return module->state;
}

/**
 * Obtiene el MID de un módulo por su nombre
 */
mid_t module_get_id(const char* name) {
    if (!initialized || name == NULL) {
        return 0;
    }
    
    module_t* module = module_find_by_name(name);
    if (module == NULL) {
        return 0;
    }
    
    return module->mid;
}

/**
 * Obtiene el nombre de un módulo por su MID
 */
const char* module_get_name(mid_t mid) {
    if (!initialized) {
        return NULL;
    }
    
    module_t* module = module_find_by_mid(mid);
    if (module == NULL) {
        return NULL;
    }
    
    return module->name;
}

/**
 * Lista todos los módulos cargados
 */
int module_list(char* buffer, size_t buffer_size) {
    if (!initialized || buffer == NULL || buffer_size == 0) {
        return 0;
    }
    
    int count = 0;
    size_t offset = 0;
    module_t* current = module_list_head;
    
    while (current != NULL && offset < buffer_size - 1) {
        // Formatear: "MID:Name:State\n"
        int written = 0;
        // Escribir MID
        char mid_str[12];
        itoa(current->mid, mid_str, 10);
        size_t mid_len = strlen(mid_str);
        if (offset + mid_len + 1 < buffer_size) {
            strcpy(buffer + offset, mid_str);
            offset += mid_len;
            buffer[offset++] = ':';
        }
        
        // Escribir nombre
        size_t name_len = strlen(current->name);
        if (offset + name_len + 1 < buffer_size) {
            strcpy(buffer + offset, current->name);
            offset += name_len;
            buffer[offset++] = ':';
        }
        
        // Escribir estado
        const char* state_str = "";
        switch (current->state) {
            case MODULE_STATE_UNLOADED: state_str = "UNLOADED"; break;
            case MODULE_STATE_LOADING: state_str = "LOADING"; break;
            case MODULE_STATE_LOADED: state_str = "LOADED"; break;
            case MODULE_STATE_RUNNING: state_str = "RUNNING"; break;
            case MODULE_STATE_STOPPED: state_str = "STOPPED"; break;
            case MODULE_STATE_UNLOADING: state_str = "UNLOADING"; break;
        }
        size_t state_len = strlen(state_str);
        if (offset + state_len + 2 < buffer_size) {
            strcpy(buffer + offset, state_str);
            offset += state_len;
            buffer[offset++] = '\n';
        }
        
        count++;
        current = current->next;
    }
    
    buffer[offset] = '\0';
    return count;
}

/**
 * Obtiene información detallada de un módulo
 */
const module_t* module_get_info(mid_t mid) {
    if (!initialized) {
        return NULL;
    }
    
    return module_find_by_mid(mid);
}

/**
 * Verifica las dependencias de un módulo
 */
int module_check_dependencies(mid_t mid) {
    if (!initialized) {
        return E_NOT_IMPL;
    }
    
    module_t* module = module_find_by_mid(mid);
    if (module == NULL) {
        return E_NOENT;
    }
    
    // Verificar cada dependencia
    for (uint32_t i = 0; i < module->dependency_count; i++) {
        mid_t dep_mid = module->dependencies[i];
        module_t* dep_module = module_find_by_mid(dep_mid);
        
        if (dep_module == NULL) {
            return E_MODULE_ERR;  // Dependencia no encontrada
        }
        
        // Verificar que la dependencia esté en estado adecuado
        if (dep_module->state != MODULE_STATE_RUNNING && 
            dep_module->state != MODULE_STATE_LOADED) {
            return E_MODULE_ERR;  // Dependencia no disponible
        }
    }
    
    return E_OK;
}

/**
 * Registra un módulo estático (compilado con el kernel)
 */
mid_t module_register_static(const char* name, module_entry_t* entry) {
    if (!initialized) {
        return E_NOT_IMPL;
    }
    
    if (name == NULL || entry == NULL) {
        return E_INVAL;
    }
    
    // Verificar si el módulo ya está cargado
    module_t* existing = module_find_by_name(name);
    if (existing != NULL) {
        return E_EXISTS;
    }
    
    // Verificar que no excedamos el límite de módulos
    if (module_count >= MAX_MODULES) {
        return E_NOMEM;
    }
    
    // Asignar memoria para el nuevo módulo
    module_t* new_module = (module_t*)kmalloc(sizeof(module_t));
    if (new_module == NULL) {
        return E_NOMEM;
    }
    
    // Inicializar el módulo
    new_module->mid = next_mid++;
    strncpy(new_module->name, name, MODULE_NAME_MAX - 1);
    new_module->name[MODULE_NAME_MAX - 1] = '\0';
    new_module->state = MODULE_STATE_LOADED;
    new_module->entry = entry;  // Asignar el entry point
    new_module->dependency_count = 0;
    new_module->load_time = timer_get_ticks();
    new_module->update_count = 0;
    new_module->message_count = 0;
    new_module->next = NULL;
    
    // Inicializar cola PMIC
    new_module->ipc_queue.head = NULL;
    new_module->ipc_queue.tail = NULL;
    new_module->ipc_queue.count = 0;
    
    // Añadir el módulo a la lista
    module_add_to_list(new_module);
    
    return new_module->mid;
}

/**
 * Envía un mensaje a un módulo por MID
 */
int module_send(mid_t mid, const void* msg, size_t size) {
    if (!initialized) {
        return E_NOT_IMPL;
    }
    
    if (msg == NULL || size == 0 || size > IPC_MAX_MESSAGE_SIZE) {
        return E_INVAL;
    }
    
    // Buscar el módulo
    module_t* module = module_find_by_mid(mid);
    if (module == NULL) {
        return E_NOENT;
    }
    
    // Verificar que el módulo esté en ejecución
    if (module->state != MODULE_STATE_RUNNING) {
        return E_INVAL;
    }
    
    // Verificar que no exceda el límite de mensajes en cola
    if (module->ipc_queue.count >= IPC_MAX_QUEUE_SIZE) {
        return E_BUSY;
    }
    
    // Crear nuevo mensaje
    ipc_queue_message_t* new_msg = (ipc_queue_message_t*)kmalloc(sizeof(ipc_queue_message_t));
    if (new_msg == NULL) {
        return E_NOMEM;
    }
    
    // Copiar datos del mensaje
    new_msg->sender_pid = 0;  // Por ahora, sender 0 = kernel/sistema
    new_msg->size = size;
    memcpy(new_msg->data, msg, size);
    new_msg->next = NULL;
    
    // Añadir a la cola (FIFO)
    if (module->ipc_queue.tail != NULL) {
        module->ipc_queue.tail->next = new_msg;
    } else {
        module->ipc_queue.head = new_msg;
    }
    module->ipc_queue.tail = new_msg;
    module->ipc_queue.count++;
    
    return E_OK;
}

/**
 * Envía un mensaje a un módulo por nombre
 */
int module_send_by_name(const char* name, const void* msg, size_t size) {
    if (!initialized || name == NULL) {
        return E_INVAL;
    }
    
    mid_t mid = module_get_id(name);
    if (mid == 0) {
        return E_NOENT;
    }
    
    return module_send(mid, msg, size);
}

/**
 * Envía un mensaje y espera respuesta (RPC a módulo)
 */
int module_call(mid_t mid, const void* request, size_t request_size, void* response, size_t* response_size) {
    if (!initialized) {
        return E_NOT_IMPL;
    }
    
    if (request == NULL || response == NULL || response_size == NULL) {
        return E_INVAL;
    }
    
    // Buscar el módulo
    module_t* module = module_find_by_mid(mid);
    if (module == NULL) {
        return E_NOENT;
    }
    
    // Verificar que el módulo esté en ejecución
    if (module->state != MODULE_STATE_RUNNING) {
        return E_INVAL;
    }
    
    // Verificar que el módulo tenga handler de mensajes
    if (module->entry == NULL || module->entry->handle_message == NULL) {
        return E_NOT_IMPL;
    }
    
    // Llamar al handler del módulo directamente (síncrono)
    int result = module->entry->handle_message(request, request_size, response, response_size);
    
    if (result == E_OK) {
        module->message_count++;
    }
    
    return result;
}

/**
 * Procesa mensajes pendientes de un módulo
 */
int module_process_messages(mid_t mid) {
    if (!initialized) {
        return E_NOT_IMPL;
    }
    
    // Buscar el módulo
    module_t* module = module_find_by_mid(mid);
    if (module == NULL) {
        return E_NOENT;
    }
    
    // Verificar que el módulo esté en ejecución
    if (module->state != MODULE_STATE_RUNNING) {
        return 0;
    }
    
    // Verificar que el módulo tenga handler de mensajes
    if (module->entry == NULL || module->entry->handle_message == NULL) {
        return 0;
    }
    
    int processed = 0;
    
    // Procesar todos los mensajes en cola
    while (module->ipc_queue.head != NULL) {
        ipc_queue_message_t* msg = module->ipc_queue.head;
        
        // Procesar mensaje (sin respuesta en este caso)
        size_t dummy_response_size = 0;
        module->entry->handle_message(msg->data, msg->size, NULL, &dummy_response_size);
        
        // Eliminar mensaje de la cola
        module->ipc_queue.head = msg->next;
        if (module->ipc_queue.head == NULL) {
            module->ipc_queue.tail = NULL;
        }
        module->ipc_queue.count--;
        
        // Liberar memoria
        kfree(msg);
        
        processed++;
        module->message_count++;
    }
    
    return processed;
}
