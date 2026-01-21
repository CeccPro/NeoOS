# NeoOS - Sistema de Errores
Este documento describe el sistema de manejo de errores en NeoOS, incluyendo los códigos de error, la función de conversión a string, y las mejores prácticas para el manejo de errores en el kernel.

## Definición de Códigos de Error

### Ubicación
- **Header**: `src/kernel/core/include/error.h`
- **Implementación**: `src/kernel/core/src/error.c`

### Códigos de Error Estándar

El archivo `error.h` define una serie de constantes que representan diferentes códigos de error. Estos códigos son utilizados en todo el sistema operativo para indicar el estado de las operaciones y facilitar la depuración.

```c
// Códigos de error generales
#define E_OK            0   // Operación exitosa
#define E_UNKNOWN      -1   // Error desconocido
#define E_NOMEM        -2   // Memoria insuficiente
#define E_INVAL        -3   // Argumento inválido
#define E_NOENT        -4   // Entrada no encontrada
#define E_EXISTS       -5   // Entrada ya existe
#define E_BUSY         -6   // Recurso ocupado
#define E_IO           -7   // Error de entrada/salida
#define E_PERM         -8   // Permiso denegado
#define E_TIMEOUT      -9   // Operación agotó el tiempo
#define E_MODULE_ERR   -10  // Error de módulo
#define E_NOT_IMPL     -11  // Funcionalidad no implementada
#define E_NOT_SUPPORTED -12 // Funcionalidad no soportada
```

## Descripción Detallada de los Códigos

### E_OK (0)
- **Significado**: La operación se completó exitosamente sin errores
- **Uso**: Valor de retorno estándar para funciones que se ejecutan correctamente
- **Ejemplo**: `pmm_init()` retorna `E_OK` si logra inicializar el bitmap de páginas

### E_UNKNOWN (-1)
- **Significado**: Ocurrió un error desconocido que no pudo ser clasificado
- **Uso**: Para situaciones excepcionales donde no se puede determinar la causa exacta
- **Ejemplo**: Errores inesperados en hardware o estados inconsistentes

### E_NOMEM (-2)
- **Significado**: No hay suficiente memoria disponible para completar la operación
- **Uso**: Cuando `pmm_alloc_page()` no encuentra páginas libres, o `kmalloc()` falla
- **Ejemplo**: `heap_init()` retorna `E_NOMEM` si no puede expandir el heap inicial

### E_INVAL (-3)
- **Significado**: Uno o más argumentos proporcionados a una función son inválidos
- **Uso**: Validación de parámetros de entrada
- **Ejemplo**: `pmm_init()` retorna `E_INVAL` si no hay información de memoria en Multiboot

### E_NOENT (-4)
- **Significado**: La entrada solicitada (como un archivo o recurso) no se encontró
- **Uso**: Operaciones de búsqueda que fallan
- **Ejemplo**: Cuando se intenta abrir un archivo que no existe (futuro)

### E_EXISTS (-5)
- **Significado**: La entrada que se intenta crear ya existe
- **Uso**: Prevención de duplicados
- **Ejemplo**: Intentar crear un archivo que ya existe (futuro)

### E_BUSY (-6)
- **Significado**: El recurso solicitado está actualmente ocupado y no puede ser accedido
- **Uso**: Sincronización y exclusión mutua
- **Ejemplo**: Intentar acceder a un dispositivo que está siendo usado (futuro)

### E_IO (-7)
- **Significado**: Ocurrió un error durante una operación de entrada/salida
- **Uso**: Errores de comunicación con hardware
- **Ejemplo**: Fallo al leer/escribir en disco (futuro)

### E_PERM (-8)
- **Significado**: La operación fue denegada debido a permisos insuficientes
- **Uso**: Control de acceso y seguridad
- **Ejemplo**: Intento de acceder a memoria del kernel desde modo usuario (futuro)

### E_TIMEOUT (-9)
- **Significado**: La operación no se completó dentro del tiempo esperado
- **Uso**: Operaciones con límite de tiempo
- **Ejemplo**: Esperar respuesta de un dispositivo que no responde (futuro)

### E_MODULE_ERR (-10)
- **Significado**: Error al cargar o ejecutar un módulo
- **Uso**: Sistema de módulos dinámicos
- **Ejemplo**: Fallo al cargar un módulo del kernel (futuro)

