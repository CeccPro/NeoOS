# NeoOS - Errores
Este documento explica `error.h` y los códigos de error utilizados en NeoOS. Estos códigos de error son fundamentales para la gestión de errores en el sistema operativo, permitiendo a los desarrolladores identificar y manejar problemas de manera eficiente.

## Definición de `error.h`
El archivo `error.h` define una serie de constantes que representan diferentes códigos de error. Estos códigos son utilizados en todo el sistema operativo para indicar el estado de las operaciones y facilitar la depuración. A continuación se presentan algunos los códigos de error definidos en `error.h`:
```c
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
```

## Descripción de los Códigos de Error
Cada código de error tiene un significado específico que ayuda a los desarrolladores a entender qué salió mal durante una operación. A continuación se describen algunos de los códigos de error más comunes:
- `E_OK (0)`: Indica que la operación se completó con éxito sin errores.
- `E_UNKNOWN (-1)`: Indica que ocurrió un error desconocido que no pudo ser clasificado.
- `E_NOMEM (-2)`: Indica que no hay suficiente memoria disponible para completar la operación solicitada.
- `E_INVAL (-3)`: Indica que uno o más argumentos proporcionados a una función son inválidos.
- `E_NOENT (-4)`: Indica que la entrada solicitada (como un archivo o recurso) no se encontró.
- `E_EXISTS (-5)`: Indica que la entrada que se intenta crear ya existe.
- `E_BUSY (-6)`: Indica que el recurso solicitado está actualmente ocupado y no puede ser accedido.
- `E_IO (-7)`: Indica que ocurrió un error durante una operación de entrada/salida.
- `E_PERM (-8)`: Indica que la operación fue denegada debido a permisos insuficientes.
- `E_TIMEOUT (-9)`: Indica que la operación no se completó dentro del tiempo esperado.

Para más detalles sobre errores específicos, lea la documentación del error detallada en ./docs/errors/[Error_Name].md

## Uso de los Códigos de Error
Estos códigos de error son utilizados en diversas funciones y módulos del sistema operativo para comunicar el estado de las operaciones. Los desarrolladores deben verificar estos códigos después de realizar operaciones críticas para manejar adecuadamente los errores y garantizar la estabilidad del sistema.

Por ejemplo, una función que intenta abrir un archivo puede devolver `E_NOENT` si el archivo no existe, permitiendo al desarrollador tomar las medidas adecuadas, como crear el archivo o notificar al usuario.

## Cómo usar `error.h`
Para utilizar los códigos de error definidos en `error.h`, simplemente incluya el archivo en su código fuente y utilice las constantes definidas para verificar el estado de las operaciones. Aquí hay un ejemplo básico:
```c
#include <error.h>
int result = open_file("mi_archivo.txt");
if (result == E_NOENT) {
    // Manejar el caso donde el archivo no existe
} else if (result != E_OK) {
    // Manejar otros errores
}
```

## Conclusión
El archivo `error.h` y los códigos de error definidos en él son esenciales para la gestión de errores en NeoOS. Proporcionan una forma estandarizada de comunicar el estado de las operaciones, facilitando la depuración y el manejo de errores en el sistema operativo. Los desarrolladores deben familiarizarse con estos códigos para garantizar un desarrollo robusto y eficiente en NeoOS.