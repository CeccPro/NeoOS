# NeoOS - sys_getpriority
La syscall `sys_getpriority(int pid)` en NeoOS se utiliza para obtener la prioridad de planificación actual de un proceso específico.

## Prototipo
```c
int sys_getpriority(int pid);
```

## Parámetros
- `pid`: El ID del proceso cuya prioridad se desea consultar. Un valor especial de 0 puede referirse al proceso actual.

## Comportamiento
Cuando un proceso llama a `sys_getpriority`, el sistema operativo realiza las siguientes acciones:
1. Verifica que el PID especificado corresponda a un proceso válido.
2. Verifica que el proceso que realiza la llamada tenga permisos para consultar la prioridad del proceso objetivo.
3. Lee el valor de prioridad del proceso desde la tabla de procesos.
4. Devuelve el valor de prioridad al proceso que realizó la llamada.

## Valor de Retorno
- Devuelve el valor de prioridad actual del proceso si la syscall se ejecuta correctamente. El valor típicamente está en el rango de:
  - 0 (prioridad más alta) a 31 (prioridad más baja), o
  - -20 (prioridad más alta) a 19 (prioridad más baja), según la convención del sistema.
- Devuelve un código de error (negativo) si ocurre un problema, como:
  - `E_NOENT`: Si el PID especificado no corresponde a ningún proceso existente.
  - `E_PERM`: Si el proceso no tiene permisos para consultar la prioridad del proceso objetivo.
  - `E_INVAL`: Si el PID es inválido.
  - Otros códigos de error definidos en `error.h`.

## Ejemplo de Uso
```c
#include <error.h>
#include <syscalls.h>
void main() {
    // Obtener la prioridad del proceso actual
    int prioridad = sys_getpriority(0);  // 0 = proceso actual
    
    if (prioridad >= 0) {
        char msg[50];
        sprintf(msg, "Mi prioridad actual es: %d\n", prioridad);
        sys_write(STDOUT, msg, strlen(msg));
    } else {
        sys_write(STDERR, "Error al obtener prioridad\n", 28);
    }
}
```

## Ejemplo de Monitoreo de Procesos
```c
#include <syscalls.h>
void mostrar_info_proceso(int pid) {
    int prioridad = sys_getpriority(pid);
    
    if (prioridad >= 0) {
        char msg[100];
        sprintf(msg, "Proceso %d - Prioridad: %d\n", pid, prioridad);
        sys_write(STDOUT, msg, strlen(msg));
    } else if (prioridad == E_NOENT) {
        char msg[100];
        sprintf(msg, "Proceso %d no existe\n", pid);
        sys_write(STDERR, msg, strlen(msg));
    }
}

void main() {
    // Monitorear varios procesos
    for (int pid = 1; pid <= 10; pid++) {
        mostrar_info_proceso(pid);
    }
}
```

## Ejemplo de Ajuste Dinámico de Prioridad
```c
#include <syscalls.h>
void ajustar_prioridad_si_necesario(int pid, int prioridad_deseada) {
    int prioridad_actual = sys_getpriority(pid);
    
    if (prioridad_actual < 0) {
        sys_write(STDERR, "Error al obtener prioridad\n", 28);
        return;
    }
    
    if (prioridad_actual != prioridad_deseada) {
        char msg[100];
        sprintf(msg, "Ajustando prioridad de %d a %d (era %d)\n",
                pid, prioridad_deseada, prioridad_actual);
        sys_write(STDOUT, msg, strlen(msg));
        
        sys_setpriority(pid, prioridad_deseada);
    } else {
        sys_write(STDOUT, "Prioridad ya está en el valor deseado\n", 39);
    }
}

void main() {
    int mi_pid = sys_getpid();
    ajustar_prioridad_si_necesario(mi_pid, 5);
}
```

## Ejemplo de Comparación de Prioridades
```c
#include <syscalls.h>
void comparar_prioridades(int pid1, int pid2) {
    int prioridad1 = sys_getpriority(pid1);
    int prioridad2 = sys_getpriority(pid2);
    
    if (prioridad1 < 0 || prioridad2 < 0) {
        sys_write(STDERR, "Error al obtener prioridades\n", 30);
        return;
    }
    
    char msg[150];
    if (prioridad1 < prioridad2) {
        sprintf(msg, "Proceso %d tiene mayor prioridad que proceso %d\n", pid1, pid2);
    } else if (prioridad1 > prioridad2) {
        sprintf(msg, "Proceso %d tiene mayor prioridad que proceso %d\n", pid2, pid1);
    } else {
        sprintf(msg, "Ambos procesos tienen la misma prioridad (%d)\n", prioridad1);
    }
    
    sys_write(STDOUT, msg, strlen(msg));
}
```

## Notas
- La capacidad de consultar la prioridad de otros procesos puede estar restringida por permisos del sistema.
- Los valores de prioridad devueltos son relativos y su interpretación depende del algoritmo de planificación del sistema.
- La prioridad de un proceso puede cambiar durante su ejecución si el sistema usa planificación dinámica.
- Consultar la prioridad es una operación rápida que no afecta la ejecución del proceso objetivo.
- Es útil para herramientas de monitoreo y diagnóstico del sistema.

## Véase también
- [sys_setpriority](./sys_setpriority.md)
- [sys_getpid](./sys_getpid.md)
- [sys_yield](./sys_yield.md)
- [Process Scheduler](../Process%20Scheduler.md)
