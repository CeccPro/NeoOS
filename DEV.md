# NeoOS - Documentación para Desarrolladores
Este documento proporciona una introducción general técnica de NeoOS, un sistema operativo de código abierto diseñado para ser ligero, rápido y seguro. Aquí se muestran superficialmente la arquitectura interna, las tecnologías utilizadas y las directrices para desarrolladores interesados en contribuir al proyecto.

## Arquitectura del Sistema
NeoOS está construido sobre una arquitectura modular que permitirá a los usuarios personalizar y optimizar su sistema operativo según sus necesidades específicas.

### Estado Actual de Implementación

#### [Implementado]
1. **Entry Point y Boot**: El kernel se carga correctamente mediante GRUB usando el estándar Multiboot. El entry point en assembly (`kmain.S`) configura la pila y llama a `kernel_main()`.

2. **Driver VGA**: Terminal VGA funcional en modo texto 80x25 con soporte para:
   - Colores configurables (16 colores de texto, 8 de fondo)
   - Scrolling automático
   - Caracteres especiales (newline, tab, carriage return)
   - Funciones de escritura hexadecimal y decimal

3. **Configuración del Kernel (kconfig)**: Sistema de configuración global que permite activar modos debug y verbose mediante parámetros de línea de comandos (`--debug`, `--verbose`).

4. **Códigos de Error**: Sistema completo de códigos de error con 13 códigos definidos y función `error_to_string()` para convertir códigos a strings descriptivos.

5. **Memory Manager Completo**:
   - **PMM (Physical Memory Manager)**: Gestión de páginas físicas con bitmap. Parsea el mapa de memoria de Multiboot y mantiene seguimiento de páginas libres/ocupadas.
   - **VMM (Virtual Memory Manager)**: Paginación de 2 niveles con identity mapping de los primeros 128MB. Usa estructuras estáticas para evitar problemas de bootstrapping.
   - **Heap del Kernel**: Asignador dinámico con `kmalloc()`/`kfree()` usando lista enlazada de bloques. Heap ubicado en 4MB con tamaño de 4MB.

6. **Biblioteca Estándar**: Funciones básicas de strings (`memset`, `memcpy`, `strlen`, `strcmp`, `strstr`) y definiciones de tipos básicos.

#### [Pendiente de Implementación]
1. **NeoCore (Núcleo Semi-Microkernel)**: El diseño conceptual existe pero los componentes core aún no están implementados:
   - GDT (Global Descriptor Table)
   - IDT (Interrupt Descriptor Table)
   - Manejo de interrupciones
   - PIC (Programmable Interrupt Controller)
   - Syscall handler

2. **Process Scheduler**: Sistema de planificación de procesos, PCB (Process Control Block), context switching.

3. **IPC Manager**: Comunicación entre procesos, message queues, signals, shared memory.

4. **Module Manager**: Sistema de carga dinámica de módulos con soporte hotplug.

5. **Sistema de Archivos (NeoFS)**: Sistema de archivos virtual y soporte para diferentes filesystems.

6. **Drivers Adicionales**: Teclado, timer PIT, controlador de disco.

7. **Transición a Modo Usuario**: Cambio de ring 0 (kernel) a ring 3 (usuario), proceso init.

2. **Interfaz de Usuario basada en WebView (NeoUI)**: La interfaz de usuario de NeoOS estará construida utilizando tecnologías web modernas a través de WebView. Esto permitirá una experiencia de usuario fluida y adaptable, facilitando la integración de aplicaciones web y servicios en el sistema operativo. Con este enfoque, los desarrolladores podrán crear aplicaciones utilizando HTML, CSS y JavaScript.

