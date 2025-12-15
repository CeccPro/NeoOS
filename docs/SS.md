# NeoOS - Simon Says (ss) - Autenticación de Administrador
La autenticación de administrador en NeoOS es un mecanismo crucial para garantizar que solo los usuarios autorizados puedan acceder a funciones y configuraciones críticas del sistema. Este proceso implica la verificación de credenciales especiales que otorgan privilegios elevados necesarios para realizar tareas administrativas.

Como en Linux y otros sistemas operativos, NeoOS implementa un sistema de autenticación robusto que protege el acceso a recursos sensibles, asegurando la integridad y seguridad del sistema operativo.

## Proceso de Autenticación
El proceso de autenticación de administrador en NeoOS generalmente sigue estos pasos:
1. **Solicitud de Credenciales**: Cuando un usuario intenta acceder a funciones administrativas, el sistema solicita credenciales especiales, como una contraseña de administrador o un token de seguridad.
2. **Verificación**: El sistema verifica las credenciales proporcionadas contra una base de datos segura de usuarios autorizados.
3. **Asignación de Privilegios**: Si las credenciales son válidas, el usuario recibe privilegios administrativos que le permiten realizar operaciones críticas.
4. **Registro de Actividad**: Todas las acciones realizadas con privilegios de administrador son registradas para auditoría y seguimiento.

## Comando ss
NeoOS proporciona el comando `ss` (Simon Says) para facilitar la gestión de privilegios administrativos, siendo un equivalente al comando `sudo` en Linux. A continuación se muestra un ejemplo básico de cómo utilizar este comando:

```nsh
ss [admin_command] [params]
```

El comando `ss` al ejecutar un comando, dará siempre el output "Simon says: [output of the command]". Ejemplo:

```nsh
ss reboot
```
Output:
```
Simon says: System is rebooting...
```

Y si el usuario no tiene permisos para ejecutar el comando, el output será:
```
Simon says: You do not have permission to execute this command.
```

### Ejemplo de Uso
Para ejecutar un comando con privilegios de administrador, un usuario autorizado puede utilizar el siguiente formato:
```nsh
ss reboot
```
Este comando reiniciará el sistema, siempre y cuando el usuario tenga los permisos necesarios.

### Configuración de Permisos
La configuración de permisos para el uso del comando `ss` se gestiona a través de un archivo de configuración (who_is_simon.conf), donde se definen qué usuarios o grupos tienen acceso a qué comandos administrativos.