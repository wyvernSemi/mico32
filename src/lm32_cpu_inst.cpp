//=============================================================
// 
// Copyright (c) 2013-2016 Simon Southwell. All rights reserved.
//
// Date: 9th April 2013
//
// Contains the instruction implementation methods for the
// lm32_cpu class
//
// This file is part of the cpumico32 instruction set simulator.
//
// cpumico32 is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// cpumico32 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with cpumico32. If not, see <http://www.gnu.org/licenses/>.
//
// $Id: lm32_cpu_inst.cpp,v 3.2 2016/09/15 18:16:24 simon Exp $
// $Source: /home/simon/CVS/src/cpu/mico32/src/lm32_cpu_inst.cpp,v $
//
//=============================================================

// -------------------------------------------------------------------------
// INCLUDES
// -------------------------------------------------------------------------

#include <cstdio>

#include "lm32_cpu.h"
#include "lm32_cpu_mico32.h"

// -------------------------------------------------------------------------
// calc_stall()
//
// Calculates the number of calc_stall cycles based on the register index inputs
// and their available time against current time. Note: ry must be a valid
// register index, but rz may be null (NULL_REG_IDX). The largest of the
// two delays (if rz specified) is chosen and returned as a number of cycles 
// to calc_stall.
//
// -------------------------------------------------------------------------

#ifndef LM32_FAST_COMPILE
lm32_time_t lm32_cpu::calc_stall(const int ry, const int rz, const lm32_time_t cycle_count)
{
    lm32_time_t calc_stall_y = 0;
    lm32_time_t calc_stall_z = 0;

    // Calculate calc_stall for ry
    calc_stall_y = rt[ry] - cycle_count;

    calc_stall_y = (calc_stall_y > 0) ? calc_stall_y : 0;

    // If rz not specified, return calc_stall for ry
    if (rz == NULL_REG_IDX)
    {
        return calc_stall_y; 
    // Calculate calc_stall for rz
    }
    else
    {
        calc_stall_z = rt[rz] - cycle_count;
        calc_stall_z = (calc_stall_z > 0) ? calc_stall_z : 0;
    }

    // Choose largest calc_stall value and return
    return (calc_stall_y > calc_stall_z) ? calc_stall_y : calc_stall_z;
}
#endif
// ------------------------------------------------------------
// Instruction execution functions.
//
// See "LatticeMico32 Reference Manual, June 2012, Chapter 5"
// ------------------------------------------------------------

// ------------------------------------------------------------
// Exception instructions
// ------------------------------------------------------------

void lm32_cpu::lm32_raise (const p_lm32_decode_t p)
{
    if (p->opcode == BREAK_INSTR)
    {
        if (state.cfg & (1 << LM32_CFG_G))
        {
            state.int_flags |= (1 << INT_ID_BREAKPOINT);
        }
    }
    else if (p->opcode == SCALL_INSTR)
    {
        state.int_flags |= (1 << INT_ID_SYSCALL);
    }

#ifndef LM32_FAST_COMPILE
    state.cycle_count += p->decode->issue_cycles;
#endif
}

// ------------------------------------------------------------

void lm32_cpu::lm32_rsrvd (const p_lm32_decode_t p)
{
    // Unimplemented instruction raises an instruction bus error
    state.int_flags |= (1 << INT_ID_IBUSERROR);
    state.pc += 4;

#ifndef LM32_FAST_COMPILE
    state.cycle_count += p->decode->issue_cycles;
#endif
}

// ------------------------------------------------------------
// Arithmetic instructions
// ------------------------------------------------------------

void lm32_cpu::lm32_add (const p_lm32_decode_t p) 
{
    state.r[p->reg2] = (int32_t)state.r[p->reg0_csr] + (int32_t)state.r[p->reg1];
    state.pc += 4;

#ifndef LM32_FAST_COMPILE
    // Get any stall cycles on the instruction's input operands
    state.cycle_count += calc_stall(p->reg0_csr, p->reg1, state.cycle_count);
    rt[p->reg2] = state.cycle_count + p->decode->result_cycles;
    state.cycle_count += p->decode->issue_cycles;
#endif
}

// ------------------------------------------------------------

void lm32_cpu::lm32_addi (const p_lm32_decode_t p)
{
    state.r[p->reg1] = (int32_t)state.r[p->reg0_csr] + SIGN_EXT16(p->imm);
    state.pc += 4;

#ifndef LM32_FAST_COMPILE
    // Get any stall cycles on the instruction's input operands
    state.cycle_count += calc_stall(p->reg0_csr, NULL_REG_IDX, state.cycle_count);
    rt[p->reg1] = state.cycle_count + p->decode->result_cycles;
    state.cycle_count += p->decode->issue_cycles;
#endif
}

// ------------------------------------------------------------

void lm32_cpu::lm32_sub (const p_lm32_decode_t p)
{
    state.r[p->reg2] = (int32_t)state.r[p->reg0_csr] - (int32_t)state.r[p->reg1];
    state.pc += 4;

#ifndef LM32_FAST_COMPILE
    // Get any stall cycles on the instruction's input operands
    state.cycle_count += calc_stall(p->reg0_csr, p->reg1, state.cycle_count);
    rt[p->reg2] = state.cycle_count + p->decode->result_cycles;
    state.cycle_count += p->decode->issue_cycles;
#endif
}

// ------------------------------------------------------------

void lm32_cpu::lm32_div (const p_lm32_decode_t p)
{
    // If divide implemented, execute instruction
    if (state.cfg & (1 << LM32_CFG_D))
    {
        if (state.r[p->reg1])
        {
            state.r[p->reg2] = (int32_t)state.r[p->reg0_csr] / (int32_t)state.r[p->reg1];
        } 
        else 
        {
            state.int_flags |= (1 << INT_ID_DIVZERO);
        }

        state.pc += 4;

#ifndef LM32_FAST_COMPILE
        // Get any stall cycles on the instruction's input operands
        state.cycle_count += calc_stall(p->reg0_csr, p->reg1, state.cycle_count);
        rt[p->reg2] = state.r[p->reg1] ? state.cycle_count + p->decode->result_cycles : rt[p->reg2];
        state.cycle_count += p->decode->issue_cycles;
#endif

    // Instruction not implemented
    } 
    else
    {
        lm32_rsrvd(p);
    }
}

// ------------------------------------------------------------

