# Makefile principal de NeoOS
.PHONY: all kernel img run clean commit help comp clean-all

all: kernel

kernel:
	@echo "Compilando el kernel..."
	@cd src/kernel && $(MAKE)

img: kernel
	@echo "Creando imagen de disco..."
	@cd src/kernel && $(MAKE) img

run:
	@echo "Ejecutando NeoOS en QEMU..."
	@cd src/kernel && $(MAKE) run

clean:
	@echo "Limpiando archivos de compilación..."
	@cd src/kernel && $(MAKE) clean
	@echo "Limpieza completada."

clean-all:
	@echo "Eliminando todos los archivos de compilación e imagen de disco..."
	@cd src/kernel && $(MAKE) clean-all
	@echo "Eliminación completada."

help:
	@echo "NeoOS - Sistema Operativo"
	@echo ""
	@echo "Comandos disponibles:"
	@echo "  make all    - Compila el kernel"
	@echo "  make kernel - Compila el kernel"
	@echo "  make img    - Crea una imagen de disco con particiones (EXT4 + NeoFS)"
	@echo "  make iso    - Crea una imagen ISO booteable (legacy)"
	@echo "  make run    - Compila y ejecuta en QEMU desde imagen de disco"
	@echo "  make clean  - Limpia archivos de compilación"
	@echo "  make help   - Muestra esta ayuda"
	@echo "  make commit - Añade, commitea y pushea los cambios a Git"

commit:
	@git add .>/dev/null 2>&1 || echo "No hay archivos para añadir."
	@git commit -m "Actualización de NeoOS">/dev/null 2>&1 || echo "No hay cambios para commitear."
	@git push>/dev/null 2>&1 || echo "No se pudo pushear los cambios."

comp: clean all img

check:
	@cd src/kernel && $(MAKE) check