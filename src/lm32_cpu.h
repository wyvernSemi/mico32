//=============================================================
// 
// Copyright (c) 2013-2016 Simon Southwell. All rights reserved.
//
// Date: 6th May 2013
//
// This is the model user definitions and top level class
// (lm32_cpu) definition for the mico32 ISS
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
// $Id: lm32_cpu.h,v 2.8 2016-09-06 06:13:47 simon Exp $
// $Source: /home/simon/CVS/src/cpu/mico32/src/lm32_cpu.h,v $
//
//=============================================================

#ifndef _LM32_CPU_H_
#define _LM32_CPU_H_

// -------------------------------------------------------------------------
// INCLUDES
// -------------------------------------------------------------------------

#include <stdio.h>
#include <stdint.h>

#include "lm32_cpu_hdr.h"
#include "lm32_cache.h"
#include "lm32_cpu_mico32.h"

// -------------------------------------------------------------------------
// DEFINES
// -------------------------------------------------------------------------

// Define LIBMICO32_DLL_LINKAGE in windows projects that will link to libmico32 as a DLL,
// else leave undefined for projects that compile the source code.
#if (defined(_WIN32) || defined(_WIN64)) && defined (LIBMICO32_DLL_LINKAGE)
// The DLL build needs to export, whereas those linking to it need import definitions
# ifdef LIBMICO32_EXPORTS
#  define LIBMICO32_API __declspec(dllexport) 
# else
#  define LIBMICO32_API __declspec(dllimport) 
# endif
#else
# define LIBMICO32_API
#endif

// -------------------------------------------------------------------------
// Class definition for mico32 model
// -------------------------------------------------------------------------

class lm32_cpu {
    
// Public User API member function declarations
public:

    // Define a class to hold all of the CPU registers. This makes it easier
    // to access all of the state as a single unit for debug purposes.
    class lm32_state {
        public:

        // General purpose registers
        uint32_t r[LM32_NUM_OF_REGISTERS];
    
        // Control and status registers
        uint32_t pc;    // Program counter
        uint32_t ie;    // Interrupt enable
        uint32_t im;    // External interrupt mask
        uint32_t ip;    // External interrupt pending (write 1 to clear)
        uint32_t eba;   // exception table base address
        uint32_t icc;   // Instruction cache control
        uint32_t dcc;   // Data cache control
        uint32_t cfg;   // Configuration
        uint32_t cfg2;  // Extended configuration
    
        // CC register not used directly in the model, but only here to allow 
        // returned state via lm32_get_cpu_state(). It is updated on calling 
        // that function and at any CC accesses via WCSR and RCSR instructions. 
        // CC internally is a function of state.cycle_count and cc_adjust.
        uint32_t cc;                   
        
        // Debug control and status registers
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

        // State not directly modelling mico32 processor internal state, but included
        // here for saving
        lm32_time_t  wakeup_time_ext_int; // Wakeup time for external interrupt callback function
        uint32_t     int_flags;           // Bitmap flags to indicate requesting interrupt
        lm32_time_t  cycle_count;         // Cycle count (doubles up as mico32 cc register)
        lm32_time_t  clk_count;           // Absolute clock ticks---increments by 1 on 'TICK' calls to model
        uint64_t     instr_count;         // Number of instructions executed since time 0
    };

    // Constructor, with default configuration values
    LIBMICO32_API lm32_cpu                     (const int            verbose             = 0,
                                                const bool           disable_reset_break = true,
                                                const bool           disable_lock_break  = false,
                                                const bool           disable_hw_break    = true,
                                                const bool           disassemble_run     = false,
                                                const uint32_t       num_mem_bytes       = LM32_DEFAULT_MEM_SIZE,
                                                const uint32_t       mem_offset          = 0,
                                                const int            mem_wait_stats      = 0,
                                                const uint32_t       entry_point_addr    = 0,
                                                const uint32_t       cfg_word            = LM32_DEFAULT_CONFIG,
                                                FILE*                ofp                 = stdout,
                                                lm32_cache_config_t* p_dcache_cfg        = NULL,
                                                lm32_cache_config_t* p_icache_cfg        = NULL,
                                                const uint32_t       disassemble_start   = 0
                                                );

