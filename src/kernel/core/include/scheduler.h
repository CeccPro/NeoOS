/**
 * NeoOS - Process Scheduler
 * Planificador de procesos con algoritmo Round Robin y prioridades
 */

#ifndef _KERNEL_SCHEDULER_H
#define _KERNEL_SCHEDULER_H

#include "../../lib/include/types.h"
#include "interrupts.h"

/**
 * Estados de un proceso
 */
typedef enum {
    PROCESS_STATE_READY,      // Listo para ejecutar
    PROCESS_STATE_RUNNING,    // Ejecutándose actualmente
    PROCESS_STATE_BLOCKED,    // Bloqueado esperando un recurso
    PROCESS_STATE_TERMINATED  // Terminado
} process_state_t;

/**
 * Niveles de prioridad de un proceso
 * Mayor número = mayor prioridad
 */
typedef enum {
    PROCESS_PRIORITY_IDLE = 0,    // Prioridad más baja (proceso idle)
    PROCESS_PRIORITY_LOW = 1,     // Prioridad baja
    PROCESS_PRIORITY_NORMAL = 2,  // Prioridad normal (por defecto)
    PROCESS_PRIORITY_HIGH = 3,    // Prioridad alta
    PROCESS_PRIORITY_REALTIME = 4 // Prioridad tiempo real (máxima)
} process_priority_t;

/**
 * Número máximo de procesos simultáneos
 */
#define MAX_PROCESSES 256

/**
 * Tamaño del stack del kernel para cada proceso (4KB)
 */
#define KERNEL_STACK_SIZE 4096

/**
 * Quantum del scheduler por nivel de prioridad (en ticks del timer)
 * Con 100 Hz: cada tick = 10ms
 * 
 * REALTIME: 10 ticks = 100ms (máximo tiempo de CPU)
 * HIGH:     7 ticks = 70ms
 * NORMAL:   5 ticks = 50ms (estándar)
 * LOW:      3 ticks = 30ms
 * IDLE:     1 tick  = 10ms (mínimo)
 */
#define QUANTUM_REALTIME  10
#define QUANTUM_HIGH      7
#define QUANTUM_NORMAL    5
#define QUANTUM_LOW       3
#define QUANTUM_IDLE      1

/**
 * Process Control Block (PCB)
 * Estructura que contiene toda la información de un proceso
 */
typedef struct process {
    uint32_t pid;                    // Process ID
    char name[32];                   // Nombre del proceso
    process_state_t state;           // Estado actual del proceso
    process_priority_t priority;     // Prioridad del proceso
    
    // Contexto del CPU (registros guardados)
    uint32_t esp;                    // Stack pointer
    uint32_t ebp;                    // Base pointer
    uint32_t ebx;                    // Registros de propósito general
    uint32_t esi;
    uint32_t edi;
    uint32_t eflags;                 // Flags del CPU
    uint32_t eip;                    // Instruction pointer
    
    // Información de memoria
    uint32_t page_directory;         // Directorio de páginas (para VMM)
    uint32_t kernel_stack;           // Puntero al stack del kernel
    
    // Estadísticas
    uint32_t time_slices;            // Número de time slices usados
    uint32_t ticks_remaining;        // Ticks restantes en el quantum actual
    
    // Enlaces para la lista de procesos
    struct process* next;            // Siguiente proceso en la cola
    struct process* prev;            // Proceso anterior en la cola
} process_t;

/**
 * Cola de procesos
 * Estructura para manejar listas de procesos
 */
typedef struct {
    process_t* head;                 // Primer proceso en la cola
    process_t* tail;                 // Último proceso en la cola
    uint32_t count;                  // Número de procesos en la cola
} process_queue_t;

/**
 * Variables globales del scheduler
 */
extern process_t* current_process;   // Proceso actualmente en ejecución
extern process_t* idle_process;      // Proceso idle (ejecutado cuando no hay otros procesos)

/**
 * Inicializar el scheduler
 * Configura las colas de procesos y crea el proceso idle
 * @param verbose: Si es true, muestra mensajes de debug
 */
void scheduler_init(bool verbose);

/**
 * Crear un nuevo proceso
 * @param name: Nombre del proceso
 * @param entry_point: Dirección de inicio del proceso
 * @param priority: Prioridad del proceso
 * @return PID del proceso creado, o 0 si hubo error
 */
uint32_t scheduler_create_process(const char* name, void (*entry_point)(void), process_priority_t priority);

/**
 * Terminar un proceso
 * @param pid: Process ID del proceso a terminar
 * @return 0 si tuvo éxito, código de error en caso contrario
 */
