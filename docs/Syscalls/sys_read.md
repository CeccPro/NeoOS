# NeoOS - sys_read
La syscall `sys_read(int fd, void *buf, int count)` en NeoOS se utiliza para leer datos desde un archivo abierto, identificado por su descriptor de archivo, hacia un buffer en memoria.

## Prototipo
```c
int sys_read(int fd, void *buf, int count);
```

## Parámetros
- `fd`: El descriptor de archivo desde el cual se leerán los datos. Debe ser un descriptor válido obtenido previamente mediante `sys_open`.
- `buf`: Un puntero al buffer en memoria donde se almacenarán los datos leídos. El buffer debe tener suficiente espacio para almacenar `count` bytes.
- `count`: El número máximo de bytes a leer del archivo.

## Comportamiento
Cuando un proceso llama a `sys_read`, el sistema operativo realiza las siguientes acciones:
1. Verifica que el descriptor de archivo sea válido y esté abierto para lectura.
2. Verifica que el puntero del buffer sea una dirección de memoria válida y accesible por el proceso.
3. Lee hasta `count` bytes desde la posición actual del archivo hacia el buffer.
4. Actualiza la posición actual del archivo para apuntar después de los datos leídos.
5. Devuelve el número de bytes realmente leídos.

## Valor de Retorno
- Devuelve el número de bytes realmente leídos si la operación es exitosa. Este valor puede ser menor que `count` si:
  - Se alcanzó el final del archivo.
  - Menos datos estaban disponibles de los solicitados.
- Devuelve 0 si se intentó leer estando ya al final del archivo (EOF).
- Devuelve un código de error (negativo) si ocurre un problema, como:
  - `E_INVAL`: Si el descriptor de archivo es inválido o el buffer es NULL.
  - `E_PERM`: Si el archivo no está abierto para lectura.
  - `E_IO`: Si ocurrió un error de entrada/salida durante la lectura.
  - Otros códigos de error definidos en `error.h`.

## Ejemplo de Uso
```c
#include <error.h>
#include <syscalls.h>
void main() {
    int fd = sys_open("/home/usuario/datos.txt", O_RDONLY);
    
    if (fd >= 0) {
        char buffer[256];
        int bytes_leidos = sys_read(fd, buffer, sizeof(buffer) - 1);
        
        if (bytes_leidos > 0) {
            buffer[bytes_leidos] = '\0';  // Null-terminar si es texto
            sys_write(STDOUT, buffer, bytes_leidos);
        } else if (bytes_leidos == 0) {
            sys_write(STDOUT, "Archivo vacío\n", 14);
        } else {
            sys_write(STDERR, "Error al leer\n", 14);
        }
        
        sys_close(fd);
    }
}
```

## Ejemplo de Lectura Completa de Archivo
```c
#include <syscalls.h>
void leer_archivo_completo(const char *filename) {
    int fd = sys_open(filename, O_RDONLY);
    
    if (fd >= 0) {
        char buffer[1024];
        int bytes_leidos;
        
        // Leer hasta el final del archivo
        while ((bytes_leidos = sys_read(fd, buffer, sizeof(buffer))) > 0) {
            sys_write(STDOUT, buffer, bytes_leidos);
        }
        
        sys_close(fd);
    }
}
```

## Notas
- La syscall `sys_read` puede leer menos bytes de los solicitados sin que sea un error. Los programadores deben verificar el valor de retorno.
- Para archivos de texto, es común dejar espacio para un carácter nulo al final del buffer y agregarlo manualmente después de leer.
- La posición del archivo avanza automáticamente con cada lectura exitosa.
- Si múltiples procesos leen del mismo archivo, cada uno mantiene su propia posición de lectura.
- Para operaciones de lectura en dispositivos especiales o pipes, el comportamiento puede variar (lectura bloqueante vs no bloqueante).

## Véase también
- [sys_open](./sys_open.md)
- [sys_write](./sys_write.md)
- [sys_close](./sys_close.md)
- [sys_mmap](./sys_mmap.md)
