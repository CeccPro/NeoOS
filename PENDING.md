# NeoOS - Pendientes de Implementación

Este documento lista todas las funcionalidades pendientes de implementar en NeoOS, organizadas por prioridad y módulo.

**Última actualización**: 20 de enero de 2026

---

## 🔴 Prioridad ALTA (Bloqueantes)

### Syscalls - Funcionalidad Básica Faltante

#### SYS_CALL (RPC síncrono)
- **Estado**: ❌ No implementado
- **Descripción**: Send + Recv atómico para llamadas RPC
- **Bloqueante**: No
- **Notas**: Útil para comunicación cliente-servidor

#### SYS_SIGNAL (Señales)
- **Estado**: ❌ No implementado
- **Descripción**: Sistema de señales básico (SIGKILL, SIGTERM, etc.)
- **Bloqueante**: Sí (necesario para kill())
- **Dependencias**: Necesita tabla de handlers en PCB

#### SYS_WAIT (Eventos)
- **Estado**: ❌ No implementado
- **Descripción**: Esperar eventos con event mask y timeout
- **Bloqueante**: Sí (necesario para waitpid())
- **Dependencias**: Sistema de eventos en scheduler

### Memory Management - Userspace

#### SYS_MAP (Mapeo de memoria)
- **Estado**: ❌ No implementado
- **Descripción**: Mapear páginas en espacio de direcciones del proceso
- **Bloqueante**: Sí (necesario para malloc en userspace)
- **Notas**: Backend VMM existe, falta wrapper para userspace
- **Implementación**:
  ```c
  // Validar dirección virtual
  // Asignar página física con pmm_alloc_page()
  // Mapear con vmm_map_page()
  ```

#### SYS_UNMAP (Desmapeo)
- **Estado**: ❌ No implementado
- **Descripción**: Desmapear región de memoria
- **Bloqueante**: Sí
- **Dependencias**: SYS_MAP

#### SYS_GRANT (Memoria compartida)
- **Estado**: ❌ No implementado
- **Descripción**: Compartir región de memoria con otro proceso
- **Bloqueante**: No (útil pero no crítico)
- **Notas**: Para SHM eficiente entre procesos

### Modo Usuario

#### Segmentos de Usuario en GDT
- **Estado**: ❌ No implementado
- **Descripción**: Agregar segmentos ring 3 a la GDT
- **Bloqueante**: Sí (necesario para modo usuario)
- **Archivo**: `src/kernel/core/src/gdt.c`

#### TSS (Task State Segment)
- **Estado**: ❌ No implementado
- **Descripción**: Configurar TSS para cambio de privilegios
- **Bloqueante**: Sí
- **Notas**: Necesario para int 0x80 desde ring 3

#### Cambio a Ring 3
- **Estado**: ❌ No implementado
- **Descripción**: Función para ejecutar código en modo usuario
- **Bloqueante**: Sí
- **Implementación**: iret con CS/SS de usuario

---

## 🟡 Prioridad MEDIA (Funcionalidad Core)

### Sistema de Información

#### SYS_GETINFO - INFO_UPTIME
- **Estado**: ⚠️ Parcialmente implementado
- **Descripción**: Retorna 0 (placeholder)
- **Necesita**: Contador de ticks en timer.c
- **Archivo**: `src/kernel/core/src/timer.c`

#### SYS_GETINFO - INFO_TIME
- **Estado**: ❌ No implementado
- **Descripción**: Timestamp actual
- **Necesita**: Driver RTC (Real-Time Clock)

### Libneo (Librería Userspace)

#### Wrapper Syscalls Básicos
- **Estado**: ❌ No iniciado
- **Descripción**: Crear libneo.so con wrappers de syscalls
- **Incluye**:
  - Wrapper amigable para cada syscall
  - Manejo de errores (errno)
  - Documentación de API

#### Funciones de Gestión de Procesos
- **Estado**: ❌ No iniciado
- **Funciones**:
  - `pid_t fork()` - Usando sys_thread_create
  - `int exec(const char *path)` - Carga de ELF
  - `pid_t run(const char *path, char **argv)` - fork + exec
  - `int kill(pid_t pid)` - Usando sys_signal
  - `int wait(int *status)` - Usando sys_wait

#### Gestión de Heap (malloc/free)
- **Estado**: ❌ No iniciado
- **Funciones**:
  - `void *malloc(size_t size)` - Usando sys_map
  - `void free(void *ptr)` - Usando sys_unmap
  - `void *sbrk(intptr_t increment)` - Gestión de heap
  - `void *calloc(size_t n, size_t size)`
  - `void *realloc(void *ptr, size_t size)`

### Servidores Userspace

#### VFS Server (Sistema de Archivos)
- **Estado**: ❌ No iniciado
- **Descripción**: Servidor de filesystem en userspace
- **Funciones**:
  - `open()`, `read()`, `write()`, `close()`
  - `mkdir()`, `rmdir()`, `unlink()`
  - `stat()`, `chdir()`, `getcwd()`
- **Comunicación**: IPC con aplicaciones

#### Process Server
- **Estado**: ❌ No iniciado
- **Descripción**: Gestor de procesos en userspace
- **Funciones**:
  - Carga de binarios ELF
  - Gestión de PIDs
  - Fork/exec avanzado
  - Árbol de procesos
