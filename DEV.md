# NeoOS - Documentación para Desarrolladores
Este documento proporciona una introducción general técnica de NeoOS, un sistema operativo de código abierto diseñado para ser ligero, rápido y seguro. Aquí se muestran superficialmente la arquitectura interna, las tecnologías utilizadas y las directrices para desarrolladores interesados en contribuir al proyecto.

## Arquitectura del Sistema
NeoOS está construido sobre una arquitectura modular que permite a los usuarios personalizar y optimizar su sistema operativo según sus necesidades específicas. La arquitectura se compone de los siguientes componentes principales:

1. **Núcleo Modular (NeoCore)**: El núcleo de NeoOS está diseñado para ser ligero y eficiente, permitiendo la carga dinámica de módulos según las necesidades del usuario. Esto facilita la personalización y mejora el rendimiento del sistema. Hay un par de módulos necesarios para el funcionamiento básico del sistema, pero la mayoría son opcionales y pueden ser añadidos o eliminados según las preferencias del usuario.

NeoCore es un semi-microkernel (Tiene características de microkernel y monolítico) que maneja las operaciones básicas del sistema, la gestión de memoria, la planificación de procesos y la comunicación entre módulos. Su diseño modular permite a los desarrolladores crear y agregar nuevos módulos sin afectar la estabilidad del núcleo principal. Internamente, solo el IPC, la gestión de syscalls, el manejo de interrupciones y el Module Manager residen en el núcleo, mientras que otros servicios del sistema operativo se implementan como módulos separados.

2. **Interfaz de Usuario basada en WebView (NeoUI)**: La interfaz de usuario de NeoOS está construida utilizando tecnologías web modernas a través de WebView. Esto permite una experiencia de usuario fluida y adaptable, facilitando la integración de aplicaciones web y servicios en el sistema operativo. Con este enfoque, los desarrolladores pueden crear aplicaciones utilizando HTML, CSS y JavaScript, lo que amplía las posibilidades de desarrollo, además de poder modificar más fácilmente la interfaz del sistema operativo con conocimiento básico de tecnologías web.

3. **Shell Integrado (NeoSH)**: NeoOS incluye un shell integrado que permite a los usuarios interactuar con el sistema operativo a través de comandos. El shell está diseñado para ser intuitivo y fácil de usar, proporcionando acceso rápido a las funciones del sistema. Se ejecuta directamente como un módulo dentro de NeoCore, lo que permite una integración perfecta con el resto del sistema operativo. (Es similar a Bash en Linux).

## Tecnologías Utilizadas
NeoOS utiliza una combinación de tecnologías modernas para garantizar un rendimiento óptimo y una experiencia de usuario excepcional. Algunas de las tecnologías clave incluyen:
- **Lenguaje de Programación**: NeoOS está desarrollado principalmente en ASM y C/C++ para el núcleo y en JavaScript para la interfaz de usuario.
- **WebView**: Utilizado para renderizar la interfaz de usuario basada en tecnologías web como HTML5, CSS3 y JavaScript.
- **Sistema de Archivos Virtual**: Un sistema de archivos virtual que permite la gestión eficiente de recursos y datos del usuario.
- **Módulos Dinámicos**: Soporte para la carga y descarga dinámica (En su mayoría hotplug) de módulos del sistema operativo.

## Directrices para Desarrolladores
Los desarrolladores interesados en contribuir a NeoOS deben seguir las siguientes directrices:
1. **Configuración del Entorno de Desarrollo**: Se recomienda utilizar un entorno de desarrollo compatible con C/C++ y JavaScript. Asegúrese de tener instaladas las herramientas necesarias para compilar y probar el sistema operativo.
2. **Contribución al Código**: Las contribuciones deben seguir las mejores prácticas de codificación y estar bien documentadas. Se recomienda utilizar un sistema de control de versiones como Git para gestionar los cambios.
3. **Pruebas y Validación**: Antes de enviar contribuciones, asegúrese de probar exhaustivamente el código para garantizar la estabilidad y el rendimiento del sistema operativo.
4. **Documentación**: Proporcione documentación clara y detallada para cualquier nueva funcionalidad o cambio realizado en el sistema, y dejando comentarios en el código cuando sea necesario.
5. **Compatibilidad con GPL v3.0**: Todas las contribuciones deben cumplir con los términos de la Licencia Pública General GNU v3.0 bajo la cual NeoOS está licenciado, y al contribuir, los desarrolladores aceptan que su código será licenciado bajo los mismos términos.

## Estructura de carpetas
La estructura de carpetas del proyecto NeoOS es la siguiente:
```NeoOS/
├── docs/                   # Documentación individual de cada parte del proyecto
├── src/                    # Código fuente del sistema operativo
│   ├── kernel/             # Código del núcleo modular
│   │   ├── arch            # Código específico de la arquitectura (x86, ARM, etc.)
│   │   ├── drivers/        # Controladores de hardware básicos
│   │   ├── fs/             # Soporte a sistemas de archivos básicos. (El soporte avanzado se monta como módulos)
│   │   ├── ipc/            # Mecanismos de comunicación entre procesos
│   │   ├── memory/         # Gestión de memoria
│   │   ├── modules/        # Módulo gestor de módulos y módulos básicos
│   │   ├── core/           # Funciones centrales del núcleo (Entry point y cosas muy low-level)
│   │   ├── syscalls/       # Implementación de llamadas al sistema
│   │   └── utils/          # Utilidades y funciones auxiliares del núcleo
│   ├── ui/                 # Código de la interfaz de usuario basada en WebView
│   └── modules/            # Módulos adicionales del sistema operativo
├── tests/                  # Pruebas y casos de prueba
├── Makefile                # Archivo de construcción del proyecto
├── LICENSE                 # Archivo de licencia GNU v3.0
├── setup.sh                # Script de configuración del entorno de desarrollo
├── README.md               # Archivo README principal
├── DOCUMENTATION_CHECKLIST.md # Lista de verificación para la documentación
└── DEV.md                  # Documentación superficial para desarrolladores
```

## Cómo compilar
Para compilar NeoOS, siga estos pasos:
1. Asegúrese de tener todas las dependencias necesarias instaladas en su sistema. (Ejecute el archivo `setup.sh` en la raíz del proyecto para instalar dependencias automáticamente en sistemas basados en Debian).
2. Abra una terminal y navegue hasta el directorio raíz del proyecto NeoOS.
3. Ejecute el siguiente comando para compilar el sistema operativo:
   ```bash
   make all && make iso
   ```
4. Una vez completada la compilación, los archivos binarios y la imagen ISO estarán disponibles en el directorio `build/`.

Para limpiar los archivos de compilación, puede ejecutar:
```bash
make clean
```

## Futuras Mejoras
NeoOS está en constante evolución, y se planean varias mejoras y características adicionales para futuras versiones, incluyendo:
- Soporte para más arquitecturas de hardware.
- Mejoras en la seguridad y privacidad del usuario.
- Expansión del ecosistema de aplicaciones y módulos disponibles.
- Herramientas adicionales para desarrolladores.