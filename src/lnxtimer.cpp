//=============================================================
// 
// Copyright (c) 2016 Simon Southwell. All rights reserved.
//
// Date: 24th August 2016
//
// Model for a timer, based on the Lattice implementation.
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
// $Id: lnxtimer.cpp,v 3.0 2016/09/07 13:15:38 simon Exp $
// $Source: /home/simon/CVS/src/cpu/mico32/src/lnxtimer.cpp,v $
//
//=============================================================

// -------------------------------------------------------------------------
// INCLUDES
// -------------------------------------------------------------------------

#include <stdint.h>
#include "lm32_cpu.h"
#include "lnxtimer.h"

// -------------------------------------------------------------------------
// DEFINES
// -------------------------------------------------------------------------

#define LM32_TIMER_STATUS_REG          0x00
#define LM32_TIMER_CONTROL_REG         0x04
#define LM32_TIMER_PERIOD_REG          0x08
#define LM32_TIMER_SNAPSHOT_REG        0x0C

#define LM32_TIMER_STATUS_WR_MASK      0x01
#define LM32_TIMER_CONTROL_WR_MASK     0x0f

#define LM32_TIMER_CONTROL_RD_MASK     0x03

#define LM32_TIMER_STATUS_TO_BIT       0x01
#define LM32_TIMER_STATUS_RUN_BIT      0x02

#define LM32_TIMER_CONTROL_ITO_BIT     0x01
#define LM32_TIMER_CONTROL_CONT_BIT    0x02
#define LM32_TIMER_CONTROL_START_BIT   0x04
#define LM32_TIMER_CONTROL_STOP_BIT    0x08

// -------------------------------------------------------------------------
// LOCAL STATIC STATE
// -------------------------------------------------------------------------

static lm32_timer_state_t timer_state = {
    {{0, 0, 0, 0}, 
     {0, 0, 0, 0}, 
     {0, 0, 0, 0}, 
     {0, 0, 0, 0}},
    {false, false, false, false},
    {0, 0, 0, 0}
};

// -------------------------------------------------------------------------
// lm32_timer_write()
//
// Timer model register write function.
//
// -------------------------------------------------------------------------

void lm32_timer_write(const uint32_t address, const uint32_t data, int cntx)
{
    switch(address)
    {
    case LM32_TIMER_STATUS_REG:
        timer_state.lm32_timer_regs[cntx][LM32_TIMER_STATUS_REG >> 2] &= (data & LM32_TIMER_STATUS_TO_BIT) | ~LM32_TIMER_STATUS_TO_BIT;

        // If the TO bit gets cleared, clear the pending interrupt state
        if (!(timer_state.lm32_timer_regs[cntx][LM32_TIMER_STATUS_REG >> 2] & LM32_TIMER_STATUS_TO_BIT))
        {
            timer_state.int_pending[cntx] = false;
        }

        break;
    case LM32_TIMER_CONTROL_REG:
        timer_state.lm32_timer_regs[cntx][LM32_TIMER_CONTROL_REG >> 2] = data & LM32_TIMER_CONTROL_WR_MASK;
        break;
    case LM32_TIMER_PERIOD_REG:
        timer_state.lm32_timer_regs[cntx][LM32_TIMER_PERIOD_REG >> 2]   = data;
        timer_state.lm32_timer_regs[cntx][LM32_TIMER_SNAPSHOT_REG >> 2] = data;
        break;
    case LM32_TIMER_SNAPSHOT_REG:
    default:
        break;
    }
}

// -------------------------------------------------------------------------
// lm32_timer_read()
//
// Timer model register read function.
//
// -------------------------------------------------------------------------

void lm32_timer_read(const uint32_t address, uint32_t* data, int cntx)
{
    *data = 0;
    switch(address)
    {
    case LM32_TIMER_STATUS_REG:
        *data = timer_state.lm32_timer_regs[cntx][LM32_TIMER_STATUS_REG >> 2];
        break;
    case LM32_TIMER_CONTROL_REG:
        *data = timer_state.lm32_timer_regs[cntx][LM32_TIMER_CONTROL_REG >> 2];
        break;
    case LM32_TIMER_PERIOD_REG:
        *data = timer_state.lm32_timer_regs[cntx][LM32_TIMER_PERIOD_REG >> 2];
        break;
    case LM32_TIMER_SNAPSHOT_REG:
        *data = timer_state.lm32_timer_regs[cntx][LM32_TIMER_SNAPSHOT_REG >> 2];
        break;
    default:
        break;
    }
}

// -------------------------------------------------------------------------
// lm32_timer_tick()
//
// Timer model tick/interrupt function. Called regularly, with time stamp
// and generates interrupts, and updates register state.
//
// -------------------------------------------------------------------------

