/**
 * @file timer.h
 * @brief Driver del timer del sistema (PIT - Programmable Interval Timer)
 * 
 * Implementa el temporizador del sistema usando el PIT 8253/8254.
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

#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

// Frecuencia del PIT (Hz)
#define PIT_FREQUENCY 1193182

// Frecuencia deseada del timer (Hz)
#define TIMER_FREQUENCY 100  // 100 Hz = 10 ms por tick

/**
 * @brief Inicializa el timer del sistema
 * 
 * Configura el PIT para generar interrupciones periódicas
 * a la frecuencia especificada.
 * 
 * @param frequency Frecuencia en Hz (interrupciones por segundo)
 */
void timer_init(uint32_t frequency);

/**
 * @brief Obtiene el número de ticks desde el arranque
 * 
 * @return Número de ticks del sistema
 */
uint64_t timer_get_ticks(void);

/**
 * @brief Obtiene el tiempo transcurrido en milisegundos
 * 
 * @return Tiempo en milisegundos desde el arranque
 */
uint64_t timer_get_ms(void);

/**
 * @brief Obtiene el tiempo transcurrido en segundos
 * 
 * @return Tiempo en segundos desde el arranque
 */
uint64_t timer_get_seconds(void);

/**
 * @brief Espera un número específico de ticks
 * 
 * @param ticks Número de ticks a esperar
 */
void timer_wait(uint32_t ticks);

/**
 * @brief Espera un número específico de milisegundos
 * 
 * @param ms Milisegundos a esperar
 */
void timer_sleep(uint32_t ms);

/**
 * @brief Handler de interrupción del timer (llamado por el IDT)
 */
void timer_handler(void);

#endif // TIMER_H