int scheduler_terminate_process(uint32_t pid);

/**
 * Cambiar la prioridad de un proceso
 * @param pid: Process ID del proceso
 * @param new_priority: Nueva prioridad
 * @return 0 si tuvo éxito, código de error en caso contrario
 */
int scheduler_set_priority(uint32_t pid, process_priority_t new_priority);

/**
 * Obtener la prioridad de un proceso
 * @param pid: Process ID del proceso
 * @return Prioridad del proceso, o -1 si el proceso no existe
 */
int scheduler_get_priority(uint32_t pid);

/**
 * Obtener el proceso actual
 * @return Puntero al PCB del proceso actual
 */
process_t* scheduler_get_current_process(void);

/**
 * Obtener un proceso por su PID
 * @param pid: Process ID del proceso
 * @return Puntero al PCB del proceso, o NULL si no existe
 */
process_t* scheduler_get_process_by_pid(uint32_t pid);

/**
 * Yield: Ceder voluntariamente la CPU
 * El proceso actual pasa al final de su cola de prioridad
 */
void scheduler_yield(void);

/**
 * Tick del scheduler
 * Llamado por el timer en cada interrupción (IRQ0)
 * Decrementa el quantum del proceso actual y realiza context switch si es necesario
 */
void scheduler_tick(void);

/**
 * Realizar un context switch al siguiente proceso
 * Guarda el contexto del proceso actual y carga el del siguiente
 */
void scheduler_switch(void);

/**
 * Bloquear el proceso actual
 * El proceso pasa al estado BLOCKED hasta que se lo desbloquee
 */
void scheduler_block_current(void);

/**
 * Desbloquear un proceso
 * @param pid: Process ID del proceso a desbloquear
 * @return 0 si tuvo éxito, código de error en caso contrario
 */
int scheduler_unblock_process(uint32_t pid);

/**
 * Obtener el número total de procesos
 * @return Número de procesos en el sistema
 */
uint32_t scheduler_get_process_count(void);

/**
 * Listar todos los procesos (para debug)
 * Muestra información de todos los procesos en el sistema
 */
void scheduler_list_processes(void);

/**
 * Proceso idle
 * Ejecutado cuando no hay otros procesos listos
 * Simplemente ejecuta HLT en un loop infinito
 */
void idle_process_entry(void);

/**
 * Funciones internas del scheduler (no deben llamarse directamente)
 */

/**
 * Seleccionar el siguiente proceso a ejecutar
 * Implementa el algoritmo Round Robin con prioridades
 * @return Puntero al siguiente proceso, o NULL si no hay procesos listos
 */
process_t* scheduler_select_next(void);

/**
 * Agregar un proceso a una cola
 * @param queue: Cola de destino
 * @param process: Proceso a agregar
 */
void scheduler_queue_add(process_queue_t* queue, process_t* process);

/**
 * Remover un proceso de una cola
 * @param queue: Cola de origen
 * @param process: Proceso a remover
 */
void scheduler_queue_remove(process_queue_t* queue, process_t* process);

/**
 * Obtener el siguiente proceso de una cola
 * @param queue: Cola de origen
 * @return Puntero al primer proceso de la cola, o NULL si está vacía
 */
process_t* scheduler_queue_pop(process_queue_t* queue);

/**
 * Funciones de context switching en assembly (arch/x86/context_switch.S)
 */

/**
 * Realizar el cambio de contexto entre procesos
 * @param old_esp: Puntero donde guardar el ESP del proceso actual
 * @param new_esp: ESP del nuevo proceso
 */
extern void switch_context(uint32_t* old_esp, uint32_t new_esp);

/**
 * Inicializar el stack de un nuevo proceso
 * @param stack_top: Puntero al tope del stack
 * @param entry_point: Dirección de inicio del proceso
 * @param exit_handler: Función a llamar si el proceso retorna
 * @return ESP inicial del proceso
 */
extern uint32_t init_process_stack(uint32_t stack_top, void (*entry_point)(void), void (*exit_handler)(void));

/**
 * Leer el valor actual del ESP
 * @return Valor del ESP
 */
extern uint32_t read_esp(void);

/**
 * Leer el valor actual del EBP
 * @return Valor del EBP
 */
extern uint32_t read_ebp(void);

/**
 * Leer el valor actual de EFLAGS
 * @return Valor de EFLAGS
 */
extern uint32_t read_eflags(void);

/**
 * Handler para procesos que terminan retornando
 */
void process_exit_handler(void);

#endif /* _KERNEL_SCHEDULER_H */
