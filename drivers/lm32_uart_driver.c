//=============================================================
// 
// Copyright (c) 2017 Simon Southwell. All rights reserved.
//
// Date: 21st April 2017
//
// This file is part of the lnxmico32 instruction set simulator.
//
// The code is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This code is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this code. If not, see <http://www.gnu.org/licenses/>.
//
// $Id: lm32_uart_driver.c,v 1.3 2017/06/29 18:40:45 simon Exp $
// $Source%$
//
//=============================================================

#define UART_BASE_ADDR    0x80000000
#define UART_RBR_OFFSET   0x00
#define UART_LSR_OFFSET   0x14

#define UART_LSR_DR_BIT   0
#define UART_LSR_TEMT_BIT 6

// -------------------------------------------------------------------------
// Assembler code to read the UART's LSR register, and return the value
// -------------------------------------------------------------------------
static int rd_uart_lsr(void)
{
    int lsr;

    __asm__ __volatile__ (
         "ori  r9,  r0,  %1     \n\t"             // Set r9 low  bits to immediate argument for UART addr (bits 15:0)
         "orhi r9,  r9,  %2     \n\t"             // Set r9 high bits to immediate argument for UART addr (bits 31:16)
         "lw   r10, (r9+%3)     \n\t"             // load LSR reg to r10
         "sw   %0,  r10             "
         : "=m" (lsr)                             // outputs
         : "i" (UART_BASE_ADDR &  0xffff),        // inputs 
           "i" (UART_BASE_ADDR >> 16),
           "i" (UART_LSR_OFFSET)
         : "r9", "r10"                            // reg clobber list
    );

    return lsr;
}

// -------------------------------------------------------------------------
// Assembler code to write a value to the UART's THR register
// -------------------------------------------------------------------------
static int wr_uart_thr (int c)
{
    __asm__ __volatile__ (
             "ori  r9,  r0,  %0     \n\t"             // Set r9 low  bits to immediate argument for UART addr (bits 15:0)
             "orhi r9,  r9,  %1     \n\t"             // Set r9 high bits to immediate argument for UART addr (bits 31:16)
             "or   r10, r0,  %2     \n\t"             // Set r10 to value of c argument
             "sw  (r9+%3),   r10        "             // Write c value to UART RBR register (offset at UART_RBR_OFFSET)
             :                                        // no outputs
             : "i" (UART_BASE_ADDR &  0xffff),        // inputs 
               "i" (UART_BASE_ADDR >> 16),
               "r" (c),
               "i" (UART_RBR_OFFSET)
             : "r9", "r10"                            // reg clobber list
             );
             
    return 0;
}
// -------------------------------------------------------------------------
// Assembler code to read a value from the UART's RBR register
// -------------------------------------------------------------------------
static int rd_uart_rbr(void)
{
    int c;
    
    // fetch byte
    __asm__ __volatile__ (
             "ori  r9,  r0,  %1     \n\t"             // Set r9 low  bits to immediate argument for UART addr (bits 15:0)
             "orhi r9,  r9,  %2     \n\t"             // Set r9 high bits to immediate argument for UART addr (bits 31:16)
             "lw   r10, (r9+%3)     \n\t"             // load LSR reg to r10
             "sw   %0,  r10             "
             : "=m" (c)                               // outputs
             : "i" (UART_BASE_ADDR &  0xffff),        // inputs 
               "i" (UART_BASE_ADDR >> 16),
               "i" (UART_RBR_OFFSET)
             : "r9", "r10"                            // reg clobber list
        );

    return c;
}

// -------------------------------------------------------------------------
// Output a single byte over the UART. Blocks on transmitter busy.
// -------------------------------------------------------------------------
int outbyte(int c)
{
    // While transmitter busy, loop
    while ((rd_uart_lsr() & (1 << UART_LSR_TEMT_BIT)) == 0);

    // Write byte
    wr_uart_thr(c);
     
    return 0;
}

// ------------------------------------------------------------------------
// Read a byte from the UART's receive buffer. Blocks on buffer empty.
// ------------------------------------------------------------------------
int inbyte(void)
{
    int lsr;

    // While not byte waiting, loop
    do
    {
        lsr = rd_uart_lsr();
    } while ((lsr & (1 << UART_LSR_DR_BIT)) == 0);

    // Fetch byte and return
    return rd_uart_rbr();
}

