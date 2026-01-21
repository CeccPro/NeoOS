# NeoOS - Sistema de Interrupciones
El sistema de interrupciones en NeoOS es un componente fundamental que permite al kernel responder a eventos tanto de hardware como de software. La arquitectura x86 proporciona un mecanismo robusto de interrupciones que NeoOS implementa mediante la GDT, IDT y el manejo de ISR/IRQ.

## Arquitectura del Sistema de Interrupciones

### Componentes Principales

1. **GDT (Global Descriptor Table)**: Define los segmentos de memoria
2. **IDT (Interrupt Descriptor Table)**: Mapea interrupciones a sus handlers
3. **ISR (Interrupt Service Routines)**: Excepciones del CPU (0-31)
4. **IRQ (Interrupt Requests)**: Interrupciones de hardware (0-15, mapeadas a 32-47)
5. **PIC (Programmable Interrupt Controller)**: Controlador de interrupciones de hardware

## Global Descriptor Table (GDT)

### Ubicación
- **Archivo de implementación**: `src/kernel/core/src/gdt.c`
- **Header**: `src/kernel/core/include/gdt.h`
- **Función de inicialización**: `gdt_init()`

### Estructura de Entrada GDT
```c
struct gdt_entry {
    uint16_t limit_low;    // Límite bits 0-15
    uint16_t base_low;     // Base bits 0-15
    uint8_t  base_middle;  // Base bits 16-23
    uint8_t  access;       // Flags de acceso
    uint8_t  granularity;  // Granularidad y límite bits 16-19
    uint8_t  base_high;    // Base bits 24-31
} __attribute__((packed));
```

### Entradas de la GDT (5 entradas)

| Índice | Selector | Descripción | Base | Límite | Ring | Uso |
|--------|----------|-------------|------|--------|------|-----|
| 0 | 0x00 | Descriptor Nulo | 0x00000000 | 0 | - | Requerido por x86 |
| 1 | 0x08 | Kernel Code | 0x00000000 | 0xFFFFFFFF | 0 | Código del kernel |
| 2 | 0x10 | Kernel Data | 0x00000000 | 0xFFFFFFFF | 0 | Datos del kernel |
| 3 | 0x18 | User Code | 0x00000000 | 0xFFFFFFFF | 3 | Código de usuario |
| 4 | 0x20 | User Data | 0x00000000 | 0xFFFFFFFF | 3 | Datos de usuario |

**Nota sobre selectores de usuario**:
- Para Ring 3, se debe agregar RPL (Requested Privilege Level) = 3
- `GDT_USER_CODE_SELECTOR = 0x18 | 0x03 = 0x1B`
- `GDT_USER_DATA_SELECTOR = 0x20 | 0x03 = 0x23`

### Flags de Acceso
```c
#define GDT_ACCESS_PRESENT    0x80  // Segmento presente
#define GDT_ACCESS_PRIV_0     0x00  // Ring 0 (kernel)
#define GDT_ACCESS_PRIV_3     0x60  // Ring 3 (usuario)
#define GDT_ACCESS_CODE_SEG   0x18  // Segmento de código ejecutable
#define GDT_ACCESS_DATA_SEG   0x10  // Segmento de datos
#define GDT_ACCESS_READ_WRITE 0x02  // Lectura/escritura permitida
```

### Flags de Granularidad
```c
#define GDT_GRAN_4K           0x80  // Límite en páginas de 4KB
#define GDT_GRAN_32BIT        0x40  // Segmento de 32 bits
```

### Proceso de Inicialización
1. Configura el puntero GDT con límite y dirección base
2. Inicializa todas las entradas a 0 con `memset()`
3. Configura las 5 entradas usando `gdt_set_gate()`
4. Carga la GDT con `gdt_flush()` (función en assembly)
5. `gdt_flush()` ejecuta `lgdt` y recarga los selectores de segmento (CS, DS, ES, FS, GS, SS)

## Interrupt Descriptor Table (IDT)

### Ubicación
- **Archivo de implementación**: `src/kernel/core/src/idt.c`
- **Header**: `src/kernel/core/include/idt.h`
- **Función de inicialización**: `idt_init()`

### Estructura de Entrada IDT
```c
struct idt_entry {
    uint16_t base_low;     // Dirección del handler bits 0-15
    uint16_t selector;     // Selector de segmento de código
    uint8_t  always0;      // Siempre 0
    uint8_t  flags;        // Tipo y atributos
    uint16_t base_high;    // Dirección del handler bits 16-31
} __attribute__((packed));
```

