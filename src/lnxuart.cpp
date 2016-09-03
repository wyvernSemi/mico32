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
// $Id: lnxuart.cpp,v 2.5 2016-09-03 07:44:06 simon Exp $
// $Source: /home/simon/CVS/src/cpu/mico32/src/lnxuart.cpp,v $
//
//=============================================================

// -------------------------------------------------------------------------
// INCLUDES
// -------------------------------------------------------------------------

#include <stdint.h>
#if defined _WIN32 || defined _WIN64
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#endif
#include "lm32_cpu.h"
#include "lnxmico32.h"
#include "lnxuart.h"

// -------------------------------------------------------------------------
// DEFINES
// -------------------------------------------------------------------------

#define LM32_UART_RBR_REG                    0x00
#define LM32_UART_IER_REG                    0x04
#define LM32_UART_IIR_REG                    0x08
#define LM32_UART_LCR_REG                    0x0C
#define LM32_UART_MCR_REG                    0x10
#define LM32_UART_LSR_REG                    0x14
#define LM32_UART_MSR_REG                    0x18
#define LM32_UART_DIV_REG                    0x1c

#define LM32_UART_RBR_WR_MASK                0x000000ff
#define LM32_UART_IER_WR_MASK                0x0000000f
#define LM32_UART_LCR_WR_MASK                0x0000007f
#define LM32_UART_MCR_WR_MASK                0x00000003
#define LM32_UART_DIV_WR_MASK                0x0000000f

#define LM32_UART_LSR_DR_BIT                 0x01
#define LM32_UART_LSR_OE_BIT                 0x02
#define LM32_UART_LSR_PE_BIT                 0x04
#define LM32_UART_LSR_FE_BIT                 0x08
#define LM32_UART_LSR_BI_BIT                 0x10
#define LM32_UART_LSR_THRR_BIT               0x20
#define LM32_UART_LSR_TEMT_BIT               0x40
                                            
#define LM32_UART_IER_RBRI_BIT               0x01
#define LM32_UART_IER_THRI_BIT               0x02
#define LM32_UART_IER_RLSI_BIT               0x04
#define LM32_UART_IER_MSI_BIT                0x08
                                            
#define LM32_UART_INT_ID_NONE                0x01
#define LM32_UART_INT_ID_ERR                 0x06
#define LM32_UART_INT_ID_RDR                 0x04
#define LM32_UART_INT_ID_EMP                 0x02
#define LM32_UART_INT_ID_MST                 0x00

#define LM32_UART_LSR_RST_VAL                (LM32_UART_LSR_THRR_BIT | LM32_UART_LSR_TEMT_BIT)
#define LM32_UART_REGS_RST_VALS              {0, 0, 0, 0, 0, LM32_UART_LSR_RST_VAL, 0, 0}

#define LM32_TERMINATE_STR_LEN 8
#if defined _WIN32 || defined _WIN32
#define LM32_NEWLINE_CHAR      0x0d
#else
#define LM32_NEWLINE_CHAR      0x0a
#endif

// -------------------------------------------------------------------------
// LOCAL CONSTANTS
// -------------------------------------------------------------------------

static const uint32_t terminate_str[LM32_TERMINATE_STR_LEN] = {'#', '!', 'e', 'x', 'i', 't', '!', LM32_NEWLINE_CHAR};

// -------------------------------------------------------------------------
// LOCAL STATICS
// -------------------------------------------------------------------------

lm32_uart_state_t uart_state = {
    {0, 0, 0, 0},
    {false, false, false, false},
    {LM32_UART_REGS_RST_VALS,
     LM32_UART_REGS_RST_VALS,
     LM32_UART_REGS_RST_VALS,
     LM32_UART_REGS_RST_VALS}
};

#if !(defined _WIN32) && !defined(_WIN64)
// -------------------------------------------------------------------------
// Keyboard input LINUX/CYGWIN emulation functions
// -------------------------------------------------------------------------


