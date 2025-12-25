/**
 * @file process.h
 * @brief Estructuras y definiciones para la gestión de procesos
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

#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>
#include "errors.h"

// ID de proceso (PID)
typedef uint32_t pid_t;

// Prioridad de proceso
typedef enum {
    PRIORITY_IDLE = 0,
    PRIORITY_LOW = 1,
    PRIORITY_NORMAL = 2,
    PRIORITY_HIGH = 3,
    PRIORITY_REALTIME = 4
} priority_t;

// Estado del proceso
typedef enum {
    PROC_STATE_CREATED,    // Creado pero no iniciado
    PROC_STATE_READY,      // Listo para ejecutarse
    PROC_STATE_RUNNING,    // En ejecución
    PROC_STATE_BLOCKED,    // Bloqueado esperando I/O u otro recurso
    PROC_STATE_TERMINATED  // Terminado
} proc_state_t;

// Estructura del contexto del proceso (registros)
typedef struct {
    uint32_t eax, ebx, ecx, edx;
    uint32_t esi, edi, ebp, esp;
    uint32_t eip;
    uint32_t eflags;
    uint32_t cs, ds, es, fs, gs, ss;
} process_context_t;

// Estructura de control de proceso (PCB)
typedef struct process {
    pid_t pid;                      // ID del proceso
    pid_t parent_pid;               // ID del proceso padre
    char name[64];                  // Nombre del proceso
    
    proc_state_t state;             // Estado actual
    priority_t priority;            // Prioridad
    
    process_context_t context;      // Contexto (registros)
    
    void* page_directory;           // Directorio de páginas (memoria virtual)
    void* kernel_stack;             // Pila del kernel para este proceso
    void* user_stack;               // Pila del usuario
    
    uint64_t cpu_time;              // Tiempo de CPU usado
    uint64_t start_time;            // Tiempo de inicio
    
    struct process* next;           // Siguiente proceso en la lista
} process_t;

#endif // PROCESS_H
