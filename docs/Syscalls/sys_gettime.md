# NeoOS - sys_gettime
La syscall `sys_gettime(struct timespec *ts)` en NeoOS se utiliza para obtener la hora actual del sistema con precisión de nanosegundos. Esta información representa el tiempo de reloj de pared (wall-clock time) y puede usarse para timestamping, sincronización y medición de tiempo absoluto.

## Prototipo
```c
int sys_gettime(struct timespec *ts);
```

## Parámetros
- `ts`: Un puntero a una estructura `timespec` donde se almacenará la hora actual. La estructura típicamente contiene:
  - `tv_sec`: Segundos desde la época (típicamente 1 de enero de 1970, 00:00:00 UTC).
  - `tv_nsec`: Nanosegundos adicionales (0-999999999).

## Comportamiento
Cuando un proceso llama a `sys_gettime`, el sistema operativo realiza las siguientes acciones:
1. Verifica que el puntero a la estructura `timespec` sea válido.
2. Lee el reloj de tiempo real del sistema (RTC - Real-Time Clock).
3. Calcula el tiempo transcurrido desde la época del sistema.
4. Llena la estructura `timespec` con los valores de segundos y nanosegundos.
5. Devuelve el resultado al proceso.

## Valor de Retorno
- Devuelve `E_OK` (0) si la syscall se ejecuta correctamente y la hora fue obtenida.
- Devuelve un código de error (negativo) si ocurre un problema, como:
  - `E_INVAL`: Si el puntero `ts` es NULL o inválido.
  - `E_NOT_SUPPORTED`: Si el sistema no tiene un reloj de tiempo real.
  - Otros códigos de error definidos en `error.h`.

## Ejemplo de Uso
```c
#include <error.h>
#include <syscalls.h>
void main() {
    struct timespec tiempo_actual;
    int result = sys_gettime(&tiempo_actual);
    
    if (result == E_OK) {
        char msg[100];
        sprintf(msg, "Tiempo actual: %ld segundos, %ld nanosegundos\n",
                tiempo_actual.tv_sec, tiempo_actual.tv_nsec);
        sys_write(STDOUT, msg, strlen(msg));
    } else {
        sys_write(STDERR, "Error al obtener tiempo\n", 24);
    }
}
```

## Ejemplo de Timestamp para Logs
```c
#include <syscalls.h>
void escribir_log_con_timestamp(const char *mensaje) {
    struct timespec ts;
    
    if (sys_gettime(&ts) == E_OK) {
        int fd = sys_open("/var/log/miapp.log", O_WRONLY | O_CREAT | O_APPEND);
        
        if (fd >= 0) {
            char buffer[256];
            sprintf(buffer, "[%ld.%09ld] %s\n", 
                    ts.tv_sec, ts.tv_nsec, mensaje);
            sys_write(fd, buffer, strlen(buffer));
            sys_close(fd);
        }
    }
}

void main() {
    escribir_log_con_timestamp("Aplicación iniciada");
    // ... código de la aplicación ...
    escribir_log_con_timestamp("Operación completada");
}
```

## Ejemplo de Medición de Duración Precisa
```c
#include <syscalls.h>
void medir_duracion_precisa() {
    struct timespec inicio, fin;
    
    // Tomar tiempo inicial
    sys_gettime(&inicio);
    
    // Realizar operación a medir
    realizar_operacion_compleja();
    
    // Tomar tiempo final
    sys_gettime(&fin);
    
    // Calcular diferencia
    long segundos = fin.tv_sec - inicio.tv_sec;
    long nanosegundos = fin.tv_nsec - inicio.tv_nsec;
    
    // Ajustar si los nanosegundos son negativos
    if (nanosegundos < 0) {
        segundos--;
        nanosegundos += 1000000000;
    }
    
    char msg[100];
    sprintf(msg, "Duración: %ld.%09ld segundos\n", segundos, nanosegundos);
    sys_write(STDOUT, msg, strlen(msg));
}
```

## Ejemplo de Conversión a Formato Legible
```c
#include <syscalls.h>
void mostrar_fecha_hora() {
    struct timespec ts;
    
    if (sys_gettime(&ts) == E_OK) {
        // Convertir segundos a fecha/hora
        // (simplificado - una implementación real necesitaría
        // manejar años bisiestos, zonas horarias, etc.)
        long segundos_totales = ts.tv_sec;
        int segundos = segundos_totales % 60;
        int minutos = (segundos_totales / 60) % 60;
        int horas = (segundos_totales / 3600) % 24;
        int dias = segundos_totales / 86400;
        
        char msg[100];
        sprintf(msg, "Tiempo: %d días, %02d:%02d:%02d\n", 
                dias, horas, minutos, segundos);
        sys_write(STDOUT, msg, strlen(msg));
    }
}
```

## Notas
- `sys_gettime` proporciona tiempo de reloj de pared, que puede ser ajustado por el sistema (cambios de hora, NTP, etc.).
- Para mediciones de duración donde la precisión es crítica y no se debe ver afectada por ajustes del reloj, considere usar `sys_uptime`.
- La resolución real depende del hardware; el sistema puede no soportar precisión de nanosegundos en todas las plataformas.
- El tiempo devuelto está en UTC (Tiempo Universal Coordinado) a menos que el sistema esté configurado de otra manera.
- Los segundos desde la época pueden desbordarse eventualmente (problema del año 2038 en sistemas de 32 bits).

## Véase también
- [sys_uptime](./sys_uptime.md)
- [sys_sleep](./sys_sleep.md)
