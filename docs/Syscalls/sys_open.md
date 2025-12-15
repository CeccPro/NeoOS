# NeoOS - sys_open
La syscall `sys_open(const char *filename, int mode)` en NeoOS se utiliza para abrir un archivo y obtener un descriptor de archivo (file descriptor) que puede ser usado en operaciones posteriores de lectura, escritura o mapeo de memoria.

## Prototipo
```c
int sys_open(const char *filename, int mode);
```

## Parámetros
- `filename`: Una cadena de caracteres que especifica la ruta del archivo que se desea abrir. Puede ser una ruta absoluta o relativa al directorio de trabajo actual del proceso.
- `mode`: Un entero que especifica el modo de apertura del archivo. Los modos comunes incluyen:
  - `O_RDONLY`: Abrir solo para lectura.
  - `O_WRONLY`: Abrir solo para escritura.
  - `O_RDWR`: Abrir para lectura y escritura.
  - `O_CREAT`: Crear el archivo si no existe.
  - `O_TRUNC`: Truncar el archivo a tamaño cero si existe.
  - `O_APPEND`: Añadir datos al final del archivo.
  - Estos modos pueden combinarse usando el operador OR bit a bit (|).

## Comportamiento
Cuando un proceso llama a `sys_open`, el sistema operativo realiza las siguientes acciones:
1. Resuelve la ruta del archivo especificada en `filename`.
2. Verifica que el archivo exista (o que se deba crear si se especificó `O_CREAT`).
3. Verifica los permisos del proceso para acceder al archivo según el modo solicitado.
4. Asigna un descriptor de archivo (un entero no negativo) para el proceso.
5. Inicializa la estructura interna del descriptor de archivo con información sobre el archivo (posición actual, modo de acceso, etc.).
6. Si se especificó `O_TRUNC`, trunca el contenido del archivo.
7. Devuelve el descriptor de archivo al proceso.

## Valor de Retorno
- Devuelve un descriptor de archivo (entero no negativo) si la syscall se ejecuta correctamente.
- Devuelve un código de error (negativo) si ocurre un problema, como:
  - `E_NOENT`: Si el archivo no existe y no se especificó `O_CREAT`.
  - `E_PERM`: Si el proceso no tiene permisos para acceder al archivo en el modo solicitado.
  - `E_NOMEM`: Si no hay suficientes recursos para abrir el archivo.
  - `E_INVAL`: Si el modo especificado es inválido.
  - `E_EXISTS`: Si se especificó `O_CREAT` y `O_EXCL` pero el archivo ya existe.
  - Otros códigos de error definidos en `error.h`.

## Ejemplo de Uso
```c
#include <error.h>
#include <syscalls.h>
void main() {
    // Abrir archivo existente para lectura
    int fd = sys_open("/home/usuario/archivo.txt", O_RDONLY);
    
    if (fd < 0) {
        // Manejar error
        if (fd == E_NOENT) {
            sys_write(STDERR, "Archivo no encontrado\n", 23);
        }
    } else {
        // Leer del archivo
        char buffer[256];
        int bytes_leidos = sys_read(fd, buffer, sizeof(buffer));
        
        // Cerrar el archivo
        sys_close(fd);
    }
}
```

## Ejemplo de Creación de Archivo
```c
#include <syscalls.h>
void crear_y_escribir_archivo() {
    // Crear archivo nuevo o truncar existente
    int fd = sys_open("/home/usuario/nuevo.txt", O_WRONLY | O_CREAT | O_TRUNC);
    
    if (fd >= 0) {
        const char *datos = "Hola, NeoOS!\n";
        sys_write(fd, datos, strlen(datos));
        sys_close(fd);
    }
}
```

## Notas
- Es responsabilidad del programador cerrar los descriptores de archivo usando `sys_close` cuando ya no se necesiten, para liberar recursos del sistema.
- Cada proceso tiene un límite en el número de archivos que puede tener abiertos simultáneamente.
- Los descriptores de archivo 0, 1 y 2 están típicamente reservados para stdin, stdout y stderr respectivamente.
- La posición inicial de lectura/escritura en el archivo es el principio (offset 0), excepto cuando se usa `O_APPEND`.

## Véase también
- [sys_close](./sys_close.md)
- [sys_read](./sys_read.md)
- [sys_write](./sys_write.md)
- [sys_mmap](./sys_mmap.md)
