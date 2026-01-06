/**
 * NeoOS - Scheduler Implementation
 * Implementación del planificador de procesos Round Robin con prioridades
 */

#include "../../core/include/scheduler.h"
#include "../../core/include/timer.h"
#include "../../core/include/error.h"
#include "../../drivers/include/vga.h"
#include "../../lib/include/string.h"
#include "../../memory/include/memory.h"

// Variables globales del scheduler
process_t* current_process = NULL;      // Proceso actualmente en ejecución
process_t* idle_process = NULL;         // Proceso idle

// Colas de procesos por prioridad
static process_queue_t ready_queues[5]; // Una cola por nivel de prioridad (0-4)
static process_queue_t blocked_queue;   // Cola de procesos bloqueados

// Tabla de procesos (para búsqueda rápida por PID)
static process_t* process_table[MAX_PROCESSES];

// Contador de PIDs (con wrap-around protegido)
static uint32_t next_pid = 1;

// Total de procesos en el sistema
static uint32_t total_processes = 0;

// Flag para indicar si el scheduler está inicializado
static bool scheduler_initialized = false;

// Contadores para weighted round-robin por prioridad
// Cada prioridad tiene un "peso" que determina cuántas veces se selecciona
static uint32_t priority_counters[5] = {0, 0, 0, 0, 0};

// Pesos por prioridad (cuántas veces seleccionar antes de pasar a la siguiente)
// REALTIME: 8 de cada 15 selecciones
// HIGH:     4 de cada 15 selecciones  
// NORMAL:   2 de cada 15 selecciones
// LOW:      1 de cada 15 selecciones
// IDLE:     solo cuando no hay nadie más
static const uint32_t priority_weights[5] = {0, 1, 2, 4, 8}; // IDLE, LOW, NORMAL, HIGH, REALTIME

/**
 * Buscar un PID libre en la tabla de procesos
 * Implementa wrap-around para reutilizar PIDs
 * @return PID libre, o 0 si no hay disponibles
 */
static uint32_t find_free_pid(void) {
    uint32_t start_pid = next_pid;
    
    // Buscar un slot libre
    do {
        if (next_pid >= MAX_PROCESSES) {
            next_pid = 1; // Wrap around (PID 0 es el idle)
        }
        
        if (process_table[next_pid] == NULL) {
            return next_pid++;
        }
        
        next_pid++;
    } while (next_pid != start_pid);
    
    // No hay PIDs disponibles
    return 0;
}

/**
 * Obtener el quantum (time slice) para una prioridad específica
 * @param priority: Nivel de prioridad del proceso
 * @return Número de ticks para el quantum
 */
static uint32_t get_quantum_for_priority(process_priority_t priority) {
    switch (priority) {
        case PROCESS_PRIORITY_REALTIME: return QUANTUM_REALTIME;
        case PROCESS_PRIORITY_HIGH:     return QUANTUM_HIGH;
        case PROCESS_PRIORITY_NORMAL:   return QUANTUM_NORMAL;
        case PROCESS_PRIORITY_LOW:      return QUANTUM_LOW;
        case PROCESS_PRIORITY_IDLE:     return QUANTUM_IDLE;
        default:                        return QUANTUM_NORMAL;
    }
}

/**
 * Inicializar una cola de procesos
 */
static void queue_init(process_queue_t* queue) {
    queue->head = NULL;
    queue->tail = NULL;
    queue->count = 0;
}

/**
 * Agregar un proceso al final de una cola
 */
void scheduler_queue_add(process_queue_t* queue, process_t* process) {
    if (queue->tail == NULL) {
        // Cola vacía
        queue->head = process;
        queue->tail = process;
        process->prev = NULL;
        process->next = NULL;
    } else {
        // Agregar al final
        queue->tail->next = process;
        process->prev = queue->tail;
        process->next = NULL;
        queue->tail = process;
    }
    queue->count++;
}

/**
 * Remover un proceso de una cola
 */
void scheduler_queue_remove(process_queue_t* queue, process_t* process) {
    if (process->prev != NULL) {
        process->prev->next = process->next;
    } else {
        // Es el primero de la cola
        queue->head = process->next;
    }
    
    if (process->next != NULL) {
        process->next->prev = process->prev;
    } else {
        // Es el último de la cola
        queue->tail = process->prev;
    }
    
    process->prev = NULL;
    process->next = NULL;
    queue->count--;
}

