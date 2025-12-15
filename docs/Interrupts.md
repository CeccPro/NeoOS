# NeoOS - Interrupciones
El sistema de interrupciones en NeoOS es un componente crucial que permite la gestión eficiente de eventos tanto de hardware como de software. Las interrupciones permiten que el sistema operativo responda rápidamente a eventos externos, como señales de dispositivos periféricos, o internos, como excepciones y llamadas al sistema.

## Tipos de Interrupciones
1. **Interrupciones de Hardware**: Generadas por dispositivos externos, como teclados, ratones, temporizadores y controladores de red. Estas interrupciones notifican al sistema operativo que un dispositivo requiere atención.
2. **Interrupciones de Software**: Generadas por el propio software, como excepciones (división por cero, violaciones de acceso) y llamadas al sistema (syscalls). Estas interrupciones permiten que el sistema operativo maneje eventos internos de manera controlada.

## Manejadores de Interrupciones
Cada tipo de interrupción tiene un manejador asociado, que es una función especial encargada de procesar la interrupción. Los manejadores de interrupciones deben ser rápidos y eficientes, ya que interrumpen el flujo normal de ejecución del sistema. Los pasos generales para manejar una interrupción son:
1. Guardar el estado del proceso actual.
2. Identificar la fuente de la interrupción.
3. Ejecutar el manejador correspondiente.
4. Restaurar el estado del proceso y reanudar la ejecución.

## Tabla de Vectores de Interrupción
NeoOS utiliza una tabla de vectores de interrupción (IVT) para mapear cada tipo de interrupción a su manejador correspondiente. La IVT es una estructura de datos que contiene punteros a las funciones manejadoras de interrupciones. Cuando ocurre una interrupción, el sistema operativo consulta la IVT para determinar qué función ejecutar. La IVT se inicializa durante el proceso de arranque del sistema y puede ser modificada dinámicamente para agregar o cambiar manejadores de interrupciones según sea necesario.

## Instrucción de Interrupción
NeoOS utiliza la instrucción `int` para generar interrupciones de software. Por ejemplo, la instrucción `int 0x80` se utiliza comúnmente para invocar llamadas al sistema en sistemas basados en x86. Al ejecutar esta instrucción, el procesador transfiere el control al manejador de interrupciones correspondiente definido en la IVT.

## Prioridades de Interrupciones
NeoOS implementa un sistema de prioridades para las interrupciones, lo que permite que ciertas interrupciones tengan preferencia sobre otras. Esto es especialmente importante en sistemas con múltiples dispositivos que pueden generar interrupciones simultáneamente. Las prioridades se asignan de la siguiente manera:
- Interrupciones críticas (por ejemplo, temporizadores del sistema) tienen la máxima prioridad.
- Interrupciones de dispositivos periféricos tienen una prioridad media.
- Interrupciones de software tienen la prioridad más baja.