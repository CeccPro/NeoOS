# NeoOS - Modules
En NeoOS, los módulos son componentes esenciales del sistema operativo que proporcionan funcionalidades específicas y permiten la expansión y personalización del entorno operativo. Los módulos pueden ser cargados y descargados dinámicamente, lo que facilita la gestión de recursos y la adaptación del sistema a diferentes necesidades.

## Gestión de Módulos
El Module Manager de NeoOS es responsable de la gestión de los módulos del sistema. Este componente permite cargar, descargar y actualizar módulos de manera segura y eficiente. Los módulos se ubican en el directorio `/src/modules/`, y el Module Manager los gestiona desde allí.

## Estructura de Módulos
Cada módulo en NeoOS está estructurado de manera que incluye un identificador único (MID - Module ID) y puede interactuar con procesos a través del sistema PMIC (Process-Module Intercomunicator). Los módulos pueden recibir y responder a mensajes utilizando sus MID, facilitando la coordinación y la gestión de tareas en el sistema operativo. 

**Nota**: Los módulos NO usan IPC (Inter-Process Communication), que está reservado para comunicación entre procesos. La comunicación con módulos se realiza mediante PMIC.

### Ejemplo de un módulo simple:
```c
#include <module_manager.h>
#include <ipc.h>
#include <error.h>

int module_init() {
    // Código de inicialización del módulo
    return E_OK;
}

int module_cleanup() {
    // Código de limpieza del módulo
    return E_OK;
}

int module_start() {
    // Código de inicio del módulo (El que se ejecuta al iniciar el módulo, no el de inicialización)
    return E_OK;
}

int module_stop() {
    // Código de detención del módulo
    return E_OK;
}

int module_update() {
    // Código principal del módulo. Aquí va la lógica del módulo.
    return E_OK;
}

typedef struct {
    int (*init)();
    int (*cleanup)();
    int (*start)();
    int (*stop)();
    int (*update)();
} module_entry_t;

module_entry_t get() {
    module_entry_t entry = {
        .init = module_init,
        .cleanup = module_cleanup,
        .start = module_start,
        .stop = module_stop,
        .update = module_update
    };
    return entry;
}
```