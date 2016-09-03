//=============================================================
// 
// Copyright (c) 2013 Simon Southwell. All rights reserved.
//
// Date: 7th June 2013
//
// The 'C' linkage interface for the model.
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
// $Id: lm32_cpu_c.cpp,v 2.4 2016-09-03 07:44:06 simon Exp $
// $Source: /home/simon/CVS/src/cpu/mico32/src/lm32_cpu_c.cpp,v $
//
//=============================================================

// -------------------------------------------------------------------------
// INCLUDES
// -------------------------------------------------------------------------

#include <cstdio>
#include <stdio.h>
#include <stdint.h>

#include "lm32_cpu.h"
#include "lm32_cpu_c.h"

// -------------------------------------------------------------------------

extern "C" lm32c_hdl lm32c_cpu_init(int verbose, int disable_reset_break, int disable_lock_break, int disable_hw_break, int disassemble_run,
                                    uint32_t num_mem_bytes, uint32_t mem_offset, int mem_wait_states, uint32_t entry_point_addr, uint32_t cfg_word, FILE* ofp,
                                    lm32_cache_config_t* p_dcache_cfg, lm32_cache_config_t* p_icache_cfg)
{
    lm32_cpu* cpu;
    cpu = new lm32_cpu(verbose, 
                       disable_reset_break ? true : false,
                       disable_lock_break  ? true : false,
                       disable_hw_break    ? true : false,
                       disassemble_run     ? true : false,
                       num_mem_bytes,
                       mem_offset,
                       mem_wait_states,
                       entry_point_addr,
                       cfg_word,
                       ofp,
                       p_dcache_cfg,
                       p_icache_cfg);

    return (lm32c_hdl) cpu;
}

// -------------------------------------------------------------------------

extern "C" int lm32c_run_program (lm32c_hdl cpu_hdl, const char* elf_fname, int run_cycles, int break_addr, int exec_type, int load_code)
{
    lm32_cpu* cpu = (lm32_cpu*)cpu_hdl;
    return cpu->lm32_run_program(elf_fname, run_cycles, break_addr, exec_type, load_code ? true : false);
}

// -------------------------------------------------------------------------

extern "C" void lm32c_reset_cpu(lm32c_hdl cpu_hdl)
{
    lm32_cpu* cpu = (lm32_cpu*)cpu_hdl;
    cpu->lm32_reset_cpu();
}

// -------------------------------------------------------------------------

extern "C" void lm32c_register_int_callback (lm32c_hdl cpu_hdl, p_lm32_intcallback_t callback_func)
{
    lm32_cpu* cpu = (lm32_cpu*)cpu_hdl;
    cpu->lm32_register_int_callback(callback_func);
}

// -------------------------------------------------------------------------

extern "C" void lm32c_register_ext_mem_callback (lm32c_hdl cpu_hdl, p_lm32_memcallback_t callback_func)
{
    lm32_cpu* cpu = (lm32_cpu*)cpu_hdl;
    cpu->lm32_register_ext_mem_callback(callback_func);
}

// --------------------------------------------------------------------------

extern "C" void lm32c_register_jtag_callback (lm32c_hdl cpu_hdl, p_lm32_jtagcallback_t callback_func)
{
    lm32_cpu* cpu = (lm32_cpu*)cpu_hdl;
    cpu->lm32_register_jtag_callback(callback_func);
}

// -------------------------------------------------------------------------

extern "C" void lm32c_set_verbosity_level(lm32c_hdl cpu_hdl, int level)
{
    lm32_cpu* cpu = (lm32_cpu*)cpu_hdl;
    cpu->lm32_set_verbosity_level(level);
}

// -------------------------------------------------------------------------

extern "C" lm32_time_t lm32c_get_num_instructions(lm32c_hdl cpu_hdl)
{
    lm32_cpu* cpu = (lm32_cpu*)cpu_hdl;
    return cpu->lm32_get_num_instructions();
}

// -------------------------------------------------------------------------

extern "C" lm32_time_t lm32c_get_current_time(lm32c_hdl cpu_hdl)
{
    lm32_cpu* cpu = (lm32_cpu*)cpu_hdl;
    return cpu->lm32_get_current_time();
}

// -------------------------------------------------------------------------

extern "C" void lm32c_set_configuration(lm32c_hdl cpu_hdl, int word)
{
    lm32_cpu* cpu = (lm32_cpu*)cpu_hdl;
    cpu->lm32_set_configuration(word);
}

// -------------------------------------------------------------------------

extern "C" uint32_t lm32c_get_configuration(lm32c_hdl cpu_hdl)
{
    lm32_cpu* cpu = (lm32_cpu*)cpu_hdl;
    return cpu->lm32_get_configuration();
}

// -------------------------------------------------------------------------

extern "C" uint32_t lm32c_read_mem (lm32c_hdl cpu_hdl, uint32_t byte_addr, int type)
{
    lm32_cpu* cpu = (lm32_cpu*)cpu_hdl;
    return cpu->lm32_read_mem(byte_addr, type);
}

// -------------------------------------------------------------------------

extern "C" void lm32c_write_mem (lm32c_hdl cpu_hdl, uint32_t byte_addr, uint32_t data, int type)
{
    lm32_cpu* cpu = (lm32_cpu*)cpu_hdl;
    cpu->lm32_write_mem(byte_addr, data, type);
}

// -------------------------------------------------------------------------

extern "C" void lm32c_dump_registers(lm32c_hdl cpu_hdl)
{
    lm32_cpu* cpu = (lm32_cpu*)cpu_hdl;
    cpu->lm32_dump_registers();
}

// -------------------------------------------------------------------------

extern "C" uint32_t lm32c_set_hw_debug_reg(lm32c_hdl cpu_hdl, uint32_t address, int type)
{
    lm32_cpu* cpu = (lm32_cpu*)cpu_hdl;
    return cpu->lm32_set_hw_debug_reg(address, type);
}

// -------------------------------------------------------------------------

extern "C" lm32c_state lm32c_get_cpu_state (lm32c_hdl cpu_hdl)
{
    lm32_cpu* cpu = (lm32_cpu*)cpu_hdl;
    lm32_cpu::lm32_state state_cpp;
    lm32c_state state_c;

    state_cpp = cpu->lm32_get_cpu_state();

    for(int idx = 0; idx < LM32_NUM_OF_REGISTERS; idx++)
    {
        state_c.r[idx] = state_cpp.r[idx];
    }

    state_c.pc   = state_cpp.pc;
    state_c.ie   = state_cpp.ie;
    state_c.im   = state_cpp.im;
    state_c.ip   = state_cpp.ip;
    state_c.eba  = state_cpp.eba;
    state_c.icc  = state_cpp.icc;
    state_c.dcc  = state_cpp.dcc;
    state_c.cfg  = state_cpp.cfg;
    state_c.cfg2 = state_cpp.cfg2;
    state_c.cc   = state_cpp.cc;
    state_c.dc   = state_cpp.dc;
    state_c.deba = state_cpp.deba;
    state_c.bp0  = state_cpp.bp0;
    state_c.bp1  = state_cpp.bp1;
    state_c.bp2  = state_cpp.bp2;
    state_c.bp3  = state_cpp.bp3;
    state_c.wp0  = state_cpp.wp0;
    state_c.wp1  = state_cpp.wp1;
    state_c.wp2  = state_cpp.wp2;
    state_c.wp3  = state_cpp.wp3;
    state_c.jtx  = state_cpp.jtx;
    state_c.jrx  = state_cpp.jrx;

    return state_c;
}