3. **Shell Integrado (NeoSH)**: NeoOS incluirá un shell integrado que permitirá a los usuarios interactuar con el sistema operativo a través de comandos. Se ejecutará directamente como un módulo dentro de NeoCore (similar a Bash en Linux).

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
La estructura de carpetas real del proyecto NeoOS es la siguiente:
```
NeoOS/
├── docs/                       # Documentación individual de cada parte del proyecto
│   ├── Boot Process.md
│   ├── Core Modules.md
│   ├── Errors.md
│   ├── Interrupts.md
│   ├── IPC.md
│   ├── Kernel Initialization.md
│   ├── Memory Manager.md
│   ├── MID and PID.md
│   ├── Module Manager.md
│   ├── Modules.md
│   ├── NeoCore.md
│   ├── NeoFS.md
│   ├── Process Scheduler.md
│   ├── SS.md
│   ├── Syscalls.md
│   ├── errors/                 # Documentación detallada de cada error
│   └── Syscalls/               # Documentación detallada de cada syscall
├── src/
│   └── kernel/                 # Código del kernel
│       ├── arch/               # Código específico de arquitectura
│       │   ├── x86/
│       │   │   └── boot/
│       │   │       └── kmain.S # Entry point x86 [Impl]
│       │   └── arm/
│       │       └── boot/
│       │           └── kmain.S # Entry point ARM [Pend]
│       ├── core/               # Núcleo del kernel [Impl]
│       │   ├── src/
│       │   │   ├── kmain.c     # Función principal [Impl]
│       │   │   ├── kconfig.c   # Configuración global [Impl]
│       │   │   └── error.c     # Códigos de error [Impl]
│       │   └── include/
│       │       ├── kmain.h
│       │       ├── kconfig.h
│       │       └── error.h
│       ├── drivers/            # Drivers de hardware
│       │   ├── src/
│       │   │   └── vga.c       # Driver VGA [Impl]
│       │   └── include/
│       │       └── vga.h
│       ├── lib/                # Biblioteca estándar del kernel [Impl]
│       │   ├── src/
│       │   │   └── string.c    # Funciones de strings [Impl]
│       │   └── include/
│       │       ├── string.h
│       │       ├── types.h     # Tipos básicos [Impl]
│       │       └── multiboot.h # Protocolo Multiboot [Impl]
│       ├── memory/             # Gestión de memoria [Impl]
│       │   ├── src/
│       │   │   ├── memory.c    # Coordinador [Impl]
│       │   │   ├── pmm.c       # Physical Memory Manager [Impl]
│       │   │   ├── vmm.c       # Virtual Memory Manager [Impl]
│       │   │   └── heap.c      # Heap del kernel [Impl]
│       │   └── include/
│       │       └── memory.h
│       ├── grub.cfg            # Configuración de GRUB [Impl]
│       ├── linker.ld           # Linker script [Impl]
│       ├── Makefile            # Build del kernel [Impl]
│       └── README.md
├── build/                      # Archivos compilados
│   ├── neoos                   # Binario del kernel
│   └── obj/                    # Archivos objeto
├── scripts/
│   └── create_disk_image.sh    # Script para crear imagen de disco
├── Makefile                    # Makefile principal del proyecto [Impl]
├── setup.sh                    # Script de configuración del entorno [Impl]
├── LICENSE                     # Licencia GNU v3.0
├── README.md                   # README principal
├── ARCHITECTURE.md             # Arquitectura del sistema
├── DEV.md                      # Documentación para desarrolladores
└── DOCUMENTATION_CHECKLIST.md  # Checklist de documentación
```

**Leyenda:**
- [Impl] = Implementado y funcional
- [Pend] = Pendiente de implementación
- (Sin símbolo) = Planificado pero no iniciado

**Nota**: Los siguientes directorios mencionados en documentación antigua **NO EXISTEN AÚN**:
- `src/kernel/fs/` - Sistema de archivos
- `src/kernel/ipc/` - Comunicación entre procesos
- `src/kernel/modules/` - Gestor de módulos
- `src/kernel/syscalls/` - Handler de syscalls
- `src/kernel/utils/` - Utilidades auxiliares
- `src/ui/` - Interfaz de usuario basada en WebView
- `src/modules/` - Módulos adicionales del sistema
- `tests/` - Pruebas y casos de prueba

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