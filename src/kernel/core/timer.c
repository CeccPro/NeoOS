/**
 * @file timer.c
 * @brief Implementación del driver del timer del sistema
 * 
 * Copyright (C) 2025 CeccPro
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "../include/timer.h"
#include "../include/kernel.h"
#include "../include/idt.h"

// Puertos del PIT
#define PIT_CHANNEL0 0x40  // Canal 0 (IRQ0 - timer del sistema)
#define PIT_COMMAND  0x43  // Puerto de comandos

// Comandos del PIT
#define PIT_SET_CHANNEL0 0x36  // Canal 0, lobyte/hibyte, rate generator

// Variables globales
static volatile uint64_t system_ticks = 0;
static uint32_t timer_freq = 0;
static uint32_t ms_per_tick = 0;

/**
 * @brief Handler de interrupción del timer
 */
void timer_handler(void) {
    system_ticks++;
}

/**
 * @brief Inicializa el timer del sistema
 */
void timer_init(uint32_t frequency) {
    // Guardar la frecuencia configurada
    timer_freq = frequency;
    ms_per_tick = 1000 / frequency;
    
    // Calcular el divisor para la frecuencia deseada
    uint32_t divisor = PIT_FREQUENCY / frequency;
    
    // Enviar el comando de inicialización
    outb(PIT_COMMAND, PIT_SET_CHANNEL0);
    
    // Enviar el divisor (primero el byte bajo, luego el alto)
    outb(PIT_CHANNEL0, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL0, (uint8_t)((divisor >> 8) & 0xFF));
    
    // Registrar el handler de interrupción del timer (IRQ0 = interrupción 32)
    // Esto se manejará en el idt.c
}

/**
 * @brief Obtiene el número de ticks desde el arranque
 */
uint64_t timer_get_ticks(void) {
    return system_ticks;
}

/**
 * @brief Obtiene el tiempo transcurrido en milisegundos
 */
uint64_t timer_get_ms(void) {
    return system_ticks * ms_per_tick;
}

/**
 * @brief Obtiene el tiempo transcurrido en segundos
 */
uint64_t timer_get_seconds(void) {
    // Evitar división de 64 bits - convertir a 32 bits primero
    return (uint32_t)(system_ticks & 0xFFFFFFFF) / timer_freq;
}

/**
 * @brief Espera un número específico de ticks
 */
void timer_wait(uint32_t ticks) {
    uint64_t target = system_ticks + ticks;
    while (system_ticks < target) {
        asm volatile("hlt");  // Esperar a la siguiente interrupción
    }
}

/**
 * @brief Espera un número específico de milisegundos
 */
void timer_sleep(uint32_t ms) {
    uint32_t ticks = (ms * timer_freq) / 1000;
    timer_wait(ticks);
}