### Tipos de Entradas IDT
```c
#define IDT_FLAG_PRESENT     0x80  // Entrada presente
#define IDT_FLAG_RING_0      0x00  // Ring 0 (kernel)
#define IDT_FLAG_RING_3      0x60  // Ring 3 (usuario)
#define IDT_FLAG_GATE_32_INT 0x0E  // Puerta de interrupción de 32 bits
#define IDT_FLAG_GATE_32_TRAP 0x0F // Puerta de trampa de 32 bits

#define IDT_TYPE_INTERRUPT   (IDT_FLAG_PRESENT | IDT_FLAG_RING_0 | IDT_FLAG_GATE_32_INT)
#define IDT_TYPE_TRAP        (IDT_FLAG_PRESENT | IDT_FLAG_RING_0 | IDT_FLAG_GATE_32_TRAP)
```

### Proceso de Inicialización
1. Configura el puntero IDT con límite (256 entradas - 1) y dirección base
2. Inicializa todas las 256 entradas a 0 con `memset()`
3. Las entradas se configuran después en `interrupts_init()`
4. Carga la IDT con `idt_flush()` (función en assembly que ejecuta `lidt`)

### Función de Configuración
```c
void idt_set_gate(uint8_t num, uint32_t base, uint16_t selector, uint8_t flags);
```

## Excepciones del CPU (ISR 0-31)

### Ubicación
- **Handlers en C**: `src/kernel/core/src/interrupts.c`
- **Stubs en Assembly**: `src/kernel/arch/x86/isr.S`
- **Header**: `src/kernel/core/include/interrupts.h`

### Tabla de Excepciones

| Número | Nombre | Código Error | Descripción |
|--------|--------|--------------|-------------|
| 0 | Division By Zero | No | División por cero |
| 1 | Debug | No | Excepción de depuración |
| 2 | Non Maskable Interrupt | No | Interrupción no enmascarable |
| 3 | Breakpoint | No | Punto de interrupción (INT 3) |
| 4 | Overflow | No | Desbordamiento (INTO) |
| 5 | Bound Range Exceeded | No | Límite de rango excedido |
| 6 | Invalid Opcode | No | Código de operación inválido |
| 7 | Device Not Available | No | Dispositivo no disponible |
| 8 | Double Fault | Sí | Fallo doble |
| 9 | Coprocessor Segment Overrun | No | Desbordamiento de segmento del coprocesador |
| 10 | Invalid TSS | Sí | TSS inválido |
| 11 | Segment Not Present | Sí | Segmento no presente |
| 12 | Stack-Segment Fault | Sí | Fallo de segmento de pila |
| 13 | General Protection Fault | Sí | Fallo de protección general |
| 14 | Page Fault | Sí | Fallo de página |
| 15 | Reserved | No | Reservado |
| 16 | x87 FPU Error | No | Error de punto flotante x87 |
| 17 | Alignment Check | Sí | Verificación de alineación |
| 18 | Machine Check | No | Verificación de máquina |
| 19 | SIMD Floating-Point Exception | No | Excepción SIMD de punto flotante |
| 20 | Virtualization Exception | No | Excepción de virtualización |
| 21-29 | Reserved | No | Reservado |
| 30 | Security Exception | Sí | Excepción de seguridad |
| 31 | Reserved | No | Reservado |

### Estructura de Registros Guardados
```c
struct registers {
    uint32_t ds;                                     // Segmento de datos
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // Registros generales (pusha)
    uint32_t int_no, err_code;                       // Número de interrupción y código de error
    uint32_t eip, cs, eflags, useresp, ss;          // Empujado automáticamente por el CPU
} __attribute__((packed));

typedef struct registers registers_t;
```

### Handler de ISR en C
```c
void isr_handler(registers_t* regs) {
    // Si hay un handler registrado, llamarlo
    if (interrupt_handlers[regs->int_no] != 0) {
        isr_handler_t handler = interrupt_handlers[regs->int_no];
        handler(regs);
    } else {
        // KERNEL PANIC: Excepción no manejada
        // Muestra información detallada de registros
        vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_RED);
        vga_write("\n\n!!! KERNEL PANIC !!!\n");
        vga_write("Unhandled Exception: ");
        vga_write(exception_messages[regs->int_no]);
        // ... más información de debug
        __asm__ volatile("cli; hlt");
        while(1);
    }
}
```

## Interrupciones de Hardware (IRQ 0-15)

### Remapeo del PIC

Por defecto, el PIC (Programmable Interrupt Controller) mapea:
- IRQ 0-7 → Interrupciones 8-15
- IRQ 8-15 → Interrupciones 70-77 (en algunos sistemas)

