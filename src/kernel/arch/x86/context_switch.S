/**
 * NeoOS - Context Switch (x86)
 * Enfoque simplificado inspirado en xv6 y kernels minimalistas
 * 
 * ESTRATEGIA:
 * - switch_context: Intercambia ESP entre proceso actual y nuevo
 * - init_process_stack: Crea un stack inicial que parece que el proceso
 *   fue interrumpido justo antes de ejecutar su entry_point
 */

.section .text
.align 4

/**
 * switch_context - Cambio de contexto simple
 * 
 * Guarda registros callee-saved en el stack del proceso actual,
 * guarda el ESP actual, carga el ESP del nuevo proceso, y restaura
 * los registros del nuevo proceso.
 * 
 * C signature: void switch_context(uint32_t* old_esp, uint32_t new_esp);
 * 
 * Parámetros:
 *   old_esp: Puntero donde guardar el ESP actual (4(%esp) al entrar)
 *   new_esp: Valor del ESP del nuevo proceso (8(%esp) al entrar)
 */
.global switch_context
.type switch_context, @function
switch_context:
    # Al entrar:
    # [ESP + 0] = return address
    # [ESP + 4] = old_esp (puntero)
    # [ESP + 8] = new_esp (valor)
    
    # Guardar parámetros en registros ANTES de modificar el stack
    movl 4(%esp), %eax      # old_esp (puntero)
    movl 8(%esp), %edx      # new_esp (valor)
    
    # Guardar registros callee-saved en el stack actual
    pushl %ebp
    pushl %ebx
    pushl %esi
    pushl %edi
    
    # Guardar el ESP actual en *old_esp
    movl %esp, (%eax)
    
    # Cambiar al nuevo ESP
    movl %edx, %esp
    
    # Restaurar registros del nuevo proceso
    popl %edi
    popl %esi
    popl %ebx
    popl %ebp
    
    # Retornar (saltar a donde estaba el nuevo proceso)
    ret

/**
 * init_process_stack - Inicializar stack de un proceso nuevo
 * 
 * Construye un stack que hace que switch_context() crea que el proceso
 * estaba ejecutando y fue guardado. Cuando switch_context() restaure
 * este stack, el proceso comenzará a ejecutar en entry_point.
 * 
 * C signature: uint32_t init_process_stack(
 *                  uint32_t stack_top,
 *                  void (*entry_point)(void),
 *                  void (*exit_handler)(void)
 *              );
 * 
 * Retorna: El ESP inicial del proceso
 */
.global init_process_stack
.type init_process_stack, @function
init_process_stack:
    # Obtener parámetros
    movl 4(%esp), %eax      # stack_top
    movl 8(%esp), %ebx      # entry_point
    movl 12(%esp), %ecx     # exit_handler
    
    # Alinear stack a 16 bytes (requerido por x86 ABI)
    andl $0xFFFFFFF0, %eax
    
    # Construir el stack como si switch_context hubiera guardado el estado
    # 
    # switch_context GUARDA (push, en este orden):
    #   pushl %ebp
    #   pushl %ebx  
    #   pushl %esi
    #   pushl %edi
    #
    # Esto deja el stack así (de menor a mayor dirección):
    #   [ESP + 0] = edi  <- ESP apunta aquí
    #   [ESP + 4] = esi
    #   [ESP + 8] = ebx
    #   [ESP + 12] = ebp
    #   [ESP + 16] = return address
    #
    # switch_context RESTAURA (pop, en este orden):
    #   popl %edi   -> lee [ESP + 0]
    #   popl %esi   -> lee [ESP + 4]
    #   popl %ebx   -> lee [ESP + 8]
    #   popl %ebp   -> lee [ESP + 12]
    #   ret         -> lee [ESP + 16]
    #
    # Por lo tanto, debemos construir en ORDEN INVERSO:
    # (construimos de arriba hacia abajo, decrementando)
    # El stack crece hacia abajo, pero lo construimos desde arriba
    
    # Construimos desde la dirección más alta hacia abajo:
    # Recordar: switch_context hace pushl en orden: ebp, ebx, esi, edi
    # Por lo tanto el stack queda (de ESP hacia arriba):
    #   [ESP + 0]  = edi  <- ESP debe apuntar aquí
    #   [ESP + 4]  = esi
    #   [ESP + 8]  = ebx
    #   [ESP + 12] = ebp
    #   [ESP + 16] = return address (entry_point)
    #
    # Construimos de arriba hacia abajo con subl:
    
    # Primero: entry_point (el 'ret' de switch_context salta aquí)
    subl $4, %eax
    movl %ebx, (%eax)       # [ESP + 16] = entry_point
    
    # Segundo: ebp (cuarto pop)
    subl $4, %eax
    movl $0, (%eax)         # [ESP + 12] = ebp = 0
    
    # Tercero: ebx (tercer pop)
    subl $4, %eax
    movl $0, (%eax)         # [ESP + 8] = ebx = 0
    
    # Cuarto: esi (segundo pop)
    subl $4, %eax
    movl $0, (%eax)         # [ESP + 4] = esi = 0
    
    # Quinto: edi (primer pop) <- ESP debe apuntar aquí
    subl $4, %eax
    movl $0, (%eax)         # [ESP + 0] = edi = 0
    
    # EAX ahora apunta a edi, que es donde debe estar ESP
    # EAX contiene el ESP inicial correcto
    ret

/**
 * read_esp - Obtener el valor actual del ESP
 */
.global read_esp
.type read_esp, @function
read_esp:
    movl %esp, %eax
    addl $4, %eax           # Compensar por el return address
    ret

/**
 * read_ebp - Obtener el valor actual del EBP
 */
.global read_ebp
.type read_ebp, @function
read_ebp:
    movl %ebp, %eax
    ret

/**
 * read_eflags - Obtener el valor actual de EFLAGS
 */
.global read_eflags
.type read_eflags, @function
read_eflags:
    pushfl
    popl %eax
    ret