void lm32_cpu::lm32_divu (const p_lm32_decode_t p) 
{
    // If divide implemented, execute instruction
    if (state.cfg & (1 << LM32_CFG_D)) 
    {
        if (state.r[p->reg1])
        {
            state.r[p->reg2] = state.r[p->reg0_csr] / state.r[p->reg1];
        } 
        else
        {
            state.int_flags |= (1 << INT_ID_DIVZERO);
        }

        state.pc += 4;

#ifndef LM32_FAST_COMPILE
        // Get any stall cycles on the instruction's input operands
        state.cycle_count += calc_stall(p->reg0_csr, p->reg1, state.cycle_count);
        rt[p->reg2] = state.r[p->reg1] ? state.cycle_count + p->decode->result_cycles : rt[p->reg2];
        state.cycle_count += p->decode->issue_cycles;
#endif

    // Instruction not implemented
    }
    else
    {
        lm32_rsrvd(p);
    }
}

// ------------------------------------------------------------

void lm32_cpu::lm32_mod (const p_lm32_decode_t p)
{
    // If divide implemented, execute instruction
    if (state.cfg & (1 << LM32_CFG_D))
    {
        if (state.r[p->reg1])
        {
            state.r[p->reg2] = (int32_t)state.r[p->reg0_csr] % (int32_t)state.r[p->reg1];
        } 
        else 
        {
            state.int_flags |= (1 << INT_ID_DIVZERO);
        }

        state.pc += 4;

#ifndef LM32_FAST_COMPILE
        // Get any stall cycles on the instruction's input operands
        state.cycle_count += calc_stall(p->reg0_csr, p->reg1, state.cycle_count);
        rt[p->reg2] = state.r[p->reg1] ? state.cycle_count + p->decode->result_cycles : rt[p->reg2];
        state.cycle_count += p->decode->issue_cycles;
#endif
    }
    else
    {
        lm32_rsrvd(p);
    }
}

// ------------------------------------------------------------

void lm32_cpu::lm32_modu (const p_lm32_decode_t p)
{
    // If divide implemented, execute instruction
    if (state.cfg & (1 << LM32_CFG_D)) 
    {
        if (state.r[p->reg1]) 
        {
            state.r[p->reg2] = state.r[p->reg0_csr] % state.r[p->reg1];
        }
        else
        {
            state.int_flags |= (1 << INT_ID_DIVZERO);
        }

        state.pc += 4;

#ifndef LM32_FAST_COMPILE
        // Get any stall cycles on the instruction's input operands
        state.cycle_count += calc_stall(p->reg0_csr, p->reg1, state.cycle_count);
        rt[p->reg2] = state.r[p->reg1] ? state.cycle_count + p->decode->result_cycles : rt[p->reg2];
        state.cycle_count += p->decode->issue_cycles;
#endif
    }
    else
    {
        lm32_rsrvd(p);
    }
}

// ------------------------------------------------------------

void lm32_cpu::lm32_mul (const p_lm32_decode_t p)
{
    // If multiply implemented, execute instruction
    if (state.cfg & (1 << LM32_CFG_M))
    {
        state.r[p->reg2] = (uint32_t)(state.r[p->reg0_csr] * state.r[p->reg1]);
        state.pc += 4;
    
#ifndef LM32_FAST_COMPILE
        // Get any stall cycles on the instruction's input operands
        state.cycle_count += calc_stall(p->reg0_csr, p->reg1, state.cycle_count);
        rt[p->reg2] = state.cycle_count + p->decode->result_cycles;
        state.cycle_count += p->decode->issue_cycles;
#endif

    // Instruction not implemented
    } 
    else
    {
        lm32_rsrvd(p);
    }
}

// ------------------------------------------------------------

void lm32_cpu::lm32_muli (const p_lm32_decode_t p)
{
    // If multiply implemented, execute instruction
    if (state.cfg & (1 << LM32_CFG_M))
    {
        state.r[p->reg1] = (uint32_t)(state.r[p->reg0_csr] * SIGN_EXT16(p->imm));
        state.pc += 4;

#ifndef LM32_FAST_COMPILE
        // Get any stall cycles on the instruction's input operands
        state.cycle_count += calc_stall(p->reg0_csr, NULL_REG_IDX, state.cycle_count);
        rt[p->reg1] = state.cycle_count + p->decode->result_cycles;
        state.cycle_count += p->decode->issue_cycles;
#endif

    // Instruction not implemented
    }
    else
    {
        lm32_rsrvd(p);
    }
}

// ------------------------------------------------------------

void lm32_cpu::lm32_sextb (const p_lm32_decode_t p)
{
    // If sign extend implemented, execute instruction
    if (state.cfg & (1 << LM32_CFG_X))
    {
        state.r[p->reg2] = SIGN_EXT8(state.r[p->reg0_csr] & BYTE_MASK);
        state.pc += 4;

#ifndef LM32_FAST_COMPILE
        // Get any stall cycles on the instruction's input operands
        state.cycle_count += calc_stall(p->reg0_csr, NULL_REG_IDX, state.cycle_count);
        rt[p->reg2] = state.cycle_count + p->decode->result_cycles;
        state.cycle_count += p->decode->issue_cycles;
#endif

    // Instruction not implemented
    }
    else
    {
        lm32_rsrvd(p);
    }
}

// ------------------------------------------------------------

void lm32_cpu::lm32_sexth (const p_lm32_decode_t p)
{
    // If sign extend implemented, execute instruction
    if (state.cfg & (1 << LM32_CFG_X)) 
    {
        state.r[p->reg2] = SIGN_EXT16(state.r[p->reg0_csr] & HWORD_MASK);
        state.pc += 4;

#ifndef LM32_FAST_COMPILE
        // Get any stall cycles on the instruction's input operands
        state.cycle_count += calc_stall(p->reg0_csr, NULL_REG_IDX, state.cycle_count);
        rt[p->reg2] = state.cycle_count + p->decode->result_cycles;
        state.cycle_count += p->decode->issue_cycles;
#endif

    // Instruction not implemented
    }
    else
    {
        lm32_rsrvd(p);
    }
}

// ------------------------------------------------------------
// Comparison instructions
// ------------------------------------------------------------

void lm32_cpu::lm32_cmpe (const p_lm32_decode_t p)
{
    state.r[p->reg2] = state.r[p->reg0_csr] == state.r[p->reg1];
    state.pc += 4;

#ifndef LM32_FAST_COMPILE
    // Get any stall cycles on the instruction's input operands
    state.cycle_count += calc_stall(p->reg0_csr, p->reg1, state.cycle_count);
    rt[p->reg2] = state.cycle_count + p->decode->result_cycles;
    state.cycle_count += p->decode->issue_cycles;
#endif
}

// ------------------------------------------------------------

void lm32_cpu::lm32_cmpei (const p_lm32_decode_t p)
{
    state.r[p->reg1] = (int32_t)state.r[p->reg0_csr] == SIGN_EXT16(p->imm);
    state.pc += 4;

#ifndef LM32_FAST_COMPILE
    // Get any stall cycles on the instruction's input operands
    state.cycle_count += calc_stall(p->reg0_csr, NULL_REG_IDX, state.cycle_count);
    rt[p->reg1] = state.cycle_count + p->decode->result_cycles;
    state.cycle_count += p->decode->issue_cycles;
#endif
}