/**
 * Obtener y remover el primer proceso de una cola
 */
process_t* scheduler_queue_pop(process_queue_t* queue) {
    if (queue->head == NULL) {
        return NULL;
    }
    
    process_t* process = queue->head;
    scheduler_queue_remove(queue, process);
    return process;
}

/**
 * Handler para procesos que terminan retornando
 * Si un proceso retorna de su entry point, se termina automáticamente
 */
void process_exit_handler(void) {
    if (current_process != NULL) {
        vga_set_color(VGA_COLOR_LIGHT_BROWN, VGA_COLOR_BLACK);
        vga_write("[SCHED] Proceso ");
        vga_write(current_process->name);
        vga_write(" (PID: ");
        vga_write_dec(current_process->pid);
        vga_write(") terminado\n");
        
        scheduler_terminate_process(current_process->pid);
    }
    
    // No debería llegar aquí, pero por seguridad
    while (1) {
        __asm__ volatile("hlt");
    }
}

/**
 * Proceso idle - ejecutado cuando no hay otros procesos
 */
void idle_process_entry(void) {
    while (1) {
        __asm__ volatile("hlt"); // Esperar a la siguiente interrupción
    }
}

/**
 * Inicializar el scheduler
 */
void scheduler_init(bool verbose) {
    if (verbose) {
        vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
        vga_write("[SCHED] Inicializando scheduler...\n");
    }
    
    // Inicializar las colas de procesos
    for (int i = 0; i < 5; i++) {
        queue_init(&ready_queues[i]);
    }
    queue_init(&blocked_queue);
    
    // Inicializar la tabla de procesos
    for (int i = 0; i < MAX_PROCESSES; i++) {
        process_table[i] = NULL;
    }
    
    // Crear el proceso idle
    if (verbose) {
        vga_write("[SCHED] Creando proceso idle...\n");
    }
    
    idle_process = (process_t*)kmalloc(sizeof(process_t));
    if (idle_process == NULL) {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_write("[SCHED] Error: no se pudo asignar memoria para el proceso idle\n");
        return;
    }
    
    // Configurar el proceso idle
    idle_process->pid = 0; // PID 0 siempre es el proceso idle
    strcpy(idle_process->name, "idle");
    idle_process->state = PROCESS_STATE_READY;
    idle_process->priority = PROCESS_PRIORITY_IDLE;
    idle_process->time_slices = 0;
    idle_process->ticks_remaining = get_quantum_for_priority(PROCESS_PRIORITY_IDLE);
    idle_process->next = NULL;
    idle_process->prev = NULL;
    
    // Configurar el stack del proceso idle
    idle_process->kernel_stack = (uint32_t)kmalloc(KERNEL_STACK_SIZE);
    if (idle_process->kernel_stack == 0) {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_write("[SCHED] Error: no se pudo asignar stack para el proceso idle\n");
        kfree(idle_process);
        idle_process = NULL;
        return;
    }
    
    // Configurar el contexto inicial del proceso idle usando assembly
    idle_process->esp = init_process_stack(
        idle_process->kernel_stack + KERNEL_STACK_SIZE,
        idle_process_entry,
        process_exit_handler
    );
    idle_process->ebp = 0;
    idle_process->eip = (uint32_t)idle_process_entry;
    idle_process->eflags = 0x202; // IF (Interrupt Flag) habilitado
    
    // Agregar el proceso idle a la tabla
    process_table[0] = idle_process;
    
    // Agregar el proceso idle a la cola de prioridad más baja
    scheduler_queue_add(&ready_queues[PROCESS_PRIORITY_IDLE], idle_process);
    
    total_processes = 1;
    
    // El proceso idle es el primer proceso en ejecutarse
    current_process = idle_process;
    current_process->state = PROCESS_STATE_RUNNING;
    
    scheduler_initialized = true;
    
    if (verbose) {
        vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
        vga_write("[SCHED] Scheduler inicializado correctamente\n");
        vga_write("[SCHED] Proceso idle creado (PID: 0)\n");
    }
}

/**
 * Crear un nuevo proceso
 * 
 * RACE CONDITION NOTE:
 * Sin SMP, esto es seguro. Con SMP necesitará locks.
 */
