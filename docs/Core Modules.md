# NeoOS - Core Modules
NeoOS utiliza un sistema modular que permite la carga y gestión de módulos esenciales del sistema operativo. Estos módulos proporcionan funcionalidades clave que son necesarias para el correcto funcionamiento del sistema operativo, como la gestión de memoria, el manejo de procesos y la comunicación entre procesos.

## Módulos Esenciales
A continuación se describen algunos de los módulos esenciales que forman parte del núcleo de NeoOS:
- **Memory Manager**: Este módulo es responsable de la gestión de la memoria del sistema, incluyendo la asignación y liberación de memoria para procesos, así como la gestión de la memoria virtual.
- **Process Scheduler**: Este módulo se encarga de la planificación y gestión de los procesos en ejecución, asegurando que los recursos del sistema se distribuyan de manera eficiente entre los procesos activos.
- **Module Manager**: Este módulo gestiona la carga, descarga y administración de otros módulos del sistema operativo, permitiendo la extensibilidad y modularidad del sistema.
- **Inter-Process Communication (IPC)**: Este módulo facilita la comunicación entre procesos, proporcionando mecanismos como colas de mensajes.
- **Process-Module Intercomunicator (PMIC)**: Sistema de comunicación entre procesos/kernel y módulos. Es diferente del IPC y permite a los procesos interactuar con módulos del sistema mediante RPC síncrono.

Algunos de estos módulos están integrados directamente en el kernel, mientras que otros se cargan como módulos separados durante el proceso de arranque y se ejecutan en modo usuario.

## Gestión de Módulos
NeoOS proporciona un gestor de módulos que permite la carga dinámica y la gestión de módulos del sistema operativo. Este gestor de módulos es responsable de:
- Cargar módulos en memoria cuando se requieren.
- Verificar la integridad y compatibilidad de los módulos antes de su carga.
- Proporcionar interfaces para que otros módulos interactúen entre sí.
- Descargar módulos que ya no son necesarios para liberar recursos del sistema.

## Véase también
- [Boot Process](./Boot%20Process.md)
- [Module Manager](./Module%20Manager.md)
- [Memory Manager](./Memory%20Manager.md)