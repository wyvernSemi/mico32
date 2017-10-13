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
// $Id: lm32_cpu_disassembler.cpp,v 3.4 2017/10/13 14:32:12 simon Exp $
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
// lm32_dump_registers()
// 
// Dump the registers to the specified output stream
// 
// -------------------------------------------------------------------------

void lm32_cpu::lm32_dump_registers()
{
    int idx;

    fprintf(ofp, "\n");
    for(idx = 0; idx < LM32_NUM_OF_REGISTERS-6; idx++)
    {
        fprintf(ofp, "r%02d = 0x%08x  ", idx, state.r[idx]);

        if ((idx % 4) == 3)
        {
            fprintf(ofp, "\n");
        }
    }

    fprintf(ofp, "gp  = 0x%08x  ",  state.r[idx++]);
    fprintf(ofp, "fp  = 0x%08x  ",  state.r[idx++]);

    fprintf(ofp, "\n");
    fprintf(ofp, "sp  = 0x%08x  ",  state.r[idx++]);
    fprintf(ofp, "ra  = 0x%08x  ",  state.r[idx++]);
    fprintf(ofp, "ea  = 0x%08x  ",  state.r[idx++]);
    fprintf(ofp, "ba  = 0x%08x\n",  state.r[idx++]);

    fprintf(ofp, "\n");
    fprintf(ofp, "pc  = 0x%08x  ",  state.pc);
    fprintf(ofp, "ie  = 0x%08x  ",  state.ie);
    fprintf(ofp, "ip  = 0x%08x  ",  state.ip);
    fprintf(ofp, "im  = 0x%08x\n",  state.im);

    fprintf(ofp, "icc = 0x%08x  ",  state.icc);
    fprintf(ofp, "dcc = 0x%08x  ",  state.dcc);
    fprintf(ofp, "cfg = 0x%08x ",   state.cfg);
    fprintf(ofp, "cfg2 = 0x%08x\n", state.cfg2);

    fprintf(ofp, "cc  = 0x%08x  ",  (uint32_t)((state.cycle_count + cc_adjust) & 0xffffffffULL));
    fprintf(ofp, "eba = 0x%08x ",   state.eba);

    fprintf(ofp, "\n");

    fprintf(ofp, "\n");
    fprintf(ofp, "bp0 = 0x%08x  ",  state.bp0);
    fprintf(ofp, "bp1 = 0x%08x  ",  state.bp1);
    fprintf(ofp, "bp2 = 0x%08x  ",  state.bp2);
    fprintf(ofp, "bp3 = 0x%08x\n",  state.bp3);

    fprintf(ofp, "wp0 = 0x%08x  ",  state.wp0);
    fprintf(ofp, "wp1 = 0x%08x  ",  state.wp1);
    fprintf(ofp, "wp2 = 0x%08x  ",  state.wp2);
    fprintf(ofp, "wp3 = 0x%08x\n",  state.wp3);

    fprintf(ofp, "dc  = 0x%08x ",  state.dc);
    fprintf(ofp, "deba = 0x%08x ",  state.deba);

    fprintf(ofp, "\n");

    #if LM32_MMU
    fprintf(ofp, "\n");
    fprintf(ofp, "psw = 0x%08x ",    state.psw);
    fprintf(ofp, "tlbvaddr = 0x%08x ",  state.tlbvaddr);
    fprintf(ofp, "tlbbadvaddr = 0x%08x\n",  state.tlbbadvaddr);

    fprintf(ofp, "\n");
#endif
}

// -------------------------------------------------------------------------
// lm32_dump_op_stats()
// -------------------------------------------------------------------------
#ifndef LM32_FAST_COMPILE
void lm32_cpu::lm32_dump_op_stats()
{
    fprintf(ofp, "\n # OPCODE     COUNT\n");

    for (int idx = 0; idx < LM32_NUM_OPCODES; idx++)
    {
        const char *op_name = decode_table[idx].instr_name;

        fprintf(ofp, "%2d %s: %lld\n", idx, op_name, op_count[idx]);
    }
}
#endif
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
            fprintf(ofp, "%s %s      %s",       fmt_register(d->reg2, str[0], false), csr_name_str[d->reg0_csr], d->reg0_csr >= LM32_CSR_ID_TLBVADDR ? "" : "    ");
        }
        else
        {
            fprintf(ofp, "%s, %s      %s",       csr_name_str[d->reg0_csr], fmt_register(d->reg1, str[0], true), d->reg0_csr >= LM32_CSR_ID_TLBVADDR ? "" : "    ");
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

