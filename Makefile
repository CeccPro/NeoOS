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
	@echo "  make commit - Añade, commitea y pushea los cambios a Git"

commit:
	@tmperr=$$(mktemp); success=1; \
	git add . 2>"$$tmperr" >/dev/null || true; \
	if [ -s "$$tmperr" ]; then echo "git add error:"; cat "$$tmperr"; success=0; fi; \
	git commit -m "Actualización de NeoOS" 2>"$$tmperr" >/dev/null || true; \
	if [ -s "$$tmperr" ]; then echo "git commit error:"; cat "$$tmperr"; success=0; fi; \
	git push 2>"$$tmperr" >/dev/null || true; \
	if [ -s "$$tmperr" ]; then echo "git push error:"; cat "$$tmperr"; success=0; fi; \
	rm -f "$$tmperr"; \
	if [ $$success -eq 1 ]; then echo "Success: cambios añadidos, commiteados y pusheados sin errores."; fi