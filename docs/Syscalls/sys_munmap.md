# NeoOS - sys_munmap
La syscall `sys_munmap(void *addr, int size)` en NeoOS se utiliza para desactivar un mapeo de memoria previamente creado con `sys_mmap`, liberando la región del espacio de direcciones virtuales del proceso.

## Prototipo
```c
int sys_munmap(void *addr, int size);
```

## Parámetros
- `addr`: La dirección de inicio de la región mapeada que se desea desactivar. Debe ser la dirección devuelta por una llamada previa a `sys_mmap`.
- `size`: El tamaño en bytes de la región a desactivar. Debe coincidir con el tamaño especificado en la llamada a `sys_mmap` correspondiente.

## Comportamiento
Cuando un proceso llama a `sys_munmap`, el sistema operativo realiza las siguientes acciones:
1. Verifica que la dirección y el tamaño correspondan a una región válida previamente mapeada.
2. Si hay cambios pendientes en la memoria mapeada (escrituras no sincronizadas), los escribe al archivo subyacente.
3. Elimina las entradas de la tabla de páginas que mapean la región virtual.
4. Libera la región del espacio de direcciones virtuales del proceso.
5. Decrementa el contador de referencias del archivo si estaba asociado a un mapeo de archivo.

## Valor de Retorno
- Devuelve `E_OK` (0) si la syscall se ejecuta correctamente y el mapeo fue desactivado.
- Devuelve un código de error (negativo) si ocurre un problema, como:
  - `E_INVAL`: Si la dirección o el tamaño son inválidos, o la región no estaba mapeada.
  - `E_IO`: Si ocurrió un error al escribir cambios pendientes al archivo.
  - Otros códigos de error definidos en `error.h`.

## Ejemplo de Uso
```c
#include <error.h>
#include <syscalls.h>
void main() {
    int fd = sys_open("/home/usuario/datos.bin", O_RDWR);
    
    if (fd >= 0) {
        int size = 4096;
        char *mapped = (char *)sys_mmap(fd, 0, size);
        
        if (mapped != NULL) {
            // Usar la memoria mapeada
            for (int i = 0; i < size; i++) {
                mapped[i] = (char)i;
            }
            
            // Desactivar el mapeo
            int result = sys_munmap(mapped, size);
            if (result != E_OK) {
                sys_write(STDERR, "Error al desactivar mapeo\n", 27);
            }
        }
        
        sys_close(fd);
    }
}
```

## Ejemplo de Gestión de Recursos
```c
#include <syscalls.h>
void procesar_archivo_mapeado(const char *filename) {
    int fd = sys_open(filename, O_RDONLY);
    if (fd < 0) return;
    
    int size = 1024 * 1024;  // 1 MB
    void *mapped = sys_mmap(fd, 0, size);
    
    if (mapped != NULL) {
        // Procesar datos mapeados
        procesar_datos(mapped, size);
        
        // Siempre desactivar el mapeo antes de salir
        sys_munmap(mapped, size);
    }
    
    sys_close(fd);
}
```

## Notas
- Es fundamental llamar a `sys_munmap` para cada región mapeada con `sys_mmap` para evitar fugas de recursos.
- Acceder a la memoria después de llamar a `sys_munmap` resulta en comportamiento indefinido, típicamente fallos de segmentación.
- Los cambios realizados en la memoria mapeada se escriben al archivo durante `sys_munmap`, pero esto no garantiza que se hayan escrito físicamente al disco.
- Si un proceso termina sin llamar a `sys_munmap`, el sistema operativo automáticamente limpia todos sus mapeos.
- La dirección y el tamaño deben coincidir exactamente con los usados en `sys_mmap`; no es posible desactivar parcialmente un mapeo.
- Desactivar un mapeo no cierra el descriptor de archivo asociado; esto debe hacerse separadamente con `sys_close`.

## Véase también
- [sys_mmap](./sys_mmap.md)
- [sys_close](./sys_close.md)
- [sys_exit](./sys_exit.md)
- [Memory Manager](../Memory%20Manager.md)
