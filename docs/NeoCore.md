# NeoOS - NeoCore
NeoCore es el núcleo del sistema operativo NeoOS, responsable de gestionar los recursos del sistema, la comunicación entre procesos y la administración de módulos. Como un semi-microkernel, NeoCore proporciona una arquitectura modular que permite una mayor flexibilidad y escalabilidad del sistema operativo.

## Funciones Principales de NeoCore
1. **Gestión de Procesos**: NeoCore administra la creación, ejecución y terminación de procesos en el sistema. Cada proceso tiene un identificador único (PID) que permite su gestión y comunicación.
2. **Comunicación entre Procesos (IPC)**: NeoCore implementa mecanismos de IPC que permiten a los procesos intercambiar datos y señales de manera eficiente y segura.
3. **Gestión de Módulos**: A través del Module Manager, NeoCore carga, descarga y administra los módulos del sistema, facilitando la expansión y personalización del entorno operativo.
4. **Manejo de Recursos**: NeoCore gestiona los recursos del sistema, incluyendo memoria, CPU y dispositivos de entrada/salida, asegurando una distribución eficiente y equitativa entre los procesos.
5. **Seguridad y Protección**: NeoCore implementa mecanismos de seguridad para proteger el sistema y los datos de accesos no autorizados y garantizar la integridad del sistema operativo.

## Arquitectura interna de NeoCore
NeoCore está diseñado con una arquitectura modular que permite la integración de diferentes componentes y servicios. Los módulos del sistema, gestionados por el Module Manager, pueden ser cargados y descargados dinámicamente, lo que facilita la adaptación del sistema a diferentes necesidades y entornos.
### ¿Cómo funciona internamente?
NeoCore se compone de varios subsistemas que trabajan juntos para proporcionar las funcionalidades esenciales del sistema operativo. Estos subsistemas incluyen:
- **Scheduler**: Responsable de la planificación y asignación de tiempo de CPU a los procesos. [IMPLEMENTADO]
- **Memory Manager**: Gestiona la asignación y liberación de memoria para los procesos (PMM, VMM, Heap). [IMPLEMENTADO]
- **IPC Manager**: Facilita la comunicación entre procesos mediante colas de mensajes. [IMPLEMENTADO]
- **Syscall Handler**: Proporciona la interfaz entre modo usuario y kernel mediante `int 0x80`. [IMPLEMENTADO]
- **Module Manager**: Carga, descarga y administra los módulos del sistema. [PENDIENTE]

## Ciclo de vida del kernel
NeoCore sigue un ciclo de vida que incluye las siguientes etapas:
1. **Inicialización**: Durante el arranque del sistema, NeoCore se inicializa siguiendo este orden:
   - Configuración del driver VGA
   - Inicialización de kconfig (parseo de cmdline)
   - Inicialización del Memory Manager (PMM, VMM, Heap)
   - Configuración de GDT (Global Descriptor Table)
   - Configuración de IDT (Interrupt Descriptor Table)
   - Remapeo del PIC e instalación de handlers
   - Inicialización del PIT (Timer a 100Hz)
   - Inicialización del Scheduler
   - Inicialización del sistema IPC
   - Instalación del Syscall Handler (int 0x80)
2. **Ejecución**: Una vez inicializado, NeoCore transfiere el control al scheduler mediante `scheduler_switch()`, que nunca retorna. A partir de ahí, el sistema opera en modo multitarea con cambios de contexto automáticos cada 10ms (IRQ0).
3. **Mantenimiento**: NeoCore realiza tareas de mantenimiento continuas como gestión de memoria, planificación de procesos y manejo de interrupciones.
4. **Terminación**: Al apagar el sistema, NeoCore libera los recursos y cierra los procesos de manera ordenada. [Aún no implementado completamente]

## Modos de Operación
NeoCore está diseñado para operar en dos modos principales:
1. **Modo Usuario (Ring 3)**: En este modo, los procesos de usuario se ejecutan con privilegios limitados, accediendo a los recursos del sistema a  través de llamadas al sistema (syscalls) proporcionadas por NeoCore mediante la instrucción `int 0x80`. [PENDIENTE - Actualmente todos los procesos corren en modo kernel]
2. **Modo Kernel (Ring 0)**: En este modo, NeoCore y sus subsistemas operan con privilegios completos, gestionando directamente los recursos del hardware y proporcionando servicios esenciales. [IMPLEMENTADO - Modo actual de operación]

**Estado Actual**: Todos los procesos actualmente se ejecutan en modo kernel (ring 0). La transición a modo usuario requiere configurar TSS, segmentos de usuario en GDT y stacks separados.