uint32_t scheduler_create_process(const char* name, void (*entry_point)(void), process_priority_t priority) {
    if (!scheduler_initialized) {
        return 0;
    }
    
    if (total_processes >= MAX_PROCESSES) {
        return 0; // No hay espacio para más procesos
    }
    
    // Deshabilitar interrupciones mientras modificamos estructuras críticas
    __asm__ volatile("cli");
    
    // Asignar memoria para el PCB
    process_t* process = (process_t*)kmalloc(sizeof(process_t));
    if (process == NULL) {
        __asm__ volatile("sti");
        return 0;
    }
    
    // Asignar un PID con validación y wrap-around
    uint32_t pid = find_free_pid();
    if (pid == 0) {
        kfree(process);
        __asm__ volatile("sti");
        return 0; // No hay PIDs disponibles
    }
    
    // Configurar el proceso
    process->pid = pid;
    strncpy(process->name, name, 31);
    process->name[31] = '\0';
    process->state = PROCESS_STATE_READY;
    process->priority = priority;
    process->time_slices = 0;
    process->ticks_remaining = get_quantum_for_priority(priority);
    process->next = NULL;
    process->prev = NULL;
    
    // Asignar el stack del kernel
    process->kernel_stack = (uint32_t)kmalloc(KERNEL_STACK_SIZE);
    if (process->kernel_stack == 0) {
        kfree(process);
        __asm__ volatile("sti");
        return 0;
    }
    
    // Configurar el contexto inicial usando assembly
    process->esp = init_process_stack(
        process->kernel_stack + KERNEL_STACK_SIZE,
        entry_point,
        process_exit_handler
    );
    process->ebp = 0;
    process->eip = (uint32_t)entry_point;
    process->eflags = 0x202; // IF habilitado
    
    // TODO: Configurar el page directory para el proceso (cuando esté VMM)
    process->page_directory = 0;
    
    // Agregar a la tabla de procesos
    process_table[pid] = process;
    
    // Agregar a la cola de ready de su prioridad
    scheduler_queue_add(&ready_queues[priority], process);
    
    // Incrementar contador de procesos (protegido por cli)
    total_processes++;
    
    // Restaurar interrupciones
    __asm__ volatile("sti");
    
    return pid;
}

/**
 * Terminar un proceso
 * 
 * DELICATE BEHAVIOR WARNING:
 * Si el proceso es current_process, scheduler_switch() cambia el contexto
 * y NUNCA vuelve aquí en el contexto del proceso terminado.
 * El kfree(process) se ejecuta en el contexto del NUEVO proceso.
 * 
 * Esto funciona porque:
 * 1. El ESP ya no apunta al stack del proceso muerto
 * 2. No hay referencias colgantes al PCB
 * 3. La tabla se limpia antes del kfree
 * 
 * Con SMP o cleanup diferido, esto necesitaría redesign.
 */
int scheduler_terminate_process(uint32_t pid) {
    if (pid == 0) {
        return E_PERM; // No se puede terminar el proceso idle
    }
    
    if (pid >= MAX_PROCESSES) {
        return E_INVAL; // PID inválido
    }
    
    // Deshabilitar interrupciones durante operaciones críticas
    __asm__ volatile("cli");
    
    process_t* process = process_table[pid];
    if (process == NULL) {
        __asm__ volatile("sti");
        return E_NOENT; // Proceso no existe
    }
    
    // CRÍTICO: Guardar el estado antes de cambiarlo
    process_state_t old_state = process->state;
    
    // Remover de la cola correspondiente ANTES de cambiar el estado
    if (old_state == PROCESS_STATE_READY) {
        scheduler_queue_remove(&ready_queues[process->priority], process);
    } else if (old_state == PROCESS_STATE_BLOCKED) {
        scheduler_queue_remove(&blocked_queue, process);
    }
    // Si está RUNNING, se quitará automáticamente en scheduler_switch
    
    // Ahora sí, cambiar el estado a TERMINATED
    process->state = PROCESS_STATE_TERMINATED;
    
    // Remover de la tabla de procesos ANTES del switch
    // (para que nunca sea seleccionado de nuevo)
    process_table[pid] = NULL;
    total_processes--;
    
    // Si es el proceso actual, hacer context switch
    // WARNING: scheduler_switch() NUNCA retorna en el contexto del proceso muerto
    // El cleanup (kfree) se ejecutará en el contexto del nuevo proceso
    bool is_current = (process == current_process);
    if (is_current) {
        // Las interrupciones se rehabilitan en scheduler_switch
        scheduler_switch();
        // NUNCA llegamos aquí si era current_process
    }
    
    // Liberar memoria (solo si NO era current_process, o si lo era, esto
    // se ejecuta en el nuevo contexto)
    if (process->kernel_stack != 0) {
        kfree((void*)process->kernel_stack);
    }
    
    // Liberar el PCB
    kfree(process);
    
    // Restaurar interrupciones si no hicimos switch
    if (!is_current) {
        __asm__ volatile("sti");
    }
    
    return E_OK;
}