    // Main execution method
    LIBMICO32_API int lm32_run_program         (const char* elf_fname  = LM32_DEFAULT_FNAME,
                                                const lm32_time_t run_cycles = LM32_FOREVER,
                                                const int         break_addr = -1,
                                                const int         exec_type  = LM32_RUN_FROM_RESET,
                                                const bool        load_code  = false);

    // Callback function registration
    LIBMICO32_API void        lm32_register_int_callback     (p_lm32_intcallback_t callback_func);
    LIBMICO32_API void        lm32_register_ext_mem_callback (p_lm32_memcallback_t callback_func);
    LIBMICO32_API void        lm32_register_jtag_callback    (p_lm32_jtagcallback_t callback_func);

    // Reset the cpu (i.e. generate a reset pin assertion event)
    LIBMICO32_API void        lm32_reset_cpu                 (void);

    // External direct memory access
    LIBMICO32_API uint32_t    lm32_read_mem                  (const uint32_t byte_addr, const int type);
    LIBMICO32_API void        lm32_write_mem                 (const uint32_t byte_addr, const uint32_t data, const int type, const bool disable_cycle_count=false);

    // Internal register access
    LIBMICO32_API void        lm32_set_gp_reg                (const unsigned index, const uint32_t val);

    // Internal register state debug access
    LIBMICO32_API void        lm32_dump_registers            (void);
    LIBMICO32_API uint32_t    lm32_set_hw_debug_reg          (const uint32_t address, const int type);

    // Return state of CPU internal registers
    LIBMICO32_API inline      lm32_state lm32_get_cpu_state(void)
    {
        // Before returning CC state, update register from state.cycle_count plus
        // adjustment due to overwrites of the CC register, but only if the
        // counter is implemented, as defined by CFG register.
        state.cc = (state.cfg & (1 << LM32_CFG_CC)) ? (uint32_t)((state.cycle_count + cc_adjust) & 0xffffffffULL) : 0;
    
        return state;
    }

    LIBMICO32_API inline void        lm32_set_cpu_state(const lm32_state new_state) { state = new_state; }

    // User run-time configuration and status routines
    LIBMICO32_API void               lm32_set_configuration         (const uint32_t word);       // Enable/disable CPU features

    // Return the current number of instructions executed
    LIBMICO32_API inline uint64_t lm32_get_num_instructions(void) { return state.instr_count; }
    
    // Return the current time
    LIBMICO32_API inline lm32_time_t lm32_get_current_time(void) {  return state.clk_count; }

    // Get the configuration word
    LIBMICO32_API inline uint32_t    lm32_get_configuration(void) { return state.cfg; }

    // User called routine to set verbosity level of debug information.
    // Current only two levels supported: 0 = off, >0 = on
    LIBMICO32_API inline void        lm32_set_verbosity_level (const int lvl) 
    {
        if (lvl == LM32_VERBOSITY_LVL_1 || lvl == LM32_VERBOSITY_LVL_OFF)
        {
            verbose = lvl;
        }
        else 
        {
            fprintf(stderr, "***ERROR: invalid verbosity level (%d)\n",lvl);                //LCOV_EXCL_LINE
            exit(LM32_USER_ERROR);                                                          //LCOV_EXCL_LINE
        }
    }

// Private member functions
private:

#ifndef LNXMICO32
    // Read ELF format program and load to memory
    void        read_elf                       (const char* const filename = LM32_DEFAULT_FNAME);
#endif
    // Instruction execution
    bool        execute_instruction            (p_lm32_decode_t d);
    
    // Internal method to load program words to memory (used by lm32_read_elf)
    void        load_mem_word                  (const uint32_t byte_addr, const uint32_t word, const uint32_t flags);

