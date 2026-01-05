# NeoOS - Boot Process
El proceso de arranque de NeoOS es una secuencia crítica que inicia el sistema operativo desde un estado apagado hasta que está completamente operativo y listo para ejecutar aplicaciones. Este proceso involucra varias etapas, desde la carga del bootloader hasta la inicialización del kernel.

## Etapas del Proceso de Arranque
1. **Bootloader (GRUB)**: El bootloader es el primer código que se ejecuta cuando la máquina se enciende. Su función principal es cargar el kernel de NeoOS en la memoria. El bootloader por defecto que utiliza NeoOS es GRUB (GRand Unified Bootloader), compatible con el estándar Multiboot.

2. **Carga del Kernel**: Una vez que el bootloader está en ejecución, localiza el binario del kernel de NeoOS (`build/neoos`) y lo carga en la memoria en la dirección `0x00100000` (1MB). El bootloader pasa al kernel:
   - Magic number de Multiboot (`0x2BADB002`)
   - Puntero a la estructura `multiboot_info_t` con información del sistema
   - Parámetros de línea de comandos (cmdline) que pueden incluir `--debug` y `--verbose`

3. **Entry Point (_start)**: El control pasa al punto de entrada en ensamblador (`src/kernel/arch/x86/boot/kmain.S`) que:
   - Configura la pila del kernel (16KB reservados en la sección `.bss`)
   - Preserva los registros EAX (magic number) y EBX (puntero a multiboot_info)
   - Llama a `kernel_main()` pasándole estos parámetros

4. **Inicialización del Kernel (kernel_main)**: En `src/kernel/core/src/kmain.c`, el kernel ejecuta la siguiente secuencia:
   - **Inicialización VGA**: Configura el driver de terminal VGA en modo texto 80x25
   - **Parseo de CMDLINE**: Lee los parámetros `--debug` y `--verbose` de multiboot
   - **Inicialización de kconfig**: Establece las variables globales de configuración del kernel
   - **Verificación Multiboot**: Valida el magic number; si falla, detiene el kernel
   - **Banner de Bienvenida**: Muestra la versión del kernel si verbose está activo
   - **Información de Memoria**: Muestra información básica de memoria si debug/verbose están activos
   - **Inicialización del Memory Manager**: Inicializa PMM, VMM y Heap del kernel

5. **Estado Actual**: Una vez completada la inicialización del Memory Manager, el kernel entra en un bucle infinito con instrucciones `hlt`. Los siguientes subsistemas están pendientes de implementación:
   - Planificador de procesos
   - Sistema de archivos (NeoFS)
   - IPC (Comunicación entre procesos)
   - Module Manager
   - Transición a modo usuario

## Grub.cfg Actual
El archivo de configuración de GRUB actual (`src/kernel/grub.cfg`) es:
```cfg
set timeout=5
set default=0

menuentry "NeoOS" {
	multiboot /boot/neoos
	boot
}
```

**Nota**: Actualmente no se utiliza InitImage ni se pasan parámetros adicionales. Los parámetros `--debug` y `--verbose` pueden agregarse en la línea `multiboot` si se desea:
```cfg
multiboot /boot/neoos --debug --verbose
```

## Notas Adicionales
- El kernel actualmente se detiene después de inicializar el Memory Manager y entra en un bucle infinito (`hlt`).
- La estructura de directorios actual del proyecto es diferente a la mencionada en documentación antigua. El kernel se encuentra en `src/kernel/` con subdirectorios para cada componente.
- El bootloader debe estar correctamente configurado para localizar el binario `neoos` en `/boot/`.
- Durante el proceso de arranque, se muestran mensajes de diagnóstico en la consola VGA si los modos debug o verbose están activos.
- El proceso de arranque puede variar ligeramente dependiendo del hardware específico y la configuración del sistema.

## Véase también
- [GRUB Documentation](https://www.gnu.org/software/grub/manual/grub/grub.html)
- [Kernel Initialization](./Kernel%20Initialization.md)
- [Memory Manager](./Memory%20Manager.md)