/**
 * Seleccionar el siguiente proceso a ejecutar (Weighted Round Robin)
 * 
 * ALGORITMO MEJORADO:
 * Usa un sistema de pesos para dar más tiempo de CPU a procesos de mayor prioridad
 * sin causar starvation de los de baja prioridad.
 * 
 * Distribución aproximada de CPU:
 * - REALTIME: ~53% del tiempo de CPU (8/15)
 * - HIGH:     ~27% del tiempo de CPU (4/15)
 * - NORMAL:   ~13% del tiempo de CPU (2/15)
 * - LOW:      ~7%  del tiempo de CPU (1/15)
 * - IDLE:     Solo cuando no hay nadie más
 * 
 * Cada prioridad también tiene un quantum diferente:
 * - Mayor prioridad = quantum más largo por selección
 * - Menor prioridad = quantum más corto por selección
 */
process_t* scheduler_select_next(void) {
    // Intentar seleccionar procesos por peso de prioridad
    // Empezamos desde REALTIME y bajamos, pero respetando los pesos
    
    // Ciclar por las prioridades de mayor a menor
    for (int priority = PROCESS_PRIORITY_REALTIME; priority >= PROCESS_PRIORITY_LOW; priority--) {
        if (ready_queues[priority].count > 0) {
            // Verificar si esta prioridad puede ser seleccionada según su peso
            uint32_t weight = priority_weights[priority];
            
            // Si el contador es menor que el peso, seleccionar de esta cola
            if (priority_counters[priority] < weight) {
                priority_counters[priority]++;
                process_t* next = scheduler_queue_pop(&ready_queues[priority]);
                
                // Reset de contadores si alcanzamos el ciclo completo
                uint32_t total = 0;
                for (int i = PROCESS_PRIORITY_LOW; i <= PROCESS_PRIORITY_REALTIME; i++) {
                    total += priority_counters[i];
                }
                if (total >= 15) { // 1+2+4+8 = 15 (ciclo completo)
                    for (int i = 0; i < 5; i++) {
                        priority_counters[i] = 0;
                    }
                }
                
                return next;
            }
        }
    }
    
    // Si ningún peso permite selección, hacer round-robin simple
    // (fallback en caso de que todos los contadores estén saturados)
    for (int priority = PROCESS_PRIORITY_REALTIME; priority >= PROCESS_PRIORITY_LOW; priority--) {
        if (ready_queues[priority].count > 0) {
            // Reset contadores y seleccionar
            for (int i = 0; i < 5; i++) {
                priority_counters[i] = 0;
            }
            process_t* next = scheduler_queue_pop(&ready_queues[priority]);
            return next;
        }
    }
    
    // No hay procesos ready (excepto idle), retornar idle
    if (ready_queues[PROCESS_PRIORITY_IDLE].count > 0) {
        return scheduler_queue_pop(&ready_queues[PROCESS_PRIORITY_IDLE]);
    }
    
    return idle_process;
}

/**
 * Realizar un context switch
 * Guarda el contexto del proceso actual y carga el contexto del nuevo proceso
 */