    // Support method for instructions to calculate stall times on pending register
    // updates
    lm32_time_t calc_stall                     (const int ry, const int rz, const lm32_time_t cycle_count);

    // Exception handling methods
    void        interrupt                      (const int interrupt_id);
    bool        process_exceptions             (void);

    // Internal reset of CPU state
    void        internal_reset_cpu             (void);

    // Disassembler methods
    char*       fmt_register                   (const int regnum, char* str, const bool end);
    void        disassemble                    (const p_lm32_decode_t d, FILE *ofp, const bool disassemble_run);

    // Instruction methods
    void        lm32_raise                     (const p_lm32_decode_t p);
    void        lm32_rsrvd                     (const p_lm32_decode_t p);
    void        lm32_add                       (const p_lm32_decode_t p);
    void        lm32_addi                      (const p_lm32_decode_t p);
    void        lm32_sub                       (const p_lm32_decode_t p);
    void        lm32_div                       (const p_lm32_decode_t p);
    void        lm32_divu                      (const p_lm32_decode_t p);
    void        lm32_mod                       (const p_lm32_decode_t p);
    void        lm32_modu                      (const p_lm32_decode_t p);
    void        lm32_mul                       (const p_lm32_decode_t p);
    void        lm32_muli                      (const p_lm32_decode_t p);
    void        lm32_sextb                     (const p_lm32_decode_t p);
    void        lm32_sexth                     (const p_lm32_decode_t p);
    void        lm32_cmpe                      (const p_lm32_decode_t p);
    void        lm32_cmpei                     (const p_lm32_decode_t p);
    void        lm32_cmpg                      (const p_lm32_decode_t p);
    void        lm32_cmpge                     (const p_lm32_decode_t p);
    void        lm32_cmpgei                    (const p_lm32_decode_t p);
    void        lm32_cmpgeu                    (const p_lm32_decode_t p);
    void        lm32_cmpgeui                   (const p_lm32_decode_t p);
    void        lm32_cmpgi                     (const p_lm32_decode_t p);
    void        lm32_cmpgu                     (const p_lm32_decode_t p);
    void        lm32_cmpgui                    (const p_lm32_decode_t p);
    void        lm32_cmpne                     (const p_lm32_decode_t p);
    void        lm32_cmpnei                    (const p_lm32_decode_t p);
    void        lm32_sl                        (const p_lm32_decode_t p);
    void        lm32_sli                       (const p_lm32_decode_t p);
    void        lm32_sr                        (const p_lm32_decode_t p);
    void        lm32_sri                       (const p_lm32_decode_t p);
    void        lm32_sru                       (const p_lm32_decode_t p);
    void        lm32_srui                      (const p_lm32_decode_t p);
    void        lm32_and                       (const p_lm32_decode_t p);
    void        lm32_andhi                     (const p_lm32_decode_t p);
    void        lm32_andi                      (const p_lm32_decode_t p);
    void        lm32_nor                       (const p_lm32_decode_t p);
    void        lm32_nori                      (const p_lm32_decode_t p);
    void        lm32_or                        (const p_lm32_decode_t p);
    void        lm32_orhi                      (const p_lm32_decode_t p);
    void        lm32_ori                       (const p_lm32_decode_t p);
    void        lm32_xnor                      (const p_lm32_decode_t p);
    void        lm32_xnori                     (const p_lm32_decode_t p);
    void        lm32_xor                       (const p_lm32_decode_t p);
    void        lm32_xori                      (const p_lm32_decode_t p);
    void        lm32_b                         (const p_lm32_decode_t p);
    void        lm32_be                        (const p_lm32_decode_t p);
    void        lm32_bg                        (const p_lm32_decode_t p);
    void        lm32_bge                       (const p_lm32_decode_t p);
    void        lm32_bgeu                      (const p_lm32_decode_t p);
    void        lm32_bgu                       (const p_lm32_decode_t p);
    void        lm32_bi                        (const p_lm32_decode_t p);
    void        lm32_bne                       (const p_lm32_decode_t p);
    void        lm32_call                      (const p_lm32_decode_t p);
    void        lm32_calli                     (const p_lm32_decode_t p);
    void        lm32_lb                        (const p_lm32_decode_t p);
    void        lm32_lbu                       (const p_lm32_decode_t p);
    void        lm32_lh                        (const p_lm32_decode_t p);
    void        lm32_lhu                       (const p_lm32_decode_t p);
    void        lm32_lw                        (const p_lm32_decode_t p);
    void        lm32_sb                        (const p_lm32_decode_t p);
    void        lm32_sh                        (const p_lm32_decode_t p);
    void        lm32_sw                        (const p_lm32_decode_t p);
    void        lm32_rcsr                      (const p_lm32_decode_t p);
    void        lm32_wcsr                      (const p_lm32_decode_t p);

// Private model state
private:

