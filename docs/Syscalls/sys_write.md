# NeoOS - sys_write
La syscall `sys_write(int fd, const void *buf, int count)` en NeoOS se utiliza para escribir datos desde un buffer en memoria hacia un archivo abierto, identificado por su descriptor de archivo.

## Prototipo
```c
int sys_write(int fd, const void *buf, int count);
```

## Parámetros
- `fd`: El descriptor de archivo en el cual se escribirán los datos. Debe ser un descriptor válido obtenido previamente mediante `sys_open` con permisos de escritura.
- `buf`: Un puntero al buffer en memoria que contiene los datos a escribir. El buffer debe contener al menos `count` bytes válidos.
- `count`: El número de bytes a escribir del buffer al archivo.

## Comportamiento
Cuando un proceso llama a `sys_write`, el sistema operativo realiza las siguientes acciones:
1. Verifica que el descriptor de archivo sea válido y esté abierto para escritura.
2. Verifica que el puntero del buffer sea una dirección de memoria válida y accesible por el proceso.
3. Escribe hasta `count` bytes desde el buffer hacia la posición actual del archivo.
4. Actualiza la posición actual del archivo para apuntar después de los datos escritos.
5. Si el archivo se abrió con `O_APPEND`, primero mueve la posición al final del archivo antes de escribir.
6. Devuelve el número de bytes realmente escritos.

## Valor de Retorno
- Devuelve el número de bytes realmente escritos si la operación es exitosa. Normalmente, este valor es igual a `count`.
- En algunos casos (como dispositivos o sistemas de archivos llenos), puede escribir menos bytes de los solicitados.
- Devuelve un código de error (negativo) si ocurre un problema, como:
  - `E_INVAL`: Si el descriptor de archivo es inválido o el buffer es NULL.
  - `E_PERM`: Si el archivo no está abierto para escritura.
  - `E_IO`: Si ocurrió un error de entrada/salida durante la escritura.
  - `E_NOMEM`: Si no hay espacio disponible en el sistema de archivos.
  - Otros códigos de error definidos en `error.h`.

## Ejemplo de Uso
```c
#include <error.h>
#include <syscalls.h>
void main() {
    int fd = sys_open("/home/usuario/salida.txt", O_WRONLY | O_CREAT | O_TRUNC);
    
    if (fd >= 0) {
        const char *mensaje = "Hola desde NeoOS!\n";
        int bytes_escritos = sys_write(fd, mensaje, strlen(mensaje));
        
        if (bytes_escritos > 0) {
            // Escritura exitosa
        } else {
            sys_write(STDERR, "Error al escribir\n", 18);
        }
        
        sys_close(fd);
    }
}
```

## Ejemplo de Escritura a STDOUT
```c
#include <syscalls.h>
void imprimir_mensaje(const char *mensaje) {
    // STDOUT es típicamente el descriptor 1
    int len = strlen(mensaje);
    sys_write(STDOUT, mensaje, len);
}

void main() {
    imprimir_mensaje("Este mensaje se imprime en la consola\n");
}
```

## Ejemplo de Log en Archivo
```c
#include <syscalls.h>
void escribir_log(const char *mensaje) {
    int fd = sys_open("/var/log/miapp.log", O_WRONLY | O_CREAT | O_APPEND);
    
    if (fd >= 0) {
        // Agregar timestamp
        int uptime = sys_uptime();
        char buffer[256];
        sprintf(buffer, "[%d] %s\n", uptime, mensaje);
        
        sys_write(fd, buffer, strlen(buffer));
        sys_close(fd);
    }
}
```

## Notas
- Es responsabilidad del programador asegurar que el buffer contenga datos válidos por al menos `count` bytes.
- Para archivos regulares, `sys_write` normalmente escribe todos los bytes solicitados o falla. Para otros tipos de archivos (pipes, sockets), puede escribir parcialmente.
- Los datos escritos pueden no reflejarse inmediatamente en el sistema de archivos debido al buffering del kernel. Para asegurar persistencia, puede ser necesario sincronizar explícitamente.
- Los descriptores estándar son: 0 (STDIN), 1 (STDOUT), 2 (STDERR).
- Si múltiples procesos escriben al mismo archivo, pueden ocurrir intercalaciones de datos a menos que se use sincronización explícita.

## Véase también
- [sys_open](./sys_open.md)
- [sys_read](./sys_read.md)
- [sys_close](./sys_close.md)
- [sys_mmap](./sys_mmap.md)