### E_NOT_IMPL (-11)
- **Significado**: La funcionalidad solicitada no está implementada aún
- **Uso**: Marcar características planificadas pero no implementadas
- **Ejemplo**: Syscalls que aún no están implementadas

### E_NOT_SUPPORTED (-12)
- **Significado**: La funcionalidad solicitada no es soportada por el sistema
- **Uso**: Características que el hardware o arquitectura no soporta
- **Ejemplo**: Intentar usar características de CPU no disponibles

## Función de Conversión a String

### Declaración
```c
const char* error_to_string(int error);
```

### Implementación
```c
const char* error_to_string(int error) {
    switch (error) {
        case E_OK:            return "E_OK";
        case E_UNKNOWN:       return "E_UNKNOWN";
        case E_NOMEM:         return "E_NOMEM";
        case E_INVAL:         return "E_INVAL";
        case E_NOENT:         return "E_NOENT";
        case E_EXISTS:        return "E_EXISTS";
        case E_BUSY:          return "E_BUSY";
        case E_IO:            return "E_IO";
        case E_PERM:          return "E_PERM";
        case E_TIMEOUT:       return "E_TIMEOUT";
        case E_MODULE_ERR:    return "E_MODULE_ERR";
        case E_NOT_IMPL:      return "E_NOT_IMPL";
        case E_NOT_SUPPORTED: return "E_NOT_SUPPORTED";
        default:              return "E_UNKNOWN";
    }
}
```

### Propósito
- Convierte códigos de error numéricos en strings legibles
- Facilita la depuración mostrando nombres de error en lugar de números
- Se usa en mensajes de error del VGA para mostrar al usuario qué falló

## Uso del Sistema de Errores

### Patrón de Retorno de Funciones

Las funciones del kernel que pueden fallar siguen este patrón:

```c
int function_that_can_fail(parameters) {
    // Realizar operación
    if (algo_salió_mal) {
        return E_NOMEM;  // o el código apropiado
    }
    
    // Operación exitosa
    return E_OK;
}
```

### Patrón de Verificación de Errores

El código que llama a funciones debe verificar el retorno:

```c
int result = function_that_can_fail(params);
if (result != E_OK) {
    // Manejar el error
    vga_write("Error: ");
    vga_write(error_to_string(result));
    vga_write("\n");
    return result;  // Propagar el error
}

// Continuar con operación exitosa
```

### Ejemplo Real: Inicialización del Memory Manager

En `src/kernel/core/src/kmain.c`:

```c
int mm_result = memory_init(mbi, kdebug, kverbose);
if (mm_result != E_OK) {
    vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
    vga_write("[FAIL] Error al inicializar el Memory Manager\n");
    vga_write("Codigo de error: ");
    vga_write(error_to_string(mm_result));
    vga_write("\n");
    
    // Detener el kernel
    while(1) {
        __asm__ volatile("hlt");
    }
}
```

Este código:
1. Llama a `memory_init()` que retorna un código de error
2. Verifica si el resultado es diferente de `E_OK`
3. Si hay error:
   - Cambia el color a rojo para resaltar
   - Muestra mensaje descriptivo
   - Usa `error_to_string()` para mostrar el nombre del error
   - Detiene el kernel de forma segura con `hlt`

## Manejo de Errores Durante la Inicialización

### Comportamiento Estándar

Cuando un componente crítico falla durante la inicialización:
1. **Reportar**: Mostrar mensaje de error en rojo con VGA
2. **Identificar**: Usar `error_to_string()` para mostrar el tipo de error
3. **Detener**: Ejecutar `cli` (deshabilitar interrupciones) y `hlt` (detener CPU)
4. **Bucle infinito**: Prevenir que el kernel continúe en estado inconsistente

### Ejemplo de Función que Retorna Errores

```c
int pmm_init(multiboot_info_t* mbi, bool kdebug, bool kverbose) {
    // Verificar que tenemos información de memoria
    if (!(mbi->flags & MULTIBOOT_INFO_MEMORY)) {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_write("[PMM] [FAIL] No hay informacion de memoria de Multiboot\n");
        return E_INVAL;  // Argumento inválido
    }
    
    // ... realizar inicialización ...
    
    if (pmm_free_pages == 0) {
        return E_NOMEM;  // Sin memoria disponible
    }
    
    return E_OK;  // Éxito
}
```

## Propagación de Errores

Los errores deben propagarse hacia arriba en la cadena de llamadas:

