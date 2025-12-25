/**
 * @file scheduler.h
 * @brief Planificador de procesos de NeoOS
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

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "process.h"
#include "errors.h"

/**
 * @brief Inicializa el planificador de procesos
 * @return E_OK si fue exitoso, código de error en caso contrario
 */
error_t scheduler_init(void);

/**
 * @brief Crea un nuevo proceso
 * @param name Nombre del proceso
 * @param entry_point Dirección de inicio del proceso
 * @param priority Prioridad del proceso
 * @param pid_out Puntero donde se almacenará el PID asignado
 * @return E_OK si fue exitoso, código de error en caso contrario
 */
error_t scheduler_create_process(const char* name, void* entry_point, 
                                  priority_t priority, pid_t* pid_out);

/**
 * @brief Inicia la ejecución de un proceso
 * @param pid ID del proceso a iniciar
 * @return E_OK si fue exitoso, código de error en caso contrario
 */
error_t scheduler_start_process(pid_t pid);

/**
 * @brief Termina un proceso
 * @param pid ID del proceso a terminar
 * @param exit_code Código de salida del proceso
 * @return E_OK si fue exitoso, código de error en caso contrario
 */
error_t scheduler_terminate_process(pid_t pid, int exit_code);

/**
 * @brief Cambia al siguiente proceso listo para ejecutarse
 * Esta función es llamada por el timer interrupt
 */
void scheduler_schedule(void);

/**
 * @brief Obtiene el proceso actual en ejecución
 * @return Puntero al PCB del proceso actual o NULL si no hay ninguno
 */
process_t* scheduler_get_current_process(void);

/**
 * @brief Obtiene un proceso por su PID
 * @param pid ID del proceso
 * @return Puntero al PCB del proceso o NULL si no existe
 */
process_t* scheduler_get_process(pid_t pid);

/**
 * @brief Cambia la prioridad de un proceso
 * @param pid ID del proceso
 * @param new_priority Nueva prioridad
 * @return E_OK si fue exitoso, código de error en caso contrario
 */
error_t scheduler_set_priority(pid_t pid, priority_t new_priority);

/**
 * @brief Bloquea el proceso actual
 * El proceso será puesto en estado BLOCKED hasta que se llame scheduler_unblock
 */
void scheduler_block_current(void);

/**
 * @brief Desbloquea un proceso
 * @param pid ID del proceso a desbloquear
 * @return E_OK si fue exitoso, código de error en caso contrario
 */
error_t scheduler_unblock(pid_t pid);

#endif // SCHEDULER_H
