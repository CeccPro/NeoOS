/**
 * NeoOS - Bootloader x86 (Multiboot)
 * Entry point del kernel con soporte para Multiboot
 */

.set ALIGN,    1<<0             # Alinear módulos cargados en límites de página
.set MEMINFO,  1<<1             # Proporcionar mapa de memoria
.set FLAGS,    ALIGN | MEMINFO  # Flags de Multiboot
.set MAGIC,    0x1BADB002       # Magic number para Multiboot
.set CHECKSUM, -(MAGIC + FLAGS) # Checksum (magic + flags + checksum debe ser 0)

# Declarar header Multiboot al principio del archivo
.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

# Reservar espacio para la pila (16KB)
.section .bss
.align 16
stack_bottom:
.skip 16384  # 16 KiB
stack_top:

# Entry point del kernel
.section .text
.global _start
.type _start, @function

_start:
    # El bootloader nos ha cargado en modo protegido de 32 bits
    # Las interrupciones están deshabilitadas
    # El paging está deshabilitado
    # El procesador está en un estado bastante bruto
    
    # Para configurar una pila, ponemos el puntero de pila (esp) 
    # apuntando al final de nuestra pila (crece hacia abajo)
    mov $stack_top, %esp
    
    # Aquí podríamos resetear EFLAGS si fuera necesario
    # push $0
    # popf
    
    # En este punto:
    # EAX contiene el magic number de Multiboot (0x2BADB002)
    # EBX contiene la dirección física de la estructura multiboot_info
    
    # Guardar los argumentos para pasarlos al kernel_main
    push %ebx   # Segundo argumento: multiboot_info
    push %eax   # Primer argumento: magic number
    
    # Llamar a la función kernel_main (definida en C)
    call kernel_main
    
    # Si kernel_main retorna (no debería), entrar en un bucle infinito
    # Deshabilitamos interrupciones por si se rehabilitaron
    cli
1:
    hlt         # Esperar a la siguiente interrupción
    jmp 1b      # Si recibimos una NMI, volver a hacer halt
    
.size _start, . - _start
