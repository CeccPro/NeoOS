# NeoOS - sys_chdir
La syscall `sys_chdir(const char *path)` en NeoOS se utiliza para cambiar el directorio de trabajo actual del proceso. El directorio de trabajo es el directorio desde el cual se resuelven las rutas relativas.

## Prototipo
```c
int sys_chdir(const char *path);
```

## Parámetros
- `path`: Una cadena de caracteres que especifica la ruta del nuevo directorio de trabajo. Puede ser una ruta absoluta o relativa al directorio de trabajo actual.

## Comportamiento
Cuando un proceso llama a `sys_chdir`, el sistema operativo realiza las siguientes acciones:
1. Verifica que la ruta especificada sea válida.
2. Resuelve la ruta completa, expandiendo componentes como `.` (directorio actual) y `..` (directorio padre).
3. Verifica que el destino sea un directorio existente y no un archivo regular.
4. Verifica que el proceso tenga permisos de acceso al directorio.
5. Actualiza el directorio de trabajo actual del proceso.
6. Las operaciones posteriores con rutas relativas se resolverán desde este nuevo directorio.

## Valor de Retorno
- Devuelve `E_OK` (0) si la syscall se ejecuta correctamente y el directorio de trabajo fue cambiado.
- Devuelve un código de error (negativo) si ocurre un problema, como:
  - `E_NOENT`: Si el directorio especificado no existe.
  - `E_PERM`: Si el proceso no tiene permisos para acceder al directorio.
  - `E_INVAL`: Si la ruta es inválida o apunta a un archivo en lugar de un directorio.
  - Otros códigos de error definidos en `error.h`.

## Ejemplo de Uso
```c
#include <error.h>
#include <syscalls.h>
void main() {
    // Cambiar al directorio home del usuario
    int result = sys_chdir("/home/usuario");
    
    if (result == E_OK) {
        // Ahora podemos usar rutas relativas desde /home/usuario
        int fd = sys_open("documentos/archivo.txt", O_RDONLY);
        // Esto abrirá /home/usuario/documentos/archivo.txt
        
        if (fd >= 0) {
            // Procesar archivo...
            sys_close(fd);
        }
    } else {
        sys_write(STDERR, "Error al cambiar directorio\n", 29);
    }
}
```

## Ejemplo de Navegación de Directorios
```c
#include <syscalls.h>
void explorar_directorios() {
    // Guardar el directorio actual (conceptualmente)
    // En un sistema real, usaríamos sys_getcwd
    
    // Navegar a un subdirectorio
    if (sys_chdir("proyecto") == E_OK) {
        // Trabajar en el directorio proyecto
        int fd = sys_open("config.txt", O_RDONLY);
        if (fd >= 0) {
            // Leer configuración...
            sys_close(fd);
        }
        
        // Volver al directorio padre
        sys_chdir("..");
    }
}
```

## Notas
- El cambio de directorio de trabajo es específico del proceso y no afecta a otros procesos.
- Los procesos hijos heredan el directorio de trabajo del proceso padre en el momento de su creación.
- Es una buena práctica verificar el valor de retorno de `sys_chdir` antes de asumir que el cambio fue exitoso.
- Usar rutas absolutas es más seguro que rutas relativas en código que puede ejecutarse desde diferentes contextos.
- El directorio de trabajo inicial de un proceso típicamente es el directorio desde el cual se lanzó el proceso.

## Véase también
- [sys_mkdir](./sys_mkdir.md)
- [sys_rmdir](./sys_rmdir.md)
- [sys_open](./sys_open.md)
- [sys_stat](./sys_stat.md)