**Problema**: Las interrupciones 8-15 se superponen con las excepciones del CPU.

**Solución**: Remapear el PIC para que:
- IRQ 0-7 (PIC1) → Interrupciones 32-39
- IRQ 8-15 (PIC2) → Interrupciones 40-47

### Puertos del PIC
```c
#define PIC1_COMMAND 0x20  // Puerto de comandos del PIC maestro
#define PIC1_DATA    0x21  // Puerto de datos del PIC maestro
#define PIC2_COMMAND 0xA0  // Puerto de comandos del PIC esclavo
#define PIC2_DATA    0xA1  // Puerto de datos del PIC esclavo
```

### Comandos del PIC
```c
#define PIC_EOI      0x20  // End of Interrupt
#define ICW1_ICW4    0x01  // ICW4 necesario
#define ICW1_INIT    0x10  // Comando de inicialización
#define ICW4_8086    0x01  // Modo 8086
```

### Proceso de Remapeo
```c
static void pic_remap(void) {
    // 1. Guardar máscaras actuales
    // 2. Enviar ICW1: Iniciar secuencia de inicialización
    // 3. Enviar ICW2: Establecer vectores de offset (32 para PIC1, 40 para PIC2)
    // 4. Enviar ICW3: Configurar cascada (IRQ2 conecta PICs)
    // 5. Enviar ICW4: Modo 8086
    // 6. Restaurar máscaras
}
```

### Tabla de IRQs

| IRQ | ISR | Dispositivo |
|-----|-----|-------------|
| 0 | 32 | System Timer (PIT) |
| 1 | 33 | Keyboard |
| 2 | 34 | Cascade (PIC esclavo) |
| 3 | 35 | Serial Port 2 (COM2) |
| 4 | 36 | Serial Port 1 (COM1) |
| 5 | 37 | Parallel Port 2 / Sound Card |
| 6 | 38 | Floppy Disk Controller |
| 7 | 39 | Parallel Port 1 |
| 8 | 40 | Real-Time Clock (RTC) |
| 9 | 41 | ACPI / SCSI / NIC |
| 10 | 42 | Disponible / SCSI / NIC |
| 11 | 43 | Disponible / SCSI / NIC |
| 12 | 44 | PS/2 Mouse |
| 13 | 45 | FPU / Coprocessor |
| 14 | 46 | Primary ATA Hard Disk |
| 15 | 47 | Secondary ATA Hard Disk |

### End of Interrupt (EOI)

Después de procesar una IRQ, se debe enviar EOI al PIC:

```c
static void pic_send_eoi(uint8_t irq_num) {
    // Si la IRQ vino del PIC esclavo, enviar EOI a ambos
    if (irq_num >= 40) {
        __asm__ volatile("outb %0, %1" : : "a"((uint8_t)PIC_EOI), "Nd"(PIC2_COMMAND));
    }
    
    // Enviar EOI al PIC maestro
    __asm__ volatile("outb %0, %1" : : "a"((uint8_t)PIC_EOI), "Nd"(PIC1_COMMAND));
}
```

### Handler de IRQ en C
```c
void irq_handler(registers_t* regs) {
    // Enviar EOI al PIC
    pic_send_eoi(regs->int_no);

    // Si hay un handler registrado, llamarlo
    if (interrupt_handlers[regs->int_no] != 0) {
        isr_handler_t handler = interrupt_handlers[regs->int_no];
        handler(regs);
    }
}
```

## Handlers en Assembly

### Ubicación
`src/kernel/arch/x86/isr.S`

### Macros para Crear Stubs

#### ISR sin Código de Error
```assembly
.macro ISR_NOERRCODE num
.global isr\num
isr\num:
    pushl $0                # Código de error dummy (0)
    pushl $\num             # Número de interrupción
    jmp isr_common_stub     # Saltar al handler común
.endm
```

#### ISR con Código de Error
```assembly
.macro ISR_ERRCODE num
.global isr\num
isr\num:
    pushl $\num             # Número de interrupción (el CPU ya empujó el código de error)
    jmp isr_common_stub     # Saltar al handler común
.endm
```

#### IRQ Handler
```assembly
.macro IRQ num, isr_num
.global irq\num
irq\num:
    cli                     # Deshabilitar interrupciones
    pushl $0                # Código de error dummy (0)
    pushl $\isr_num         # Número de interrupción (32 + num)
    jmp irq_common_stub     # Saltar al handler común de IRQ
.endm
```

