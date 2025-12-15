# NeoOS - sys_unlink
La syscall `sys_unlink(const char *path)` en NeoOS se utiliza para eliminar un archivo del sistema de archivos. Esta syscall elimina el enlace (link) del nombre del archivo a su inodo, y si no quedan más enlaces, el archivo se borra del sistema.

## Prototipo
```c
int sys_unlink(const char *path);
```

## Parámetros
- `path`: Una cadena de caracteres que especifica la ruta del archivo a eliminar. Puede ser una ruta absoluta o relativa al directorio de trabajo actual del proceso.

## Comportamiento
Cuando un proceso llama a `sys_unlink`, el sistema operativo realiza las siguientes acciones:
1. Verifica que la ruta especificada sea válida y apunte a un archivo existente.
2. Verifica que el proceso tenga permisos para eliminar el archivo (típicamente permisos de escritura en el directorio padre).
3. Elimina la entrada del directorio que apunta al archivo.
4. Decrementa el contador de enlaces (link count) del inodo del archivo.
5. Si el contador de enlaces llega a 0 y no hay descriptores de archivo abiertos para ese archivo:
   - Libera los bloques de datos del archivo.
   - Libera el inodo.
6. Si hay descriptores de archivo abiertos, el archivo se marca para eliminación pero los datos permanecen accesibles hasta que todos los descriptores se cierren.

## Valor de Retorno
- Devuelve `E_OK` (0) si la syscall se ejecuta correctamente y el archivo fue eliminado.
- Devuelve un código de error (negativo) si ocurre un problema, como:
  - `E_NOENT`: Si el archivo especificado no existe.
  - `E_PERM`: Si el proceso no tiene permisos para eliminar el archivo.
  - `E_INVAL`: Si la ruta apunta a un directorio en lugar de un archivo.
  - `E_BUSY`: Si el archivo está siendo usado de una manera que previene su eliminación.
  - `E_IO`: Si ocurrió un error de entrada/salida.
  - Otros códigos de error definidos en `error.h`.

## Ejemplo de Uso
```c
#include <error.h>
#include <syscalls.h>
void main() {
    // Eliminar un archivo
    int result = sys_unlink("/home/usuario/archivo_temporal.txt");
    
    if (result == E_OK) {
        sys_write(STDOUT, "Archivo eliminado exitosamente\n", 32);
    } else if (result == E_NOENT) {
        sys_write(STDERR, "El archivo no existe\n", 21);
    } else if (result == E_PERM) {
        sys_write(STDERR, "Sin permisos para eliminar\n", 28);
    } else {
        sys_write(STDERR, "Error al eliminar archivo\n", 27);
    }
}
```

## Ejemplo de Limpieza de Archivos Temporales
```c
#include <syscalls.h>
void limpiar_archivos_temporales() {
    const char *archivos_temp[] = {
        "/tmp/app_cache.tmp",
        "/tmp/app_session.tmp",
        "/tmp/app_log.tmp",
        NULL
    };
    
    for (int i = 0; archivos_temp[i] != NULL; i++) {
        int result = sys_unlink(archivos_temp[i]);
        
        // Ignorar error si el archivo no existe
        if (result != E_OK && result != E_NOENT) {
            char msg[100];
            sprintf(msg, "Advertencia: no se pudo eliminar %s\n", archivos_temp[i]);
            sys_write(STDERR, msg, strlen(msg));
        }
    }
}
```

## Ejemplo con Archivo Abierto
```c
#include <syscalls.h>
void ejemplo_archivo_abierto() {
    const char *temp_file = "/tmp/datos_temporales.dat";
    
    // Crear y abrir un archivo temporal
    int fd = sys_open(temp_file, O_RDWR | O_CREAT);
    
    if (fd >= 0) {
        // Escribir algunos datos
        sys_write(fd, "datos temporales", 16);
        
        // Eliminar el archivo mientras está abierto
        // El archivo se marcará para eliminación pero los datos
        // permanecerán accesibles a través del descriptor
        sys_unlink(temp_file);
        
        // Podemos seguir leyendo y escribiendo
        char buffer[20];
        sys_read(fd, buffer, sizeof(buffer));
        
        // Cuando cerramos el descriptor, el archivo se elimina definitivamente
        sys_close(fd);
    }
}
```

## Notas
- `sys_unlink` no puede usarse para eliminar directorios; para eso se debe usar `sys_rmdir`.
- Si el archivo tiene múltiples enlaces (hard links), solo se elimina el enlace especificado, no el archivo en sí.
- Un archivo abierto puede ser "eliminado" con `sys_unlink`, pero seguirá accesible a través de descriptores de archivo abiertos hasta que todos se cierren.
- Esta técnica de eliminar archivos abiertos es útil para archivos temporales que deben desaparecer automáticamente.
- El proceso necesita permisos de escritura en el directorio que contiene el archivo, no necesariamente en el archivo mismo.
- Algunos sistemas de archivos especiales pueden tener restricciones adicionales sobre qué archivos pueden eliminarse.

## Véase también
- [sys_rmdir](./sys_rmdir.md)
- [sys_open](./sys_open.md)
- [sys_close](./sys_close.md)
- [sys_stat](./sys_stat.md)