// ------------------------------------------------------------

void lm32_cpu::lm32_cmpg (const p_lm32_decode_t p)
{
    state.r[p->reg2] = (int32_t)state.r[p->reg0_csr] > (int32_t)state.r[p->reg1];
    state.pc += 4;

#ifndef LM32_FAST_COMPILE
    // Get any stall cycles on the instruction's input operands
    state.cycle_count += calc_stall(p->reg0_csr, p->reg1, state.cycle_count);
    rt[p->reg2] = state.cycle_count + p->decode->result_cycles;
    state.cycle_count += p->decode->issue_cycles;
#endif
}

// ------------------------------------------------------------

void lm32_cpu::lm32_cmpge (const p_lm32_decode_t p)
{
    state.r[p->reg2] = (int32_t)state.r[p->reg0_csr] >= (int32_t)state.r[p->reg1];
    state.pc += 4;

#ifndef LM32_FAST_COMPILE
    // Get any stall cycles on the instruction's input operands
    state.cycle_count += calc_stall(p->reg0_csr, p->reg1, state.cycle_count);
    rt[p->reg2] = state.cycle_count + p->decode->result_cycles;
    state.cycle_count += p->decode->issue_cycles;
#endif
}

// ------------------------------------------------------------

void lm32_cpu::lm32_cmpgei (const p_lm32_decode_t p)
{
    state.r[p->reg1] = (int32_t)state.r[p->reg0_csr] >= SIGN_EXT16(p->imm);
    state.pc += 4;

#ifndef LM32_FAST_COMPILE
    // Get any stall cycles on the instruction's input operands
    state.cycle_count += calc_stall(p->reg0_csr, NULL_REG_IDX, state.cycle_count);
    rt[p->reg1] = state.cycle_count + p->decode->result_cycles;
    state.cycle_count += p->decode->issue_cycles;
#endif
}

// ------------------------------------------------------------

void lm32_cpu::lm32_cmpgeu (const p_lm32_decode_t p)
{
    state.r[p->reg2] = state.r[p->reg0_csr] >= state.r[p->reg1];
    state.pc += 4;

#ifndef LM32_FAST_COMPILE
    // Get any stall cycles on the instruction's input operands
    state.cycle_count += calc_stall(p->reg0_csr, p->reg1, state.cycle_count);
    rt[p->reg2] = state.cycle_count + p->decode->result_cycles;
    state.cycle_count += p->decode->issue_cycles;
#endif
}

// ------------------------------------------------------------

void lm32_cpu::lm32_cmpgeui (const p_lm32_decode_t p)
{
    state.r[p->reg1] = state.r[p->reg0_csr] >= p->imm;
    state.pc += 4;

#ifndef LM32_FAST_COMPILE
    // Get any stall cycles on the instruction's input operands
    state.cycle_count += calc_stall(p->reg0_csr, NULL_REG_IDX, state.cycle_count);
    rt[p->reg1] = state.cycle_count + p->decode->result_cycles;
    state.cycle_count += p->decode->issue_cycles;
#endif
}

// ------------------------------------------------------------

void lm32_cpu::lm32_cmpgi (const p_lm32_decode_t p) 
{
    state.r[p->reg1] = (int32_t)state.r[p->reg0_csr] > SIGN_EXT16(p->imm);
    state.pc += 4;

#ifndef LM32_FAST_COMPILE
    // Get any stall cycles on the instruction's input operands
    state.cycle_count += calc_stall(p->reg0_csr, NULL_REG_IDX, state.cycle_count);
    rt[p->reg1] = state.cycle_count + p->decode->result_cycles;
    state.cycle_count += p->decode->issue_cycles;
#endif
}

// ------------------------------------------------------------

void lm32_cpu::lm32_cmpgu (const p_lm32_decode_t p)
{
    state.r[p->reg2] = state.r[p->reg0_csr] > state.r[p->reg1];
    state.pc += 4;

#ifndef LM32_FAST_COMPILE
    // Get any stall cycles on the instruction's input operands
    state.cycle_count += calc_stall(p->reg0_csr, p->reg1, state.cycle_count);
    rt[p->reg2] = state.cycle_count + p->decode->result_cycles;
    state.cycle_count += p->decode->issue_cycles;
#endif
}

// ------------------------------------------------------------

void lm32_cpu::lm32_cmpgui (const p_lm32_decode_t p)
{
    state.r[p->reg1] = state.r[p->reg0_csr] >  p->imm;
    state.pc += 4;

#ifndef LM32_FAST_COMPILE
    // Get any stall cycles on the instruction's input operands
    state.cycle_count += calc_stall(p->reg0_csr, NULL_REG_IDX, state.cycle_count);
    rt[p->reg1] = state.cycle_count + p->decode->result_cycles;
    state.cycle_count += p->decode->issue_cycles;
#endif
}

// ------------------------------------------------------------

void lm32_cpu::lm32_cmpne (const p_lm32_decode_t p)
{
    state.r[p->reg2] = state.r[p->reg0_csr] != state.r[p->reg1];
    state.pc += 4;

#ifndef LM32_FAST_COMPILE
    // Get any stall cycles on the instruction's input operands
    state.cycle_count += calc_stall(p->reg0_csr, p->reg1, state.cycle_count);
    rt[p->reg2] = state.cycle_count + p->decode->result_cycles;
    state.cycle_count += p->decode->issue_cycles;
#endif
}

// ------------------------------------------------------------

void lm32_cpu::lm32_cmpnei (const p_lm32_decode_t p)
{
    state.r[p->reg1] = state.r[p->reg0_csr] != SIGN_EXT16(p->imm);
    state.pc += 4;

#ifndef LM32_FAST_COMPILE
    // Get any stall cycles on the instruction's input operands
    state.cycle_count += calc_stall(p->reg0_csr, NULL_REG_IDX, state.cycle_count);
    rt[p->reg1] = state.cycle_count + p->decode->result_cycles;
    state.cycle_count += p->decode->issue_cycles;
#endif
}

// ------------------------------------------------------------
// Shift Instructions
// ------------------------------------------------------------

void lm32_cpu::lm32_sl (const p_lm32_decode_t p)
{
    // If shifter implemented, execute instruction
    if (state.cfg & (1 << LM32_CFG_S))
    {
        state.r[p->reg2] = state.r[p->reg0_csr] << state.r[p->reg1];
        state.pc += 4;

#ifndef LM32_FAST_COMPILE
        // Get any stall cycles on the instruction's input operands
        state.cycle_count += calc_stall(p->reg0_csr, p->reg1, state.cycle_count);
        rt[p->reg2] = state.cycle_count + p->decode->result_cycles;
        state.cycle_count += p->decode->issue_cycles;
#endif

    // Instruction not implemented
    }
    else
    {
        lm32_rsrvd(p);
    }
}

