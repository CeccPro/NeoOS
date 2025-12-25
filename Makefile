# Makefile principal de NeoOS

.PHONY: all kernel iso run clean commit help

all: kernel

kernel:
	@echo "Compilando el kernel..."
	@cd src/kernel && $(MAKE)

iso: kernel
	@echo "Creando imagen ISO..."
	@cd src/kernel && $(MAKE) iso

run: iso
	@echo "Ejecutando NeoOS en QEMU..."
	@cd src/kernel && $(MAKE) run

clean:
	@echo "Limpiando archivos de compilación..."
	@cd src/kernel && $(MAKE) clean
	@echo "Limpieza completada."

help:
	@echo "NeoOS - Sistema Operativo"
	@echo ""
	@echo "Comandos disponibles:"
	@echo "  make all    - Compila el kernel"
	@echo "  make kernel - Compila el kernel"
	@echo "  make iso    - Crea una imagen ISO booteable"
	@echo "  make run    - Compila y ejecuta en QEMU"
	@echo "  make clean  - Limpia archivos de compilación"
	@echo "  make help   - Muestra esta ayuda"

commit:
	@git add .
	@git commit -m "Actualización de NeoOS"
	@git push
