# NeoOS - sys_mkdir
La syscall `sys_mkdir(const char *path)` en NeoOS se utiliza para crear un nuevo directorio en el sistema de archivos.

## Prototipo
```c
int sys_mkdir(const char *path);
```

## Parámetros
- `path`: Una cadena de caracteres que especifica la ruta del nuevo directorio a crear. Puede ser una ruta absoluta o relativa al directorio de trabajo actual del proceso.

## Comportamiento
Cuando un proceso llama a `sys_mkdir`, el sistema operativo realiza las siguientes acciones:
1. Verifica que la ruta especificada sea válida.
2. Resuelve la ruta completa y verifica que el directorio padre exista.
3. Verifica que no exista ya un archivo o directorio con el mismo nombre en esa ubicación.
4. Verifica que el proceso tenga permisos para crear directorios en el directorio padre.
5. Crea una nueva entrada de directorio en el sistema de archivos.
6. Inicializa el directorio con las entradas especiales `.` (apunta al directorio mismo) y `..` (apunta al directorio padre).
7. Establece los permisos y propiedades predeterminados del nuevo directorio.

## Valor de Retorno
- Devuelve `E_OK` (0) si la syscall se ejecuta correctamente y el directorio fue creado.
- Devuelve un código de error (negativo) si ocurre un problema, como:
  - `E_EXISTS`: Si ya existe un archivo o directorio con el mismo nombre.
  - `E_NOENT`: Si el directorio padre no existe.
  - `E_PERM`: Si el proceso no tiene permisos para crear el directorio.
  - `E_NOMEM`: Si no hay suficiente espacio en el sistema de archivos.
  - `E_INVAL`: Si la ruta es inválida.
  - Otros códigos de error definidos en `error.h`.

## Ejemplo de Uso
```c
#include <error.h>
#include <syscalls.h>
void main() {
    // Crear un nuevo directorio
    int result = sys_mkdir("/home/usuario/nuevo_proyecto");
    
    if (result == E_OK) {
        sys_write(STDOUT, "Directorio creado exitosamente\n", 32);
        
        // Ahora podemos usar el directorio
        sys_chdir("/home/usuario/nuevo_proyecto");
        
        // Crear archivos en el nuevo directorio
        int fd = sys_open("README.md", O_WRONLY | O_CREAT);
        if (fd >= 0) {
            sys_write(fd, "# Nuevo Proyecto\n", 17);
            sys_close(fd);
        }
    } else if (result == E_EXISTS) {
        sys_write(STDERR, "El directorio ya existe\n", 24);
    } else {
        sys_write(STDERR, "Error al crear directorio\n", 27);
    }
}
```

## Ejemplo de Creación de Estructura de Directorios
```c
#include <syscalls.h>
void crear_estructura_proyecto() {
    const char *directorios[] = {
        "/home/usuario/proyecto",
        "/home/usuario/proyecto/src",
        "/home/usuario/proyecto/docs",
        "/home/usuario/proyecto/bin",
        NULL
    };
    
    for (int i = 0; directorios[i] != NULL; i++) {
        int result = sys_mkdir(directorios[i]);
        
        if (result != E_OK && result != E_EXISTS) {
            // Manejar error
            char msg[100];
            sprintf(msg, "Error creando: %s\n", directorios[i]);
            sys_write(STDERR, msg, strlen(msg));
            return;
        }
    }
    
    sys_write(STDOUT, "Estructura de proyecto creada\n", 31);
}
```

## Notas
- `sys_mkdir` crea solo un nivel de directorio a la vez. Si necesita crear una ruta completa de directorios, debe crear cada nivel secuencialmente.
- El proceso debe tener permisos de escritura en el directorio padre para crear un nuevo directorio.
- Los permisos del nuevo directorio son típicamente configurados según la umask del proceso y las políticas del sistema.
- Si el directorio ya existe, la llamada falla con `E_EXISTS`. Esto puede usarse para verificar la existencia de directorios.
- Es importante verificar el valor de retorno para determinar si la operación fue exitosa.

## Véase también
- [sys_rmdir](./sys_rmdir.md)
- [sys_chdir](./sys_chdir.md)
- [sys_stat](./sys_stat.md)
- [sys_unlink](./sys_unlink.md)
