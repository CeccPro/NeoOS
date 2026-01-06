/**
 * NeoOS - PIT (Programmable Interval Timer)
 * Sistema de temporizador para generar interrupciones periódicas
 */

#ifndef _KERNEL_TIMER_H
#define _KERNEL_TIMER_H

#include "../../lib/include/types.h"
#include "interrupts.h"

/**
 * Frecuencia base del PIT en Hz
 * El PIT recibe una señal de 1.193182 MHz
 */
#define PIT_FREQUENCY 1193182

/**
 * Puertos del PIT
 */
#define PIT_CHANNEL0_DATA 0x40  // Canal 0 (Timer del sistema)
#define PIT_CHANNEL1_DATA 0x41  // Canal 1 (Refresh de RAM - obsoleto)
#define PIT_CHANNEL2_DATA 0x42  // Canal 2 (Speaker del PC)
#define PIT_COMMAND       0x43  // Registro de comando/modo

/**
 * Comandos del PIT
 */
#define PIT_CHANNEL0       0x00  // Seleccionar canal 0
#define PIT_ACCESS_LOHI    0x30  // Acceso: low byte, luego high byte
#define PIT_MODE_SQUARE    0x06  // Modo 3: generador de onda cuadrada
#define PIT_MODE_RATE_GEN  0x04  // Modo 2: generador de frecuencia

/**
 * Frecuencia del timer por defecto (en Hz)
 * 100 Hz = 10ms por tick (usado comúnmente en sistemas UNIX)
 */
#define TIMER_DEFAULT_FREQUENCY 100

/**
 * Quantum del scheduler (en ticks del timer)
 * Con 100 Hz, 5 ticks = 50ms
 */
#define TIMER_QUANTUM 5

/**
 * Variables globales del timer
 */
extern volatile uint32_t timer_ticks;    // Contador de ticks desde el inicio
extern volatile uint32_t timer_seconds;  // Segundos desde el inicio

/**
 * Inicializar el PIT (Programmable Interval Timer)
 * Configura el PIT para generar interrupciones a la frecuencia especificada
 * @param frequency: Frecuencia en Hz (100 Hz por defecto = 10ms por interrupción)
 * @param verbose: Si es true, muestra mensajes de debug
 */
void timer_init(uint32_t frequency, bool verbose);

/**
 * Obtener el número de ticks transcurridos desde el inicio
 * @return Número de ticks
 */
uint32_t timer_get_ticks(void);

/**
 * Obtener el número de segundos transcurridos desde el inicio
 * @return Número de segundos
 */
uint32_t timer_get_seconds(void);

/**
 * Obtener el tiempo en milisegundos desde el inicio
 * @return Tiempo en milisegundos
 */
uint32_t timer_get_ms(void);

/**
 * Esperar un número específico de ticks
 * Implementa un busy-wait loop
 * @param ticks: Número de ticks a esperar
 */
void timer_wait_ticks(uint32_t ticks);

/**
 * Esperar un número específico de milisegundos
 * @param ms: Milisegundos a esperar
 */
void timer_wait_ms(uint32_t ms);

/**
 * Handler interno del timer (llamado por la IRQ0)
 * No debe ser llamado directamente desde el código del usuario
 */
void timer_handler(registers_t* regs);

#endif /* _KERNEL_TIMER_H */
