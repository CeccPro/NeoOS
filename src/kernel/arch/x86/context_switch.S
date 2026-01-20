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
    
    # Guardar EFLAGS
    pushf
    
    # Guardar el ESP actual en *old_esp
    movl %esp, (%eax)
    
    # Cambiar al nuevo ESP
    movl %edx, %esp
    
    # Restaurar EFLAGS
    popf
    
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
 * IMPORTANTE: Cuando entry_point retorne, debe saltar a exit_handler.
 * Para lograr esto, ponemos exit_handler en la posición de "return address"
 * del entry_point en su propio stack frame.
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
    
    # Construir el stack del proceso:
    # 
    # Cuando switch_context restaura, hace:
    #   popf (EFLAGS)
    #   popl %edi, %esi, %ebx, %ebp
    #   ret  -> salta a entry_point
    #
    # Cuando entry_point ejecuta, su stack debe tener:
    #   [ESP] = exit_handler (para que ret salte ahí)
    #
    # Entonces construimos DOS frames:
    # 1. Frame para switch_context (EFLAGS, registros, return=entry_point)
    # 2. Frame para entry_point (return=exit_handler)
    
    # === Frame del entry_point (construir primero, está más arriba) ===
    # Return address para cuando entry_point haga ret
    subl $4, %eax
    movl %ecx, (%eax)       # return address = exit_handler
    
    # === Frame para switch_context ===
    # Primero: entry_point (el 'ret' de switch_context salta aquí)
    subl $4, %eax
    movl %ebx, (%eax)       # return address para switch_context = entry_point
    
    # Segundo: ebp (quinto pop)
    subl $4, %eax
    movl $0, (%eax)         # ebp = 0
    
    # Tercero: ebx (cuarto pop)
    subl $4, %eax
    movl $0, (%eax)         # ebx = 0
    
    # Cuarto: esi (tercer pop)
    subl $4, %eax
    movl $0, (%eax)         # esi = 0
    
    # Quinto: edi (segundo pop)
    subl $4, %eax
    movl $0, (%eax)         # edi = 0
    
    # Sexto: EFLAGS (primer pop - popf) <- ESP debe apuntar aquí
    # EFLAGS con interrupciones habilitadas: 0x202 (IF bit set)
    subl $4, %eax
    movl $0x202, (%eax)     # EFLAGS = 0x202
    
    # EAX ahora apunta a EFLAGS, que es donde debe estar ESP
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