// Implement _kbhit() locally for non-windows platforms
int _kbhit(void)
{
  struct termios oldt, newt;
  int ch;
  int oldf;
 
  tcgetattr(STDIN_FILENO, &oldt);
  newt           = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);

  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
 
  ch = getchar();
 
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);
 
  if(ch != EOF)
  {
      ungetc(ch, stdin);
      return 1;
  }
 
  return 0;
}

// getchar() is okay for _getch() on non-windows platforms
#define _getch getchar

#endif

// -------------------------------------------------------------------------
// lm32_uart_write()
//
// UART model register write function.
//
// -------------------------------------------------------------------------

void lm32_uart_write(const uint32_t address, const uint32_t data, const int cntx)
{
    switch(address)
    {
    case LM32_UART_RBR_REG:
        // Print the character
        putchar(data & LM32_UART_RBR_WR_MASK);

#ifdef CYGWIN
        // Flush the output. When compiling with optimisations under Cygwin, not all
        // relevant output makes it to the screen consistently without it.
        fflush(stdout);
#endif

        // Clear the TEMPT and THRR (aka THRE) bit
        uart_state.lm32_uart_regs[cntx][LM32_UART_LSR_REG >> 2] = uart_state.lm32_uart_regs[cntx][LM32_UART_LSR_REG >> 2] & ~(LM32_UART_LSR_TEMT_BIT | LM32_UART_LSR_THRR_BIT);
        uart_state.int_pending[cntx] = false;
        break;

    case LM32_UART_IER_REG:
        uart_state.lm32_uart_regs[cntx][LM32_UART_IER_REG >> 2] = data & LM32_UART_IER_WR_MASK;
        break;

    case LM32_UART_LCR_REG:
        uart_state.lm32_uart_regs[cntx][LM32_UART_LCR_REG >> 2] = data & LM32_UART_LCR_WR_MASK;
        break;

    case LM32_UART_MCR_REG:
        uart_state.lm32_uart_regs[cntx][LM32_UART_MCR_REG >> 2] = data & LM32_UART_MCR_WR_MASK;
        break;

    case LM32_UART_DIV_REG:
        uart_state.lm32_uart_regs[cntx][LM32_UART_DIV_REG >> 2] = data & 0xffff;
        break;

    default:
        break;
    }
}

// -------------------------------------------------------------------------
// lm32_uart_read()
//
// UART model register read function.
//
// -------------------------------------------------------------------------

void lm32_uart_read(const uint32_t address, uint32_t* data, const int cntx)
{
    *data = 0;
    switch(address)
    {
    case LM32_UART_RBR_REG:
        *data = uart_state.lm32_uart_regs[cntx][LM32_UART_RBR_REG >> 2];
        uart_state.lm32_uart_regs[cntx][LM32_UART_LSR_REG >> 2] &= ~LM32_UART_LSR_DR_BIT;
        break;

    case LM32_UART_IIR_REG:
        *data = uart_state.lm32_uart_regs[cntx][LM32_UART_IIR_REG >> 2];
        break;

    case LM32_UART_LSR_REG:
        *data = uart_state.lm32_uart_regs[cntx][LM32_UART_LSR_REG >> 2];
        break;

    case LM32_UART_MSR_REG:
        *data = uart_state.lm32_uart_regs[cntx][LM32_UART_MSR_REG >> 2];
        break;
    }
}

// -------------------------------------------------------------------------
// lm32_uart_tick()
//
// UART model tick/interrupt function. Called regularly, with time stamp
// and generates interrupts, and updates register state.
//
// -------------------------------------------------------------------------

