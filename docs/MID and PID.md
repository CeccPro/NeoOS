# NeoOS - MID y PID
En NeoOS, cada proceso y módulo del sistema operativo está identificado por dos tipos de identificadores únicos: MID (Module ID) y PID (Process ID). Estos identificadores son esenciales para la gestión de procesos y la comunicación entre ellos.

## MID (Module ID)
El MID es un identificador único asignado a cada módulo del sistema operativo. Dado que NeoOS es un semi-microkernel, los módulos son procesos independientes que realizan funciones específicas del sistema operativo. El MID permite al sistema y a otros procesos identificar y comunicarse con estos módulos de manera eficiente.

### Asignación de MID
Los MID se asignan automáticamente cuando un módulo es cargado en el sistema. Cada módulo recibe un MID único que no se reutiliza mientras el módulo esté activo.
### Uso de MID
Los procesos pueden utilizar el MID para enviar mensajes o solicitudes a módulos específicos. Esto se realiza mediante PMIC (Process-Module Intercomunicator), que es diferente del IPC usado para comunicación entre procesos. Para obtener el MID de un módulo, se puede utilizar la función `module_get_id(module_name)` o la syscall `sys_modgetid(module_name)`.

**Importante**: Los módulos NO usan IPC. La comunicación con módulos se hace exclusivamente a través de PMIC.

## PID (Process ID)
El PID es un identificador único asignado a cada proceso en ejecución dentro del sistema operativo. El PID permite al sistema gestionar y controlar los procesos de manera efectiva, facilitando la asignación de recursos y la comunicación entre procesos.

### Asignación de PID
Los PID se asignan automáticamente cuando un proceso es creado. Cada proceso recibe un PID único que no se reutiliza mientras el proceso esté activo.
### Uso de PID
Los procesos pueden utilizar el PID para enviar mensajes o señales a otros procesos. Esto es fundamental para la Comunicación entre Procesos (IPC) en NeoOS. Para obtener el PID de un proceso, se puede utilizar la función `get_process_id(process_name)`.

## Conclusión
Los identificadores MID y PID son componentes clave del sistema operativo NeoOS, facilitando la gestión de procesos y módulos, así como la comunicación entre ellos. Estos identificadores aseguran que cada proceso y módulo pueda ser identificado de manera única, permitiendo una operación eficiente y segura del sistema operativo.