### Common Stub para ISR
```assembly
isr_common_stub:
    # Guardar el estado del CPU
    pushal                  # Empuja EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI
    
    # Guardar el segmento de datos
    movw %ds, %ax
    pushl %eax
    
    # Cargar el segmento de datos del kernel
    movw $0x10, %ax         # GDT_KERNEL_DATA_SEGMENT = 0x10
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    
    # Llamar al handler de C
    pushl %esp
    call isr_handler
    addl $4, %esp
    
    # Restaurar el segmento de datos original
    popl %eax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    
    # Restaurar el estado del CPU
    popal
    
    # Limpiar el número de interrupción y código de error
    addl $8, %esp
    
    # Retornar de la interrupción (restaura CS, EIP, EFLAGS automáticamente)
    iret
```

### Common Stub para IRQ
Similar a `isr_common_stub`, pero llama a `irq_handler` en lugar de `isr_handler`.

## Inicialización del Sistema de Interrupciones

### Función Principal
```c
void interrupts_init(bool verbose);
```

### Proceso de Inicialización
1. **Inicializar array de handlers**:
   ```c
   memset(&interrupt_handlers, 0, sizeof(isr_handler_t) * 256);
   ```

2. **Remapear el PIC**:
   ```c
   pic_remap();
   ```

3. **Configurar ISR 0-31**:
   ```c
   idt_set_gate(0,  (uint32_t)isr0,  GDT_KERNEL_CODE_SEGMENT, IDT_TYPE_INTERRUPT);
   idt_set_gate(1,  (uint32_t)isr1,  GDT_KERNEL_CODE_SEGMENT, IDT_TYPE_INTERRUPT);
   // ... hasta isr31
   ```

4. **Configurar IRQ 0-15**:
   ```c
   idt_set_gate(32, (uint32_t)irq0,  GDT_KERNEL_CODE_SEGMENT, IDT_TYPE_INTERRUPT);
   idt_set_gate(33, (uint32_t)irq1,  GDT_KERNEL_CODE_SEGMENT, IDT_TYPE_INTERRUPT);
   // ... hasta irq15
   ```

5. **Habilitar interrupciones**:
   ```c
   __asm__ volatile("sti");
   ```

## Registro de Handlers Personalizados

### Función
```c
void interrupts_register_handler(uint8_t num, isr_handler_t handler);
```

### Tipo de Handler
```c
typedef void (*isr_handler_t)(registers_t*);
```

### Ejemplo de Uso
```c
void my_timer_handler(registers_t* regs) {
    // Código del handler del timer
}

// Registrar el handler para IRQ0 (Timer)
interrupts_register_handler(IRQ0, my_timer_handler);
```

## Kernel Panic

Cuando ocurre una excepción no manejada, el kernel entra en pánico y muestra:
- Nombre de la excepción
- Número de interrupción
- Código de error (si aplica)
- Estado de los registros: EIP, CS, EFLAGS, EAX, EBX, ECX, EDX, ESI, EDI, EBP, ESP
- Mensaje en fondo rojo con texto blanco

Luego ejecuta `cli; hlt` y entra en bucle infinito.

## Estado Actual de la Implementación

### Componentes Implementados
- GDT con 5 entradas (kernel y usuario)
- IDT con 256 entradas
- ISR 0-31 (excepciones del CPU)
- IRQ 0-15 (interrupciones de hardware)
- Remapeo del PIC
- Handlers en assembly con common stubs
- Sistema de registro de handlers personalizados
- Kernel panic con información detallada

### Componentes Pendientes ⏳
- Handlers específicos para timer (IRQ0)
- Handler de teclado (IRQ1)
- Syscalls (INT 0x80)
- APIC (Advanced Programmable Interrupt Controller) para sistemas modernos
- MSI (Message Signaled Interrupts)

## Notas Importantes

1. **Deshabilitar Interrupciones**: Durante operaciones críticas, usar `cli` para deshabilitar interrupciones
2. **Habilitar Interrupciones**: Usar `sti` solo cuando el sistema esté listo
3. **Atomicidad**: Las secciones críticas deben protegerse deshabilitando interrupciones
4. **EOI**: Siempre enviar EOI al PIC después de procesar una IRQ
5. **Stack Overflow**: Los handlers de interrupciones usan la pila del kernel, cuidado con operaciones que usen mucho stack

## Véase También
- [Boot Process](./Boot%20Process.md) - Proceso de arranque del sistema
- [Kernel Initialization](./Kernel%20Initialization.md) - Inicialización detallada del kernel
- [Memory Manager](./Memory%20Manager.md) - Gestión de memoria
- [Errors](./Errors.md) - Códigos de error del kernel
- [Intel Software Developer Manual](https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html) - Documentación oficial de Intel
