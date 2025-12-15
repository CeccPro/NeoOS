# NeoOS - sys_close
La syscall `sys_close(int fd)` en NeoOS se utiliza para cerrar un descriptor de archivo previamente abierto, liberando los recursos asociados con él en el sistema operativo.

## Prototipo
```c
int sys_close(int fd);
```

## Parámetros
- `fd`: El descriptor de archivo que se desea cerrar. Debe ser un descriptor válido obtenido previamente mediante `sys_open` o similar.

## Comportamiento
Cuando un proceso llama a `sys_close`, el sistema operativo realiza las siguientes acciones:
1. Verifica que el descriptor de archivo sea válido y pertenezca al proceso que realiza la llamada.
2. Escribe cualquier dato pendiente en buffers del kernel al archivo (flush).
3. Libera las estructuras de datos internas asociadas al descriptor de archivo.
4. Marca el descriptor de archivo como disponible para reutilización.
5. Si este era el último descriptor que apuntaba al archivo, pueden liberarse recursos adicionales del sistema de archivos.

## Valor de Retorno
- Devuelve `E_OK` (0) si la syscall se ejecuta correctamente y el descriptor fue cerrado.
- Devuelve un código de error (negativo) si ocurre un problema, como:
  - `E_INVAL`: Si el descriptor de archivo es inválido o ya estaba cerrado.
  - `E_IO`: Si ocurrió un error al escribir datos pendientes al archivo.
  - Otros códigos de error definidos en `error.h`.

## Ejemplo de Uso
```c
#include <error.h>
#include <syscalls.h>
void main() {
    int fd = sys_open("/home/usuario/datos.txt", O_RDONLY);
    
    if (fd >= 0) {
        char buffer[256];
        int bytes_leidos = sys_read(fd, buffer, sizeof(buffer));
        
        // Procesar datos...
        
        // Cerrar el archivo cuando ya no se necesite
        int result = sys_close(fd);
        if (result != E_OK) {
            sys_write(STDERR, "Error al cerrar archivo\n", 24);
        }
    }
}
```

## Ejemplo con Múltiples Archivos
```c
#include <syscalls.h>
void copiar_archivo(const char *origen, const char *destino) {
    int fd_origen = sys_open(origen, O_RDONLY);
    int fd_destino = sys_open(destino, O_WRONLY | O_CREAT | O_TRUNC);
    
    if (fd_origen >= 0 && fd_destino >= 0) {
        char buffer[1024];
        int bytes_leidos;
        
        while ((bytes_leidos = sys_read(fd_origen, buffer, sizeof(buffer))) > 0) {
            sys_write(fd_destino, buffer, bytes_leidos);
        }
    }
    
    // Cerrar ambos archivos
    if (fd_origen >= 0) sys_close(fd_origen);
    if (fd_destino >= 0) sys_close(fd_destino);
}
```

## Notas
- Es fundamental cerrar los descriptores de archivo cuando ya no se necesiten para evitar fugas de recursos.
- Cada proceso tiene un límite en el número de archivos que puede tener abiertos simultáneamente.
- Cerrar un descriptor de archivo no garantiza que los datos se hayan escrito físicamente en el dispositivo de almacenamiento; solo garantiza que se han escrito en los buffers del kernel.
- Intentar usar un descriptor de archivo después de cerrarlo resulta en comportamiento indefinido o errores.
- Cuando un proceso termina (mediante `sys_exit` o `sys_kill`), el sistema operativo automáticamente cierra todos sus descriptores de archivo abiertos.
- Los descriptores estándar (STDIN, STDOUT, STDERR) generalmente no deben cerrarse a menos que sea absolutamente necesario.

## Véase también
- [sys_open](./sys_open.md)
- [sys_read](./sys_read.md)
- [sys_write](./sys_write.md)
- [sys_exit](./sys_exit.md)
