/**
 * NeoOS - PIT Implementation
 * Implementación del Programmable Interval Timer
 */

#include "../../core/include/timer.h"
#include "../../core/include/interrupts.h"
#include "../../core/include/scheduler.h"
#include "../../drivers/include/early_vga.h"

// Variables globales del timer
volatile uint32_t timer_ticks = 0;      // Contador de ticks desde el inicio
volatile uint32_t timer_seconds = 0;    // Segundos desde el inicio
static uint32_t timer_frequency = 0;    // Frecuencia actual del timer en Hz

/**
 * Handler de la interrupción del timer (IRQ0)
 * Se llama cada vez que el PIT genera una interrupción
 */
void timer_handler(registers_t* regs) {
    // Evitar warning de parámetro no usado
    (void)regs;
    
    // Incrementar el contador de ticks
    timer_ticks++;
    
    // Cada 'timer_frequency' ticks = 1 segundo
    if (timer_ticks % timer_frequency == 0) {
        timer_seconds++;
    }
    
    // Llamar al scheduler para realizar context switching si es necesario
    scheduler_tick();
}

/**
 * Inicializar el PIT (Programmable Interval Timer)
 * @param frequency: Frecuencia en Hz (por defecto 100 Hz = 10ms por tick)
 * @param verbose: Si es true, muestra mensajes de debug
 */
void timer_init(uint32_t frequency, bool verbose) {
    if (verbose) {
        vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
        vga_write("[TIMER] Inicializando PIT...\n");
    }
    
    // Guardar la frecuencia configurada
    timer_frequency = frequency;
    
    // Calcular el divisor para la frecuencia deseada
    // divisor = PIT_FREQUENCY / frecuencia_deseada
    uint32_t divisor = PIT_FREQUENCY / frequency;
    
    if (verbose) {
        vga_write("[TIMER] Frecuencia: ");
        vga_write_dec(frequency);
        vga_write(" Hz (");
        vga_write_dec(1000 / frequency);
        vga_write("ms por tick)\n");
        vga_write("[TIMER] Divisor: ");
        vga_write_dec(divisor);
        vga_write("\n");
    }
    
    // Enviar el comando al PIT
    // Canal 0, acceso low/high byte, modo 3 (generador de onda cuadrada)
    uint8_t command = PIT_CHANNEL0 | PIT_ACCESS_LOHI | PIT_MODE_SQUARE;
    __asm__ volatile("outb %0, %1" : : "a"(command), "Nd"(PIT_COMMAND));
    
    // Enviar el divisor
    // Primero el byte bajo
    uint8_t low = (uint8_t)(divisor & 0xFF);
    __asm__ volatile("outb %0, %1" : : "a"(low), "Nd"(PIT_CHANNEL0_DATA));
    
    // Luego el byte alto
    uint8_t high = (uint8_t)((divisor >> 8) & 0xFF);
    __asm__ volatile("outb %0, %1" : : "a"(high), "Nd"(PIT_CHANNEL0_DATA));
    
    // Registrar el handler del timer en IRQ0 (interrupción 32)
    interrupts_register_handler(IRQ0, timer_handler);
    
    if (verbose) {
        vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
        vga_write("[TIMER] PIT inicializado correctamente\n");
    }
}

/**
 * Obtener el número de ticks transcurridos desde el inicio
 * @return Número de ticks
 */
uint32_t timer_get_ticks(void) {
    return timer_ticks;
}

/**
 * Obtener el número de segundos transcurridos desde el inicio
 * @return Número de segundos
 */
uint32_t timer_get_seconds(void) {
    return timer_seconds;
}

/**
 * Obtener el tiempo en milisegundos desde el inicio
 * @return Tiempo en milisegundos
 */
uint32_t timer_get_ms(void) {
    // ms = (ticks * 1000) / frequency
    return (timer_ticks * 1000) / timer_frequency;
}

/**
 * Esperar un número específico de ticks
 * Implementa un busy-wait loop
 * @param ticks: Número de ticks a esperar
 */
void timer_wait_ticks(uint32_t ticks) {
    uint32_t end_ticks = timer_ticks + ticks;
    while (timer_ticks < end_ticks) {
        __asm__ volatile("hlt"); // Esperar a la siguiente interrupción
    }
}

/**
 * Esperar un número específico de milisegundos
 * @param ms: Milisegundos a esperar
 */
void timer_wait_ms(uint32_t ms) {
    // Convertir ms a ticks: ticks = (ms * frequency) / 1000
    uint32_t ticks = (ms * timer_frequency) / 1000;
    if (ticks == 0) ticks = 1; // Al menos 1 tick
    timer_wait_ticks(ticks);
}
