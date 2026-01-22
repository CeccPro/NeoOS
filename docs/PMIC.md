# NeoOS - PMIC (Process-Module Intercomunicator)

El PMIC (Process-Module Intercomunicator) es el mecanismo de comunicación entre procesos y módulos del kernel en NeoOS. Es **diferente** del IPC tradicional que se usa para comunicación entre procesos.

## Estado de Implementación: COMPLETAMENTE FUNCIONAL

**Ubicación**: `src/kernel/core/src/module.c` y `src/kernel/core/include/module.h`

## Diferencias entre IPC y PMIC

| Característica | IPC (Inter-Process Communication) | PMIC (Process-Module Intercomunicator) |
|----------------|-----------------------------------|----------------------------------------|
| **Propósito** | Comunicación entre procesos | Comunicación entre procesos/kernel y módulos |
| **Identificadores** | PID (Process ID) | MID (Module ID) |
| **Colas** | Una por proceso (en PCB) | Una por módulo (en module_t) |
| **Modo principal** | Asíncrono (send/recv) | Síncrono (RPC con module_call) |
| **Ubicación** | `src/kernel/core/src/ipc.c` | `src/kernel/core/src/module.c` |

## Filosofía de Diseño

En un microkernel como NeoOS:
- **PMIC conecta procesos con servicios del kernel** implementados como módulos
- Los módulos son componentes que extienden la funcionalidad del kernel
- La comunicación es principalmente **síncrona (RPC)** para simplificar el diseño
- Cada módulo tiene su propia cola PMIC independiente

## Arquitectura del Sistema PMIC

### 1. Cola PMIC por Módulo

Cada módulo tiene una cola PMIC integrada en su estructura:

```c
typedef struct module {
    mid_t mid;                          // Module ID
    char name[MODULE_NAME_MAX];         // Nombre del módulo
    module_state_t state;               // Estado actual
    module_entry_t* entry;              // Punto de entrada del módulo
    
    // Cola PMIC del módulo
    ipc_queue_t ipc_queue;              // Reutiliza la estructura de IPC
    
    // ...
} module_t;
```

**Nota**: Aunque reutiliza `ipc_queue_t` por conveniencia de implementación, es una cola PMIC separada y no se mezcla con IPC de procesos.

### 2. Handler de Mensajes

Cada módulo implementa un handler para procesar mensajes PMIC:

```c
typedef struct {
    int (*init)(void);
    int (*cleanup)(void);
    int (*start)(void);
    int (*stop)(void);
    int (*update)(void);
    int (*handle_message)(const void* msg, size_t size, 
                         void* response, size_t* response_size);  // Handler PMIC
} module_entry_t;
```

## API del Sistema PMIC

### Envío de Mensajes Asíncronos

```c
int module_send(mid_t mid, const void* msg, size_t size);
```

**Parámetros**:
- `mid`: Module ID del destinatario
- `msg`: Puntero al buffer con el mensaje
- `size`: Tamaño del mensaje en bytes

**Retorna**:
- `E_OK`: Mensaje enviado correctamente
- `E_INVAL`: MID inválido o mensaje NULL
- `E_BUSY`: Cola del módulo llena
- `E_NOENT`: Módulo no existe

**Funcionamiento**:
1. Valida parámetros (mid, msg no NULL, size <= 4KB)
2. Busca el módulo por MID
3. Verifica que el módulo esté en estado RUNNING
4. Añade el mensaje a la cola PMIC del módulo
5. Retorna código de éxito/error

### Llamadas RPC Síncronas

```c
int module_call(mid_t mid, const void* request, size_t request_size, 
                void* response, size_t* response_size);
```

**Parámetros**:
- `mid`: Module ID del destinatario
- `request`: Buffer con la petición
- `request_size`: Tamaño de la petición
- `response`: Buffer para la respuesta
- `response_size`: Tamaño del buffer de respuesta (entrada/salida)

**Retorna**:
- `E_OK`: Llamada exitosa
- `E_INVAL`: Parámetros inválidos
- `E_NOENT`: Módulo no existe
- `E_NOT_IMPL`: Módulo no tiene handler

**Funcionamiento**:
1. Valida parámetros
2. Busca el módulo por MID
3. Verifica que el módulo esté en estado RUNNING
4. Llama directamente al `handle_message` del módulo
5. El módulo procesa la petición y genera la respuesta
6. Retorna el resultado

**Ejemplo de uso**:

```c
// Petición al módulo early_neofs
early_neofs_ipc_request_t req = {
    .command = EARLY_NEOFS_CMD_OPEN,
    .path = "/test/file.txt",
    .flags = EARLY_NEOFS_O_RDONLY
};

early_neofs_ipc_response_t resp;
size_t resp_size = sizeof(resp);

int result = module_call(neofs_mid, &req, sizeof(req), &resp, &resp_size);
if (result == E_OK && resp.result >= 0) {
    int fd = resp.result;
    // Usar el file descriptor...
}
```

### Envío por Nombre

```c
int module_send_by_name(const char* name, const void* msg, size_t size);
```

Alias conveniente que busca el MID por nombre y llama a `module_send()`.

### Procesamiento de Mensajes en Cola

```c
int module_process_messages(mid_t mid);
```