```c
int memory_init(multiboot_info_t* mbi, bool kdebug, bool kverbose) {
    // Inicializar PMM
    int result = pmm_init(mbi, kdebug, kverbose);
    if (result != E_OK) {
        return result;  // Propagar el error
    }
    
    // Inicializar VMM
    result = vmm_init(kdebug, kverbose);
    if (result != E_OK) {
        return result;  // Propagar el error
    }
    
    // Inicializar Heap
    result = heap_init(KERNEL_HEAP_START, KERNEL_HEAP_SIZE, kdebug, kverbose);
    if (result != E_OK) {
        return result;  // Propagar el error
    }
    
    return E_OK;
}
```

## Mejores Prácticas

### 1. Usar Códigos Apropiados
- No usar `E_UNKNOWN` a menos que sea realmente desconocido
- Elegir el código que mejor describa la causa del error
- Agregar nuevos códigos si los existentes no son suficientes

### 2. Mensajes Descriptivos
```c
// Bueno: Mensaje descriptivo + código de error
vga_write("[PMM] Error al inicializar: ");
vga_write(error_to_string(result));

// Malo: Solo el código numérico
vga_write("Error: -2");
```

### 3. Verificar Siempre el Retorno
```c
// Bueno
int result = critical_operation();
if (result != E_OK) {
    handle_error(result);
}

// Malo: Ignorar el error
critical_operation();  // ¿Y si falla?
```

### 4. Liberar Recursos en Caso de Error
```c
int complex_operation() {
    void* resource = allocate_resource();
    if (resource == NULL) {
        return E_NOMEM;
    }
    
    int result = do_something(resource);
    if (result != E_OK) {
        free_resource(resource);  // Limpiar antes de retornar
        return result;
    }
    
    return E_OK;
}
```

## Estado Actual de Implementación

### Componentes que Usan el Sistema de Errores (Implementado)
- `pmm_init()`: Retorna `E_OK` o `E_INVAL`
- `vmm_init()`: Retorna `E_OK`
- `heap_init()`: Retorna `E_OK` o `E_NOMEM`
- `memory_init()`: Coordina y propaga errores de los tres anteriores
- `error_to_string()`: Convierte códigos a strings

### Uso en Kernel Panic (Implementado)
Cuando ocurre una excepción no manejada, el sistema muestra:
- Nombre de la excepción
- Código de error (si aplica)
- Estado de registros
- Todo en color rojo sobre fondo negro para máxima visibilidad

## Extensión del Sistema de Errores

Para agregar un nuevo código de error:

1. **Definir en `error.h`**:
```c
#define E_NEW_ERROR -13  // Descripción del error
```

2. **Agregar a `error_to_string()` en `error.c`**:
```c
const char* error_to_string(int error) {
    switch (error) {
        // ... casos existentes ...
        case E_NEW_ERROR:  return "E_NEW_ERROR";
        default:           return "E_UNKNOWN";
    }
}
```

3. **Documentar en este archivo**:
- Agregar descripción detallada
- Explicar cuándo usar el código
- Proporcionar ejemplos de uso

## Documentación Individual de Errores

Para información más detallada sobre errores específicos, consulta la documentación individual en el directorio `docs/errors/`:

- [E_OK.md](./errors/E_OK.md)
- [E_UNKNOWN.md](./errors/E_UNKNOWN.md)
- [E_NOMEM.md](./errors/E_NOMEM.md)
- [E_INVAL.md](./errors/E_INVAL.md)
- [E_NOENT.md](./errors/E_NOENT.md)
- [E_EXISTS.md](./errors/E_EXISTS.md)
- [E_BUSY.md](./errors/E_BUSY.md)
- [E_IO.md](./errors/E_IO.md)
- [E_PERM.md](./errors/E_PERM.md)
- [E_TIMEOUT.md](./errors/E_TIMEOUT.md)
- [E_MODULE_ERR.md](./errors/E_MODULE_ERR.md)
- [E_NOT_IMPL.md](./errors/E_NOT_IMPL.md)
- [E_NOT_SUPPORTED.md](./errors/E_NOT_SUPPORTED.md)

## Véase También
- [Boot Process](./Boot%20Process.md) - Proceso de arranque del sistema
- [Kernel Initialization](./Kernel%20Initialization.md) - Inicialización del kernel
- [Memory Manager](./Memory%20Manager.md) - Gestión de memoria
- [Interrupts](./Interrupts.md) - Sistema de interrupciones
