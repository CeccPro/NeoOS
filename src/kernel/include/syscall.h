/**
 * @file syscall.h
 * @brief Definiciones de las llamadas al sistema de NeoOS
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

#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>
#include <stddef.h>
#include "errors.h"

// Números de syscalls
#define SYS_EXIT         0
#define SYS_READ         1
#define SYS_WRITE        2
#define SYS_OPEN         3
#define SYS_CLOSE        4
#define SYS_GETPID       5
#define SYS_FORK         6
#define SYS_YIELD        7
#define SYS_SLEEP        8
#define SYS_GETTIME      9
#define SYS_UPTIME       10
#define SYS_MMAP         11
#define SYS_MUNMAP       12
#define SYS_SBRK         13
#define SYS_IPC_SEND     14
#define SYS_IPC_RECV     15
#define SYS_IPC_FREE     16
#define SYS_SETPRIORITY  17
#define SYS_GETPRIORITY  18
#define SYS_KILL         19
#define SYS_WAIT         20
#define SYS_CLONE        21
#define SYS_RUN          22
#define SYS_SIGNAL       23
#define SYS_STAT         24
#define SYS_MKDIR        25
#define SYS_RMDIR        26
#define SYS_CHDIR        27
#define SYS_UNLINK       28

/**
 * @brief Inicializa el sistema de syscalls
 * @return E_OK si fue exitoso, código de error en caso contrario
 */
error_t syscall_init(void);

/**
 * @brief Handler de syscalls
 * Esta función es llamada desde el interrupt handler cuando ocurre una syscall
 * @param syscall_num Número de la syscall
 * @param arg1 Primer argumento
 * @param arg2 Segundo argumento
 * @param arg3 Tercer argumento
 * @param arg4 Cuarto argumento
 * @param arg5 Quinto argumento
 * @return Valor de retorno de la syscall
 */
int32_t syscall_handler(uint32_t syscall_num, uint32_t arg1, uint32_t arg2,
                        uint32_t arg3, uint32_t arg4, uint32_t arg5);

#endif // SYSCALL_H
