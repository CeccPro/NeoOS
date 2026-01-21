# NeoOS - sys_rmdir
La syscall `sys_rmdir(const char *path)` en NeoOS se utiliza para eliminar un directorio existente del sistema de archivos. El directorio debe estar vacío para poder ser eliminado.

## Prototipo
```c
int sys_rmdir(const char *path);
```

## Parámetros
- `path`: Una cadena de caracteres que especifica la ruta del directorio a eliminar. Puede ser una ruta absoluta o relativa al directorio de trabajo actual del proceso.

## Comportamiento
Cuando un proceso llama a `sys_rmdir`, el sistema operativo realiza las siguientes acciones:
1. Verifica que la ruta especificada sea válida y apunte a un directorio existente.
2. Verifica que el proceso tenga permisos para eliminar el directorio.
3. Verifica que el directorio esté vacío (solo contenga las entradas `.` y `..`).
4. Verifica que ningún proceso esté usando el directorio como directorio de trabajo actual.
5. Elimina la entrada del directorio del sistema de archivos.
6. Libera los recursos asociados (inodos, bloques de datos, etc.).

## Valor de Retorno
- Devuelve `E_OK` (0) si la syscall se ejecuta correctamente y el directorio fue eliminado.
- Devuelve un código de error (negativo) si ocurre un problema, como:
  - `E_NOENT`: Si el directorio especificado no existe.
  - `E_PERM`: Si el proceso no tiene permisos para eliminar el directorio.
  - `E_BUSY`: Si el directorio está siendo usado como directorio de trabajo por algún proceso.
  - `E_INVAL`: Si la ruta no apunta a un directorio o si se intenta eliminar `.` o `..`.
  - `E_NOT_SUPPORTED`: Si el directorio no está vacío.
  - `E_IO`: Si ocurrió un error de entrada/salida.
  - Otros códigos de error definidos en `error.h`.

## Ejemplo de Uso
```c
#include <error.h>
#include <syscalls.h>
void main() {
    // Eliminar un directorio vacío
    int result = sys_rmdir("/home/usuario/temp");
    
    if (result == E_OK) {
        sys_write(STDOUT, "Directorio eliminado exitosamente\n", 35);
    } else if (result == E_NOENT) {
        sys_write(STDERR, "El directorio no existe\n", 24);
    } else if (result == E_NOT_SUPPORTED) {
        sys_write(STDERR, "El directorio no está vacío\n", 29);
    } else {
        sys_write(STDERR, "Error al eliminar directorio\n", 30);
    }
}
```

## Ejemplo de Limpieza de Directorio Temporal
```c
#include <syscalls.h>
void limpiar_directorio_temporal() {
    const char *dir_temp = "/tmp/mi_app";
    
    // Primero, eliminar todos los archivos en el directorio
    // (esto requeriría una syscall para listar directorios, no mostrada aquí)
    
    // Eliminar archivos conocidos
    sys_unlink("/tmp/mi_app/temp1.txt");
    sys_unlink("/tmp/mi_app/temp2.txt");
    
    // Ahora que está vacío, eliminar el directorio
    int result = sys_rmdir(dir_temp);
    
    if (result == E_OK) {
        sys_write(STDOUT, "Directorio temporal limpiado\n", 30);
    }
}
```

## Ejemplo con Verificación
```c
#include <syscalls.h>
int eliminar_directorio_seguro(const char *path) {
    // Verificar que el directorio existe
    struct stat info;
    if (sys_stat(path, &info) != E_OK) {
        return E_NOENT;
    }
    
    // Intentar eliminar
    int result = sys_rmdir(path);
    
    if (result != E_OK) {
        char msg[100];
        sprintf(msg, "No se pudo eliminar %s\n", path);
        sys_write(STDERR, msg, strlen(msg));
    }
    
    return result;
}
```

## Notas
- El directorio debe estar completamente vacío (excepto por `.` y `..`) para poder ser eliminado.
- No se puede eliminar un directorio que sea el directorio de trabajo de algún proceso en ejecución.
- No se pueden eliminar los directorios especiales `.` (directorio actual) y `..` (directorio padre).
- Para eliminar un directorio con contenido, primero deben eliminarse todos los archivos y subdirectorios recursivamente.
- La operación es atómica: el directorio se elimina completamente o no se elimina en absoluto.
- Algunos sistemas tienen directorios especiales del sistema que no pueden ser eliminados incluso con permisos de superusuario.

## Véase también
- [sys_mkdir](./sys_mkdir.md)
- [sys_unlink](./sys_unlink.md)
- [sys_chdir](./sys_chdir.md)
- [sys_stat](./sys_stat.md)