void scheduler_switch(void) {
    if (!scheduler_initialized || current_process == NULL) {
        return;
    }
    
    // Deshabilitar interrupciones durante el context switch
    __asm__ volatile("cli");
    
    // Guardar el estado del proceso actual
    process_t* old_process = current_process;
    
    // CRÍTICO: Solo reencolar si el proceso está RUNNING y no fue bloqueado/terminado
    // Esto previene bugs cuando un proceso se bloquea o termina justo antes del switch
    if (old_process->state == PROCESS_STATE_RUNNING) {
        // El proceso todavía está activo, moverlo a READY
        old_process->state = PROCESS_STATE_READY;
        scheduler_queue_add(&ready_queues[old_process->priority], old_process);
    }
    // Si state != RUNNING, significa que:
    // - BLOCKED: ya está en blocked_queue (o será agregado por quien lo bloqueó)
    // - TERMINATED: ya fue removido de todas las colas
    // - READY: no debería pasar, pero no hacemos nada
    
    // Seleccionar el siguiente proceso
    process_t* next_process = scheduler_select_next();
    
    if (next_process == NULL) {
        // No hay procesos listos, usar el idle
        next_process = idle_process;
    }
    
    // Si el siguiente proceso es el mismo, no hacer nada
    if (next_process == old_process) {
        next_process->state = PROCESS_STATE_RUNNING;
        __asm__ volatile("sti");
        return;
    }
    
    // Cambiar al nuevo proceso
    next_process->state = PROCESS_STATE_RUNNING;
    next_process->ticks_remaining = get_quantum_for_priority(next_process->priority);
    next_process->time_slices++;
    
    current_process = next_process;
    
    // TODO: Cambiar el page directory si es necesario (cuando esté implementado VMM)
    // if (next_process->page_directory != 0) {
    //     switch_page_directory(next_process->page_directory);
    // }
    
    // Realizar el context switch en assembly
    // Esta función guarda el ESP del proceso actual y carga el ESP del nuevo proceso
    // Al retornar, estaremos ejecutando el nuevo proceso
    switch_context(&old_process->esp, next_process->esp);
    
    // Cuando volvamos aquí, ya estaremos en el contexto del proceso que fue cambiado
    // Habilitar interrupciones de nuevo
    __asm__ volatile("sti");
}

/**
 * Tick del scheduler (llamado por el timer)
 * IMPORTANTE: Esta función se llama desde una IRQ, debe ser rápida
 * 
 * RACE CONDITION WARNING:
 * Esta función ya está dentro de una IRQ (interrupciones deshabilitadas),
 * pero cuando se agregue SMP, necesitará spinlocks.
 */
void scheduler_tick(void) {
    if (!scheduler_initialized || current_process == NULL) {
        return;
    }
    
    // CRITICAL: No hacer nada si el proceso actual está bloqueado o terminado
    // Solo decrementar quantum para procesos RUNNING
    if (current_process->state != PROCESS_STATE_RUNNING) {
        return;
    }
    
    // Decrementar el quantum del proceso actual
    if (current_process->ticks_remaining > 0) {
        current_process->ticks_remaining--;
    }
    
    // Si se acabó el quantum, hacer context switch
    if (current_process->ticks_remaining == 0) {
        scheduler_switch();
    }
}

/**
 * Ceder voluntariamente la CPU (yield)
 */
void scheduler_yield(void) {
    if (!scheduler_initialized) {
        return;
    }
    
    // Resetear el quantum del proceso actual
    current_process->ticks_remaining = 0;
    
    // Hacer context switch
    scheduler_switch();
}

/**
 * Bloquear el proceso actual
 * RACE CONDITION: Protegido por el hecho de que ya estamos en el proceso
 */
void scheduler_block_current(void) {
    if (!scheduler_initialized || current_process == NULL) {
        return;
    }
    
    // Deshabilitar interrupciones durante cambio de estado
    __asm__ volatile("cli");
    
    // CRÍTICO: Cambiar estado ANTES de hacer switch
    // Así scheduler_switch NO lo reencola en ready_queue
    current_process->state = PROCESS_STATE_BLOCKED;
    scheduler_queue_add(&blocked_queue, current_process);
    
    // Hacer context switch a otro proceso
    // scheduler_switch verá que state != RUNNING y no lo reencolará
    // Las interrupciones se rehabilitan en scheduler_switch
    scheduler_switch();
}

/**
 * Desbloquear un proceso
 * RACE CONDITION: Con SMP necesitará locks
 */
