# NeoOS - RAM Disk Module

El módulo RAM Disk proporciona un disco virtual en memoria RAM que puede ser utilizado para almacenar datos temporales de forma rápida y eficiente.

## Características

- **Tamaño**: 1 MB (configurable en `RAMDISK_SIZE`)
- **Sectores**: 2048 sectores de 512 bytes cada uno
- **Operaciones**: Lectura y escritura de sectores completos
- **Tipo**: Módulo estático (compilado con el kernel)

## Arquitectura

El ramdisk es un módulo que se carga durante la inicialización del kernel. Proporciona una interfaz simple para leer y escribir datos en bloques de 512 bytes (sectores).

```
┌─────────────────────────────────────┐
│         Kernel / Filesystem         │
└──────────────┬──────────────────────┘
               │
               ▼
┌─────────────────────────────────────┐
│        Ramdisk Module API           │
│  - ramdisk_read()                   │
│  - ramdisk_write()                  │
│  - ramdisk_format()                 │
└──────────────┬──────────────────────┘
               │
               ▼
┌─────────────────────────────────────┐
│     RAM Buffer (1MB)                │
│     2048 sectores × 512 bytes       │
└─────────────────────────────────────┘
```

## API del Módulo

### Funciones Públicas

#### `int ramdisk_read(uint32_t sector, uint32_t count, void* buffer)`
Lee datos del ramdisk.

**Parámetros:**
- `sector`: Sector inicial (0-2047)
- `count`: Número de sectores a leer
- `buffer`: Buffer donde se almacenarán los datos (debe tener al menos `count * 512` bytes)

**Retorna:** Número de sectores leídos, o código de error negativo

**Ejemplo:**
```c
uint8_t buffer[512];
int result = ramdisk_read(0, 1, buffer);
if (result > 0) {
    printf("Leído %d sector(es)\n", result);
}
```

#### `int ramdisk_write(uint32_t sector, uint32_t count, const void* buffer)`
Escribe datos en el ramdisk.

**Parámetros:**
- `sector`: Sector inicial (0-2047)
- `count`: Número de sectores a escribir
- `buffer`: Buffer con los datos a escribir (debe tener al menos `count * 512` bytes)

**Retorna:** Número de sectores escritos, o código de error negativo

**Ejemplo:**
```c
uint8_t data[512] = "Hola, ramdisk!";
int result = ramdisk_write(0, 1, data);
if (result > 0) {
    printf("Escrito %d sector(es)\n", result);
}
```

#### `int ramdisk_format(void)`
Formatea el ramdisk (lo llena de ceros).

**Retorna:** `E_OK` si éxito, código de error en caso contrario

**Ejemplo:**
```c
if (ramdisk_format() == E_OK) {
    printf("Ramdisk formateado\n");
}
```

#### `uint32_t ramdisk_get_size(void)`
Obtiene el tamaño del ramdisk en bytes.

**Retorna:** Tamaño en bytes (1048576 = 1MB)

#### `uint32_t ramdisk_get_sectors(void)`
Obtiene el número de sectores del ramdisk.

**Retorna:** Número de sectores (2048)

## Ciclo de Vida del Módulo

El módulo ramdisk sigue el ciclo de vida estándar de los módulos de NeoOS:

1. **Registro**: Se registra durante la inicialización del kernel con `module_register_static()`
2. **Inicialización**: `module_init()` → `ramdisk_init()` asigna memoria para el buffer
3. **Inicio**: `module_start()` marca el módulo como listo para usar
4. **Ejecución**: Las funciones `ramdisk_read()` y `ramdisk_write()` están disponibles
5. **Detención**: `module_stop()` detiene el módulo (sin liberar memoria)
6. **Limpieza**: `module_cleanup()` → `ramdisk_cleanup()` libera la memoria

## Uso desde Kmain

El ramdisk se carga automáticamente durante el arranque del kernel:

```c
// En kmain.c
module_entry_t* ramdisk_entry = (module_entry_t*)ramdisk_get_entry();
mid_t ramdisk_mid = module_register_static("ramdisk", ramdisk_entry);
module_start(ramdisk_mid);
```

## Uso desde Syscalls

Una vez que el módulo está cargado, se puede acceder a través de syscalls (cuando se implementen):

```c
// Futuro: syscall para acceder al ramdisk
sys_ramdisk_read(sector, count, buffer);
sys_ramdisk_write(sector, count, buffer);
```

## Configuración

El tamaño del ramdisk se puede modificar en el archivo `ramdisk.h`:

```c
#define RAMDISK_SIZE (1024 * 1024)  // 1MB
#define RAMDISK_SECTOR_SIZE 512      // 512 bytes por sector
```

## Casos de Uso

1. **Sistema de archivos temporal**: Usar el ramdisk como almacenamiento temporal para archivos
2. **Cache**: Almacenar datos de caché para acceso rápido
3. **Datos de configuración**: Guardar configuraciones temporales
4. **Swap temporal**: Usar como memoria swap de emergencia
5. **Testing**: Probar operaciones de I/O sin acceso a disco real

## Limitaciones Actuales

- Tamaño fijo de 1MB
- No persistente (se pierde al reiniciar)
- No tiene sistema de archivos (solo sectores raw)
- Operaciones síncronas (bloquean hasta completar)

## Próximos Pasos

- [ ] Agregar soporte para tamaños dinámicos
- [ ] Implementar sistema de archivos sobre el ramdisk
- [ ] Agregar syscalls específicos para ramdisk
- [ ] Implementar compresión de datos
- [ ] Agregar estadísticas de uso
