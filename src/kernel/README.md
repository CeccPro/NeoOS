# NeoOS Kernel

Este directorio contiene el código fuente del kernel NeoCore de NeoOS.

## Estructura del Proyecto

```
src/kernel/
├── arch/                     # Código específico de arquitectura
│   ├── x86/
│   │   └── boot/
│   │       └── kmain.S       # Entry point x86 [Impl]
│   └── arm/
│       └── boot/
│           └── kmain.S       # Entry point ARM [Pend]
├── core/                     # Núcleo del kernel [Impl]
│   ├── src/
│   │   ├── kmain.c           # Función principal [Impl]
│   │   ├── kconfig.c         # Configuración global [Impl]
│   │   └── error.c           # Códigos de error [Impl]
│   └── include/
│       ├── kmain.h
│       ├── kconfig.h
│       └── error.h
├── drivers/                  # Controladores de hardware
│   ├── src/
│   │   └── vga.c            # Driver VGA [Impl]
│   └── include/
│       └── vga.h
├── lib/                      # Biblioteca estándar [Impl]
│   ├── src/
│   │   └── string.c         # Funciones de strings [Impl]
│   └── include/
│       ├── string.h
│       ├── types.h          # Tipos básicos [Impl]
│       └── multiboot.h      # Protocolo Multiboot [Impl]
├── memory/                   # Gestión de memoria [Impl]
│   ├── src/
│   │   ├── memory.c         # Coordinador [Impl]
│   │   ├── pmm.c            # Physical Memory Manager [Impl]
│   │   ├── vmm.c            # Virtual Memory Manager [Impl]
│   │   └── heap.c           # Heap del kernel [Impl]
│   └── include/
│       └── memory.h
├── Makefile                  # Sistema de compilación [Impl]
├── linker.ld                 # Script del linker [Impl]
├── grub.cfg                  # Configuración de GRUB [Impl]
└── README.md

Leyenda:
[Impl] = Implementado y funcional
[Pend] = Pendiente de implementación
```

## Requisitos de Compilación

Para compilar NeoOS necesitas:

### En sistemas Debian/Ubuntu:
```bash
sudo apt-get install build-essential nasm
sudo apt-get install gcc-multilib g++-multilib
sudo apt-get install grub-pc-bin xorriso
sudo apt-get install qemu-system-x86
```

### Cross-Compiler (i686-elf)
Necesitas un cross-compiler para i686-elf. Puedes:
1. Ejecutar el script `setup.sh` en la raíz del proyecto
2. O seguir la guía en https://wiki.osdev.org/GCC_Cross-Compiler

## Compilación

Desde el directorio raíz del proyecto:
```bash
make all        # Compila el kernel
make img        # Crea una imagen ISO booteable
make run        # Compila y ejecuta en QEMU
```

Desde este directorio (src/kernel):
```bash
make            # Compila el kernel
make img        # Crea ISO
make run        # Ejecuta en QEMU
make clean      # Limpia archivos compilados
```

## Verificar Dependencias

```bash
make check
```

## Ejecución

Para ejecutar NeoOS en QEMU:
```bash
make run
```

Para ejecutar con GDB para debugging:
```bash
make debug
# En otra terminal: gdb build/neoos
# (gdb) target remote localhost:1234
```

## Arquitectura del Kernel

NeoCore es un semi-microkernel que incluye:
- **IPC**: Comunicación entre procesos
- **Syscalls**: Interfaz de llamadas al sistema
- **Interrupts**: Manejo de interrupciones
- **Module Manager**: Gestor de módulos dinámicos
- **Basic Drivers**: Controladores básicos (ej. VGA)
- **Soporte para NeoFS**: Sistema de archivos nativo

Los demás servicios se implementan como módulos separados.

## Próximos Pasos

1. Implementar gestor de memoria (GDT, paginación)
2. Implementar manejo de interrupciones (IDT)
3. Implementar planificador de procesos
4. Implementar sistema IPC
5. Implementar Module Manager
6. Implementar syscalls básicas

## Referencias

- [Documentación de NeoOS](../../docs/)
- [OSDev Wiki](https://wiki.osdev.org/)
- [Multiboot Specification](https://www.gnu.org/software/grub/manual/multiboot/)
