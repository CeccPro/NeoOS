# NeoOS - sys_signal
La syscall `sys_signal(int sig, void (*handler)(int))` en NeoOS se utiliza para registrar un manejador (handler) de señales personalizado para una señal específica. Las señales son mecanismos de comunicación asíncrona utilizados para notificar a procesos sobre eventos específicos.

## Prototipo
```c
void (*sys_signal(int sig, void (*handler)(int)))(int);
```

## Parámetros
- `sig`: El número de la señal para la cual se desea registrar el manejador. Las señales comunes incluyen:
  - `SIGINT`: Interrupción desde el teclado (Ctrl+C).
  - `SIGTERM`: Solicitud de terminación.
  - `SIGKILL`: Terminación forzosa (no capturable).
  - `SIGSEGV`: Violación de segmentación.
  - `SIGCHLD`: Un proceso hijo terminó.
  - Otras señales definidas en el sistema.
- `handler`: Un puntero a la función que se ejecutará cuando se reciba la señal. La función debe tener la firma `void handler(int signum)`. Valores especiales:
  - `SIG_DFL`: Restaurar el comportamiento por defecto de la señal.
  - `SIG_IGN`: Ignorar la señal.
  - Un puntero a función personalizada.

## Comportamiento
Cuando un proceso llama a `sys_signal`, el sistema operativo realiza las siguientes acciones:
1. Verifica que el número de señal sea válido.
2. Verifica que la señal sea capturable (algunas señales como `SIGKILL` no pueden ser capturadas).
3. Guarda el manejador anterior de la señal para devolverlo.
4. Registra el nuevo manejador en la tabla de señales del proceso.
5. Cuando se recibe la señal posteriormente:
   - Interrumpe la ejecución normal del proceso.
   - Guarda el contexto actual.
   - Ejecuta el manejador de señales registrado.
   - Restaura el contexto y continúa la ejecución normal.

## Valor de Retorno
- Devuelve un puntero al manejador anterior de la señal si la syscall se ejecuta correctamente.
- Devuelve `SIG_ERR` (típicamente -1 o NULL) si ocurre un problema, como:
  - `E_INVAL`: Si el número de señal es inválido o la señal no es capturable.
  - Otros códigos de error definidos en `error.h`.

## Ejemplo de Uso
```c
#include <error.h>
#include <syscalls.h>

void manejador_sigint(int signum) {
    const char *msg = "Recibida señal de interrupción (Ctrl+C)\n";
    sys_write(STDOUT, msg, 41);
    // No llamar a sys_exit aquí si queremos que el programa continúe
}

void main() {
    // Registrar manejador personalizado para SIGINT
    void (*old_handler)(int) = sys_signal(SIGINT, manejador_sigint);
    
    if (old_handler == SIG_ERR) {
        sys_write(STDERR, "Error al registrar manejador de señal\n", 39);
        sys_exit(1);
    }
    
    sys_write(STDOUT, "Presiona Ctrl+C para probar el manejador...\n", 45);
    
    // Bucle principal del programa
    while (1) {
        sys_sleep(100);
    }
}
```

## Ejemplo de Limpieza con Señales
```c
#include <syscalls.h>

volatile int terminar = 0;

void manejador_terminar(int signum) {
    terminar = 1;
}

void main() {
    // Registrar manejador para terminación limpia
    sys_signal(SIGTERM, manejador_terminar);
    sys_signal(SIGINT, manejador_terminar);
    
    int fd = sys_open("/tmp/mi_archivo.tmp", O_WRONLY | O_CREAT);
    
    while (!terminar) {
        // Procesar datos...
        sys_write(fd, "datos\n", 6);
        sys_sleep(100);
    }
    
    // Limpieza al recibir señal de terminación
    sys_close(fd);
    sys_unlink("/tmp/mi_archivo.tmp");
    sys_write(STDOUT, "Limpieza completada\n", 20);
    sys_exit(0);
}
```

## Ejemplo de Ignorar Señales
```c
#include <syscalls.h>
void main() {
    // Ignorar SIGINT para que Ctrl+C no interrumpa
    sys_signal(SIGINT, SIG_IGN);
    
    sys_write(STDOUT, "Este programa ignora Ctrl+C\n", 29);
    
    // Realizar trabajo crítico que no debe interrumpirse
    realizar_trabajo_critico();
    
    // Restaurar comportamiento por defecto
    sys_signal(SIGINT, SIG_DFL);
    
    sys_write(STDOUT, "Ctrl+C restaurado\n", 19);
}
```

## Notas
- Los manejadores de señales deben ser funciones reentrantes y evitar operaciones complejas.
- Es seguro usar syscalls como `sys_write`, pero debe evitarse el uso de funciones de biblioteca estándar no reentrantes.
- Algunas señales como `SIGKILL` y `SIGSTOP` no pueden ser capturadas ni ignoradas.
- Los procesos hijos heredan los manejadores de señales del proceso padre en el momento de la creación.
- Usar variables globales volátiles (`volatile`) para comunicación entre el manejador de señales y el código principal.
- El manejador de señales puede interrumpir syscalls bloqueantes, potencialmente causando que devuelvan errores.

## Véase también
- [sys_kill](./sys_kill.md)
- [sys_exit](./sys_exit.md)
- [sys_wait](./sys_wait.md)
