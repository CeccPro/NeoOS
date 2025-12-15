# NeoOS - sys_stat
La syscall `sys_stat(const char *path, struct stat *buf)` en NeoOS se utiliza para obtener información detallada sobre un archivo o directorio, incluyendo su tamaño, permisos, fechas de modificación y otros metadatos.

## Prototipo
```c
int sys_stat(const char *path, struct stat *buf);
```

## Parámetros
- `path`: Una cadena de caracteres que especifica la ruta del archivo o directorio sobre el cual se desea obtener información. Puede ser una ruta absoluta o relativa al directorio de trabajo actual.
- `buf`: Un puntero a una estructura `stat` donde se almacenará la información del archivo. Esta estructura típicamente contiene campos como:
  - `st_size`: Tamaño del archivo en bytes.
  - `st_mode`: Permisos y tipo de archivo.
  - `st_ino`: Número de inodo.
  - `st_nlink`: Número de enlaces al archivo.
  - `st_uid`: ID del propietario.
  - `st_gid`: ID del grupo.
  - `st_atime`: Tiempo de último acceso.
  - `st_mtime`: Tiempo de última modificación.
  - `st_ctime`: Tiempo de último cambio de estado.

## Comportamiento
Cuando un proceso llama a `sys_stat`, el sistema operativo realiza las siguientes acciones:
1. Verifica que la ruta especificada sea válida.
2. Resuelve la ruta completa del archivo o directorio.
3. Verifica que el archivo o directorio exista.
4. Obtiene el inodo del archivo o directorio.
5. Lee la información del inodo y la copia a la estructura `stat` proporcionada.
6. No requiere que el archivo esté abierto con `sys_open`.

## Valor de Retorno
- Devuelve `E_OK` (0) si la syscall se ejecuta correctamente y la información fue obtenida.
- Devuelve un código de error (negativo) si ocurre un problema, como:
  - `E_NOENT`: Si el archivo o directorio no existe.
  - `E_INVAL`: Si el puntero `buf` es NULL o la ruta es inválida.
  - `E_PERM`: Si el proceso no tiene permisos para acceder a la información del archivo.
  - `E_IO`: Si ocurrió un error de entrada/salida.
  - Otros códigos de error definidos en `error.h`.

## Ejemplo de Uso
```c
#include <error.h>
#include <syscalls.h>
void main() {
    struct stat info;
    int result = sys_stat("/home/usuario/archivo.txt", &info);
    
    if (result == E_OK) {
        char msg[100];
        sprintf(msg, "Tamaño del archivo: %d bytes\n", info.st_size);
        sys_write(STDOUT, msg, strlen(msg));
        
        if (S_ISDIR(info.st_mode)) {
            sys_write(STDOUT, "Es un directorio\n", 17);
        } else if (S_ISREG(info.st_mode)) {
            sys_write(STDOUT, "Es un archivo regular\n", 22);
        }
    } else if (result == E_NOENT) {
        sys_write(STDERR, "El archivo no existe\n", 21);
    }
}
```

## Ejemplo de Verificación de Existencia
```c
#include <syscalls.h>
int archivo_existe(const char *path) {
    struct stat info;
    return sys_stat(path, &info) == E_OK;
}

int es_directorio(const char *path) {
    struct stat info;
    if (sys_stat(path, &info) == E_OK) {
        return S_ISDIR(info.st_mode);
    }
    return 0;
}

void main() {
    if (archivo_existe("/home/usuario/config.txt")) {
        sys_write(STDOUT, "El archivo de configuración existe\n", 36);
    }
    
    if (es_directorio("/home/usuario/docs")) {
        sys_write(STDOUT, "El directorio docs existe\n", 27);
    }
}
```

## Ejemplo de Comparación de Fechas
```c
#include <syscalls.h>
void verificar_archivo_modificado(const char *path) {
    struct stat info;
    
    if (sys_stat(path, &info) == E_OK) {
        // Obtener tiempo actual del sistema
        int tiempo_actual = sys_uptime();
        
        // Verificar si el archivo fue modificado recientemente
        // (suponiendo que st_mtime está en ticks del sistema)
        int diferencia = tiempo_actual - info.st_mtime;
        
        if (diferencia < 6000) {  // Menos de 60 segundos
            sys_write(STDOUT, "Archivo modificado recientemente\n", 34);
        } else {
            sys_write(STDOUT, "Archivo no ha sido modificado recientemente\n", 45);
        }
    }
}
```

## Notas
- `sys_stat` no requiere abrir el archivo, por lo que es más eficiente que `sys_open` cuando solo se necesita información sobre el archivo.
- La estructura `stat` puede variar según la implementación del sistema operativo; consulte los archivos de encabezado para detalles específicos.
- Los campos de tiempo (`st_atime`, `st_mtime`, `st_ctime`) pueden estar en diferentes formatos según el sistema.
- Para verificar si una ruta es un archivo regular, directorio, enlace simbólico, etc., use las macros `S_ISREG()`, `S_ISDIR()`, `S_ISLNK()`, etc.
- Esta syscall sigue enlaces simbólicos. Para obtener información del enlace mismo, algunos sistemas proporcionan `sys_lstat`.

## Véase también
- [sys_open](./sys_open.md)
- [sys_chdir](./sys_chdir.md)
- [sys_mkdir](./sys_mkdir.md)
- [sys_unlink](./sys_unlink.md)
