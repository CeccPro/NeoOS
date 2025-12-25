# Makefile para el kernel de NeoOS

# Herramientas
AS = nasm
CC = gcc
LD = ld

# Flags
ASFLAGS = -f elf32
CFLAGS = -m32 -ffreestanding -nostdlib -fno-builtin -fno-stack-protector \
         -Wall -Wextra -O2 -Iinclude
LDFLAGS = -m elf_i386 -T linker.ld

# Directorios
BUILD_DIR = ../../build
KERNEL_BIN = $(BUILD_DIR)/neoos.bin

# Archivos fuente
ASM_SRC = boot/boot.asm
C_SRC = core/kernel_main.c \
        drivers/vga.c

# Archivos objeto
ASM_OBJ = $(BUILD_DIR)/boot.o
C_OBJ = $(C_SRC:%.c=$(BUILD_DIR)/%.o)

# Regla por defecto
all: $(KERNEL_BIN)

# Crear directorios necesarios
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)/core $(BUILD_DIR)/drivers

# Compilar ensamblador
$(BUILD_DIR)/boot.o: $(ASM_SRC) | $(BUILD_DIR)
	$(AS) $(ASFLAGS) $< -o $@

# Compilar C
$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Enlazar el kernel
$(KERNEL_BIN): $(ASM_OBJ) $(C_OBJ)
	$(LD) $(LDFLAGS) $^ -o $@
	@echo "Kernel compilado exitosamente: $(KERNEL_BIN)"

# Crear ISO booteable
iso: $(KERNEL_BIN)
	mkdir -p $(BUILD_DIR)/iso/boot/grub
	cp $(KERNEL_BIN) $(BUILD_DIR)/iso/boot/neoos.bin
	echo 'set timeout=0' > $(BUILD_DIR)/iso/boot/grub/grub.cfg
	echo 'set default=0' >> $(BUILD_DIR)/iso/boot/grub/grub.cfg
	echo '' >> $(BUILD_DIR)/iso/boot/grub/grub.cfg
	echo 'menuentry "NeoOS" {' >> $(BUILD_DIR)/iso/boot/grub/grub.cfg
	echo '	multiboot /boot/neoos.bin' >> $(BUILD_DIR)/iso/boot/grub/grub.cfg
	echo '	boot' >> $(BUILD_DIR)/iso/boot/grub/grub.cfg
	echo '}' >> $(BUILD_DIR)/iso/boot/grub/grub.cfg
	grub-mkrescue -o $(BUILD_DIR)/neoos.iso $(BUILD_DIR)/iso
	@echo "ISO creado exitosamente: $(BUILD_DIR)/neoos.iso"

# Ejecutar con QEMU
run: iso
	if [ -f $(BUILD_DIR)/neoos.iso ]; then \
		qemu-system-i386 -cdrom $(BUILD_DIR)/neoos.iso; \
	else \
		echo "Error: La imagen ISO no existe. Por favor, ejecuta 'make iso' primero."; \
		exit 1; \
	fi

# Limpiar archivos generados
clean:
	rm -rf $(BUILD_DIR)

.PHONY: all iso run clean