// ------------------------------------------------------------

void lm32_cpu::lm32_sli (const p_lm32_decode_t p)
{
    // If shifter implemented, execute instruction
    if (state.cfg & (1 << LM32_CFG_S)) 
    {
        state.r[p->reg1] = state.r[p->reg0_csr] << p->imm;
        state.pc += 4;

#ifndef LM32_FAST_COMPILE
        // Get any stall cycles on the instruction's input operands
        state.cycle_count += calc_stall(p->reg0_csr, NULL_REG_IDX, state.cycle_count);
        rt[p->reg1] = state.cycle_count + p->decode->result_cycles;
        state.cycle_count += p->decode->issue_cycles;
#endif

    // Instruction not implemented
    }
    else
    {
        lm32_rsrvd(p);
    }
}

// ------------------------------------------------------------

void lm32_cpu::lm32_sr (const p_lm32_decode_t p)
{
    // If shifter implemented, execute instruction
    if (state.cfg & (1 << LM32_CFG_S))
    {
        state.r[p->reg2] = (state.r[p->reg0_csr] >> state.r[p->reg1]) | 
                           ((state.r[p->reg0_csr] & MASK_SIGN_BIT32) ? ~(WORD_MASK >>state.r[p->reg1]) : 0);
        state.pc += 4;

#ifndef LM32_FAST_COMPILE
        // Get any stall cycles on the instruction's input operands
        state.cycle_count += calc_stall(p->reg0_csr, p->reg1, state.cycle_count);
        rt[p->reg2] = state.cycle_count + p->decode->result_cycles;
        state.cycle_count += p->decode->issue_cycles;
#endif

    // Instruction not implemented
    } 
    else
    {
        lm32_rsrvd(p);
    }
}

// ------------------------------------------------------------

void lm32_cpu::lm32_sri (const p_lm32_decode_t p)
{
    // If shifter implemented, execute instruction
    if (state.cfg & (1 << LM32_CFG_S))
    {
        state.r[p->reg1] = (state.r[p->reg0_csr] >> p->imm) | 
                           ((state.r[p->reg0_csr] & MASK_SIGN_BIT32) ? ~(WORD_MASK >> p->imm) : 0);
        state.pc += 4;

#ifndef LM32_FAST_COMPILE
        // Get any stall cycles on the instruction's input operands
        state.cycle_count += calc_stall(p->reg0_csr, NULL_REG_IDX, state.cycle_count);
        rt[p->reg1] = state.cycle_count + p->decode->result_cycles;
        state.cycle_count += p->decode->issue_cycles;
#endif

    // Instruction not implemented
    }
    else
    {
        lm32_rsrvd(p);
    }
}

// ------------------------------------------------------------

void lm32_cpu::lm32_sru (const p_lm32_decode_t p)
{
    // If shifter implemented, execute instruction
    if (state.cfg & (1 << LM32_CFG_S)) 
    {
        state.r[p->reg2] = state.r[p->reg0_csr] >> state.r[p->reg1];  
        state.pc += 4;

#ifndef LM32_FAST_COMPILE
        // Get any stall cycles on the instruction's input operands
        state.cycle_count += calc_stall(p->reg0_csr, p->reg1, state.cycle_count);
        rt[p->reg2] = state.cycle_count + p->decode->result_cycles;
        state.cycle_count += p->decode->issue_cycles;
#endif

    // Instruction not implemented
    } 
    else
    {
        lm32_rsrvd(p);
    }
}

// ------------------------------------------------------------

void lm32_cpu::lm32_srui (const p_lm32_decode_t p)
{
    // If shifter implemented, execute instruction
    if (state.cfg & (1 << LM32_CFG_S))
    {
        state.r[p->reg1] = state.r[p->reg0_csr] >> p->imm;
        state.pc += 4;

#ifndef LM32_FAST_COMPILE
        // Get any stall cycles on the instruction's input operands
        state.cycle_count += calc_stall(p->reg0_csr, NULL_REG_IDX, state.cycle_count);
        rt[p->reg1] = state.cycle_count + p->decode->result_cycles;
        state.cycle_count += p->decode->issue_cycles;
#endif

    // Instruction not implemented
    }
    else
    {
        lm32_rsrvd(p);
    }
}

// ------------------------------------------------------------
// Logical instructions
// ------------------------------------------------------------

void lm32_cpu::lm32_and (const p_lm32_decode_t p)
{
    state.r[p->reg2] = state.r[p->reg0_csr] & state.r[p->reg1];
    state.pc += 4;

#ifndef LM32_FAST_COMPILE
    // Get any stall cycles on the instruction's input operands
    state.cycle_count += calc_stall(p->reg0_csr, p->reg1, state.cycle_count);
    rt[p->reg2] = state.cycle_count + p->decode->result_cycles;
    state.cycle_count += p->decode->issue_cycles;
#endif
}

// ------------------------------------------------------------

void lm32_cpu::lm32_andhi (const p_lm32_decode_t p)
{
    state.r[p->reg1] = state.r[p->reg0_csr] & (p->imm << 16);
    state.pc += 4;

#ifndef LM32_FAST_COMPILE
    // Get any stall cycles on the instruction's input operands
    state.cycle_count += calc_stall(p->reg0_csr, NULL_REG_IDX, state.cycle_count);
    rt[p->reg1] = state.cycle_count + p->decode->result_cycles;
    state.cycle_count += p->decode->issue_cycles;
#endif
}

// ------------------------------------------------------------

void lm32_cpu::lm32_andi (const p_lm32_decode_t p)
{
    state.r[p->reg1] = state.r[p->reg0_csr] & p->imm;
    state.pc += 4;

#ifndef LM32_FAST_COMPILE
    // Get any stall cycles on the instruction's input operands
    state.cycle_count += calc_stall(p->reg0_csr, NULL_REG_IDX, state.cycle_count);
    rt[p->reg1] = state.cycle_count + p->decode->result_cycles;
    state.cycle_count += p->decode->issue_cycles;
#endif
}

// ------------------------------------------------------------

void lm32_cpu::lm32_nor (const p_lm32_decode_t p)
{
    state.r[p->reg2] = ~(state.r[p->reg0_csr] | state.r[p->reg1]);
    state.pc += 4;

#ifndef LM32_FAST_COMPILE
    // Get any stall cycles on the instruction's input operands
    state.cycle_count += calc_stall(p->reg0_csr, p->reg1, state.cycle_count);
    rt[p->reg2] = state.cycle_count + p->decode->result_cycles;
    state.cycle_count += p->decode->issue_cycles;
#endif
}