Procesa todos los mensajes pendientes en la cola PMIC de un módulo. Útil para módulos que manejan mensajes de forma asíncrona.

## Syscalls PMIC

Las syscalls del kernel exponen PMIC a modo usuario:

```c
// sys_modsend: Syscall 21 - Envío asíncrono
int sys_modsend(mid_t mid, const void* msg, size_t size);

// sys_modsend_name: Syscall 22 - Envío por nombre
int sys_modsend_name(const char* name, const void* msg, size_t size);

// sys_modcall: Syscall 23 - RPC síncrono
int sys_modcall(mid_t mid, const void* request, size_t request_size,
                void* response, size_t* response_size);

// sys_modgetid: Syscall 24 - Obtener MID por nombre
mid_t sys_modgetid(const char* name);
```

Ver [Syscalls.md](./Syscalls.md) para más detalles.

## Demo Funcional: Early NeoFS

NeoOS incluye una demo completa de PMIC con el módulo Early NeoFS implementada en `kernel_main()`:

```c
// Obtener MID del módulo early_neofs
mid_t neofs_mid = module_get_id("early_neofs");

// Crear un directorio via PMIC
early_neofs_ipc_request_t req = {
    .command = EARLY_NEOFS_CMD_MKDIR,
    .path = "/test",
    .permissions = 0755
};

early_neofs_ipc_response_t resp;
size_t resp_size = sizeof(resp);

int result = module_call(neofs_mid, &req, sizeof(req), &resp, &resp_size);
if (result == E_OK && resp.result == E_OK) {
    vga_write("[OK] Directorio /test creado via PMIC\n");
}
```

El test completo demuestra:
- Creación de directorios y archivos
- Apertura, lectura y escritura de archivos
- Obtención de información (stat)
- Listado de directorios (readdir)
- Eliminación de archivos

## Integración con el Module Manager

El PMIC está integrado con el Module Manager:

1. **Registro de módulos**: Al registrar un módulo, se inicializa su cola PMIC
2. **Estado del módulo**: Solo los módulos en estado `MODULE_STATE_RUNNING` pueden recibir mensajes
3. **Handler obligatorio**: Los módulos deben implementar `handle_message` para responder a PMIC

## Comparación con Otros Sistemas

| Sistema | Comunicación con Módulos |
|---------|--------------------------|
| **NeoOS** | PMIC dedicado con RPC síncrono |
| Linux | Llamadas de función directas (monolítico) |
| Minix 3 | IPC compartido entre procesos y drivers |
| seL4 | Endpoints uniformes para todo |

## Ventajas del PMIC de NeoOS

- **Separación clara**: IPC para procesos, PMIC para módulos
- **RPC síncrono**: Simplifica el diseño de módulos (no requiere estado complejo)
- **Tipado fuerte**: Cada módulo define sus propias estructuras de petición/respuesta
- **Extensible**: Nuevos módulos se añaden sin modificar el kernel

## Limitaciones Actuales

- **Solo síncrono para module_call**: No hay modo asíncrono completo (aunque existe module_send)
- **Sin Timeout**: Las llamadas RPC esperan indefinidamente
- **Sin Prioridad**: Los mensajes se procesan en orden FIFO
- **Colas Fijas**: Máximo 32 mensajes por módulo

## Futuras Mejoras

1. **Timeout en RPC**: Límite de tiempo para evitar bloqueos
2. **Callbacks**: Notificaciones asíncronas del módulo al proceso
3. **Priorización**: Mensajes urgentes con prioridad alta
4. **Shared Memory**: Transferencias grandes sin copia
5. **Broadcast**: Envío a múltiples módulos simultáneamente

## Casos de Uso en NeoOS

### 1. Sistema de Archivos (Early NeoFS)

```c
// Crear archivo via PMIC
early_neofs_ipc_request_t req = {
    .command = EARLY_NEOFS_CMD_CREATE,
    .path = "/data/config.txt",
    .permissions = 0644
};

early_neofs_ipc_response_t resp;
size_t resp_size = sizeof(resp);
module_call(neofs_mid, &req, sizeof(req), &resp, &resp_size);
```

### 2. Ramdisk

```c
// Leer información del ramdisk via PMIC
ramdisk_request_t req = { .command = RAMDISK_CMD_GETINFO };
ramdisk_response_t resp;
size_t resp_size = sizeof(resp);

module_call(ramdisk_mid, &req, sizeof(req), &resp, &resp_size);
```

### 3. Drivers de Hardware (Futuro)

```c
// Configurar driver de red via PMIC
network_request_t req = {
    .command = NET_CMD_CONFIGURE,
    .ip_address = 0xC0A80001,  // 192.168.0.1
    .netmask = 0xFFFFFF00
};

network_response_t resp;
size_t resp_size = sizeof(resp);
module_call(network_mid, &req, sizeof(req), &resp, &resp_size);
```

## Conclusión

El PMIC es el puente entre procesos y módulos en NeoOS, permitiendo una arquitectura de microkernel limpia donde:
- **IPC** maneja la comunicación entre procesos (asíncrona, basada en colas)
- **PMIC** maneja la comunicación con módulos del kernel (síncrona, basada en RPC)

Esta separación clara hace que NeoOS sea más mantenible y seguro, con responsabilidades bien definidas para cada mecanismo de comunicación.