- **Comunicación**: IPC

#### Device Manager
- **Estado**: ❌ No iniciado
- **Descripción**: Gestor de drivers en userspace
- **Funciones**:
  - Registro de drivers
  - Mapeo de IRQs (usando sys_wait)
  - Hot-plug de dispositivos
  - Acceso a puertos I/O (con permisos)

---

## 🟢 Prioridad BAJA (Mejoras y Optimizaciones)

### Drivers

#### Keyboard Driver
- **Estado**: ❌ No implementado
- **Archivo**: `src/kernel/drivers/keyboard.c`
- **Notas**: Debería moverse a userspace eventualmente

#### Mouse Driver (PS/2)
- **Estado**: ❌ No implementado
- **Prioridad**: Baja

#### RTC (Real-Time Clock)
- **Estado**: ❌ No implementado
- **Necesario para**: sys_getinfo(INFO_TIME)

### Filesystem

#### RAM Disk
- **Estado**: ❌ No implementado
- **Descripción**: Disco en memoria para initrd
- **Prioridad**: Media
- **Notas**: Útil para boot temprano

#### NeoFS (Filesystem Nativo)
- **Estado**: ❌ No diseñado
- **Descripción**: Filesystem propio de NeoOS
- **Prioridad**: Baja
- **Alternativa**: Usar ext2/FAT32 inicialmente

### Optimizaciones

#### Copy-on-Write (COW) para fork()
- **Estado**: ❌ No implementado
- **Descripción**: No copiar toda la memoria en fork()
- **Beneficio**: Performance

#### Lazy Allocation de Memoria
- **Estado**: ❌ No implementado
- **Descripción**: No asignar páginas hasta primer acceso

#### Cache de Pages
- **Estado**: ❌ No implementado
- **Descripción**: Pool de páginas pre-asignadas

---

## 📊 Estado Actual del Proyecto

### ✅ Completado

- [x] Memory Manager (PMM, VMM, Heap)
- [x] Sistema de Interrupciones (GDT, IDT, PIC, PIT)
- [x] Scheduler Multitarea (Round Robin con prioridades)
- [x] IPC (Send, Recv, Free)
- [x] Syscall Dispatcher (int 0x80)
- [x] Syscalls Implementadas:
  - [x] SYS_SEND
  - [x] SYS_RECV
  - [x] SYS_THREAD_CREATE
  - [x] SYS_THREAD_EXIT
  - [x] SYS_YIELD
  - [x] SYS_SETPRIORITY
  - [x] SYS_GETPRIORITY
  - [x] SYS_GETINFO (PID, MEMORY)
  - [x] SYS_DEBUG

### ⏳ En Progreso

- [ ] Transición a modo usuario
- [ ] Sistema de señales básico
- [ ] Gestión de memoria para userspace

### ❌ Pendiente

- [ ] Syscalls faltantes (SYS_CALL, SYS_SIGNAL, SYS_WAIT, SYS_MAP, etc.)
- [ ] Libneo (librería userspace)
- [ ] Servidores userspace (VFS, Process, Device)
- [ ] Drivers adicionales
- [ ] Filesystem

---

## 🎯 Roadmap Sugerido

### Fase 1: Modo Usuario Funcional
1. ✅ ~~Syscall dispatcher~~
2. Agregar segmentos ring 3 a GDT
3. Implementar TSS
4. Crear proceso de prueba en ring 3
5. Implementar SYS_MAP/SYS_UNMAP
6. Implementar SYS_SIGNAL básico

### Fase 2: Libneo Básica
1. Crear estructura de proyecto libneo
2. Implementar wrappers de syscalls
3. Implementar malloc/free usando sys_map
4. Implementar fork() básico
5. Testing de libneo

### Fase 3: Primer Servidor (VFS)
1. Diseñar protocolo IPC para VFS
2. Implementar VFS server básico (RAM disk)
3. Adaptar libneo para usar VFS
4. Testing de open/read/write

### Fase 4: Process Server
1. Implementar cargador ELF
2. Implementar exec()
3. Gestión avanzada de procesos
4. Testing completo

---

## 📝 Notas de Desarrollo

### Consideraciones de Diseño

- **Filosofía**: Mantener el kernel minimalista. Si puede estar en userspace, debe estarlo.
- **Seguridad**: Validar TODOS los parámetros de syscalls que vienen de userspace.
- **Performance**: IPC debe ser extremadamente rápido (zero-copy cuando sea posible).
- **Testing**: Crear suite de tests para cada syscall implementada.

### Bugs Conocidos

- Ninguno reportado actualmente

### Mejoras Futuras

- SMP (Soporte multiprocesador)
- ACPI (Advanced Configuration and Power Interface)
- Networking stack en userspace
- GPU driver framework

---

## 📚 Referencias

- [docs/Syscalls.md](docs/Syscalls.md) - Documentación de syscalls
- [ARCHITECTURE.md](ARCHITECTURE.md) - Arquitectura del sistema
- [DEV.md](DEV.md) - Guía de desarrollo

---

**Contribuciones**: Si implementas algo de esta lista, actualiza este documento y marca como completado.
