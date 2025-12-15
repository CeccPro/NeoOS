# NeoOS - sys_sleep
La syscall `sys_sleep(int ticks)` en NeoOS se utiliza para suspender la ejecución del proceso actual durante un número especificado de ticks del reloj del sistema. Esta syscall es útil para implementar temporizadores, esperas controladas y sincronización temporal.

## Prototipo
```c
int sys_sleep(int ticks);
```

## Parámetros
- `ticks`: El número de ticks del reloj del sistema durante los cuales el proceso debe permanecer suspendido. Un tick representa la unidad básica de tiempo del planificador del sistema. El valor debe ser no negativo.

## Comportamiento
Cuando un proceso llama a `sys_sleep`, el sistema operativo realiza las siguientes acciones:
1. Verifica que el número de ticks sea válido (no negativo).
2. Si `ticks` es 0, la syscall devuelve inmediatamente sin suspender el proceso.
3. Guarda el contexto del proceso actual.
4. Marca el proceso como "durmiendo" (sleeping) y lo coloca en una cola de espera temporal.
5. Registra el tiempo de despertar del proceso (tiempo actual + ticks).
6. Invoca al planificador para seleccionar otro proceso a ejecutar.
7. Cuando el número especificado de ticks transcurre, el kernel mueve el proceso de vuelta a la cola de procesos listos.
8. El planificador eventualmente selecciona el proceso y este continúa su ejecución.

## Valor de Retorno
- Devuelve `E_OK` (0) si el proceso durmió por el tiempo completo especificado.
- Devuelve un código de error si ocurre un problema, como:
  - `E_INVAL`: Si el número de ticks es negativo.
  - Otros códigos de error definidos en `error.h`.

## Ejemplo de Uso
```c
#include <error.h>
#include <syscalls.h>
void main() {
    // Imprimir un mensaje cada segundo (suponiendo 100 ticks = 1 segundo)
    while (1) {
        sys_write(STDOUT, "Un segundo ha pasado\n", 21);
        sys_sleep(100);  // Dormir por 100 ticks
    }
}
```

## Ejemplo de Uso Avanzado
```c
#include <syscalls.h>
void temporizador_periodico(int periodo_ticks, int iteraciones) {
    for (int i = 0; i < iteraciones; i++) {
        // Realizar alguna tarea
        realizar_tarea();
        
        // Esperar hasta el siguiente periodo
        sys_sleep(periodo_ticks);
    }
}
```

## Notas
- El tiempo de suspensión es aproximado y depende de la resolución del reloj del sistema y la carga del sistema.
- Durante el tiempo de suspensión, el proceso no consume tiempo de CPU, permitiendo que otros procesos se ejecuten.
- El proceso puede despertar ligeramente después del tiempo especificado si el sistema está muy cargado.
- `sys_sleep(0)` puede usarse como una forma alternativa de `sys_yield` en algunos contextos.
- La duración real de un tick del sistema depende de la configuración del hardware y el kernel.

## Véase también
- [sys_yield](./sys_yield.md)
- [sys_uptime](./sys_uptime.md)
- [sys_gettime](./sys_gettime.md)
- [Process Scheduler](../Process%20Scheduler.md)
