# NeoOS - Syscalls
El sistema de llamadas al sistema (syscalls) en NeoOS es un componente fundamental que permite a los procesos en modo usuario interactuar con el núcleo del sistema operativo. A través de las syscalls, los procesos pueden solicitar servicios del kernel, como la gestión de memoria, la comunicación entre procesos, la gestión de archivos y dispositivos, entre otros.

## Mecanismo de Syscalls
Cuando un proceso en modo usuario necesita realizar una operación que requiere privilegios del kernel, realiza una syscall. Este proceso implica los siguientes pasos:
1. El proceso coloca los parámetros necesarios para la syscall en registros específicos.
2. El proceso ejecuta una instrucción especial que provoca una interrupción o excepción, transfiriendo el control al kernel.
3. El kernel identifica la syscall solicitada y ejecuta la función correspondiente.
4. Una vez completada la operación, el kernel devuelve el control al proceso en modo usuario, junto con cualquier valor de retorno necesario.