// ------------------------------------------------------------

void lm32_cpu::lm32_nori (const p_lm32_decode_t p)
{
    state.r[p->reg1] = ~(state.r[p->reg0_csr] | p->imm);
    state.pc += 4;

#ifndef LM32_FAST_COMPILE
    // Get any stall cycles on the instruction's input operands
    state.cycle_count += calc_stall(p->reg0_csr, NULL_REG_IDX, state.cycle_count);
    rt[p->reg1] = state.cycle_count + p->decode->result_cycles;
    state.cycle_count += p->decode->issue_cycles;
#endif
}

// ------------------------------------------------------------

void lm32_cpu::lm32_or (const p_lm32_decode_t p)
{
    state.r[p->reg2] = state.r[p->reg0_csr] | state.r[p->reg1];
    state.pc += 4;

#ifndef LM32_FAST_COMPILE
    // Get any stall cycles on the instruction's input operands
    state.cycle_count += calc_stall(p->reg0_csr, p->reg1, state.cycle_count);
    rt[p->reg2] = state.cycle_count + p->decode->result_cycles;
    state.cycle_count += p->decode->issue_cycles;
#endif
}

// ------------------------------------------------------------

void lm32_cpu::lm32_orhi (const p_lm32_decode_t p)
{
    state.r[p->reg1] = state.r[p->reg0_csr] | (p->imm << 16);
    state.pc += 4;

#ifndef LM32_FAST_COMPILE
    // Get any stall cycles on the instruction's input operands
    state.cycle_count += calc_stall(p->reg0_csr, NULL_REG_IDX, state.cycle_count);
    rt[p->reg1] = state.cycle_count + p->decode->result_cycles;
    state.cycle_count += p->decode->issue_cycles;
#endif
}

// ------------------------------------------------------------

void lm32_cpu::lm32_ori (const p_lm32_decode_t p)
{
    state.r[p->reg1] = state.r[p->reg0_csr] | p->imm;
    state.pc += 4;

#ifndef LM32_FAST_COMPILE
    // Get any stall cycles on the instruction's input operands
    state.cycle_count += calc_stall(p->reg0_csr, NULL_REG_IDX, state.cycle_count);
    rt[p->reg1] = state.cycle_count + p->decode->result_cycles;
    state.cycle_count += p->decode->issue_cycles;
#endif
}

// ------------------------------------------------------------

void lm32_cpu::lm32_xnor (const p_lm32_decode_t p)
{
    state.r[p->reg2] = ~(state.r[p->reg0_csr] ^ state.r[p->reg1]);
    state.pc += 4;

#ifndef LM32_FAST_COMPILE
    // Get any stall cycles on the instruction's input operands
    state.cycle_count += calc_stall(p->reg0_csr, p->reg1, state.cycle_count);
    rt[p->reg2] = state.cycle_count + p->decode->result_cycles;
    state.cycle_count += p->decode->issue_cycles;
#endif
}

// ------------------------------------------------------------

void lm32_cpu::lm32_xnori (const p_lm32_decode_t p)
{
    state.r[p->reg1] = ~(state.r[p->reg0_csr] ^ p->imm);
    state.pc += 4;

#ifndef LM32_FAST_COMPILE
    // Get any stall cycles on the instruction's input operands
    state.cycle_count += calc_stall(p->reg0_csr, NULL_REG_IDX, state.cycle_count);
    rt[p->reg1] = state.cycle_count + p->decode->result_cycles;
    state.cycle_count += p->decode->issue_cycles;
#endif
}

// ------------------------------------------------------------

void lm32_cpu::lm32_xor (const p_lm32_decode_t p)
{
    state.r[p->reg2] =  state.r[p->reg0_csr] ^ state.r[p->reg1];
    state.pc += 4;

#ifndef LM32_FAST_COMPILE
    // Get any stall cycles on the instruction's input operands
    state.cycle_count += calc_stall(p->reg0_csr, p->reg1, state.cycle_count);
    rt[p->reg2] = state.cycle_count + p->decode->result_cycles;
    state.cycle_count += p->decode->issue_cycles;
#endif
}

// ------------------------------------------------------------

void lm32_cpu::lm32_xori (const p_lm32_decode_t p)
{
    state.r[p->reg1] = state.r[p->reg0_csr] ^ p->imm;
    state.pc += 4;

#ifndef LM32_FAST_COMPILE
    // Get any stall cycles on the instruction's input operands
    state.cycle_count += calc_stall(p->reg0_csr, NULL_REG_IDX, state.cycle_count);
    rt[p->reg1] = state.cycle_count + p->decode->result_cycles;
    state.cycle_count += p->decode->issue_cycles;
#endif
}

// ------------------------------------------------------------
// Flow control instructions
// ------------------------------------------------------------

void lm32_cpu::lm32_b (const p_lm32_decode_t p)
{
    if (p->reg0_csr == BA_REG_IDX)
    {
        state.ie = (state.ie & ~IE_IE_MASK) | ((state.ie & IE_BIE_MASK) ? IE_IE_MASK : 0);
    } else if (p->reg0_csr == EA_REG_IDX)
    {
        state.ie = (state.ie & ~IE_IE_MASK) | ((state.ie & IE_EIE_MASK) ? IE_IE_MASK : 0);
    }
    state.pc = state.r[p->reg0_csr];

#ifndef LM32_FAST_COMPILE
    state.cycle_count += p->decode->issue_cycles;
#endif
}

// ------------------------------------------------------------

void lm32_cpu::lm32_be (const p_lm32_decode_t p)
{
    if (state.r[p->reg0_csr] == state.r[p->reg1])
    {
        state.pc = (uint32_t)((int32_t)state.pc + SIGN_EXT18(p->imm << 2));
    }
    else
    {
        state.pc += 4;
    }

#ifndef LM32_FAST_COMPILE
    // Get any stall cycles on the instruction's input operands
    state.cycle_count += calc_stall(p->reg0_csr, p->reg1, state.cycle_count);
    state.cycle_count += (state.r[p->reg0_csr] == state.r[p->reg1]) ? p->decode->issue_cycles : p->decode->issue_not_takencycles;
#endif
}

// ------------------------------------------------------------