int scheduler_unblock_process(uint32_t pid) {
    if (pid >= MAX_PROCESSES) {
        return E_INVAL; // PID inválido
    }
    
    // Deshabilitar interrupciones durante operaciones de cola
    __asm__ volatile("cli");
    
    process_t* process = process_table[pid];
    if (process == NULL) {
        __asm__ volatile("sti");
        return E_NOENT;
    }
    
    if (process->state != PROCESS_STATE_BLOCKED) {
        __asm__ volatile("sti");
        return E_INVAL; // El proceso no está bloqueado
    }
    
    // Remover de la cola de bloqueados
    scheduler_queue_remove(&blocked_queue, process);
    
    // Cambiar el estado a READY y agregarlo a su cola de prioridad
    process->state = PROCESS_STATE_READY;
    scheduler_queue_add(&ready_queues[process->priority], process);
    
    __asm__ volatile("sti");
    return E_OK;
}

/**
 * Cambiar la prioridad de un proceso
 */
int scheduler_set_priority(uint32_t pid, process_priority_t new_priority) {
    if (pid == 0) {
        return E_PERM; // No se puede cambiar la prioridad del proceso idle
    }
    
    if (pid >= MAX_PROCESSES) {
        return E_INVAL; // PID inválido
    }
    
    process_t* process = process_table[pid];
    if (process == NULL) {
        return E_NOENT;
    }
    
    if (new_priority < PROCESS_PRIORITY_IDLE || new_priority > PROCESS_PRIORITY_REALTIME) {
        return E_INVAL;
    }
    
    process_priority_t old_priority = process->priority;
    
    // Si el proceso está en READY, moverlo de cola
    if (process->state == PROCESS_STATE_READY) {
        scheduler_queue_remove(&ready_queues[old_priority], process);
        process->priority = new_priority;
        scheduler_queue_add(&ready_queues[new_priority], process);
    } else {
        // Solo cambiar la prioridad
        process->priority = new_priority;
    }
    
    return E_OK;
}

/**
 * Obtener la prioridad de un proceso
 */
int scheduler_get_priority(uint32_t pid) {
    process_t* process = process_table[pid];
    if (process == NULL) {
        return -1;
    }
    return process->priority;
}

/**
 * Obtener el proceso actual
 */
process_t* scheduler_get_current_process(void) {
    return current_process;
}

/**
 * Obtener un proceso por su PID
 */
process_t* scheduler_get_process_by_pid(uint32_t pid) {
    if (pid >= MAX_PROCESSES) {
        return NULL;
    }
    return process_table[pid];
}

/**
 * Obtener el número total de procesos
 */
uint32_t scheduler_get_process_count(void) {
    return total_processes;
}

/**
 * Listar todos los procesos (para debug)
 */
void scheduler_list_processes(void) {
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_write("\n=== Lista de Procesos ===\n");
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_write("PID  Nombre              Estado    Prioridad  Time Slices\n");
    vga_write("---  ------------------  --------  ---------  -----------\n");
    
    for (uint32_t i = 0; i < MAX_PROCESSES; i++) {
        process_t* proc = process_table[i];
        if (proc != NULL) {
            // PID
            if (proc->pid < 10) {
                vga_write("  ");
            } else if (proc->pid < 100) {
                vga_write(" ");
            }
            vga_write_dec(proc->pid);
            vga_write("  ");
            
            // Nombre (máximo 18 caracteres)
            char name_padded[19];
            strncpy(name_padded, proc->name, 18);
            name_padded[18] = '\0';
            vga_write(name_padded);
            for (int j = strlen(name_padded); j < 18; j++) {
                vga_write(" ");
            }
            vga_write("  ");
            
            // Estado
            const char* state_str;
            switch (proc->state) {
                case PROCESS_STATE_READY:      state_str = "READY   "; break;
                case PROCESS_STATE_RUNNING:    state_str = "RUNNING "; break;
                case PROCESS_STATE_BLOCKED:    state_str = "BLOCKED "; break;
                case PROCESS_STATE_TERMINATED: state_str = "TERM    "; break;
                default:                       state_str = "UNKNOWN "; break;
            }
            vga_write(state_str);
            vga_write("  ");
            
            // Prioridad
            vga_write_dec(proc->priority);
            vga_write("          ");
            
            // Time slices
            vga_write_dec(proc->time_slices);
            vga_write("\n");
        }
    }
    
    vga_write("\nTotal de procesos: ");
    vga_write_dec(total_processes);
    vga_write("\n\n");
}
