# NeoOS - sys_uptime
La syscall `sys_uptime()` en NeoOS se utiliza para obtener el tiempo total que el sistema ha estado en funcionamiento desde el último arranque. Esta información es útil para monitoreo del sistema, depuración, estadísticas y cálculos de tiempo relativo.

## Prototipo
```c
int sys_uptime(void);
```

## Parámetros
- Ninguno.

## Comportamiento
Cuando un proceso llama a `sys_uptime`, el sistema operativo simplemente lee y devuelve el contador de tiempo del sistema que se incrementa desde el arranque. Esta es una operación rápida que no bloquea al proceso.

## Valor de Retorno
- Devuelve el número de ticks del reloj del sistema que han transcurrido desde el arranque del sistema.
- El valor es siempre no negativo.
- Esta syscall nunca falla.

## Ejemplo de Uso
```c
#include <syscalls.h>
void main() {
    int uptime = sys_uptime();
    
    // Convertir ticks a segundos (suponiendo 100 ticks por segundo)
    int segundos = uptime / 100;
    int minutos = segundos / 60;
    int horas = minutos / 60;
    
    segundos = segundos % 60;
    minutos = minutos % 60;
    
    char message[100];
    sprintf(message, "Sistema activo: %02d:%02d:%02d\n", horas, minutos, segundos);
    sys_write(STDOUT, message, strlen(message));
}
```

## Ejemplo de Uso para Medición de Tiempo
```c
#include <syscalls.h>
void medir_tiempo_de_operacion() {
    int inicio = sys_uptime();
    
    // Realizar operación a medir
    realizar_operacion_compleja();
    
    int fin = sys_uptime();
    int duracion_ticks = fin - inicio;
    
    char message[50];
    sprintf(message, "Operación tomó %d ticks\n", duracion_ticks);
    sys_write(STDOUT, message, strlen(message));
}
```

## Notas
- El valor devuelto está en ticks del reloj del sistema. Para convertirlo a unidades de tiempo real (segundos, milisegundos), es necesario conocer la frecuencia del reloj del sistema.
- El contador de uptime puede eventualmente desbordarse si el sistema permanece en funcionamiento durante un tiempo extremadamente largo. Los desarrolladores deben considerar este caso en aplicaciones de larga duración.
- Esta syscall es útil para implementar timeouts relativos y medir duraciones de operaciones.
- A diferencia de `sys_gettime`, que proporciona tiempo de reloj de pared (wall-clock time), `sys_uptime` proporciona tiempo monotónico desde el arranque.

## Véase también
- [sys_sleep](./sys_sleep.md)
- [sys_gettime](./sys_gettime.md)
- [sys_yield](./sys_yield.md)
