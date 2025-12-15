# NeoOS - sys_yield
La syscall `sys_yield()` en NeoOS se utiliza para que un proceso ceda voluntariamente el control del CPU al planificador (scheduler). Esto permite que otros procesos listos para ejecutarse tengan la oportunidad de usar el procesador, mejorando la capacidad de respuesta y equidad del sistema.

## Prototipo
```c
void sys_yield(void);
```

## Parámetros
- Ninguno.

## Comportamiento
Cuando un proceso llama a `sys_yield`, el sistema operativo realiza las siguientes acciones:
1. Guarda el contexto completo del proceso actual (registros, contador de programa, etc.).
2. Marca el proceso como listo para ejecutarse (runnable) en lugar de en ejecución (running).
3. Invoca al planificador de procesos para seleccionar el siguiente proceso a ejecutar.
4. El planificador puede elegir el mismo proceso u otro diferente según la política de planificación.
5. Restaura el contexto del proceso seleccionado y le transfiere el control del CPU.

## Valor de Retorno
- Esta syscall no devuelve ningún valor. El proceso simplemente continúa su ejecución después de que el planificador le devuelva el control del CPU.

## Ejemplo de Uso
```c
#include <syscalls.h>
void main() {
    while (1) {
        // Realizar algún trabajo
        procesar_datos();
        
        // Ceder el CPU para dar oportunidad a otros procesos
        sys_yield();
    }
}
```

## Ejemplo de Uso Avanzado
```c
#include <syscalls.h>
void espera_activa_cooperativa(int *flag) {
    while (!*flag) {
        // En lugar de consumir CPU continuamente,
        // ceder para permitir que otros procesos ejecuten
        sys_yield();
    }
}
```

## Notas
- `sys_yield` es útil en bucles de espera activa para evitar consumir innecesariamente tiempo de CPU.
- En sistemas con planificación cooperativa, es esencial que los procesos llamen periódicamente a `sys_yield` para mantener la capacidad de respuesta del sistema.
- En sistemas con planificación preventiva, el planificador puede interrumpir procesos automáticamente, pero `sys_yield` sigue siendo útil para procesos que desean ser "buenos ciudadanos" del sistema.
- Esta syscall no garantiza que otros procesos se ejecutarán; el planificador puede inmediatamente devolver el control al mismo proceso si no hay otros procesos listos.

## Véase también
- [sys_sleep](./sys_sleep.md)
- [sys_setpriority](./sys_setpriority.md)
- [sys_getpriority](./sys_getpriority.md)
- [Process Scheduler](../Process%20Scheduler.md)
