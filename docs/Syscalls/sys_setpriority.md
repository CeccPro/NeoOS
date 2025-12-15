# NeoOS - sys_setpriority
La syscall `sys_setpriority(int pid, int priority)` en NeoOS se utiliza para establecer la prioridad de planificación de un proceso específico. La prioridad determina cuánto tiempo de CPU recibe un proceso en relación con otros procesos.

## Prototipo
```c
int sys_setpriority(int pid, int priority);
```

## Parámetros
- `pid`: El ID del proceso cuya prioridad se desea cambiar. Un valor especial de 0 puede referirse al proceso actual.
- `priority`: El nuevo valor de prioridad para el proceso. Los valores típicamente van de:
  - 0 (prioridad más alta) a 31 (prioridad más baja), o
  - -20 (prioridad más alta) a 19 (prioridad más baja), según la convención del sistema.
  - Valores más bajos generalmente indican mayor prioridad y más tiempo de CPU.

## Comportamiento
Cuando un proceso llama a `sys_setpriority`, el sistema operativo realiza las siguientes acciones:
1. Verifica que el PID especificado corresponda a un proceso válido.
2. Verifica que el proceso que realiza la llamada tenga permisos para cambiar la prioridad del proceso objetivo.
3. Valida que el valor de prioridad esté dentro del rango permitido.
4. Actualiza la prioridad del proceso en la tabla de procesos.
5. Si el proceso está en la cola de procesos listos, puede ser reordenado según su nueva prioridad.
6. El planificador usará la nueva prioridad en decisiones futuras de planificación.

## Valor de Retorno
- Devuelve `E_OK` (0) si la syscall se ejecuta correctamente y la prioridad fue cambiada.
- Devuelve un código de error (negativo) si ocurre un problema, como:
  - `E_NOENT`: Si el PID especificado no corresponde a ningún proceso existente.
  - `E_PERM`: Si el proceso no tiene permisos para cambiar la prioridad del proceso objetivo.
  - `E_INVAL`: Si el valor de prioridad está fuera del rango válido.
  - Otros códigos de error definidos en `error.h`.

## Ejemplo de Uso
```c
#include <error.h>
#include <syscalls.h>
void main() {
    // Reducir la prioridad del proceso actual (hacerlo "más amigable")
    int result = sys_setpriority(0, 10);  // 0 = proceso actual
    
    if (result == E_OK) {
        sys_write(STDOUT, "Prioridad reducida exitosamente\n", 33);
    } else if (result == E_PERM) {
        sys_write(STDERR, "Sin permisos para cambiar prioridad\n", 37);
    } else {
        sys_write(STDERR, "Error al cambiar prioridad\n", 28);
    }
}
```

## Ejemplo de Ajuste de Prioridad de Proceso Hijo
```c
#include <syscalls.h>
void main() {
    char *args[] = {"proceso_intensivo", NULL};
    int pid = sys_run("/bin/proceso_intensivo", args);
    
    if (pid > 0) {
        // Reducir prioridad del proceso hijo para que no monopolice CPU
        int result = sys_setpriority(pid, 15);
        
        if (result == E_OK) {
            sys_write(STDOUT, "Prioridad del hijo ajustada\n", 29);
        }
        
        // Esperar a que termine
        int status;
        sys_wait(&status);
    }
}
```

## Ejemplo de Sistema de Prioridades
```c
#include <syscalls.h>

#define PRIORIDAD_ALTA 0
#define PRIORIDAD_NORMAL 10
#define PRIORIDAD_BAJA 20

void ejecutar_con_prioridad(const char *programa, int prioridad) {
    char *args[] = {programa, NULL};
    int pid = sys_run(programa, args);
    
    if (pid > 0) {
        if (sys_setpriority(pid, prioridad) == E_OK) {
            char msg[100];
            sprintf(msg, "Proceso %d iniciado con prioridad %d\n", pid, prioridad);
            sys_write(STDOUT, msg, strlen(msg));
        }
    }
}

void main() {
    // Ejecutar diferentes procesos con diferentes prioridades
    ejecutar_con_prioridad("/bin/tarea_critica", PRIORIDAD_ALTA);
    ejecutar_con_prioridad("/bin/tarea_normal", PRIORIDAD_NORMAL);
    ejecutar_con_prioridad("/bin/tarea_fondo", PRIORIDAD_BAJA);
}
```

## Notas
- Cambiar la prioridad de procesos de otros usuarios típicamente requiere privilegios elevados (superusuario).
- Un proceso generalmente puede reducir su propia prioridad sin restricciones, pero aumentarla puede requerir permisos especiales.
- Las prioridades son relativas; un proceso de baja prioridad aún recibirá tiempo de CPU si no hay procesos de mayor prioridad listos.
- El efecto exacto de la prioridad depende del algoritmo de planificación utilizado por el sistema operativo.
- Usar prioridades muy altas puede causar que otros procesos no reciban suficiente tiempo de CPU (inanición).
- La prioridad se hereda por procesos hijos a menos que se cambie explícitamente.

## Véase también
- [sys_getpriority](./sys_getpriority.md)
- [sys_yield](./sys_yield.md)
- [sys_getpid](./sys_getpid.md)
- [Process Scheduler](../Process%20Scheduler.md)
