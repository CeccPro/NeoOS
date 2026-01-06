Mira wn:

#!/bin/bash
# Script para crear imagen de disco con particiones para NeoOS
# Partición 1: EXT4 (boot) con GRUB y kernel
# Partición 2: NeoFS (sistema de archivos principal)

set -e

# Colores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Directorios y archivos
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_ROOT/build"
KERNEL_ELF="$BUILD_DIR/neoos"
IMG_FILE="$BUILD_DIR/neoos.img"
MOUNT_POINT="/tmp/neoos_mount"

# Tamaños de las particiones (en MB)
BOOT_SIZE=100      # 100MB para partición de boot (EXT4)
NEOFS_SIZE=400     # 400MB para partición NeoFS
TOTAL_SIZE=$((BOOT_SIZE + NEOFS_SIZE))

echo -e "${GREEN}=== Creando imagen de disco para NeoOS ===${NC}"

# Verificar que el kernel existe
if [ ! -f "$KERNEL_ELF" ]; then
    echo -e "${RED}Error: No se encontró el kernel en $KERNEL_ELF${NC}"
    exit 1
fi

# Si la imagen ya existe, solo actualizar el kernel
if [ -f "$IMG_FILE" ]; then
    echo -e "${YELLOW}Imagen existente detectada. Actualizando solo el kernel...${NC}"
    
    # Configurar dispositivo loop
    LOOP_DEV=$(sudo losetup -f --show -P "$IMG_FILE")
    sleep 1
    
    # Montar partición de boot
    sudo mkdir -p "$MOUNT_POINT"
    sudo mount "${LOOP_DEV}p1" "$MOUNT_POINT" 2>/dev/null
    
    if [ $? -eq 0 ]; then
        # Reemplazar kernel
        sudo cp "$KERNEL_ELF" "$MOUNT_POINT/boot/neoos"
        echo -e "${GREEN}✓ Kernel actualizado: $(ls -lh "$KERNEL_ELF" | awk '{print $5}')${NC}"
        
        # Desmontar
        sudo umount "$MOUNT_POINT"
        sudo rmdir "$MOUNT_POINT" 2>/dev/null || true
        sudo losetup -d "$LOOP_DEV"
        
        echo -e "${GREEN}✓ Actualización completada${NC}"
        exit 0
    else
        echo -e "${YELLOW}No se pudo montar la imagen existente. Recreando...${NC}"
        sudo losetup -d "$LOOP_DEV"
    fi
fi

echo -e "${YELLOW}[1/8]${NC} Creando imagen de disco de ${TOTAL_SIZE}MB..."
dd if=/dev/zero of="$IMG_FILE" bs=1M count=$TOTAL_SIZE status=progress 2>/dev/null || dd if=/dev/zero of="$IMG_FILE" bs=1M count=$TOTAL_SIZE

echo -e "${YELLOW}[2/8]${NC} Creando tabla de particiones..."
# Crear tabla de particiones MBR (usar rutas completas para compatibilidad)
/usr/sbin/parted -s "$IMG_FILE" mklabel msdos

# Partición 1: Boot (EXT4) - Bootable
/usr/sbin/parted -s "$IMG_FILE" mkpart primary ext4 1MiB ${BOOT_SIZE}MiB
/usr/sbin/parted -s "$IMG_FILE" set 1 boot on

# Partición 2: NeoFS (Sin formato aún)
/usr/sbin/parted -s "$IMG_FILE" mkpart primary ${BOOT_SIZE}MiB 100%

echo -e "${YELLOW}[3/8]${NC} Configurando dispositivos loop..."
# Crear dispositivo loop para la imagen
LOOP_DEV=$(sudo losetup -f --show -P "$IMG_FILE")
echo "Dispositivo loop: $LOOP_DEV"

# Esperar a que los dispositivos de partición estén disponibles
sleep 1

# Verificar particiones
if [ ! -e "${LOOP_DEV}p1" ]; then
    echo -e "${RED}Error: No se crearon las particiones correctamente${NC}"
    sudo losetup -d "$LOOP_DEV"
    exit 1
fi

echo -e "${YELLOW}[4/8]${NC} Formateando partición de boot como EXT4..."
sudo mkfs.ext4 -F "${LOOP_DEV}p1" -L "NEOOS_BOOT" >/dev/null 2>&1

echo -e "${YELLOW}[5/8]${NC} Montando partición de boot..."
sudo mkdir -p "$MOUNT_POINT"
sudo mount "${LOOP_DEV}p1" "$MOUNT_POINT"

echo -e "${YELLOW}[6/8]${NC} Instalando GRUB..."
# Crear estructura de directorios
sudo mkdir -p "$MOUNT_POINT/boot/grub"

# Copiar kernel ELF (GRUB necesita el formato ELF para Multiboot)
sudo cp "$KERNEL_ELF" "$MOUNT_POINT/boot/neoos"
echo "Kernel copiado: $(basename "$KERNEL_ELF") - $(ls -lh "$KERNEL_ELF" | awk '{print $5}')"
# Crear configuración de GRUB
sudo cp "$PROJECT_ROOT/src/kernel/grub.cfg" "$MOUNT_POINT/boot/grub/grub.cfg"

# Instalar GRUB en el MBR
sudo grub-install --target=i386-pc --boot-directory="$MOUNT_POINT/boot" "$LOOP_DEV" 2>&1 | grep -v "Installation finished"

echo -e "${YELLOW}[7/8]${NC} Preparando partición NeoFS..."
# Inicializar la partición NeoFS con un header básico (puedes modificar esto más adelante)
# Por ahora, solo ponemos un identificador
echo "NEOFS_V1" | sudo dd of="${LOOP_DEV}p2" bs=512 count=1 status=none 2>/dev/null

echo -e "${YELLOW}[8/8]${NC} Limpiando..."
sudo umount "$MOUNT_POINT"
sudo rmdir "$MOUNT_POINT" 2>/dev/null || true
sudo losetup -d "$LOOP_DEV"

echo -e "${GREEN}✓ Imagen de disco creada exitosamente: $IMG_FILE${NC}"
echo -e "${GREEN}  Partición 1: EXT4 (${BOOT_SIZE}MB) - Boot con GRUB y kernel${NC}"
echo -e "${GREEN}  Partición 2: NeoFS (${NEOFS_SIZE}MB) - Sistema de archivos principal${NC}"
echo ""
echo -e "Información de la imagen:"
ls -lh "$IMG_FILE" | awk '{print "  Tamaño:", $5}'
echo ""
echo -e "Para ejecutar: ${YELLOW}make run${NC}"