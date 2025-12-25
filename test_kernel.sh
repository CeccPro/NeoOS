#!/bin/bash
# Script para probar el kernel de NeoOS

echo "Matando procesos QEMU existentes..."
killall -9 qemu-system-i386 2>/dev/null || true
sleep 1

echo "Ejecutando NeoOS en QEMU..."
echo "Presiona Ctrl+C para salir"
echo ""

qemu-system-i386 \
    -cdrom build/neoos.iso \
    -serial stdio \
    -m 256M \
    -display gtk

echo ""
echo "QEMU terminado."