void lm32_cpu::lm32_bg (const p_lm32_decode_t p)
{
    if ((int32_t)state.r[p->reg0_csr] > (int32_t)state.r[p->reg1])
    {
        state.pc = (uint32_t)((int32_t)state.pc + SIGN_EXT18(p->imm << 2));
    }
    else
    {
        state.pc += 4;
    }

#ifndef LM32_FAST_COMPILE
    // Get any stall cycles on the instruction's input operands
    state.cycle_count += calc_stall(p->reg0_csr, p->reg1, state.cycle_count);
    state.cycle_count += ((int32_t)state.r[p->reg0_csr] > (int32_t)state.r[p->reg1]) ? p->decode->issue_cycles : p->decode->issue_not_takencycles;
#endif
}

// ------------------------------------------------------------

void lm32_cpu::lm32_bge (const p_lm32_decode_t p)
{
    if ((int32_t)state.r[p->reg0_csr] >= (int32_t)state.r[p->reg1])
    {
        state.pc = (uint32_t)((int32_t)state.pc + SIGN_EXT18(p->imm << 2));
    }
    else
    {
        state.pc += 4;
    }

#ifndef LM32_FAST_COMPILE
    // Get any stall cycles on the instruction's input operands
    state.cycle_count += calc_stall(p->reg0_csr, p->reg1, state.cycle_count);
    state.cycle_count += ((int32_t)state.r[p->reg0_csr] >= (int32_t)state.r[p->reg1]) ? p->decode->issue_cycles : p->decode->issue_not_takencycles;
#endif
}

// ------------------------------------------------------------

void lm32_cpu::lm32_bgeu (const p_lm32_decode_t p)
{
    if (state.r[p->reg0_csr] >= state.r[p->reg1])
    {
        state.pc = (uint32_t)((int32_t)state.pc + SIGN_EXT18(p->imm << 2));
    }
    else
    {
        state.pc += 4;
    }

#ifndef LM32_FAST_COMPILE
    // Get any stall cycles on the instruction's input operands
    state.cycle_count += calc_stall(p->reg0_csr, p->reg1, state.cycle_count);
    state.cycle_count += (state.r[p->reg0_csr] >= state.r[p->reg1]) ? p->decode->issue_cycles : p->decode->issue_not_takencycles;
#endif
}

// ------------------------------------------------------------

void lm32_cpu::lm32_bgu (const p_lm32_decode_t p)
{
    if (state.r[p->reg0_csr] > state.r[p->reg1])
    {
        state.pc = (uint32_t)((int32_t)state.pc + SIGN_EXT18(p->imm << 2));
    }
    else
    {
        state.pc += 4;
    }

#ifndef LM32_FAST_COMPILE
    // Get any stall cycles on the instruction's input operands
    state.cycle_count += calc_stall(p->reg0_csr, p->reg1, state.cycle_count);
    state.cycle_count += (state.r[p->reg0_csr] > state.r[p->reg1]) ? p->decode->issue_cycles : p->decode->issue_not_takencycles;
#endif
}

// ------------------------------------------------------------

void lm32_cpu::lm32_bi (const p_lm32_decode_t p)
{
    state.pc = (uint32_t)((int32_t)state.pc + SIGN_EXT28(p->imm << 2));

#ifndef LM32_FAST_COMPILE
    // Get any stall cycles on the instruction's input operands
    state.cycle_count += calc_stall(p->reg0_csr, p->reg1, state.cycle_count);
    state.cycle_count += p->decode->issue_cycles;
#endif
}

// ------------------------------------------------------------

void lm32_cpu::lm32_bne (const p_lm32_decode_t p)
{
    if (state.r[p->reg1] != state.r[p->reg0_csr])
    {
        state.pc = (uint32_t)((int32_t)state.pc + SIGN_EXT18(p->imm << 2));
    }
    else
    {
        state.pc += 4;
    }

#ifndef LM32_FAST_COMPILE
    // Get any stall cycles on the instruction's input operands
    state.cycle_count += calc_stall(p->reg0_csr, p->reg1, state.cycle_count);
    state.cycle_count += (state.r[p->reg1] != state.r[p->reg0_csr]) ? p->decode->issue_cycles : p->decode->issue_not_takencycles;
#endif
}

// ------------------------------------------------------------

void lm32_cpu::lm32_call (const p_lm32_decode_t p)
{
    state.r[RA_REG_IDX] = state.pc + 4;
    state.pc = state.r[p->reg0_csr];

#ifndef LM32_FAST_COMPILE
    // Get any stall cycles on the instruction's input operands
    state.cycle_count += calc_stall(p->reg0_csr, NULL_REG_IDX, state.cycle_count);
    rt[RA_REG_IDX] = state.cycle_count + p->decode->result_cycles;
    state.cycle_count += p->decode->issue_cycles;
#endif
}

// ------------------------------------------------------------

void lm32_cpu::lm32_calli (const p_lm32_decode_t p)
{

    state.r[RA_REG_IDX] = state.pc + 4;
    state.pc = (uint32_t)((int32_t)state.pc + SIGN_EXT28(p->imm << 2));

#ifndef LM32_FAST_COMPILE
    rt[RA_REG_IDX] = state.cycle_count + p->decode->result_cycles;
    state.cycle_count += p->decode->issue_cycles;
#endif
}

// ------------------------------------------------------------
// Memory access instructions
// ------------------------------------------------------------

void lm32_cpu::lm32_lb (const p_lm32_decode_t p)
{
#ifndef LNXMICO32
    state.r[p->reg1] = SIGN_EXT8(lm32_read_mem(state.r[p->reg0_csr]+SIGN_EXT16(p->imm), LM32_MEM_RD_ACCESS_BYTE));
#else
    state.r[p->reg1] = SIGN_EXT8(lm32_read_byte(state.r[p->reg0_csr] + SIGN_EXT16(p->imm)));
#endif
    state.pc += 4;

#ifndef LM32_FAST_COMPILE
    // Get any stall cycles on the instruction's input operands
    state.cycle_count += calc_stall(p->reg0_csr, NULL_REG_IDX, state.cycle_count);
    rt[p->reg1] = state.cycle_count + p->decode->result_cycles;
    state.cycle_count += p->decode->issue_cycles;
#endif
}

// ------------------------------------------------------------

void lm32_cpu::lm32_lbu (const p_lm32_decode_t p)
{
#ifndef LNXMICO32
    state.r[p->reg1] = lm32_read_mem(state.r[p->reg0_csr]+SIGN_EXT16(p->imm), LM32_MEM_RD_ACCESS_BYTE);
#else
    state.r[p->reg1] = lm32_read_byte(state.r[p->reg0_csr] + SIGN_EXT16(p->imm));
#endif
    state.pc += 4;

#ifndef LM32_FAST_COMPILE
    // Get any stall cycles on the instruction's input operands
    state.cycle_count += calc_stall(p->reg0_csr, NULL_REG_IDX, state.cycle_count);
    rt[p->reg1] = state.cycle_count + p->decode->result_cycles;
    state.cycle_count += p->decode->issue_cycles;
#endif
}

