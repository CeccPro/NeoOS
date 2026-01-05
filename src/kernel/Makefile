# NeoOS Kernel Makefile
# Compilación del kernel para arquitectura x86

.PHONY: all img run clean info check debug

# Herramientas de compilación
# Intentar usar cross-compiler, si no está disponible usar el del sistema
ifeq ($(shell which i686-elf-gcc 2>/dev/null),)
    # Cross-compiler no disponible, usar compilador del sistema
    AS = as --32
    CC = gcc -m32
    LD = gcc -m32
else
    # Usar cross-compiler
    AS = i686-elf-as
    CC = i686-elf-gcc
    LD = i686-elf-gcc
endif

# Directorios
ARCH = x86
ARCH_DIR = arch/$(ARCH)
BUILD_DIR = ../../build
OBJ_DIR = $(BUILD_DIR)/obj
ISO_DIR = $(BUILD_DIR)/iso
BOOT_DIR = $(ISO_DIR)/boot
GRUB_DIR = $(BOOT_DIR)/grub

# Flags de compilación
CFLAGS = -std=gnu99 -ffreestanding -O2 -Wall -Wextra -Ilib/include -Idrivers/include -Imemory/include -fno-pie -fno-stack-protector
LDFLAGS = -ffreestanding -O2 -nostdlib -lgcc -no-pie -z noexecstack
ASFLAGS = --noexecstack

# Archivos fuente
ASM_SOURCES = $(ARCH_DIR)/boot/kmain.S
C_SOURCES = core/src/kmain.c \
			core/src/error.c \
			core/src/kconfig.c \
            memory/src/memory.c \
            memory/src/pmm.c \
            memory/src/vmm.c \
            memory/src/heap.c \
            drivers/src/vga.c \
            lib/src/string.c

# Archivos objeto (en el directorio build)
ASM_OBJECTS = $(patsubst %.S,$(OBJ_DIR)/%.o,$(ASM_SOURCES))
C_OBJECTS = $(patsubst %.c,$(OBJ_DIR)/%.o,$(C_SOURCES))
OBJECTS = $(ASM_OBJECTS) $(C_OBJECTS)

# Kernel ELF (usado por GRUB con Multiboot)
KERNEL = neoos

# Target principal
all: $(BUILD_DIR)/$(KERNEL)

# Compilar archivos ensamblador
$(OBJ_DIR)/%.o: %.S
	@echo "AS  $<"
	@mkdir -p $(dir $@)
	@$(AS) $(ASFLAGS) $< -o $@

# Compilar archivos C
$(OBJ_DIR)/%.o: %.c
	@echo "CC  $<"
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@

# Linkear el kernel (generar ELF para GRUB Multiboot)
$(BUILD_DIR)/$(KERNEL): $(OBJECTS) linker.ld
	@echo "LD  $@"
	@mkdir -p $(BUILD_DIR)
	@$(LD) -T linker.ld -o $@ $(LDFLAGS) $(OBJECTS)
	@echo "Kernel compilado exitosamente: $@"
	@ls -lh $@ | awk '{print "Tamaño del kernel:", $$5}'

# Crear imagen de disco con particiones
img: $(BUILD_DIR)/$(KERNEL)
	@echo "Creando imagen de disco con particiones..."
	@bash ../../scripts/create_disk_image.sh
	@echo "Imagen de disco creada: $(BUILD_DIR)/neoos.img"

# Ejecutar en QEMU (desde imagen de disco)
run:
	@echo "Ejecutando NeoOS en QEMU desde imagen de disco..."
	@qemu-system-i386 -drive file=$(BUILD_DIR)/neoos.img,format=raw -m 512M

# Ejecutar en QEMU con debug
debug:
	@echo "Ejecutando NeoOS en QEMU con debug..."
	@qemu-system-i386 -drive file=$(BUILD_DIR)/neoos.img,format=raw -m 512M -s -S

# Limpiar archivos de compilación
clean:
	@echo "Limpiando archivos de compilación..."
	@rm -rf $(BUILD_DIR)/*
	@echo "Limpieza completada."

# Información de compilación
info:
	@echo "NeoOS Kernel Build System"
	@echo "========================="
	@echo "Arquitectura: $(ARCH)"
	@echo "Compilador C: $(CC)"
	@echo "Ensamblador: $(AS)"
	@echo "Linker: $(LD)"
	@echo ""
	@echo "Archivos fuente:"
	@echo "  ASM: $(ASM_SOURCES)"
	@echo "  C:   $(C_SOURCES)"
	@echo ""
	@echo "Targets disponibles:"
	@echo "  all         - Compila el kernel"
	@echo "  img         - Crea imagen de disco con particiones (EXT4 + NeoFS)"
	@echo "  run         - Ejecuta en QEMU desde imagen de disco"
	@echo "  run-terminal- Ejecuta en QEMU con terminal"
	@echo "  debug       - Ejecuta en QEMU con GDB"
	@echo "  clean       - Limpia archivos compilados"
	@echo "  info        - Muestra esta información"

# Verificar dependencias
check:
	@echo "Verificando dependencias..."
	@if which i686-elf-gcc > /dev/null 2>&1; then \
		echo "i686-elf-gcc encontrado (cross-compiler recomendado)"; \
	else \
		echo "i686-elf-gcc no encontrado, usando compilador del sistema"; \
		which gcc > /dev/null 2>&1 && echo "gcc encontrado" || echo "gcc no encontrado"; \
	fi
	@which grub-mkrescue > /dev/null 2>&1 && echo "grub-mkrescue encontrado" || echo "grub-mkrescue no encontrado"
	@which qemu-system-i386 > /dev/null 2>&1 && echo "qemu-system-i386 encontrado" || echo "qemu-system-i386 no encontrado"
	@echo "Verificación completada."
