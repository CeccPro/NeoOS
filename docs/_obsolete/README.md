# Documentación Obsoleta

Esta carpeta contiene documentación de funcionalidades que han sido rediseñadas o movidas a userspace en el nuevo diseño de microkernel de NeoOS.

## Syscalls Obsoletas (syscalls/)

Las siguientes syscalls estaban definidas en el diseño original pero han sido **eliminadas del kernel** o **movidas a userspace**:

### Movidas a VFS Server (userspace)
- `sys_open`, `sys_read`, `sys_write`, `sys_close`
- `sys_mkdir`, `sys_rmdir`, `sys_unlink`, `sys_chdir`
- `sys_stat`

### Movidas a libneo (userspace)
- `sys_run` - Ahora implementada en libneo usando `sys_thread_create`
- `sys_clone` - Ahora implementada en libneo usando `sys_thread_create`
- `sys_kill` - Ahora implementada en libneo usando `sys_signal`
- `sys_exit` - Reemplazada por `sys_thread_exit`
- `sys_sbrk` - Ahora implementada en libneo usando `sys_map`/`sys_unmap`

### Reemplazadas o consolidadas
- `sys_getpid` - Integrada en `sys_getinfo(INFO_PID, ...)`
- `sys_uptime` - Integrada en `sys_getinfo(INFO_UPTIME, ...)`
- `sys_gettime` - Integrada en `sys_getinfo(INFO_TIME, ...)`
- `sys_sleep` - Ahora se usa `sys_wait` con timeout
- `sys_wait` - Reemplazada por `sys_wait(event_mask, timeout)` más general
- `sys_mmap`/`sys_munmap` - Reemplazadas por `sys_map`/`sys_unmap` más genéricas

## Razón del Cambio

NeoOS ha migrado a un **diseño de microkernel puro** donde:

1. **El kernel solo proporciona mecanismos básicos** (IPC, threads, memoria)
2. **Las políticas se implementan en userspace** (filesystem, gestión de procesos)
3. **Mayor seguridad** mediante reducción de código en kernel
4. **Mayor modularidad** permitiendo reemplazar componentes sin afectar el kernel

Para la documentación actualizada, consulte:
- [docs/Syscalls.md](../Syscalls.md) - Nuevo esquema de syscalls (~15 syscalls)
- [ARCHITECTURE.md](../../ARCHITECTURE.md) - Arquitectura actualizada
