# NeoOS - sys_mmap
La syscall `sys_mmap(int fd, int offset, int size)` en NeoOS se utiliza para mapear un archivo (o una porción de él) en el espacio de direcciones virtuales del proceso, permitiendo acceso directo a través de punteros de memoria en lugar de operaciones de lectura/escritura tradicionales.

## Prototipo
```c
void *sys_mmap(int fd, int offset, int size);
```

## Parámetros
- `fd`: El descriptor de archivo del archivo que se desea mapear en memoria. Debe ser un descriptor válido obtenido mediante `sys_open`.
- `offset`: El desplazamiento en bytes desde el inicio del archivo donde comenzará el mapeo. Típicamente debe estar alineado a páginas.
- `size`: El tamaño en bytes de la región del archivo a mapear en memoria.

## Comportamiento
Cuando un proceso llama a `sys_mmap`, el sistema operativo realiza las siguientes acciones:
1. Verifica que el descriptor de archivo sea válido y que el proceso tenga permisos adecuados.
2. Valida que el offset y el tamaño sean valores razonables y que no excedan el tamaño del archivo.
3. Reserva una región en el espacio de direcciones virtuales del proceso.
4. Configura las entradas de la tabla de páginas para mapear la región virtual al contenido del archivo.
5. Devuelve un puntero a la región de memoria mapeada.
6. Los accesos posteriores a esta región de memoria se traducen automáticamente en lecturas/escrituras del archivo subyacente mediante el sistema de paginación.

## Valor de Retorno
- Devuelve un puntero a la región de memoria mapeada si la syscall se ejecuta correctamente.
- Devuelve `NULL` o un código de error si ocurre un problema, como:
  - `E_INVAL`: Si el descriptor de archivo es inválido, o el offset/tamaño son incorrectos.
  - `E_NOMEM`: Si no hay suficiente espacio en el espacio de direcciones virtuales del proceso.
  - `E_PERM`: Si el proceso no tiene permisos para mapear el archivo.
  - `E_IO`: Si ocurrió un error al acceder al archivo.
  - Otros códigos de error definidos en `error.h`.

## Ejemplo de Uso
```c
#include <error.h>
#include <syscalls.h>
void main() {
    int fd = sys_open("/home/usuario/datos.bin", O_RDWR);
    
    if (fd >= 0) {
        // Mapear los primeros 4096 bytes del archivo
        char *mapped = (char *)sys_mmap(fd, 0, 4096);
        
        if (mapped != NULL) {
            // Acceder al archivo como si fuera memoria
            char primer_byte = mapped[0];
            mapped[10] = 'X';  // Modificar el archivo
            
            // Desactivar el mapeo cuando ya no se necesite
            sys_munmap(mapped, 4096);
        }
        
        sys_close(fd);
    }
}
```

## Ejemplo de Lectura de Estructura
```c
#include <syscalls.h>
typedef struct {
    int id;
    char nombre[64];
    float valor;
} Registro;

void leer_registros(const char *archivo) {
    int fd = sys_open(archivo, O_RDONLY);
    
    if (fd >= 0) {
        int size = 1024 * sizeof(Registro);
        Registro *registros = (Registro *)sys_mmap(fd, 0, size);
        
        if (registros != NULL) {
            // Acceder a los registros directamente
            for (int i = 0; i < 1024; i++) {
                if (registros[i].id > 0) {
                    // Procesar registro
                }
            }
            
            sys_munmap(registros, size);
        }
        
        sys_close(fd);
    }
}
```

## Notas
- El mapeo de memoria es más eficiente que `sys_read`/`sys_write` para archivos grandes que se acceden frecuentemente o de forma aleatoria.
- Los cambios realizados en la memoria mapeada pueden no reflejarse inmediatamente en el archivo físico debido al caching del sistema.
- Es importante llamar a `sys_munmap` cuando ya no se necesite el mapeo para liberar recursos.
- El offset típicamente debe estar alineado al tamaño de página del sistema (comúnmente 4KB).
- Múltiples procesos pueden mapear el mismo archivo, permitiendo comunicación entre procesos mediante memoria compartida.
- Acceder más allá del tamaño mapeado puede causar fallos de segmentación.

## Véase también
- [sys_munmap](./sys_munmap.md)
- [sys_open](./sys_open.md)
- [sys_read](./sys_read.md)
- [sys_write](./sys_write.md)
- [Memory Manager](../Memory%20Manager.md)
