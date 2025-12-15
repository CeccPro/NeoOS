# NeoOS - sys_ipc_free
La syscall `sys_ipc_free(ipc_message_t *msg)` en NeoOS se utiliza para liberar los recursos asociados a un mensaje IPC previamente recibido con `sys_ipc_recv`. Es fundamental llamar a esta syscall para evitar fugas de memoria en el sistema.

## Prototipo
```c
int sys_ipc_free(ipc_message_t *msg);
```

## Parámetros
- `msg`: Un puntero a la estructura `ipc_message_t` que contiene el mensaje cuyos recursos se desean liberar. Debe ser una estructura previamente llenada por una llamada exitosa a `sys_ipc_recv`.

## Comportamiento
Cuando un proceso llama a `sys_ipc_free`, el sistema operativo realiza las siguientes acciones:
1. Verifica que el puntero a la estructura de mensaje sea válido.
2. Verifica que el mensaje no haya sido liberado previamente (doble liberación).
3. Libera la memoria asignada para los datos del mensaje (`msg->data`).
4. Libera cualquier otra estructura interna asociada al mensaje.
5. Marca el mensaje como liberado para prevenir uso después de liberación.
6. Invalida los punteros en la estructura del mensaje.

## Valor de Retorno
- Devuelve `E_OK` (0) si la syscall se ejecuta correctamente y los recursos fueron liberados.
- Devuelve un código de error (negativo) si ocurre un problema, como:
  - `E_INVAL`: Si el puntero del mensaje es NULL o la estructura es inválida.
  - Otros códigos de error definidos en `error.h`.

## Ejemplo de Uso
```c
#include <error.h>
#include <syscalls.h>
void main() {
    ipc_message_t mensaje;
    
    // Recibir mensaje
    if (sys_ipc_recv(&mensaje, 0) == E_OK) {
        // Procesar mensaje
        sys_write(STDOUT, "Mensaje: ", 9);
        sys_write(STDOUT, mensaje.data, mensaje.size);
        sys_write(STDOUT, "\n", 1);
        
        // Liberar recursos del mensaje
        int result = sys_ipc_free(&mensaje);
        if (result != E_OK) {
            sys_write(STDERR, "Error al liberar mensaje\n", 26);
        }
    }
}
```

## Ejemplo de Procesamiento en Bucle
```c
#include <syscalls.h>
void servidor_mensajes() {
    while (1) {
        ipc_message_t mensaje;
        
        if (sys_ipc_recv(&mensaje, 0) == E_OK) {
            // Procesar mensaje
            procesar_mensaje(&mensaje);
            
            // IMPORTANTE: Siempre liberar después de procesar
            sys_ipc_free(&mensaje);
        }
    }
}
```

## Ejemplo con Copia de Datos
```c
#include <syscalls.h>
void guardar_mensaje_para_despues() {
    ipc_message_t mensaje;
    
    if (sys_ipc_recv(&mensaje, 0) == E_OK) {
        // Copiar datos si necesitamos retenerlos
        char *copia = malloc(mensaje.size);
        if (copia != NULL) {
            memcpy(copia, mensaje.data, mensaje.size);
            
            // Ahora podemos liberar el mensaje original
            sys_ipc_free(&mensaje);
            
            // Usar la copia más tarde
            procesar_mas_tarde(copia, mensaje.size);
            
            // No olvidar liberar la copia cuando ya no se necesite
            free(copia);
        } else {
            // Si no podemos copiar, liberamos de todos modos
            sys_ipc_free(&mensaje);
        }
    }
}
```

## Ejemplo de Manejo de Errores
```c
#include <syscalls.h>
int procesar_mensaje_seguro(ipc_message_t *mensaje) {
    int resultado = E_OK;
    
    // Procesar mensaje
    if (mensaje->size > 0 && mensaje->data != NULL) {
        // Procesar datos...
        resultado = realizar_procesamiento(mensaje->data, mensaje->size);
    } else {
        resultado = E_INVAL;
    }
    
    // Siempre liberar, incluso si hubo error en el procesamiento
    int free_result = sys_ipc_free(mensaje);
    if (free_result != E_OK) {
        sys_write(STDERR, "Advertencia: error al liberar mensaje\n", 39);
    }
    
    return resultado;
}

void main() {
    ipc_message_t mensaje;
    
    if (sys_ipc_recv(&mensaje, 0) == E_OK) {
        procesar_mensaje_seguro(&mensaje);
    }
}
```

## Ejemplo con Múltiples Mensajes
```c
#include <syscalls.h>
void procesar_lote_mensajes(int max_mensajes) {
    int procesados = 0;
    
    for (int i = 0; i < max_mensajes; i++) {
        ipc_message_t mensaje;
        
        // Intentar recibir sin bloquear
        if (sys_ipc_recv(&mensaje, IPC_NONBLOCKING) == E_OK) {
            // Procesar mensaje
            procesar_mensaje(&mensaje);
            
            // Liberar inmediatamente después de procesar
            sys_ipc_free(&mensaje);
            
            procesados++;
        } else {
            // No hay más mensajes
            break;
        }
    }
    
    char info[50];
    sprintf(info, "Procesados %d mensajes\n", procesados);
    sys_write(STDOUT, info, strlen(info));
}
```

## Notas
- Es OBLIGATORIO llamar a `sys_ipc_free` para cada mensaje recibido con `sys_ipc_recv`, incluso si el procesamiento del mensaje falla.
- No liberar mensajes causa fugas de memoria que eventualmente agotarán los recursos del sistema.
- Después de llamar a `sys_ipc_free`, el puntero `msg->data` y otros campos de la estructura quedan inválidos; no deben usarse.
- Si necesita retener los datos del mensaje más allá del procesamiento inmediato, debe copiarlos antes de llamar a `sys_ipc_free`.
- No intente liberar el mismo mensaje dos veces (doble liberación), esto causará comportamiento indefinido.
- En caso de error al procesar un mensaje, igualmente debe llamarse a `sys_ipc_free`.

## Véase también
- [sys_ipc_recv](./sys_ipc_recv.md)
- [sys_ipc_send](./sys_ipc_send.md)
- [IPC](../IPC.md)
