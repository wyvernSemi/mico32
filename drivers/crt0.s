/* =============================================================
// 
// Copyright (c) 2017 Simon Southwell. All rights reserved.
//
// Date: 13th April 2017
//
// This is a simple crt0 program for use with the lm32 ISS.
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
// $Id: crt0.s,v 1.4 2017/07/04 11:57:31 simon Exp $
// $Source: /home/simon/CVS/src/cpu/mico32/drivers/crt0.s,v $
//
//============================================================= */

/* =====================================================
// DEFINITIONS
// ===================================================== */

        .set SIGINT,           2   /* interrupt */
        .set SIGTRAP,          5   /* trace trap */
        .set SIGFPE,           8   /* arithmetic exception */
        .set SIGSEGV,          11  /* segmentation violation */
        
        .set CRT_RUN_IND_ADDR, 0xfffffffc

/* =====================================================
// Define this as the boot section, and set global
// _start value.
// ===================================================== */

        .section    .boot, "ax", @progbits

        .global _start
_start:

/* =====================================================
// Exception handlers - Must be 32 bytes long.
// ===================================================== */

/* -----------------------------------------------------
// Handler for reset exception
// -----------------------------------------------------*/
        .global _reset_handler
        .type   _reset_handler, @function
_reset_handler:
        xor     r0, r0, r0
        wcsr    IE, r0
        wcsr    IM, r0
        mvhi    r1, hi(_reset_handler)
        ori     r1, r1, lo(_reset_handler)
        wcsr    EBA, r1
        bi     _crt0
        nop
        .size   _reset_handler, .-_reset_handler

/* -----------------------------------------------------
// Handler for breakpoint debug exception
// -----------------------------------------------------*/        
        .global _breakpoint_handler
        .type   _breakpoint_handler, @function       
_breakpoint_handler:
        sw (sp+0), ra
        calli _save_all
        mvi r1, SIGTRAP
        calli _raise
        bi _restore_all_and_bret
        nop
        nop
        nop
        .size   _breakpoint_handler, .-_breakpoint_handler

/* -----------------------------------------------------
// Handler for instruction bus error exception
// -----------------------------------------------------*/
        .global _instruction_bus_error_handler
        .type   _instruction_bus_error_handler, @function          
_instruction_bus_error_handler:
        sw (sp+0), ra
        calli _save_all
        mvi r1, SIGSEGV
        calli _raise
        bi _restore_all_and_eret
        nop
        nop
        nop
        .size   _instruction_bus_error_handler, .-_instruction_bus_error_handler

/* -----------------------------------------------------
// Handler for watchpoint debug exception
// -----------------------------------------------------*/
        .global _watchpoint_handler
        .type   _watchpoint_handler, @function        
_watchpoint_handler:
        sw (sp+0), ra
        calli _save_all
        mvi r1, SIGTRAP
        calli _raise
        bi _restore_all_and_bret
        nop
        nop
        nop
        .size   _watchpoint_handler, .-_watchpoint_handler

/* -----------------------------------------------------
// Handler for data bus error exception
// -----------------------------------------------------*/
        .global _data_bus_error_handler
        .type   _data_bus_error_handler, @function         
_data_bus_error_handler:
        sw (sp+0), ra
        calli _save_all
        mvi r1, SIGSEGV
        calli _raise
        bi _restore_all_and_eret
        nop
        nop
        nop
        .size   _data_bus_error_handler, .-_data_bus_error_handler

/* -----------------------------------------------------
// Handler for divide-by-zero exception
// -----------------------------------------------------*/
        .global _divide_by_zero_handler
        .type   _divide_by_zero_handler, @function 
_divide_by_zero_handler:
        sw (sp+0), ra
        calli _save_all
        mvi r1, SIGFPE
        calli _raise
        bi _restore_all_and_eret
        nop
        nop
        nop
        .size   _divide_by_zero_handler, .-_divide_by_zero_handler

/* -----------------------------------------------------
// Handler for external interrupts
// -----------------------------------------------------*/
        .global _interrupt_handler
        .type   _interrupt_handler, @function        
_interrupt_handler:
        sw (sp+0), ra
        calli _save_all
        mvi r1, SIGINT
        calli _raise
        bi _restore_all_and_eret
        nop
        nop
        nop
        .size   _interrupt_handler, .-_interrupt_handler

/* -----------------------------------------------------
// Handler for system call exception
// -----------------------------------------------------*/
        .global _system_call_handler
        .type   _system_call_handler, @function        
_system_call_handler:
        sw (sp+0), ra
        calli _save_all
        mv r1, sp
        calli _handle_scall
        bi _restore_all_and_eret
        nop
        nop
        nop
        .size   _system_call_handler, .-_system_call_handler

/* =====================================================
// Support routines
// ===================================================== */

/* -----------------------------------------------------
// Function to save key registers onto the stack
// -----------------------------------------------------*/
        .global _save_all
        .type   _save_all, @function         
