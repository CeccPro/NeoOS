# NeoOS - Module Manager
El Module Manager de NeoOS es un componente crucial del sistema operativo que se encarga de la gestión y administración de los módulos del kernel. Su función principal es cargar, descargar y gestionar los módulos del sistema, permitiendo una mayor flexibilidad y personalización del entorno operativo.

## Funciones Principales
1. **Carga de Módulos**: El Module Manager permite cargar módulos del kernel en tiempo de ejecución, facilitando la adición de nuevas funcionalidades sin necesidad de reiniciar el sistema.
2. **Descarga de Módulos**: Cuando un módulo ya no es necesario, el Module Manager puede descargarlo del sistema, liberando recursos y manteniendo el sistema optimizado.
3. **Gestión de Dependencias**: El Module Manager se encarga de gestionar las dependencias entre módulos, asegurando que todos los módulos necesarios estén cargados antes de activar uno nuevo.
4. **Actualización de Módulos**: Permite actualizar módulos del kernel de manera segura, garantizando la estabilidad del sistema durante el proceso.

## Ejemplos de Uso
### Carga de Módulos
Para cargar un módulo, se puede utilizar el comando `modload` seguido del nombre del módulo:
```nsh
ss modload nombre_del_modulo
```
O desde la librería de C/C++:
```c
#include <module_manager.h>
#include <error.h>
int result = load_module("nombre_del_modulo");
if (result == 0) {
    return E_OK;
} else {
    return E_MODULE_ERR;
}
```

### Descarga de Módulos
Para descargar un módulo, se utiliza el comando `modunload`:
```nsh
ss modunload nombre_del_modulo
```
O desde la librería de C/C++:
```c
#include <module_manager.h>
#include <error.h>
int result = unload_module("nombre_del_modulo");
if (result == 0) {
    return E_OK;
} else {
    return E_MODULE_ERR;
}
```

**Nota: Para cargar/descargar módulos, se requieren privilegios de administrador (ss).**

### Carga al Inicio
El Module Manager puede configurarse para cargar ciertos módulos automáticamente durante el arranque del sistema. Esto se puede lograr editando el archivo de configuración `modules.conf`, donde se especifican los módulos que deben cargarse al inicio. Ejemplo de entrada en `modules.json`:
```json
{
  "modules": [
    {
      "name": "nombre_del_modulo",
      "options": "opcion1, opcion2",
      "load_at_startup": true
    }
  ]
}
```
### Obtener status de un módulo
Para verificar el estado de un módulo cargado, se puede utilizar el comando `modstatus`:
```nsh
ss modstatus nombre_del_modulo
```
Y devuelve el modstatus del módulo, indicando si está cargado o no.

Los módulos se ubican en el directorio `/src/modules/`, y el Module Manager los gestiona desde allí. **Nota: El nombre del módulo debe coincidir con el nombre del archivo**

## Buenas Prácticas
- **Verificación de Módulos**: Antes de cargar un módulo, asegúrese de que es compatible con la versión actual del kernel.
- **Monitoreo de Recursos**: Después de cargar o descargar módulos, monitoree el uso de recursos del sistema para garantizar un rendimiento óptimo.
- **Documentación**: Mantenga una documentación clara de los módulos cargados y sus funcionalidades para facilitar la administración del sistema.