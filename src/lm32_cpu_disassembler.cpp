//=============================================================
// 
// Copyright (c) 2013-2016 Simon Southwell. All rights reserved.
//
// Date: 9th April 2013
//
// Contains the disassembler methods for lm32_cpu class
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
// $Id: lm32_cpu_disassembler.cpp,v 2.5 2016-09-03 07:44:05 simon Exp $
// $Source: /home/simon/CVS/src/cpu/mico32/src/lm32_cpu_disassembler.cpp,v $
//
//=============================================================

// -------------------------------------------------------------------------
// INCLUDES
// -------------------------------------------------------------------------

#include <cstdio>

#include "lm32_cpu.h"
#include "lm32_cpu_mico32.h"

// -------------------------------------------------------------------------
// fmt_register()
//
// Generates string from register number for use in disassemble()
//
// -------------------------------------------------------------------------

char *lm32_cpu::fmt_register(const int regnum, char* str, const bool end)
{
    if (regnum == GP_REG_IDX)
    {
        sprintf(str, "gp%s ", end ? "" : ",");
    }
    else if (regnum == FP_REG_IDX)
    {
        sprintf(str, "fp%s ", end ? "" : ",");
    }
    else if (regnum == SP_REG_IDX)
    {
        sprintf(str, "sp%s ", end ? "" : ",");
    }
    else if (regnum == RA_REG_IDX)
    {
        sprintf(str, "ra%s ", end ? "" : ",");
    }
    else if (regnum == EA_REG_IDX)
    {
        sprintf(str, "ea%s ", end ? "" : ",");
    }
    else if (regnum == BA_REG_IDX)
    {
        sprintf(str, "ba%s ", end ? "" : ",");
    }
    else if (regnum < 10)
    {
        sprintf(str, "r%1d%s ", regnum, end ? "" : ",");
    }
    else
    {
        sprintf(str, "r%2d%s", regnum, end ? "" : ",");
    }

    return str;
}

// -------------------------------------------------------------------------
// disassemble()
//
// Disassembles received (and decoded) instruction, placing string 
// represetation on the specified output stream.
//
// -------------------------------------------------------------------------

void lm32_cpu::disassemble(const p_lm32_decode_t d, FILE *ofp, const bool disassemble_run)
{
    // Remember the last disassembled PC to aid in display of jumps
    static int last_disassembled_pc = -1;

    // List of CSR register strings
    static const char* csr_name_str[] = CSR_NAMES;

    // Some space for constructing strings
    static char str[3][20];

    int32_t imm_ext;

    if (last_disassembled_pc != -1 && (last_disassembled_pc+4) != state.pc)
    {
        fprintf(ofp, "*\n");
    }

    fprintf(ofp, "0x%08x: (0x%08x)   %s", state.pc, d->opcode, (d->opcode == INSTR_NOP)   ? "nop  " :
                                                               (d->opcode == INSTR_BREAK) ? "break    " :
                                                               (d->opcode == INSTR_SCALL) ? "scall    " :
                                                                                           d->decode->instr_name);

    switch(d->decode->instr_fmt)
    {
    case INSTR_FMT_RI:
 
        // Sign extend the immediate value, if indicated in the decode table
        imm_ext = (d->decode->signed_imm == INSTR_SE_TRUE) ? SIGN_EXT16(d->imm) : d->imm;

        if (d->opcode == INSTR_NOP)
        {
            fprintf(ofp, "                       ");
        }
        // Load instruction special case: rx, (ry + imm)
        else if (d->decode->instr_name[0] == 'l')
        {
            fprintf(ofp, "%s(%s%+06d)    ",  fmt_register(d->reg1, str[0], false), fmt_register(d->reg0_csr, str[1], true), imm_ext);
        }
        // Store instruction special case: (rx + imm), ry
        else if (d->decode->instr_name[0] == 's' && (d->decode->instr_name[1] == 'b' || d->decode->instr_name[1] == 'h' || d->decode->instr_name[1] == 'w'))
        {
            fprintf(ofp, "(%s%+06d), %s   ",  fmt_register(d->reg0_csr, str[0], true), imm_ext, fmt_register(d->reg1, str[1], true));
        }
        // Branch imm16 instruction special case : bxxx rx, ry, imm16
        else if (d->decode->instr_name[0] == 'b')
        {
            fprintf(ofp, "%s %s %07d  ",   fmt_register(d->reg0_csr, str[0], false), fmt_register(d->reg1, str[1], false), imm_ext << 2);
        }
        // Logic instruction special case
        else if (d->decode->signed_imm == INSTR_SE_FALSE_HEX)
        {
            fprintf(ofp, "%s %s 0x%04x   ",   fmt_register(d->reg1, str[0], false), fmt_register(d->reg0_csr, str[1], false), imm_ext);
        }
        // All other RI instructions
        else
        {
            fprintf(ofp, "%s %s %06d   ",   fmt_register(d->reg1, str[0], false), fmt_register(d->reg0_csr, str[1], false), imm_ext);
        }
        break;

    case INSTR_FMT_RR:
        fprintf(ofp, "%s %s %s      ",        fmt_register(d->reg2, str[0], false), fmt_register(d->reg0_csr, str[1], false), fmt_register(d->reg1, str[2], true));
        break;

    case INSTR_FMT_BR:
        fprintf(ofp, "%s                ",    fmt_register(d->reg0_csr, str[0], true));
        break;

    case INSTR_FMT_CR:
        if (d->decode->instr_name[0] == 'r')
        {
            fprintf(ofp, "%s %s          ",       fmt_register(d->reg2, str[0], false), csr_name_str[d->reg0_csr]);
        }
        else
        {
            fprintf(ofp, "%s, %s          ",       csr_name_str[d->reg0_csr], fmt_register(d->reg1, str[0], true));
        }
        break;

    case INSTR_FMT_RC:
        fprintf(ofp, "%s %s           ",      fmt_register(d->reg2, str[0], false), fmt_register(d->reg0_csr, str[1], true));
        break;

    case INSTR_FMT_IM:
        fprintf(ofp, "[0x%08x]       ",       state.pc + SIGN_EXT28(d->imm*4));
        break;

    case INSTR_FMT_BK:
        fprintf(ofp, "                   ");
        break;

    default:
        fprintf(ofp, "** invalid format **");  //LCOV_EXCL_LINE
        break;                                 //LCOV_EXCL_LINE
    }

    if (disassemble_run == false)
    {
        fprintf(ofp, "  @%ld", state.cycle_count);
    }

    fprintf(ofp, "\n");

    last_disassembled_pc = state.pc;
}