_save_all:
        addi sp, sp, -56
        /* Save all caller save registers onto the stack */
        sw (sp+4),  r1
        sw (sp+8),  r2
        sw (sp+12), r3
        sw (sp+16), r4
        sw (sp+20), r5
        sw (sp+24), r6
        sw (sp+28), r7
        sw (sp+32), r8
        sw (sp+36), r9
        sw (sp+40), r10
        sw (sp+48), ea
        sw (sp+52), ba
        /* ra needs to be moved from initial stack location */
        lw r1, (sp+56)
        sw (sp+44), r1
        ret
        .size   _save_all, .-_save_all

/* -----------------------------------------------------
// Restore all registers and return from exception      
/* -----------------------------------------------------*/
        .global _restore_all_and_eret
        .type   _restore_all_and_eret, @function        
_restore_all_and_eret:
        lw r1, (sp+4)
        lw r2, (sp+8)
        lw r3, (sp+12)
        lw r4, (sp+16)
        lw r5, (sp+20)
        lw r6, (sp+24)
        lw r7, (sp+28)
        lw r8, (sp+32)
        lw r9, (sp+36)
        lw r10,(sp+40)
        lw ra, (sp+44)
        lw ea, (sp+48)
        lw ba, (sp+52)
        addi sp, sp, 56
        eret
        .size   _restore_all_and_eret, .-_restore_all_and_eret

/* -----------------------------------------------------
// Restore all registers and return from breakpoint     
// -----------------------------------------------------*/
        .global _restore_all_and_bret
        .type   _restore_all_and_bret, @function  
_restore_all_and_bret:
        lw r1, (sp+4)
        lw r2, (sp+8)
        lw r3, (sp+12)
        lw r4, (sp+16)
        lw r5, (sp+20)
        lw r6, (sp+24)
        lw r7, (sp+28)
        lw r8, (sp+32)
        lw r9, (sp+36)
        lw r10,(sp+40)
        lw ra, (sp+44)
        lw ea, (sp+48)
        lw ba, (sp+52)
        addi sp, sp, 56
        bret 
        .size   _restore_all_and_bret, .-_restore_all_and_bret

/* -----------------------------------------------------
// Start-up C run-time initialisation function
// -----------------------------------------------------*/      
    .global _crt0
    .type   _crt0, @function    
_crt0:
    xor     r0, r0, r0
    
#ifndef CRT_DISABLE_CPU_RUN_INDICATOR    
    /* Flag to outside world that we're running */
    mvhi    r2, hi(CRT_RUN_IND_ADDR)
    ori     r2, r2, lo(CRT_RUN_IND_ADDR)
    sw      (r2+0), r0
#endif
    
    /* Setup stack, global pointer */
    mvhi    sp, hi(_fstack)
    ori     sp, sp, lo(_fstack)
    mvhi    gp, hi(_gp)
    ori     gp, gp, lo(_gp)

    /* Conditionally clear BSS. In simulation, this may take too long. */
#ifdef CRT_INIT_BSS
    /* Init BSS  */
    mvhi    r2, hi(_fbss)
    ori     r2, r1, lo(_fbss)
    mvhi    r3, hi(_ebss)
    ori     r3, r3, lo(_ebss)
.bssloop:
    sw      (r2+0), r0
    addi    r2, r2, 4
    bne     r2, r3, .bssloop
#endif

    /* Enable interrupts */
    ori     r1, r0, 7
    wcsr    IM, r1
    ori     r1, r0, 1
    wcsr    IE, r1

    /* Call main() */
    calli main
    
#ifndef CRT_DISABLE_CPU_RUN_INDICATOR 
    /* Flag to outside world that we're exiting */
    mvhi    r2, hi(CRT_RUN_IND_ADDR)
    ori     r2, r2, lo(CRT_RUN_IND_ADDR)
    ori     r3, r0, 1
    sw      (r2+0), r3    
#endif    

    /* Jump to exit */
    bi _exit
    .size   _crt0, .-_crt0

/* -----------------------------------------------------
// Top level exception handler routine (signal in r1).
// Does nothing right now.
// -----------------------------------------------------*/      
    .global _raise
    .type   _raise, @function   
_raise:
    ret
    .size   _raise, .-_raise
  
/* -----------------------------------------------------
// System call exception handler. Does nothing right now.
// -----------------------------------------------------*/      
    .global _handle_scall
    .type   _handle_scall, @function   
_handle_scall:
    ret
    .size   _handle_scall, .-_handle_scall    
 
/* -----------------------------------------------------
// The exit function. Currently loops forever. Will 
// cause ISS to terminate run loop for lock condition
// (unless disabled).
// -----------------------------------------------------*/
    .global _exit
    .type   _exit, @function 
 _exit:
    bi _exit
    .size   _exit, .-_exit
