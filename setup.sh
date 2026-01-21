#!/bin/bash
# Script de configuración del entorno de desarrollo para NeoOS

echo "======================================"
echo "  NeoOS - Setup del Entorno de Desarrollo"
echo "======================================"
echo ""

# Verificar que estamos en un sistema basado en Debian/Ubuntu
if ! command -v apt-get &> /dev/null; then
    echo "[!] Este script está diseñado para sistemas basados en Debian/Ubuntu"
    echo "Para otros sistemas, instala manualmente las siguientes herramientas:"
    echo "  - gcc"
    echo "  - nasm"
    echo "  - ld (binutils)"
    echo "  - make"
    echo "  - grub-mkrescue (grub2-common)"
    echo "  - xorriso"
    echo "  - qemu-system-i386"
    exit 1
fi

echo "Actualizando lista de paquetes..."
sudo apt-get update

echo ""
echo "Instalando herramientas de compilación..."

# Instalar herramientas necesarias
sudo apt-get install -y \
    build-essential \
    nasm \
    gcc \
    g++ \
    make \
    grub-pc-bin \
    grub-common \
    xorriso \
    mtools \
    qemu-system-x86

# Verificar instalación
echo ""
echo "======================================"
echo "Verificando instalación..."
echo "======================================"

errors=0

if ! command -v gcc &> /dev/null; then
    echo "[X] gcc no está instalado"
    errors=$((errors + 1))
else
    echo "[OK] gcc: $(gcc --version | head -n1)"
fi

if ! command -v nasm &> /dev/null; then
    echo "[X] nasm no está instalado"
    errors=$((errors + 1))
else
    echo "[OK] nasm: $(nasm -v)"
fi

if ! command -v ld &> /dev/null; then
    echo "[X] ld no está instalado"
    errors=$((errors + 1))
else
    echo "[OK] ld: $(ld --version | head -n1)"
fi

if ! command -v make &> /dev/null; then
    echo "[X] make no está instalado"
    errors=$((errors + 1))
else
    echo "[OK] make: $(make --version | head -n1)"
fi

if ! command -v grub-mkrescue &> /dev/null; then
    echo "[X] grub-mkrescue no está instalado"
    errors=$((errors + 1))
else
    echo "[OK] grub-mkrescue está instalado"
fi

if ! command -v xorriso &> /dev/null; then
    echo "[X] xorriso no está instalado"
    errors=$((errors + 1))
else
    echo "[OK] xorriso: $(xorriso --version 2>&1 | head -n1)"
fi

if ! command -v qemu-system-i386 &> /dev/null; then
    echo "[X] qemu-system-i386 no está instalado"
    errors=$((errors + 1))
else
    echo "[OK] qemu-system-i386: $(qemu-system-i386 --version | head -n1)"
fi

echo ""
echo "======================================"

if [ $errors -eq 0 ]; then
    echo "[OK] Configuración completada exitosamente!"
    echo ""
    echo "Para compilar NeoOS, ejecuta:"
    echo "  cd src/kernel"
    echo "  make all"
    echo ""
    echo "Para crear una ISO booteable:"
    echo "  make iso"
    echo ""
    echo "Para ejecutar en QEMU:"
    echo "  make run"
else
    echo "[X] Hubo $errors errores durante la configuración"
    echo "Por favor, revisa los errores anteriores e instala los paquetes faltantes."
fi

echo "======================================"