// ------------------------------------------------------------

void lm32_cpu::lm32_lh (const p_lm32_decode_t p)
{
#ifndef LNXMICO32
    state.r[p->reg1] = SIGN_EXT16(lm32_read_mem(state.r[p->reg0_csr]+SIGN_EXT16(p->imm), LM32_MEM_RD_ACCESS_HWORD));
#else
    state.r[p->reg1] = SIGN_EXT16(lm32_read_hword(state.r[p->reg0_csr] + SIGN_EXT16(p->imm)));
#endif
    state.pc += 4;

#ifndef LM32_FAST_COMPILE
    // Get any stall cycles on the instruction's input operands
    state.cycle_count += calc_stall(p->reg0_csr, NULL_REG_IDX, state.cycle_count);
    rt[p->reg1] = state.cycle_count + p->decode->result_cycles;
    state.cycle_count += p->decode->issue_cycles;
#endif
}

// ------------------------------------------------------------

void lm32_cpu::lm32_lhu (const p_lm32_decode_t p)
{
#ifndef LNXMICO32
    state.r[p->reg1] = lm32_read_mem(state.r[p->reg0_csr]+SIGN_EXT16(p->imm), LM32_MEM_RD_ACCESS_HWORD);
#else
    state.r[p->reg1] = lm32_read_hword(state.r[p->reg0_csr] + SIGN_EXT16(p->imm));
#endif
    state.pc += 4;

#ifndef LM32_FAST_COMPILE
    // Get any stall cycles on the instruction's input operands
    state.cycle_count += calc_stall(p->reg0_csr, NULL_REG_IDX, state.cycle_count);
    rt[p->reg1] = state.cycle_count + p->decode->result_cycles;
    state.cycle_count += p->decode->issue_cycles;
#endif
}

// ------------------------------------------------------------

void lm32_cpu::lm32_lw (const p_lm32_decode_t p)
{
#ifndef LNXMICO32
    state.r[p->reg1] = lm32_read_mem(state.r[p->reg0_csr]+SIGN_EXT16(p->imm), LM32_MEM_RD_ACCESS_WORD);
#else
    state.r[p->reg1] = lm32_read_word(state.r[p->reg0_csr] + SIGN_EXT16(p->imm));
#endif
    state.pc += 4;

#ifndef LM32_FAST_COMPILE
    // Get any stall cycles on the instruction's input operands
    state.cycle_count += calc_stall(p->reg0_csr, NULL_REG_IDX, state.cycle_count);
    rt[p->reg1] = state.cycle_count + p->decode->result_cycles;
    state.cycle_count += p->decode->issue_cycles;
#endif
}

// ------------------------------------------------------------

void lm32_cpu::lm32_sb (const p_lm32_decode_t p)
{
#ifndef LNXMICO32
    lm32_write_mem(state.r[p->reg0_csr]+SIGN_EXT16(p->imm), state.r[p->reg1] & BYTE_MASK, LM32_MEM_WR_ACCESS_BYTE);
#else
    lm32_write_byte(state.r[p->reg0_csr] + SIGN_EXT16(p->imm), state.r[p->reg1] & BYTE_MASK);
#endif
    state.pc += 4;

#ifndef LM32_FAST_COMPILE
    // Get any stall cycles on the instruction's input operands
    state.cycle_count += calc_stall(p->reg0_csr, NULL_REG_IDX, state.cycle_count);
    state.cycle_count += p->decode->issue_cycles;
#endif
}

// ------------------------------------------------------------

void lm32_cpu::lm32_sh (const p_lm32_decode_t p)
{
#ifndef LNXMICO32
    lm32_write_mem(state.r[p->reg0_csr]+SIGN_EXT16(p->imm), state.r[p->reg1] & HWORD_MASK, LM32_MEM_WR_ACCESS_HWORD);
#else
    lm32_write_hword(state.r[p->reg0_csr] + SIGN_EXT16(p->imm), state.r[p->reg1] & HWORD_MASK);
#endif
    state.pc += 4;

#ifndef LM32_FAST_COMPILE
    // Get any stall cycles on the instruction's input operands
    state.cycle_count += calc_stall(p->reg0_csr, NULL_REG_IDX, state.cycle_count);
    state.cycle_count += p->decode->issue_cycles;
#endif
}

// ------------------------------------------------------------

void lm32_cpu::lm32_sw (const p_lm32_decode_t p)
{
#ifndef LNXMICO32
    lm32_write_mem(state.r[p->reg0_csr]+SIGN_EXT16(p->imm), state.r[p->reg1], LM32_MEM_WR_ACCESS_WORD);
#else
    lm32_write_word(state.r[p->reg0_csr] + SIGN_EXT16(p->imm), state.r[p->reg1]);
#endif
    state.pc += 4;

#ifndef LM32_FAST_COMPILE
    // Get any stall cycles on the instruction's input operands
    state.cycle_count += calc_stall(p->reg0_csr, NULL_REG_IDX, state.cycle_count);
    state.cycle_count += p->decode->issue_cycles;
#endif
}

// ------------------------------------------------------------
// Control and status register access instructions
// ------------------------------------------------------------