bool lm32_timer_tick(const lm32_time_t time, const int cntx)
{
    bool irq = false;

    // If not running, and start bit set...
    if (!(timer_state.lm32_timer_regs[cntx][LM32_TIMER_STATUS_REG >> 2] & LM32_TIMER_STATUS_RUN_BIT) && 
         (timer_state.lm32_timer_regs[cntx][LM32_TIMER_CONTROL_REG >> 2] & LM32_TIMER_CONTROL_START_BIT))
    {
        // Clear the start bit
        timer_state.lm32_timer_regs[cntx][LM32_TIMER_CONTROL_REG >> 2] &= ~LM32_TIMER_CONTROL_START_BIT;

        // Set the running bit
        timer_state.lm32_timer_regs[cntx][LM32_TIMER_STATUS_REG >> 2]  |= LM32_TIMER_STATUS_RUN_BIT;

        // Remember the start time
        timer_state.start_time[cntx] = time;
    }

    // If running, and stop bit set...
    if ((timer_state.lm32_timer_regs[cntx][LM32_TIMER_STATUS_REG >> 2] & LM32_TIMER_STATUS_RUN_BIT) &&
        (timer_state.lm32_timer_regs[cntx][LM32_TIMER_CONTROL_REG >> 2] & LM32_TIMER_CONTROL_STOP_BIT))
    {
        // Clear the stop bit
        timer_state.lm32_timer_regs[cntx][LM32_TIMER_CONTROL_REG >> 2] &= ~LM32_TIMER_CONTROL_STOP_BIT;

        // Clear the running bit
        timer_state.lm32_timer_regs[cntx][LM32_TIMER_STATUS_REG >> 2]  &= ~LM32_TIMER_STATUS_RUN_BIT;
    }

    // If running ...
    if (timer_state.lm32_timer_regs[cntx][LM32_TIMER_STATUS_REG >> 2] & LM32_TIMER_STATUS_RUN_BIT)
    {
        // Decrement the counter by the elapsed time since started
        timer_state.lm32_timer_regs[cntx][LM32_TIMER_SNAPSHOT_REG >> 2] -= (uint32_t)(time - timer_state.start_time[cntx]);

        lm32_time_t  overrun = 0;

        // Ensure we don't go below zero, if time since last call doesn't hit exactly
        if ((int32_t)timer_state.lm32_timer_regs[cntx][LM32_TIMER_SNAPSHOT_REG >> 2] < 0)
        {
            // Remember how much over we went
            overrun = (lm32_time_t)(0 - timer_state.lm32_timer_regs[cntx][LM32_TIMER_SNAPSHOT_REG >> 2]);

            // Set the timer to 0, so TO is detected
            timer_state.lm32_timer_regs[cntx][LM32_TIMER_SNAPSHOT_REG >> 2] = 0;
        }

        // Update start time for this point in time, as we have counted all up to this point
        timer_state.start_time[cntx] = time;

        // When the counter reaches zero...
        if (timer_state.lm32_timer_regs[cntx][LM32_TIMER_SNAPSHOT_REG >> 2] == 0)
        {
            // Set the TO bit
            timer_state.lm32_timer_regs[cntx][LM32_TIMER_STATUS_REG >> 2] |= LM32_TIMER_STATUS_TO_BIT;

            // If not continuing, clear the running bit
            if (!(timer_state.lm32_timer_regs[cntx][LM32_TIMER_CONTROL_REG >> 2] & LM32_TIMER_CONTROL_CONT_BIT))
            {
                timer_state.lm32_timer_regs[cntx][LM32_TIMER_STATUS_REG >> 2] &= ~LM32_TIMER_STATUS_RUN_BIT;
            }
            // If continuing, reload the snapshot register
            else
            {
                timer_state.lm32_timer_regs[cntx][LM32_TIMER_SNAPSHOT_REG >> 2] = timer_state.lm32_timer_regs[cntx][LM32_TIMER_PERIOD_REG >> 2];

                // Compensate for any overrrun we had at TO detection, otherwise we'll get creep
                // (and not just jitter)
                timer_state.lm32_timer_regs[cntx][LM32_TIMER_SNAPSHOT_REG >> 2] -= (uint32_t)overrun;
            }
        }
    }

    // If interrupts enabled, and the timeout has fired, raise an interrupt
    if ((timer_state.lm32_timer_regs[cntx][LM32_TIMER_CONTROL_REG >> 2] & LM32_TIMER_CONTROL_ITO_BIT) &&
        (timer_state.lm32_timer_regs[cntx][LM32_TIMER_STATUS_REG >> 2] & LM32_TIMER_STATUS_TO_BIT) && !timer_state.int_pending[cntx])
    {
        irq = true;
        timer_state.int_pending[cntx] = true;
    }

    return irq;
}

// -------------------------------------------------------------------------
// lm32_get_timer_state()
//
// Utility function to get the timer's entire internal state, for use
// in save and restore functionality.
//
// -------------------------------------------------------------------------

lm32_timer_state_t lm32_get_timer_state (void)
{
    return timer_state;
}

// -------------------------------------------------------------------------
// lm32_set_timer_state()
//
// Utility function to set the timer's entire internal state, for use
// in save and restore functionality.
//
// -------------------------------------------------------------------------
void lm32_set_timer_state (const lm32_timer_state_t state)
{
    timer_state = state;
}