    // Callback function pointers
    p_lm32_intcallback_t       pIntCallback;
    p_lm32_memcallback_t       pMemCallback;
    p_lm32_jtagcallback_t      pJtagCallback;

    // Wakeup time for external interrupt callback function
    //lm32_time_t wakeup_time_ext_int;

    // Control variables
    FILE*                      ofp;                  // Stream for output data
    bool                       disable_reset_break;  // Disable/enable breakpoint on reset of CPU
    bool                       disable_lock_break;   // Disable/enable breakpoint on lock conditions
    bool                       disable_hw_break;     // Disable break on a h/w breakpoint
    int                        verbose;              // Verbosity level
    uint32_t                   num_mem_bytes;        // User definable size of code/data memory
    uint32_t                   num_mem_bytes_mask;   // MAsk for specified number of bytes (num_mem_bytes-1)
    uint32_t                   mem_offset;           // User defined offset for internal memory
    uint32_t                   entry_point_addr;     // User defined entry point address (reset vector start)
    int                        mem_wait_states;      // Number of wait states for internal memory access
    bool                       disassemble_run;        

    // Flag to indicate a termination/break point specified externally
    uint32_t                   break_point;

    // Main memory pointer (space defined at runtime). Memory is byte oriented,
    // and mico32 is 32 bit big-endian
    uint8_t*                   mem;
    uint8_t*                   mem_tag;
    uint16_t*                  mem16;
    uint32_t*                  mem32;

    // Pointer to decode table of pointers to instruction functions
    pFunc_t                    tbl_p[LM32_NUM_OPCODES];

    // Instruction decode information table. Static, as never changes
    static const lm32_decode_table_t decode_table [LM32_NUM_OPCODES] ;

    // variable to hold the CPU internal register state
    lm32_state                 state;

    // Array to hold the availability time of the 32 CPU registers for
    // timing model calculations (see calc_stall())
    lm32_time_t                rt[LM32_NUM_OF_REGISTERS];

    // As state.cycle_count must not be changed by writing to cc register,
    // an adjustment value is kept, which keeps track of the relative
    // distance between state.cycle_count and the expected cc count since last
    // changed with 'wcsr CC' instruction. An 'rcsr CC' instruction will
    // return the state.cycle_count plus the adjustment to give a proper CC
    // value. This avoids the need to keep two separate counters that
    // would need keeping in sync.
    lm32_time_t                cc_adjust;

    // Flags for memory callback to indicate cache invalidation set by processor
    bool                       dcc_invalidate;
    bool                       icc_invalidate;

    // Pointers to cache configurations
    lm32_cache_config_t*       p_dcache_cfg;
    lm32_cache_config_t*       p_icache_cfg;

    // Cache object pointers
    lm32_cache*                icache_p;
    lm32_cache*                dcache_p;

    // Start address for dissassembling
    uint32_t                   disassemble_start;
    bool                       disassemble_active;
};
#endif