bool lm32_uart_tick(const lm32_time_t time, bool &terminate, const bool kbd_connected, const int cntx)
{
    bool irq = false;

    // If we're transmitting, but start time is 0, TX has just started
    if ((uart_state.lm32_uart_regs[cntx][LM32_UART_LSR_REG >> 2] & (LM32_UART_LSR_TEMT_BIT | LM32_UART_LSR_THRR_BIT)) == 0 && uart_state.start_time[cntx] == 0)
    {
        // Load start time with current time
        uart_state.start_time[cntx] = time;
    }

    // When a transmission time has elapsed (assuming start, stop, parity and 8 data bits),
    // clear the time and set the transmit status
    if (time >= (uart_state.start_time[cntx] + LM32_UART_TICKS_PER_BIT))
    {
        // Clear start time
        uart_state.start_time[cntx] = 0;
        uart_state.lm32_uart_regs[cntx][LM32_UART_LSR_REG >> 2] |= LM32_UART_LSR_TEMT_BIT | LM32_UART_LSR_THRR_BIT;
    }

    // When UART with keyboard connected, and input is waiting, get the value and put in RBR register,
    // then flag data ready status
    if (kbd_connected && _kbhit())
    {
        // Since only one UART can be the input, only need one terminate index state
        static int term_idx = 0;

        // Get the input character and put in the RBR register
        uint32_t  cur = _getch() & 0xffU;
        uart_state.lm32_uart_regs[cntx][LM32_UART_RBR_REG >> 2] = cur;

        // Set the data received flag in the LSR register
        uart_state.lm32_uart_regs[cntx][LM32_UART_LSR_REG >> 2] |= LM32_UART_LSR_DR_BIT;

        // If the input character matches the current termination character...
        if (cur == terminate_str[term_idx])
        {
            // Increment the index to the next charcaterin the table.
            term_idx++;
            if (term_idx == LM32_TERMINATE_STR_LEN)
            {
                terminate = true;
            }
        }
        // Character did not match the next termination character, so reset the index
        else
        {
            // If the failing character is also the first of the terminating characters,
            // set index to 1, otherwise reset to 0.
            term_idx = (cur == terminate_str[0]) ? 1 : 0;
        }
    }

    // Update interrupts...

    // By default, the status is that there are no interrupts pending
    uart_state.lm32_uart_regs[cntx][LM32_UART_IIR_REG >> 2] = LM32_UART_INT_ID_NONE;

    // Overrun error interrupt (PE, FE and BI not yet supported)
    if ((uart_state.lm32_uart_regs[cntx][LM32_UART_LSR_REG >> 2] & LM32_UART_LSR_OE_BIT) &&
        (uart_state.lm32_uart_regs[cntx][LM32_UART_IER_REG >> 2] & LM32_UART_IER_RLSI_BIT))
    {
        irq = true;
        uart_state.int_pending[cntx] = true;
        uart_state.lm32_uart_regs[cntx][LM32_UART_IIR_REG >> 2] = LM32_UART_INT_ID_ERR;
    }
    // Data received
    else if ((uart_state.lm32_uart_regs[cntx][LM32_UART_LSR_REG >> 2] & LM32_UART_LSR_DR_BIT) &&
             (uart_state.lm32_uart_regs[cntx][LM32_UART_IER_REG >> 2] & LM32_UART_IER_RBRI_BIT))
    {
        irq = true;
        uart_state.int_pending[cntx] = true;
        uart_state.lm32_uart_regs[cntx][LM32_UART_IIR_REG >> 2] = LM32_UART_INT_ID_RDR;
    }
    // Transmit buffer empty (transmit ready to send)
    else if ((uart_state.lm32_uart_regs[cntx][LM32_UART_LSR_REG >> 2] & LM32_UART_LSR_THRR_BIT) &&
             (uart_state.lm32_uart_regs[cntx][LM32_UART_IER_REG >> 2] & LM32_UART_IER_THRI_BIT))
    {
        irq = true;
        uart_state.int_pending[cntx] = true;
        uart_state.lm32_uart_regs[cntx][LM32_UART_IIR_REG >> 2] = LM32_UART_INT_ID_EMP;
    }

    return irq;
}

// -------------------------------------------------------------------------
// lm32_get_uart_state()
//
// Utility function to get the UARTS's entire internal state, for use
// in save and restore functionality.
//
// -------------------------------------------------------------------------

lm32_uart_state_t lm32_get_uart_state(void)
{
    return uart_state;
}

// -------------------------------------------------------------------------
// lm32_set_uart_state()
//
// Utility function to set the UARTS's entire internal state, for use
// in save and restore functionality.
//
// -------------------------------------------------------------------------

void lm32_set_uart_state (const lm32_uart_state_t state)
{
    uart_state = state;
}
