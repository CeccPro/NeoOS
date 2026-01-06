/**
 * NeoOS - Context Switch (x86)
 * Funciones en assembly para realizar el cambio de contexto entre procesos
 */

.section .text
.align 4

/**
 * switch_context - Realizar un cambio de contexto entre procesos
 * 
 * Guarda el contexto del proceso actual y carga el contexto del nuevo proceso
 * 
 * @param old_esp: Puntero donde guardar el ESP del proceso actual (4(%esp))
 * @param new_esp: Valor del ESP del nuevo proceso (8(%esp))
 * 
 * C signature: void switch_context(uint32_t* old_esp, uint32_t new_esp);
 * 
 * CALLING CONVENTION:
 * - EAX, ECX, EDX son caller-saved (no se preservan)
 * - EBX, ESI, EDI, EBP son callee-saved (se preservan aquí)
 * - EFLAGS se preserva para mantener estado de interrupciones
 */
.global switch_context
.type switch_context, @function
switch_context:
    # Stack al entrar:
    # [esp + 0] = return address
    # [esp + 4] = old_esp (puntero)
    # [esp + 8] = new_esp (valor)
    
    # Guardar todos los registros callee-saved
    pushl %ebx              # esp -= 4
    pushl %esi              # esp -= 8
    pushl %edi              # esp -= 12
    pushl %ebp              # esp -= 16
    
    # Guardar EFLAGS
    pushfl                  # esp -= 20
    
    # Stack después de los pushes:
    # [esp + 0]  = eflags
    # [esp + 4]  = ebp
    # [esp + 8]  = edi
    # [esp + 12] = esi
    # [esp + 16] = ebx
    # [esp + 20] = return address
    # [esp + 24] = old_esp (puntero) ← CRÍTICO: offset correcto
    # [esp + 28] = new_esp (valor)   ← CRÍTICO: offset correcto
    
    # Guardar el ESP actual en *old_esp
    movl 24(%esp), %eax     # eax = old_esp (puntero)
    movl %esp, (%eax)       # *old_esp = esp actual
    
    # Cargar el nuevo ESP (contexto del nuevo proceso)
    movl 28(%esp), %esp     # esp = new_esp
    
    # Restaurar el contexto del nuevo proceso
    popfl                    # Restaurar EFLAGS
    popl %ebp
    popl %edi
    popl %esi
    popl %ebx
    
    # Retornar (esto carga EIP del nuevo proceso)
    ret

/**
 * init_process_stack - Inicializar el stack de un nuevo proceso
 * 
 * Configura el stack de un nuevo proceso para que cuando se haga el primer
 * context switch, el proceso comience a ejecutarse desde su entry point.
 * 
 * CONTRATO MENTAL:
 * Esta función NO simula que switch_context fue llamado.
 * Lo que hace es CONSTRUIR un stack que PARECE que switch_context
 * guardó el contexto de un proceso que estaba corriendo.
 * 
 * Es decir:
 * - Fingimos que el proceso ya estaba en ejecución
 * - Fingimos que switch_context guardó su estado
 * - Fingimos que cuando switch_context retorne, volverá a entry_point
 * - Fingimos que si entry_point retorna, va a exit_handler
 * 
 * Esto es una "mentira noble" necesaria para arrancar procesos nuevos.
 * 
 * @param stack_top: Puntero al tope del stack (4(%esp))
 * @param entry_point: Dirección de inicio del proceso (8(%esp))
 * @param exit_handler: Función a llamar si el proceso retorna (12(%esp))
 * @return: ESP inicial del proceso
 * 
 * C signature: uint32_t init_process_stack(uint32_t stack_top, 
 *                                          void (*entry_point)(void),
 *                                          void (*exit_handler)(void));
 */
.global init_process_stack
.type init_process_stack, @function
init_process_stack:
    # Obtener los parámetros
    movl 4(%esp), %eax       # stack_top
    movl 8(%esp), %ebx       # entry_point
    movl 12(%esp), %ecx      # exit_handler
    
    # El stack crece hacia abajo, construimos de arriba hacia abajo
    # Vamos a crear un stack que PARECE que switch_context lo guardó
    
    # === Simular el stack frame del entry_point ===
    # Push del exit_handler (dirección de retorno SI el entry_point retorna)
    subl $4, %eax
    movl %ecx, (%eax)
    
    # === Simular el stack frame de switch_context ===
    # Push del entry_point (return address para switch_context)
    # Cuando switch_context haga 'ret', saltará aquí
    subl $4, %eax
    movl %ebx, (%eax)
    
    # Push de los registros (valores iniciales = 0)
    # EBX
    subl $4, %eax
    movl $0, (%eax)
    
    # ESI
    subl $4, %eax
    movl $0, (%eax)
    
    # EDI
    subl $4, %eax
    movl $0, (%eax)
    
    # EBP
    subl $4, %eax
    movl $0, (%eax)
    
    # EFLAGS (con interrupciones habilitadas: IF = bit 9)
    subl $4, %eax
    movl $0x202, (%eax)
    
    # Retornar el nuevo ESP
    ret

/**
 * read_esp - Leer el valor actual del ESP
 * 
 * @return: Valor actual del ESP
 * 
 * C signature: uint32_t read_esp(void);
 */
.global read_esp
.type read_esp, @function
read_esp:
    movl %esp, %eax          # Retornar ESP en EAX
    ret

/**
 * read_ebp - Leer el valor actual del EBP
 * 
 * @return: Valor actual del EBP
 * 
 * C signature: uint32_t read_ebp(void);
 */
.global read_ebp
.type read_ebp, @function
read_ebp:
    movl %ebp, %eax          # Retornar EBP en EAX
    ret

/**
 * read_eflags - Leer el valor actual de EFLAGS
 * 
 * @return: Valor actual de EFLAGS
 * 
 * C signature: uint32_t read_eflags(void);
 */
.global read_eflags
.type read_eflags, @function
read_eflags:
    pushfl                   # Push EFLAGS al stack
    popl %eax                # Pop EFLAGS a EAX
    ret
