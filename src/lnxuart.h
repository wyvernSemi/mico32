//=============================================================
// 
// Copyright (c) 2016 Simon Southwell. All rights reserved.
//
// Date: 24th August 2016
//
// Header for a model of a timer, based on the Lattice implementation.
//
// This file is part of the lnxmico32 ISS linux system model.
//
// lnxmico32 is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// lnxmico32 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with lnxmico32. If not, see <http://www.gnu.org/licenses/>.
//
// $Id: lnxuart.h,v 2.4 2016-09-03 07:44:05 simon Exp $
// $Source: /home/simon/CVS/src/cpu/mico32/src/lnxuart.h,v $
//
//=============================================================

#ifndef _LNXUART_H_
#define _LNXUART_H_

// -------------------------------------------------------------------------
// INCLUDES
// -------------------------------------------------------------------------

#include "lm32_cpu.h"

// -------------------------------------------------------------------------
// PUBLIC DEFINES
// -------------------------------------------------------------------------

// The Lattice UART has 4 registers
#define LM32_UART_NUM_REGS                   8

// Up to 4 UART contexts supported
#define LM32_MAX_NUM_UARTS                   4

// -------------------------------------------------------------------------
// PUBLIC TYPE DEFINITIONS
// -------------------------------------------------------------------------

typedef struct
{
    lm32_time_t start_time[LM32_MAX_NUM_UARTS];
    bool        int_pending[LM32_MAX_NUM_UARTS];
    uint32_t    lm32_uart_regs[LM32_MAX_NUM_UARTS][LM32_UART_NUM_REGS];
} lm32_uart_state_t;

// -------------------------------------------------------------------------
// PUBLIC TYPE DEFINITIONS
// -------------------------------------------------------------------------

extern void              lm32_uart_write     (const uint32_t address, const uint32_t data, const int cntx = 0);
extern void              lm32_uart_read      (const uint32_t address, uint32_t* data, const int cntx = 0);
extern bool              lm32_uart_tick      (const lm32_time_t time, bool &terminate, const bool kbd_connected = false, const int cntx = 0);
extern lm32_uart_state_t lm32_get_uart_state (void);
extern void              lm32_set_uart_state (const lm32_uart_state_t state);
#endif
