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
// $Id: lnxtimer.h,v 3.0 2016/09/07 13:15:39 simon Exp $
// $Source: /home/simon/CVS/src/cpu/mico32/src/lnxtimer.h,v $
//
//=============================================================

#ifndef _LNXTIMER_H_
#define _LNXTIMER_H_

// -------------------------------------------------------------------------
// INCLUDES
// -------------------------------------------------------------------------

#include "lm32_cpu.h"

// -------------------------------------------------------------------------
// PUBLIC DEFINES
// -------------------------------------------------------------------------

#define LM32_TIMER_NUM_REGS            4
#define LM32_MAX_NUM_TIMERS            4

// -------------------------------------------------------------------------
// PUBLIC TYPE DEFINITIONS
// -------------------------------------------------------------------------

typedef struct
{
    uint32_t    lm32_timer_regs[LM32_MAX_NUM_TIMERS][LM32_TIMER_NUM_REGS];
    bool        int_pending[LM32_MAX_NUM_TIMERS];
    lm32_time_t start_time[LM32_MAX_NUM_TIMERS];
} lm32_timer_state_t;

// -------------------------------------------------------------------------
// PUBLIC TYPE DEFINITIONS
// -------------------------------------------------------------------------

extern void               lm32_timer_write     (const uint32_t address, const uint32_t data, const int cntx = 0);
extern void               lm32_timer_read      (const uint32_t address, uint32_t* data, const int cntx = 0);
extern bool               lm32_timer_tick      (const lm32_time_t time, const int cntx = 0);
extern lm32_timer_state_t lm32_get_timer_state (void);
extern void               lm32_set_timer_state (const lm32_timer_state_t state);

#endif