void lm32_cpu::lm32_rcsr (const p_lm32_decode_t p)
{
    switch(p->reg0_csr) {

    case CSR_ID_IE:   state.r[p->reg2]  =   state.ie;  break;
    case CSR_ID_IM:   state.r[p->reg2]  =   state.im;  break;
    case CSR_ID_IP:   state.r[p->reg2]  =   state.ip;  break;
    case CSR_ID_ICC:  state.r[p->reg2]  =          0;  break;
    case CSR_ID_DCC:  state.r[p->reg2]  =          0;  break;
    case CSR_ID_CFG:  state.r[p->reg2]  =  state.cfg;  break;
    case CSR_ID_EBA:  state.r[p->reg2]  =  state.eba;  break;
    case CSR_ID_CFG2: state.r[p->reg2]  = state.cfg2;  break;

    case CSR_ID_DC:   state.r[p->reg2]  =   state.dc;  break;
    case CSR_ID_DEBA: state.r[p->reg2]  = state.deba;  break;
    case CSR_ID_BP0:  state.r[p->reg2]  = (((state.cfg >> LM32_CFG_BP) & LM32_CFG_BP_MASK) > 0) ?  state.bp0 : 0;  break;
    case CSR_ID_BP1:  state.r[p->reg2]  = (((state.cfg >> LM32_CFG_BP) & LM32_CFG_BP_MASK) > 1) ?  state.bp1 : 0;  break;
    case CSR_ID_BP2:  state.r[p->reg2]  = (((state.cfg >> LM32_CFG_BP) & LM32_CFG_BP_MASK) > 2) ?  state.bp2 : 0;  break;
    case CSR_ID_BP3:  state.r[p->reg2]  = (((state.cfg >> LM32_CFG_BP) & LM32_CFG_BP_MASK) > 3) ?  state.bp3 : 0;  break;
    case CSR_ID_WP0:  state.r[p->reg2]  = (((state.cfg >> LM32_CFG_WP) & LM32_CFG_WP_MASK) > 0) ?  state.wp0 : 0;  break;
    case CSR_ID_WP1:  state.r[p->reg2]  = (((state.cfg >> LM32_CFG_WP) & LM32_CFG_WP_MASK) > 1) ?  state.wp1 : 0;  break;
    case CSR_ID_WP2:  state.r[p->reg2]  = (((state.cfg >> LM32_CFG_WP) & LM32_CFG_WP_MASK) > 2) ?  state.wp2 : 0;  break;
    case CSR_ID_WP3:  state.r[p->reg2]  = (((state.cfg >> LM32_CFG_WP) & LM32_CFG_WP_MASK) > 3) ?  state.wp3 : 0;  break;

    case CSR_ID_JTX:
        state.r[p->reg2] = 0;
        if ((state.cfg >> LM32_CFG_J) & 1)
        {
            if (pJtagCallback != NULL)
            {
                pJtagCallback (&state.jtx, LM32_JTX_RD, state.cycle_count);
            }
                
            state.jtx &= ~(0xfffffe00);
            state.r[p->reg2] = state.jtx;
        }
        break;

    case CSR_ID_JRX:
        state.r[p->reg2] = 0;
        if ((state.cfg >> LM32_CFG_J) & 1)
        {
            if (pJtagCallback != NULL)
            {
                pJtagCallback (&state.jrx, LM32_JRX_RD, state.cycle_count);
            }
                
            state.jrx &= ~(0xfffffe00);
            state.r[p->reg2] = state.jrx;
        }
        break;

    case CSR_ID_CC:
        state.cc          = (uint32_t)((state.cfg & (1 << LM32_CFG_CC)) ? (uint32_t)((state.cycle_count + cc_adjust) & 0xffffffffULL) : 0);
        state.r[p->reg2]  = state.cc;
        break;

    default:
        fprintf(stderr, "***ERROR: invalid CSR index (%d)\n", p->reg0_csr); //LCOV_EXCL_LINE
        exit(LM32_INSTR_ERROR);                                             //LCOV_EXCL_LINE
        break;
    }

    state.pc += 4;

#ifndef LM32_FAST_COMPILE
    rt[p->reg2] = state.cycle_count + p->decode->result_cycles;
    state.cycle_count += p->decode->issue_cycles;
#endif
}

// ------------------------------------------------------------

void lm32_cpu::lm32_wcsr (const p_lm32_decode_t p)
{
    int num_ints;

    switch(p->reg0_csr)
    {
    case CSR_ID_IE:   state.ie          = state.r[p->reg1] & 0x00000007;  break;
    case CSR_ID_IM:
        num_ints = (state.cfg >> LM32_CFG_INT) & LM32_CFG_INT_MASK;
        if (num_ints > 0)
        {
            state.im  = state.r[p->reg1] & ((1ULL << num_ints)-1);
        }
        break;
    case CSR_ID_IP:   state.ip          = state.ip & ~state.r[p->reg1];   break;            // Write 1 to clear
    case CSR_ID_CFG:                                                      break;
    case CSR_ID_EBA:  state.eba         = state.r[p->reg1] & 0xffffff00;  break;
    case CSR_ID_CFG2:                                                     break;

    case CSR_ID_DC:   state.dc          = state.r[p->reg1];               break; 
    case CSR_ID_DEBA: state.deba        = state.r[p->reg1] & 0xffffff00;  break;
    case CSR_ID_BP0:  state.bp0         = (((state.cfg >> LM32_CFG_BP) & LM32_CFG_BP_MASK) > 0) ? state.r[p->reg1] & 0xfffffffd : 0;  break;
    case CSR_ID_BP1:  state.bp1         = (((state.cfg >> LM32_CFG_BP) & LM32_CFG_BP_MASK) > 1) ? state.r[p->reg1] & 0xfffffffd : 0;  break;
    case CSR_ID_BP2:  state.bp2         = (((state.cfg >> LM32_CFG_BP) & LM32_CFG_BP_MASK) > 2) ? state.r[p->reg1] & 0xfffffffd : 0;  break;
    case CSR_ID_BP3:  state.bp3         = (((state.cfg >> LM32_CFG_BP) & LM32_CFG_BP_MASK) > 3) ? state.r[p->reg1] & 0xfffffffd : 0;  break;
    case CSR_ID_WP0:  state.wp0         = (((state.cfg >> LM32_CFG_BP) & LM32_CFG_WP_MASK) > 0) ? state.r[p->reg1] : 0;               break;
    case CSR_ID_WP1:  state.wp1         = (((state.cfg >> LM32_CFG_BP) & LM32_CFG_WP_MASK) > 1) ? state.r[p->reg1] : 0;               break;
    case CSR_ID_WP2:  state.wp2         = (((state.cfg >> LM32_CFG_BP) & LM32_CFG_WP_MASK) > 2) ? state.r[p->reg1] : 0;               break;
    case CSR_ID_WP3:  state.wp3         = (((state.cfg >> LM32_CFG_BP) & LM32_CFG_WP_MASK) > 3) ? state.r[p->reg1] : 0;               break;
    case CSR_ID_JRX:                                                      break;

    case CSR_ID_ICC:
        icc_invalidate = true;
        break;

    case CSR_ID_DCC:
        dcc_invalidate = true;
        break;

    case CSR_ID_JTX:
        if ((state.cfg >> LM32_CFG_J) & 1)
        {
            state.jtx  = state.r[p->reg1] & ~(0xffffff00);

            if (pJtagCallback != NULL)
            {
                pJtagCallback (&state.jtx, LM32_JTX_WR, state.cycle_count);
            }
        }
        break;

    case CSR_ID_CC:   
        cc_adjust         = (lm32_time_t)state.r[p->reg1] - state.cycle_count;
        state.cc          = (state.cfg & (1 << LM32_CFG_CC)) ? state.r[p->reg1] : 0;
        break;

    default:
        fprintf(stderr, "***ERROR: invalid CSR index (%d)\n", p->reg0_csr); //LCOV_EXCL_LINE
        exit(LM32_INSTR_ERROR);                                             //LCOV_EXCL_LINE
        break;
    }

    state.pc += 4;

#ifndef LM32_FAST_COMPILE
    state.cycle_count += p->decode->issue_cycles;
#endif
}
