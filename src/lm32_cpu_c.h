//=============================================================
// 
// Copyright (c) 2013 Simon Southwell. All rights reserved.
//
// Date: 7th June 2013
//
// This file is the top level header for linking to C based
// programs (as opposed to C++). 
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
// $Id: lm32_cpu_c.h,v 2.3 2016-09-03 07:44:05 simon Exp $
// $Source: /home/simon/CVS/src/cpu/mico32/src/lm32_cpu_c.h,v $
//
//=============================================================

#ifndef _LM32_CPU_C_H_
#define _LM32_CPU_C_H_

// -------------------------------------------------------------------------
// INCLUDES
// -------------------------------------------------------------------------

#include <stdio.h>
#include <stdint.h>

#include "lm32_cpu_hdr.h"

// -------------------------------------------------------------------------
// DEFINES
// -------------------------------------------------------------------------

#ifndef TRUE
#define TRUE (1==1)
#endif

#ifndef FALSE
#define FALSE (1==2)
#endif

// -------------------------------------------------------------------------
// TYPEDEFS
// -------------------------------------------------------------------------

// In C++ compilation, ensure this file's contents maintains pure C linkage
#ifdef __cplusplus
extern "C" {
#endif

// Define a handle type for the C++ object that can be returned to the world of C
typedef void* lm32c_hdl;

// Structure to return CPU register state
typedef struct  {

    uint32_t r[LM32_NUM_OF_REGISTERS];
    uint32_t pc;    // Program counter
    uint32_t ie;    // Interrupt enable
    uint32_t im;    // External interrupt mask
    uint32_t ip;    // External interrupt pending (write 1 to clear)
    uint32_t eba;   // exception table base address
    uint32_t icc;   // Instruction cache control
    uint32_t dcc;   // Data cache control
    uint32_t cfg;   // Configuration
    uint32_t cfg2;  // Extended configuration
    uint32_t cc;    // Cycle counter register               
    uint32_t dc;    // Debug control
    uint32_t deba;  // Debug exception table base address
    uint32_t bp0;   // H/W breakpoint 0
    uint32_t bp1;   // H/W breakpoint 1
    uint32_t bp2;   // H/W breakpoint 2
    uint32_t bp3;   // H/W breakpoint 3
    uint32_t wp0;   // H/W watchpoint 0
    uint32_t wp1;   // H/W watchpoint 1
    uint32_t wp2;   // H/W watchpoint 2
    uint32_t wp3;   // H/W watchpoint 3
    uint32_t jtx;   // JTAG TX register
    uint32_t jrx;   // JTAG RX register

} lm32c_state;

// -------------------------------------------------------------------------
//  C linkage function prototypes
// -------------------------------------------------------------------------

// Creates an ISS object, with specified configuration, and returns 
// a handle to it for subsequent referencing of internal functions,
// as defined below.
lm32c_hdl   lm32c_cpu_init                  (int                  verbose, 
                                             int                  disable_reset_break,
                                             int                  disable_lock_break,
                                             int                  disable_hw_break,
                                             int                  disassemble_run,
                                             uint32_t             num_mem_bytes,
                                             uint32_t             mem_offset,
                                             int                  mem_wait_states,
                                             uint32_t             entry_point_addr,
                                             uint32_t             cfg_word,
                                             FILE*                ofp,
                                             lm32_cache_config_t* p_dcache_cfg,
                                             lm32_cache_config_t* p_icache_cfg);

// Program execution
int         lm32c_run_program               (lm32c_hdl cpu_hdl, const char* elf_fname, int run_cycles, int break_addr, int exec_type, int load_code);

// Generation of a reset event
void        lm32c_reset_cpu                 (lm32c_hdl cpu_hdl);

// Callback function registration
void        lm32c_register_int_callback     (lm32c_hdl cpu_hdl, p_lm32_intcallback_t  callback_func);
void        lm32c_register_ext_mem_callback (lm32c_hdl cpu_hdl, p_lm32_memcallback_t  callback_func);
void        lm32c_register_jtag_callback    (lm32c_hdl cpu_hdl, p_lm32_jtagcallback_t callback_func);

// Run-time re-configuration and status
void        lm32c_set_verbosity_level       (lm32c_hdl cpu_hdl, int level);
lm32_time_t lm32c_get_current_time          (lm32c_hdl cpu_hdl);
void        lm32c_set_configuration         (lm32c_hdl cpu_hdl, int word);
uint32_t    lm32c_get_configuration         (lm32c_hdl cpu_hdl);
lm32_time_t lm32c_get_num_instructions      (lm32c_hdl cpu_hdl);

// Internal memory access
uint32_t    lm32c_read_mem                  (lm32c_hdl cpu_hdl, uint32_t byte_addr, int access_type);
void        lm32c_write_mem                 (lm32c_hdl cpu_hdl, uint32_t byte_addr, uint32_t data, int access_type);

// Debug support routines
void        lm32c_dump_registers            (lm32c_hdl cpu_hdl);
uint32_t    lm32c_set_hw_debug_reg          (lm32c_hdl cpu_hdl, uint32_t address, int register_type);
lm32c_state lm32c_get_cpu_state             (lm32c_hdl cpu_hdl);


#ifdef __cplusplus
}
#endif

#endif
