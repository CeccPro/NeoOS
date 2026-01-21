# NeoOS - sys_sbrk
La syscall `sys_sbrk(int increment)` en NeoOS se utiliza para extender o contraer el heap (montículo) del proceso, que es la región de memoria dinámica utilizada por funciones como `malloc` y `free`. Esta syscall permite al proceso solicitar o liberar memoria dinámica.

## Prototipo
```c
void *sys_sbrk(int increment);
```

## Parámetros
- `increment`: El número de bytes para extender (si es positivo) o contraer (si es negativo) el heap del proceso. Un valor de 0 devuelve la posición actual del límite del heap sin modificarlo.

## Comportamiento
Cuando un proceso llama a `sys_sbrk`, el sistema operativo realiza las siguientes acciones:
1. Obtiene la posición actual del límite del heap del proceso (program break).
2. Si `increment` es 0, simplemente devuelve la posición actual sin cambios.
3. Si `increment` es positivo, intenta extender el heap:
   - Verifica que hay suficiente memoria disponible.
   - Verifica que la extensión no exceda los límites del proceso.
   - Asigna las páginas de memoria necesarias.
   - Actualiza el límite del heap.
4. Si `increment` es negativo, intenta contraer el heap:
   - Verifica que la contracción no vaya más allá del inicio del heap.
   - Libera las páginas de memoria que ya no se necesitan.
   - Actualiza el límite del heap.
5. Devuelve un puntero a la posición anterior del límite del heap.

## Valor de Retorno
- Devuelve un puntero a la posición anterior del límite del heap si la syscall se ejecuta correctamente.
- Para `increment` de 0, devuelve la posición actual del límite del heap.
- Devuelve `NULL` o un valor especial de error si ocurre un problema, como:
  - `E_NOMEM`: Si no hay suficiente memoria disponible para la extensión solicitada.
  - `E_INVAL`: Si el incremento resultaría en un heap inválido (por ejemplo, contraer más allá del inicio).
  - Otros códigos de error definidos en `error.h`.

## Ejemplo de Uso
```c
#include <error.h>
#include <syscalls.h>
void main() {
    // Obtener la posición actual del heap
    void *heap_actual = sys_sbrk(0);
    
    // Extender el heap por 4096 bytes
    void *heap_anterior = sys_sbrk(4096);
    
    if (heap_anterior != NULL) {
        // Ahora tenemos 4096 bytes adicionales disponibles
        // desde heap_anterior hasta heap_anterior + 4096
        char *mi_memoria = (char *)heap_anterior;
        
        // Usar la memoria...
        for (int i = 0; i < 4096; i++) {
            mi_memoria[i] = 0;
        }
        
        // Opcionalmente, contraer el heap cuando ya no se necesite
        sys_sbrk(-4096);
    }
}
```

## Ejemplo de Implementación Simple de malloc
```c
#include <syscalls.h>
void *simple_malloc(int size) {
    // Alinear a múltiplo de 8 bytes
    size = (size + 7) & ~7;
    
    void *ptr = sys_sbrk(size);
    
    if (ptr == NULL || ptr == (void *)-1) {
        return NULL;  // Fallo al asignar memoria
    }
    
    return ptr;
}

void simple_free(void *ptr, int size) {
    // Nota: Este es un ejemplo muy simplificado
    // Una implementación real necesitaría tracking más sofisticado
    (void)ptr;  // No usado en este ejemplo simple
    sys_sbrk(-size);
}
```

## Notas
- `sys_sbrk` es una syscall de bajo nivel típicamente utilizada por implementaciones de asignadores de memoria como `malloc`, no directamente por aplicaciones.
- El uso directo de `sys_sbrk` puede ser complicado y propenso a errores. Generalmente es mejor usar `malloc`/`free`.
- Las páginas de memoria asignadas con `sys_sbrk` están inicializadas a cero por seguridad.
- Contraer el heap no garantiza que la memoria se devuelva inmediatamente al sistema; puede permanecer reservada para el proceso.
- Múltiples llamadas a `sys_sbrk` con valores pequeños pueden ser ineficientes; es mejor solicitar bloques más grandes.
- El heap crece hacia direcciones más altas en memoria, mientras que la pila típicamente crece hacia direcciones más bajas.

## Véase también
- [sys_mmap](./sys_mmap.md)
- [sys_munmap](./sys_munmap.md)
- [Memory Manager](../Memory%20Manager.md)
