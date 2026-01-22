/**
 * NeoOS - Module Manager
 * Sistema de gestión de módulos del kernel
 */

#ifndef _KERNEL_MODULE_H
#define _KERNEL_MODULE_H

#include "../../lib/include/types.h"
#include "error.h"
#include "ipc.h"

/**
 * Estado de un módulo
 */
typedef enum {
    MODULE_STATE_UNLOADED = 0,    // No cargado
    MODULE_STATE_LOADING,         // En proceso de carga
    MODULE_STATE_LOADED,          // Cargado pero no iniciado
    MODULE_STATE_RUNNING,         // En ejecución
    MODULE_STATE_STOPPED,         // Detenido
    MODULE_STATE_UNLOADING        // En proceso de descarga
} module_state_t;

/**
 * Número máximo de módulos simultáneos
 */
#define MAX_MODULES 64

/**
 * Longitud máxima del nombre de un módulo
 */
#define MODULE_NAME_MAX 32

/**
 * Estructura de entrada de un módulo
 * Define las funciones que un módulo debe implementar
 */
typedef struct {
    int (*init)(void);      // Inicialización del módulo
    int (*cleanup)(void);   // Limpieza del módulo
    int (*start)(void);     // Inicio del módulo
    int (*stop)(void);      // Detención del módulo
    int (*update)(void);    // Actualización/tick del módulo
    int (*handle_message)(const void* msg, size_t size, void* response, size_t* response_size);  // Handler de mensajes IPC
} module_entry_t;

/**
 * Estructura de información de un módulo
 */
typedef struct module {
    mid_t mid;                          // Module ID
    char name[MODULE_NAME_MAX];         // Nombre del módulo
    module_state_t state;               // Estado actual
    module_entry_t* entry;              // Punto de entrada del módulo
    
    // Cola IPC del módulo (usar typedef de ipc.h)
    ipc_queue_t ipc_queue;
    
    // Información de dependencias
    mid_t dependencies[MAX_MODULES];    // MIDs de módulos requeridos
    uint32_t dependency_count;          // Número de dependencias
    
    // Estadísticas
    uint32_t load_time;                 // Tick de carga
    uint32_t update_count;              // Número de actualizaciones
    uint32_t message_count;             // Número de mensajes procesados
    
    // Enlaces para la lista de módulos
    struct module* next;                // Siguiente módulo en la lista
} module_t;

/**
 * Inicializa el Module Manager
 * @param verbose: Modo verbose para debugging
 * @return E_OK si éxito, código de error en caso contrario
 */
int module_manager_init(bool verbose);

/**
 * Carga un módulo en el sistema
 * @param name: Nombre del módulo a cargar
 * @return MID del módulo cargado, o valor negativo en caso de error
 */
mid_t module_load(const char* name);

/**
 * Descarga un módulo del sistema
 * @param mid: Module ID del módulo a descargar
 * @return E_OK si éxito, código de error en caso contrario
 */
int module_unload(mid_t mid);

/**
 * Inicia un módulo cargado
 * @param mid: Module ID del módulo a iniciar
 * @return E_OK si éxito, código de error en caso contrario
 */
int module_start(mid_t mid);

/**
 * Detiene un módulo en ejecución
 * @param mid: Module ID del módulo a detener
 * @return E_OK si éxito, código de error en caso contrario
 */
int module_stop(mid_t mid);

/**
 * Actualiza todos los módulos en ejecución
 * Llama a la función update() de cada módulo
 * @return E_OK si éxito, código de error en caso contrario
 */
int module_update_all(void);

/**
 * Obtiene el estado de un módulo
 * @param mid: Module ID del módulo
 * @return Estado del módulo, o MODULE_STATE_UNLOADED si no existe
 */
module_state_t module_get_state(mid_t mid);

/**
 * Obtiene el MID de un módulo por su nombre
 * @param name: Nombre del módulo
 * @return MID del módulo, o 0 si no existe
 */
mid_t module_get_id(const char* name);

/**
 * Obtiene el nombre de un módulo por su MID
 * @param mid: Module ID del módulo
 * @return Nombre del módulo, o NULL si no existe
 */
const char* module_get_name(mid_t mid);

/**
 * Lista todos los módulos cargados
 * @param buffer: Buffer donde se escribirá la lista
 * @param buffer_size: Tamaño del buffer
 * @return Número de módulos listados
 */
int module_list(char* buffer, size_t buffer_size);

/**
 * Obtiene información detallada de un módulo
 * @param mid: Module ID del módulo
 * @return Puntero a la estructura del módulo, o NULL si no existe
 */
const module_t* module_get_info(mid_t mid);

/**
 * Verifica las dependencias de un módulo
 * @param mid: Module ID del módulo
 * @return E_OK si todas las dependencias están cargadas, código de error en caso contrario
 */
int module_check_dependencies(mid_t mid);

/**
 * Registra un módulo estático (compilado con el kernel)
 * @param name: Nombre del módulo
 * @param entry: Punto de entrada del módulo
 * @return MID del módulo registrado, o valor negativo en caso de error
 */
mid_t module_register_static(const char* name, module_entry_t* entry);

/**
 * Envía un mensaje a un módulo por MID
 * @param mid: Module ID del destinatario
 * @param msg: Buffer con el mensaje
 * @param size: Tamaño del mensaje
 * @return E_OK si éxito, código de error en caso contrario
 */
int module_send(mid_t mid, const void* msg, size_t size);

/**
 * Envía un mensaje a un módulo por nombre
 * @param name: Nombre del módulo
 * @param msg: Buffer con el mensaje
 * @param size: Tamaño del mensaje
 * @return E_OK si éxito, código de error en caso contrario
 */
int module_send_by_name(const char* name, const void* msg, size_t size);

/**
 * Envía un mensaje y espera respuesta (RPC a módulo)
 * @param mid: Module ID del destinatario
 * @param request: Buffer con la petición
 * @param request_size: Tamaño de la petición
 * @param response: Buffer para la respuesta
 * @param response_size: Tamaño del buffer de respuesta (entrada/salida)
 * @return E_OK si éxito, código de error en caso contrario
 */
int module_call(mid_t mid, const void* request, size_t request_size, void* response, size_t* response_size);

/**
 * Procesa mensajes pendientes de un módulo
 * @param mid: Module ID del módulo
 * @return Número de mensajes procesados, o código de error negativo
 */
int module_process_messages(mid_t mid);

#endif /* _KERNEL_MODULE_H */
