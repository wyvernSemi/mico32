//=============================================================
// 
// Copyright (c) 2013-2016 Simon Southwell. All rights reserved.
//
// Date: 9th April 2013
//
// Contains the instruction execution methods for the
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
// $Id: lm32_cpu.cpp,v 3.3 2016-09-15 18:14:04 simon Exp $
// $Source: /home/simon/CVS/src/cpu/mico32/src/lm32_cpu.cpp,v $
//
//=============================================================

// -------------------------------------------------------------------------
// INCLUDES
// -------------------------------------------------------------------------

#include <cstdio>

#include "lm32_cpu.h"
#include "lm32_cpu_mico32.h"

#ifndef LNXMICO32
#include "lm32_cpu_elf.h"
#endif

// -------------------------------------------------------------------------
// DEFINES
// -------------------------------------------------------------------------

//#define LM32_LITTLE_ENDIAN

// -------------------------------------------------------------------------
// GLOBALS
// -------------------------------------------------------------------------

const lm32_decode_table_t lm32_cpu::decode_table [LM32_NUM_OPCODES] = DECODE_TABLE;

// -------------------------------------------------------------------------
// lm32_cpu()
//
// Constructor
//
// -------------------------------------------------------------------------

lm32_cpu::lm32_cpu (const int verbose_in, 
                    const bool disable_reset_break_in, 
                    const bool disable_lock_break_in, 
                    const bool disable_hw_break_in,
                    bool disassemble_run_in, 
                    const uint32_t num_mem_bytes_in, 
                    const uint32_t mem_offset_in, 
                    const int mem_wait_states_in,
                    const uint32_t entry_point_addr_in, 
                    const uint32_t cfg_in, 
                    FILE* ofp_in, 
                    lm32_cache_config_t* p_dcache_cfg_in,
                    lm32_cache_config_t* p_icache_cfg_in, 
                    const uint32_t disassemble_start_in) :

    verbose(verbose_in),                         // Verbosity level
    disable_lock_break(disable_lock_break_in),   // Disable/enable breakpoint on lock conditions
    disable_reset_break(disable_reset_break_in),
    disable_hw_break(disable_hw_break_in),
    disassemble_run(disassemble_run_in),         // Enable/disable a disassemble run
    num_mem_bytes(num_mem_bytes_in),             // User definable size of code/data memory
    mem_offset(mem_offset_in),
    mem_wait_states(mem_wait_states_in),
    entry_point_addr(entry_point_addr_in),
    ofp(ofp_in),
    p_icache_cfg(p_icache_cfg_in),
    p_dcache_cfg(p_dcache_cfg_in),
    disassemble_start(disassemble_start_in)
{

    icache_p = dcache_p = NULL;

    disassemble_active = disassemble_start ? false : true;

    // Round up the count of byte to a word boundary
    num_mem_bytes = (num_mem_bytes + 3) & 0xfffffffc;

    num_mem_bytes_mask = num_mem_bytes - 1;

    // Round down the memory offset to the nearest word boundary
    mem_offset &= 0xfffffffc;

    // Reset the register time values
    for (int idx = 0; idx < LM32_NUM_OF_REGISTERS; idx++)
    {
        rt[idx] = 0;
    }

    // No callback until user configures
    pIntCallback        = NULL;
    pMemCallback        = NULL;
    pJtagCallback       = NULL;

    // Start with wakeup time always in the past
    state.wakeup_time_ext_int = 0;

    // Clear major interrupt state
    state.int_flags     = 0;

    // Cycle count (doubles up as mico32 cc register)
    state.cycle_count   = 0;

    // Other statistical count
    state.clk_count     = 0;
    state.instr_count   = 0;

    // CPU's CC count is aligned with cycle_count to start with
    cc_adjust           = 0;

    // Reset the internal CPU state
    internal_reset_cpu();

    // Set defined configuration
    lm32_set_configuration(cfg_in);

    // No internal memory to start with, to allow user configuration
    mem                 = NULL;
    mem16               = NULL;
    mem32               = NULL;
    mem_tag             = NULL;

    dcc_invalidate      = false;
    icc_invalidate      = false;

    // Populate the table with pointers to instruction member functions, 
    // in the order of opcode index
    int idx = 0;
    tbl_p[idx++] = &lm32_cpu::lm32_srui;
    tbl_p[idx++] = &lm32_cpu::lm32_nori;
    tbl_p[idx++] = &lm32_cpu::lm32_muli;
    tbl_p[idx++] = &lm32_cpu::lm32_sh;
    tbl_p[idx++] = &lm32_cpu::lm32_lb;
    tbl_p[idx++] = &lm32_cpu::lm32_sri;
    tbl_p[idx++] = &lm32_cpu::lm32_xori;
    tbl_p[idx++] = &lm32_cpu::lm32_lh;
    tbl_p[idx++] = &lm32_cpu::lm32_andi;
    tbl_p[idx++] = &lm32_cpu::lm32_xnori;
    tbl_p[idx++] = &lm32_cpu::lm32_lw;
    tbl_p[idx++] = &lm32_cpu::lm32_lhu;
    tbl_p[idx++] = &lm32_cpu::lm32_sb;
    tbl_p[idx++] = &lm32_cpu::lm32_addi;
    tbl_p[idx++] = &lm32_cpu::lm32_ori;
    tbl_p[idx++] = &lm32_cpu::lm32_sli;
    tbl_p[idx++] = &lm32_cpu::lm32_lbu;
    tbl_p[idx++] = &lm32_cpu::lm32_be;
    tbl_p[idx++] = &lm32_cpu::lm32_bg;
    tbl_p[idx++] = &lm32_cpu::lm32_bge;
    tbl_p[idx++] = &lm32_cpu::lm32_bgeu;
    tbl_p[idx++] = &lm32_cpu::lm32_bgu;
    tbl_p[idx++] = &lm32_cpu::lm32_sw;
    tbl_p[idx++] = &lm32_cpu::lm32_bne;
    tbl_p[idx++] = &lm32_cpu::lm32_andhi;
    tbl_p[idx++] = &lm32_cpu::lm32_cmpei;
    tbl_p[idx++] = &lm32_cpu::lm32_cmpgi;
    tbl_p[idx++] = &lm32_cpu::lm32_cmpgei;
    tbl_p[idx++] = &lm32_cpu::lm32_cmpgeui;
    tbl_p[idx++] = &lm32_cpu::lm32_cmpgui;
    tbl_p[idx++] = &lm32_cpu::lm32_orhi;
    tbl_p[idx++] = &lm32_cpu::lm32_cmpnei;
    tbl_p[idx++] = &lm32_cpu::lm32_sru;
    tbl_p[idx++] = &lm32_cpu::lm32_nor;
    tbl_p[idx++] = &lm32_cpu::lm32_mul;
    tbl_p[idx++] = &lm32_cpu::lm32_divu;
    tbl_p[idx++] = &lm32_cpu::lm32_rcsr;
    tbl_p[idx++] = &lm32_cpu::lm32_sr;
    tbl_p[idx++] = &lm32_cpu::lm32_xor;
    tbl_p[idx++] = &lm32_cpu::lm32_div;
    tbl_p[idx++] = &lm32_cpu::lm32_and;
    tbl_p[idx++] = &lm32_cpu::lm32_xnor;
    tbl_p[idx++] = &lm32_cpu::lm32_rsrvd;
    tbl_p[idx++] = &lm32_cpu::lm32_raise;
    tbl_p[idx++] = &lm32_cpu::lm32_sextb;
    tbl_p[idx++] = &lm32_cpu::lm32_add;
    tbl_p[idx++] = &lm32_cpu::lm32_or;
    tbl_p[idx++] = &lm32_cpu::lm32_sl;
    tbl_p[idx++] = &lm32_cpu::lm32_b;
    tbl_p[idx++] = &lm32_cpu::lm32_modu;
    tbl_p[idx++] = &lm32_cpu::lm32_sub;
    tbl_p[idx++] = &lm32_cpu::lm32_rsrvd;
    tbl_p[idx++] = &lm32_cpu::lm32_wcsr;
    tbl_p[idx++] = &lm32_cpu::lm32_mod;
    tbl_p[idx++] = &lm32_cpu::lm32_call;
    tbl_p[idx++] = &lm32_cpu::lm32_sexth;
    tbl_p[idx++] = &lm32_cpu::lm32_bi;
    tbl_p[idx++] = &lm32_cpu::lm32_cmpe;
    tbl_p[idx++] = &lm32_cpu::lm32_cmpg;
    tbl_p[idx++] = &lm32_cpu::lm32_cmpge;
    tbl_p[idx++] = &lm32_cpu::lm32_cmpgeu;
    tbl_p[idx++] = &lm32_cpu::lm32_cmpgu;
    tbl_p[idx++] = &lm32_cpu::lm32_calli;
    tbl_p[idx++] = &lm32_cpu::lm32_cmpne;

}

// -------------------------------------------------------------------------
// lm32_set_configuration()
//
// Set the configuration word
//
// -------------------------------------------------------------------------

void lm32_cpu::lm32_set_configuration(const uint32_t word)
{
    // Only allow updates to valid and supported fields
    state.cfg = word & LM32_CONFIG_WRITE_MASK;

    // If configured an ICACHE, and none exists, create one now
    if ((state.cfg & (1 << LM32_CFG_IC)) && icache_p == NULL)
    {
        if (p_icache_cfg == NULL)
        {
            icache_p = new lm32_cache();                                                        //LCOV_EXCL_LINE
        }
        else
        {
            icache_p = new lm32_cache(p_icache_cfg->cache_bytes_per_line,
                                      p_icache_cfg->cache_num_ways,
                                      p_icache_cfg->cache_num_sets,
                                      p_icache_cfg->cache_base_addr,
                                      p_icache_cfg->cache_limit
                                      );
        }

    // If configured no ICACHE, and one exists, delete it
    }
    else if (!(state.cfg & (1 << LM32_CFG_IC)) && icache_p != NULL)
    {
        delete icache_p;
        icache_p = NULL;
    }

    // If configured a DCACHE, and none exists, create one now
    if ((state.cfg & (1 << LM32_CFG_DC)) && dcache_p == NULL)
    {
        if (p_dcache_cfg == NULL)
        {
            dcache_p = new lm32_cache();                                                        //LCOV_EXCL_LINE
        }
        else
        {
            dcache_p = new lm32_cache(p_dcache_cfg->cache_bytes_per_line,
                                      p_dcache_cfg->cache_num_ways,
                                      p_dcache_cfg->cache_num_sets,
                                      p_dcache_cfg->cache_base_addr,
                                      p_dcache_cfg->cache_limit
                                      );
        }

    // If configured no DCACHE, and one exists, delete it
    } 
    else if (!(state.cfg & (1 << LM32_CFG_DC)) && dcache_p != NULL)
    {
        delete dcache_p;
        dcache_p = NULL;
    }
}

// -------------------------------------------------------------------------
// lm32_set_gp_reg()
//
// External access to setting general purpose registers
//
// -------------------------------------------------------------------------
void lm32_cpu::lm32_set_gp_reg (const unsigned index, const uint32_t val)
{
    state.r[index & (LM32_NUM_OF_REGISTERS-1)] = val;
}

// -------------------------------------------------------------------------
// lm32_set_hw_debug_reg()
//
// External access to setting H/W debug registers
//
// -------------------------------------------------------------------------

uint32_t lm32_cpu::lm32_set_hw_debug_reg (const uint32_t address, const int type)
{
    int register_type = type & 0x1f;
    int wp_access_type = (type >> 5) & 0x3;

    switch(register_type)
    {
    case LM32_CSR_ID_BP0:
        if (((state.cfg >> LM32_CFG_BP) & LM32_CFG_BP_MASK) > 0)
        {
            state.bp0 = address & 0xfffffffd;  
        }
        return state.bp0;
        break;
    case LM32_CSR_ID_BP1:
        if (((state.cfg >> LM32_CFG_BP) & LM32_CFG_BP_MASK) > 1)
        {
            state.bp1 = address & 0xfffffffd;
        }
        return state.bp1;
        break;
    case LM32_CSR_ID_BP2:
        if (((state.cfg >> LM32_CFG_BP) & LM32_CFG_BP_MASK) > 2)
        {
            state.bp2 = address & 0xfffffffd;
        }
        return state.bp2;
        break;
    case LM32_CSR_ID_BP3:
        if (((state.cfg >> LM32_CFG_BP) & LM32_CFG_BP_MASK) > 3)
        {
           state.bp3 = address & 0xfffffffd;
        }
        return state.bp3;
        break;
    case LM32_CSR_ID_WP0:
        if (((state.cfg >> LM32_CFG_WP) & LM32_CFG_WP_MASK) > 0)
        {
            state.wp0 = address;
            state.dc |= (state.dc & ~(0x0000000c)) | (wp_access_type << 2);
        }
        return state.wp0;
        break;
    case LM32_CSR_ID_WP1:
        if (((state.cfg >> LM32_CFG_WP) & LM32_CFG_WP_MASK) > 1)
        {
            state.wp1 = address;
            state.dc |= (state.dc & ~(0x00000030)) | (wp_access_type << 4);
        }
        return state.wp1;
        break;
    case LM32_CSR_ID_WP2:
        if (((state.cfg >> LM32_CFG_WP) & LM32_CFG_WP_MASK) > 2)
        {
            state.wp2 = address;
            state.dc |= (state.dc & ~(0x000000c0)) | (wp_access_type << 6);
        }
        return state.wp2;
        break;
    case LM32_CSR_ID_WP3:
        if (((state.cfg >> LM32_CFG_WP) & LM32_CFG_WP_MASK) > 3)
        {
            state.wp3 = address;
            state.dc |= (state.dc & ~(0x00000300)) | (wp_access_type << 8);
        }
        return state.wp3;
        break;
    default:
        fprintf(stderr, "***ERROR: invalid debug register access type (%d)\n", type);           //LCOV_EXCL_LINE
        exit(LM32_USER_ERROR);                                                                  //LCOV_EXCL_LINE
        break;
    }
}

// -------------------------------------------------------------------------
// lm32_read_mem()
//
// Read memory. Calls any memory access callback function. Note that the
// mico32 is big-endian. Any callback must honour this (if it supports
// sub-word acceses).
//
// -------------------------------------------------------------------------

uint32_t lm32_cpu::lm32_read_mem (const uint32_t byte_addr_raw, const int type)
{
    // Local holder for read data
    uint32_t data;
    
    int  mem_callback_delay;

    // Local cache state
    int  cache_hit             = LM32_CACHE_MISS;       // Flag if cache hit/miss (or miss if no cache access)

#ifndef LM32_FAST_COMPILE
    uint32_t byte_addr = byte_addr_raw % num_mem_bytes;

    bool cache_access          = false;                 // Flag if access was via cache
    int  cache_words_per_line;                          // Number of words per line for accessed cache

    // Check for any outstanding cache invalidations (i.e. writes
    // to DCC or ICC registers since last call)
    if (icc_invalidate && icache_p != NULL)
    {
        icache_p->lm32_cache_invalidate();
    }

    if (dcc_invalidate && dcache_p != NULL)
    {
        dcache_p->lm32_cache_invalidate();
    }

    // Clear any cache invalidation state
    dcc_invalidate = false;
    icc_invalidate = false;

    // Check the caches on instruction or data reads (and update)
    if (type == LM32_MEM_RD_INSTR) 
    {
        // If ICACHE configured, call the cache model
        if (icache_p != NULL) 
        {
            cache_hit = icache_p->lm32_cache_access(byte_addr_raw);
            cache_access = true;
            cache_words_per_line = icache_p->get_line_width() >> 2;
        }

    } 
    else if (type & LM32_MEM_RNW_MASK) 
    {
        // If DCACHE configured, call the cache model
        if (dcache_p != NULL) {
            cache_hit = dcache_p->lm32_cache_access(byte_addr_raw);
            cache_access = true;
            cache_words_per_line = dcache_p->get_line_width() >> 2;
        }
    }

    // By default do a local memory access
    bool do_local_access = true;
#else
    // This fast calculation requires num_mem_bytes to be a power of 2
    uint32_t byte_addr = byte_addr_raw & num_mem_bytes_mask;
#endif


    // If there's a memory access callback function, call it and flag if it's
    // intercepted the access
    if (pMemCallback != NULL
#ifdef LM32_FAST_COMPILE
        && !(byte_addr_raw >= mem_offset && byte_addr_raw < (mem_offset + num_mem_bytes))
#endif
        )
    {
        // Execute callback function
        mem_callback_delay = pMemCallback(byte_addr_raw, &data, type, cache_hit, state.cycle_count);

        // Advance time by the returned amount, if intercepted, and flag no local access
        if (mem_callback_delay != LM32_EXT_MEM_NOT_PROCESSED)
        {

#ifndef LM32_FAST_COMPILE
            do_local_access = false;

            // Cycle count is incremented by returned amount when not cached, or returned amount scaled for
            // the number of words to update a cache line if cached and a miss. When a cache hit, no wait
            // states are added
            state.cycle_count += (!cache_hit) ? (lm32_time_t)mem_callback_delay * (cache_access ? cache_words_per_line : 1) : 0;
#endif
        }
    }
#ifdef LM32_FAST_COMPILE
    else
    {
#else
    // If not a callback access, do local memory access
    if (do_local_access)
    {

        // Cycle count is incremented by memory wait states when not cached, or wait states scaled for
        // the number of words to update a cache line if cached and a miss. When a cache hit, no wait
        // states are added
        state.cycle_count += (!cache_hit) ? mem_wait_states * (cache_access ? cache_words_per_line : 1) : 0;

        // Check input is a valid address
        if (byte_addr_raw > (num_mem_bytes + mem_offset) || (byte_addr_raw < mem_offset)) 
        {
            state.int_flags |= (1 << ((type == LM32_MEM_RD_INSTR) ? INT_ID_IBUSERROR : INT_ID_DBUSERROR)); 
            return 0;
        }
#endif

        switch(type)
        {
        case LM32_MEM_RD_ACCESS_BYTE:

            data = mem[byte_addr];
#ifndef LM32_FAST_COMPILE
            mem_tag[byte_addr+0] |= MEM_DATA_RD;
#endif
            break;

        case LM32_MEM_RD_ACCESS_HWORD:
#ifndef LM32_FAST_COMPILE
            if (byte_addr & 0x1U) 
            {
                state.int_flags |= (1 << INT_ID_DBUSERROR);
                return 0;
            }
            mem_tag[byte_addr+0] |= MEM_DATA_RD;
            mem_tag[byte_addr+1] |= MEM_DATA_RD;
#endif
            data = SWAPHALF(mem16[byte_addr>>1]);
            break;

        case LM32_MEM_RD_ACCESS_WORD:
        case LM32_MEM_RD_INSTR:
#ifndef LM32_FAST_COMPILE
            if (byte_addr & 0x3U)
            {
                state.int_flags |= (1 << ((type == LM32_MEM_RD_INSTR) ? INT_ID_IBUSERROR : INT_ID_DBUSERROR));
                return 0;
            }

            mem_tag[byte_addr+0] |= MEM_DATA_RD;
            mem_tag[byte_addr+1] |= MEM_DATA_RD;
            mem_tag[byte_addr+2] |= MEM_DATA_RD;
            mem_tag[byte_addr+3] |= MEM_DATA_RD;
#endif
            data = SWAP(mem32[byte_addr>>2]);

            break;

        default:
            fprintf(stderr, "***ERROR: invalid read access type (%d)\n", type);         //LCOV_EXCL_LINE
            exit(LM32_USER_ERROR);                                                      //LCOV_EXCL_LINE
            break;
        }
    } 

#ifndef LM32_FAST_COMPILE
    // Watch point processing
    if (state.cfg & (1 << LM32_CFG_H))
    {
        if (((GET_DC_C0(state.dc) & WP_RD_MASK) && (state.wp0 == byte_addr_raw)) || 
            ((GET_DC_C1(state.dc) & WP_RD_MASK) && (state.wp1 == byte_addr_raw)) ||
            ((GET_DC_C2(state.dc) & WP_RD_MASK) && (state.wp2 == byte_addr_raw)) || 
            ((GET_DC_C3(state.dc) & WP_RD_MASK) && (state.wp3 == byte_addr_raw))) 
        {

            state.int_flags |= (1 << INT_ID_WATCHPOINT); 
        }
    }
#endif

    return data;
}

// -------------------------------------------------------------------------
// lm32_write_mem()
//
// Write to memory. Calls any memory access callback function. Note that the
// mico32 is bigendian. Any callback must honour this (if it supports
// sub-word acceses).
//
// -------------------------------------------------------------------------

void lm32_cpu::lm32_write_mem(const uint32_t byte_addr_raw, const uint32_t data, const int type, const bool disable_cycle_count)
{
    int  mem_callback_delay;

    // Make a copy of the input word to avoid possibility of callback over-writing
    // input and then flagging for a local update
    uint32_t word = data;

    // Make sure we have some memory
    if (mem == NULL) 
    {
         if ((mem = (uint8_t *)malloc(num_mem_bytes/sizeof(uint8_t))) == NULL)          //LCOV_EXCL_LINE
         {
            fprintf(stderr, "***ERROR: memory allocation failure\n");                   //LCOV_EXCL_LINE
            exit(LM32_INTERNAL_ERROR);                                                  //LCOV_EXCL_LINE
         }

         mem16 = (uint16_t*)mem;
         mem32 = (uint32_t*)mem;
    }

#ifndef LM32_FAST_COMPILE
    uint32_t byte_addr = byte_addr_raw % num_mem_bytes;

    // Allocate some space for the memory tag as well, initialised to 0
    if (mem_tag == NULL)
    {
        if ((mem_tag = (uint8_t *)calloc(num_mem_bytes/sizeof(uint8_t), sizeof(uint8_t))) == NULL)
        {
            fprintf(stderr, "***ERROR: memory allocation failure\n");                           //LCOV_EXCL_LINE
            exit(LM32_INTERNAL_ERROR);                                                          //LCOV_EXCL_LINE
        }
    }

    // By default, do a local access
    bool do_local_access = true;
    
#else
    // This fast calculation requires num_mem_bytes to be a power of 2
    uint32_t byte_addr = byte_addr_raw & num_mem_bytes_mask;
#endif

    // If there's a memory access callback function, and not accessing internal memory,
    // call it and flag if it intercepted the access
    if (pMemCallback != NULL
#ifdef LM32_FAST_COMPILE
        && !(byte_addr_raw >= mem_offset && byte_addr_raw < (mem_offset + num_mem_bytes))
#endif
        )
    {
        // Execute callback function
        mem_callback_delay = pMemCallback(byte_addr_raw, &word, type, LM32_CACHE_MISS, state.cycle_count);

        // Advance time by the returned amount, if intercepted, and flag no local access
        if (mem_callback_delay != LM32_EXT_MEM_NOT_PROCESSED && !disable_cycle_count) 
        {

#ifndef LM32_FAST_COMPILE
            do_local_access = false;
            state.cycle_count += (lm32_time_t)mem_callback_delay;
#endif
        }
    }
    // If not a callback address, do local memory access
#ifdef LM32_FAST_COMPILE
    else
    {
#else
    if (do_local_access)
    {
        // Add wait states for internal memory accesses
        if (!disable_cycle_count)
        {
            state.cycle_count += mem_wait_states;
        }

        // Check input is a valid address
        if ((byte_addr_raw  > (num_mem_bytes + mem_offset)) || (byte_addr_raw  < mem_offset))
        {
            state.int_flags |= (1 << INT_ID_DBUSERROR);
            return;
        }
#endif

        switch(type) 
        {
        case LM32_MEM_WR_ACCESS_BYTE:
            mem[byte_addr]      = data;
#ifndef LM32_FAST_COMPILE
            // Byte write can be used to load binary data to memeory, but only when disable_cycle_count is true,
            // when data should be marked as an 'instruction', for disassemble purposes.
            mem_tag[byte_addr] |= !disable_cycle_count ? MEM_DATA_WR : MEM_INSTRUCTION_WR;
#endif
            break;

        case LM32_MEM_WR_ACCESS_HWORD:
#ifndef LM32_FAST_COMPILE

            if (byte_addr & 0x1U)
            {
                state.int_flags |= (1 << INT_ID_DBUSERROR); 
                return;
            }

            mem_tag[byte_addr]   |= MEM_DATA_WR;
            mem_tag[byte_addr+1] |= MEM_DATA_WR;
#endif
            mem16[byte_addr>>1] = SWAPHALF(data);
            break;

        case LM32_MEM_WR_ACCESS_INSTR:
        case LM32_MEM_WR_ACCESS_WORD:
#ifndef LM32_FAST_COMPILE
            if (byte_addr & 0x3U)
            {
                state.int_flags |= (1 << INT_ID_DBUSERROR);
                return;
            }

            mem_tag[byte_addr]   |= (type == LM32_MEM_WR_ACCESS_WORD) ? MEM_DATA_WR : MEM_INSTRUCTION_WR;
            mem_tag[byte_addr+1] |= (type == LM32_MEM_WR_ACCESS_WORD) ? MEM_DATA_WR : MEM_INSTRUCTION_WR;
            mem_tag[byte_addr+2] |= (type == LM32_MEM_WR_ACCESS_WORD) ? MEM_DATA_WR : MEM_INSTRUCTION_WR;
            mem_tag[byte_addr+3] |= (type == LM32_MEM_WR_ACCESS_WORD) ? MEM_DATA_WR : MEM_INSTRUCTION_WR;
#endif
            mem32[byte_addr>>2] = SWAP(data);
            break;

        default:
            fprintf(stderr, "***ERROR: invalid write access type (%d)\n", type);        //LCOV_EXCL_LINE
            exit(LM32_USER_ERROR);                                                      //LCOV_EXCL_LINE
            break;
        }
    }

#ifndef LM32_FAST_COMPILE
    // Watch point processing
    if (state.cfg & (1 << LM32_CFG_H)) 
    {
        if (((GET_DC_C0(state.dc) & WP_WR_MASK) && (state.wp0 == byte_addr_raw)) ||
            ((GET_DC_C1(state.dc) & WP_WR_MASK) && (state.wp1 == byte_addr_raw)) ||
            ((GET_DC_C2(state.dc) & WP_WR_MASK) && (state.wp2 == byte_addr_raw)) ||
            ((GET_DC_C3(state.dc) & WP_WR_MASK) && (state.wp3 == byte_addr_raw)))
        {

            state.int_flags |= (1 << INT_ID_WATCHPOINT); 
        }
    }
#endif
}

// -------------------------------------------------------------------------
//  load_mem()
// 
//  load memory with program code/data words. For use mainly by read_elf().
// 
// -------------------------------------------------------------------------

void lm32_cpu::load_mem_word (const uint32_t byte_addr, const uint32_t word, const uint32_t flags)
{

    lm32_write_mem(byte_addr, word, (flags & PF_X) ? LM32_MEM_WR_ACCESS_INSTR : LM32_MEM_WR_ACCESS_WORD);
}

// -------------------------------------------------------------------------
// lm32_register_int_callback()
//
// Routine for registering user functions as callbacks for generating 
// external interrupts.
//
// -------------------------------------------------------------------------

void lm32_cpu::lm32_register_int_callback (p_lm32_intcallback_t callback_func)
{
    pIntCallback = callback_func;
}

// -------------------------------------------------------------------------
// lm32_register_ext_mem_callback()
//
// Routine for registering user function as callback for accesses to
// external memory.
//
// -------------------------------------------------------------------------

void lm32_cpu::lm32_register_ext_mem_callback (p_lm32_memcallback_t callback_func)
{
    pMemCallback = callback_func;
}

// -------------------------------------------------------------------------
// lm32_register_jtag_callback()
//
// Routine for registering user function as callback for accesses to
// JTAG interface.
//
// -------------------------------------------------------------------------

void lm32_cpu::lm32_register_jtag_callback (p_lm32_jtagcallback_t callback_func)
{
    state.cfg |= (1 << LM32_CFG_J);
    pJtagCallback = callback_func;
}

// -------------------------------------------------------------------------
// internal_reset_cpu()
// 
// Reset the state of the CPU as at power-up, or reset pin asserted.
//
// -------------------------------------------------------------------------

void lm32_cpu::internal_reset_cpu()
{
    state.pc   = entry_point_addr;
    state.ie   = 0;
    state.im   = 0;
    state.ip   = 0;
    state.icc  = 0;
    state.dcc  = 0;
    state.cfg  = LM32_DEFAULT_CONFIG;
    state.eba  = LM32_EBA_RESET;
    state.cfg2 = 0;

    // Do the debug registers get reset?
    state.dc   = 0;
    state.deba = LM32_DEBA_RESET;
    state.jtx  = 0;
    state.jrx  = 0;
    state.bp0  = 0;
    state.bp1  = 0;
    state.bp2  = 0;
    state.bp3  = 0;
    state.wp0  = 0;
    state.wp1  = 0;
    state.wp2  = 0;
    state.wp3  = 0;

    // Model state
    break_point    = 0;

    // The cc register will be reset to 0, but state.cycle_count is
    // not cleared. So cc_adjust is set to -cycle_count, so that
    // reads of the cc register start from 0 after reset.
    cc_adjust      = -1 * state.cycle_count;

    // Flag to invalidate the caches (if any)
    dcc_invalidate = true;
    icc_invalidate = true;
}

// -------------------------------------------------------------------------
// interrupt()
//
// Handle exceptions, not handled by instructions directly (such as break).
// Used mainly for external exceptions generated by interrupt callback.
//
// -------------------------------------------------------------------------

void lm32_cpu::interrupt (const int interrupt_id)
{
    switch(interrupt_id)
    {
    case INT_ID_RESET:
        internal_reset_cpu();
    case INT_ID_IBUSERROR:
    case INT_ID_DBUSERROR:
    case INT_ID_DIVZERO:
    case INT_ID_SYSCALL:
    case INT_ID_EXTINT:
        state.r[EA_REG_IDX] = state.pc;
        state.ie = (state.ie & ~IE_EIE_MASK) | ((state.ie & IE_IE_MASK) ? IE_EIE_MASK : 0);
        state.pc = ((state.dc & DEBUG_REMAP_ALL) ? state.deba : state.eba) + (interrupt_id << 5);
        break;

    case INT_ID_BREAKPOINT:
    case INT_ID_WATCHPOINT:
        state.r[BA_REG_IDX] = state.pc;
        state.ie = (state.ie & ~IE_BIE_MASK) | ((state.ie & IE_IE_MASK) ? IE_BIE_MASK : 0);
        state.pc = state.deba + (interrupt_id << 5);
        break;

    default:
        fprintf(stderr, "***ERROR: invalid exeption ID (%d)\n", interrupt_id);                  //LCOV_EXCL_LINE
        exit(LM32_INTERNAL_ERROR);                                                              //LCOV_EXCL_LINE
        break;
    }

    // Clear the master interrupt enable
    state.ie &= ~IE_IE_MASK;

    // Clear the flag the caused the interrupt
    state.int_flags &= ~(1ULL << interrupt_id);

}

// -------------------------------------------------------------------------
// process_exceptions()
//
// Called once per instruction to monitor for exceptions
//
// -------------------------------------------------------------------------

bool lm32_cpu::process_exceptions() 
{
    int int_id;

    // If there's a callback function registered, and we're at or beyond the
    // wakeup time, call the function and handle external interrupt state
    if (pIntCallback != NULL)
    {
        if (state.cycle_count >= state.wakeup_time_ext_int)
        {
            // Get interrupts from callback function, up to the maximum supported. Any
            // interrupt outside supported number is ignored.
            state.ip |= ((1ULL << ((state.cfg >> LM32_CFG_INT) & LM32_CFG_INT_MASK))-1) & pIntCallback(state.cycle_count, &state.wakeup_time_ext_int);

            // If the callback function returns a negative wakeup value, it is requesting
            // termination. Return immediately with a 'true' status.
            if (state.wakeup_time_ext_int < 0)
            {
                return true;
            }

            // If not masked, flag any external interrupt
            state.int_flags = (state.int_flags & ~(1 << INT_ID_EXTINT)) | ((state.ip & state.im) ? (1 << INT_ID_EXTINT) : 0);
        }
    }

    // If interrupts enabled, and anyone is interrupting, call the interrupt function
    if ((state.ie & IE_IE_MASK) && state.int_flags)
    {
        // Find the highest priority interrupt
        for (int_id = 0; int_id < INT_ID_NUM; int_id++) 
        {
            // If the bit is set in state.int_flags, highest priority found,
            // so break out of loop
            if ((1 << int_id) & state.int_flags)
            {
                break;
            }
        }

        // Call interrupt with highest priority ID
        interrupt(int_id);
    }
    return false;
}

// -------------------------------------------------------------------------
// lm32_reset_cpu()
//
// Externally called function to emulate the reset pin being asserted and
// release. Raises an interrupt request (highest priority) which will
// be serviced before the next instruction is executed.
//
// -------------------------------------------------------------------------

void lm32_cpu::lm32_reset_cpu()
{
    state.int_flags |= 1 << INT_ID_RESET;
}

// -------------------------------------------------------------------------
// execute_instruction()
//
// Main execution function. Fetches the opcode pointed to by the PC, does
// a lookup in the main table, decodes any arguments from the 
// opcode and then calls the function retrieved in the table lookup to
// action the instruction.
//
// -------------------------------------------------------------------------

bool lm32_cpu::execute_instruction (p_lm32_decode_t d)
{
    // Check for exceptions, and process if necessary. Returns true if
    // external callbacks requesting to terminate
    if (process_exceptions())
    {
        return true;
    }

#ifndef LM32_FAST_COMPILE

    // Flag any h/w breakpoints for this PC, but continue to execute instruction
    // and catch exception on the next iteration
    if (state.cfg & (1 << LM32_CFG_H))
    {
        if ((state.bp0 & 1) && ((state.bp0 & MASK_INSTR_ADDR) == state.pc) ||
            (state.bp1 & 1) && ((state.bp1 & MASK_INSTR_ADDR) == state.pc) ||
            (state.bp2 & 1) && ((state.bp2 & MASK_INSTR_ADDR) == state.pc) ||
            (state.bp3 & 1) && ((state.bp3 & MASK_INSTR_ADDR) == state.pc))
        {
            state.int_flags |= (1 << INT_ID_BREAKPOINT);
        }
    }
#endif

#ifndef LNXMICO32
    // Fetch the next instruction opcode
    d->opcode = lm32_read_mem(state.pc, LM32_MEM_RD_INSTR);
#else
    d->opcode = lm32_read_instr(state.pc);
#endif

    // Calculate the index into the decode tables from the opcode
    int table_index = d->opcode >> OPCODE_START_BIT;

    // Get decode information from the master table
    d->decode = &decode_table[table_index];

    // To avoid decoding the format of the instruction here, extract all possible
    // fields and have the instruction function use the fields it needs.
    d->reg0_csr = (d->opcode & MASK_Rx_REG0) >> Rx_REG0_START_BIT;
    d->reg1     = (d->opcode & MASK_Rx_REG1) >> Rx_REG1_START_BIT;
    d->reg2     = (d->opcode & MASK_Rx_REG2) >> Rx_REG2_START_BIT;
    d->imm      =  d->opcode & ((d->opcode & MASK_BIT31) == 0 ? MASK_RI_IMM : MASK_I_IMM);

#ifndef LM32_FAST_COMPILE
    // Check if we've reached an cycle count at disassemble start count,
    // set the active flag, which will remain on for the rest of the run.
    if (state.cycle_count >= disassemble_start)
    {
        disassemble_active = true;
    }

    // Disassemble 
    if (((verbose  && disassemble_active) || disassemble_run) && ((state.pc < (mem_offset + num_mem_bytes))
#ifndef LNXMICO32
        // By default only disassemble memory labelled as instructions
        && (mem_tag[state.pc - mem_offset] & MEM_INSTRUCTION_WR)
#endif
        ))
    {
        disassemble(d, ofp, disassemble_run);                                           //LCOV_EXCL_LINE
    }

    // Execute the indexed instruction
    if (disassemble_run == false) 
    {
        (this->*tbl_p[table_index])(d);
    } 
    else
    {
        state.pc = state.pc + 4;                                                        //LCOV_EXCL_LINE
    }
#else
    (this->*tbl_p[table_index])(d);
    state.cycle_count += 1;
#endif
    return false;
}

// -------------------------------------------------------------------------
// lm32_run_program()
//
// Main program loop. It will load the code to code memory from a specified 
// filename argument. It's responsible for servicing the interrupts and 
// timer callbacks (if enabled), before executing the next instruction
// indicated by the PC. Rudimentary break points are supported, with user
// configurable breaking on an address, or on a detectable 'loop forever'
// instruction. The parameters include a cycle count which specifies looping
// from a single instruction to 'LM32_FOREVER' (barring break points).
//
// -------------------------------------------------------------------------

int lm32_cpu::lm32_run_program (const char* filename, const lm32_time_t cycles, const int break_addr, const int exec_type, const bool load_code)
{
    lm32_decode_t decode;
    p_lm32_decode_t d = &decode;

    // Make sure we have some memory
    if (mem == NULL) 
    {
         if ((mem = (uint8_t *)malloc(num_mem_bytes/sizeof(uint8_t))) == NULL)
         {
            fprintf(stderr, "***ERROR: memory allocation failure\n");                   //LCOV_EXCL_LINE
            exit(LM32_INTERNAL_ERROR);                                                  //LCOV_EXCL_LINE
         }
         mem16 = (uint16_t*)mem;
         mem32 = (uint32_t*)mem;
    }

#ifndef LNXMICO32
    // Load program if asked to do so, or running form reset, but only if a filename specified
    if ((exec_type == LM32_RUN_FROM_RESET || load_code) && filename != NULL)
    {
        read_elf(filename);
    }
#endif

    // Clear breakpoint
    break_point = 0;

#ifdef LM32_FAST_COMPILE
    while(LM32_FOREVER)
    {
        // Execute the next instruction
        if (execute_instruction(d))
        {
           return LM32_USER_BREAK;
        }
        
        // Keep a tally of the number of instructions executed
        state.instr_count++;
    }
#else
    // Loop executing instructions either forever (if cycles negative) or
    // until state.cycle_count >= cycles, or until the major breakpoint is reached
    while (((cycles == LM32_FOREVER) || (state.cycle_count < cycles)) && (!break_point))
    {
        // If call type was a 'tick', check that state.clk_count has reached the executed
        // cycled_count...
        if (exec_type == LM32_RUN_TICK)
        {
            // If the not reached the state.cycle_count as at the last executed instruction,
            // keep incrementing on each 'tick' call until the same, when then next
            // instruction can be executed.
            if (state.clk_count < state.cycle_count)
            {
                state.clk_count++;
                return LM32_TICK_BREAK;
            }
            else
            {
                // Next instruction to be executed, but break afterwards
                break_point = LM32_TICK_BREAK;
            }
        } 

        // If not a 'tick' execution, simply make the cycle count the same as the state.cycle_count
        if (exec_type != LM32_RUN_TICK)
        {
            state.clk_count = state.cycle_count;
        }

        // Execute the next instruction
        bool external_break = execute_instruction(d);

        // Keep a tally of the number of instructions executed
        state.instr_count++;

        // Trap on lock conditions (often used at end of program)...
        if (!disable_lock_break && !disassemble_run)
        {
            // label0 : be r0, r0, label0 (0x44000000) or
            // label1 : bi 0 (0xe0000000)
            if (d->opcode == BREAK_LOCK_INSTR_R0 || d->opcode == BREAK_LOCK_INSTR_BI)
            {
                break_point = LM32_LOCK_BREAK;
                break;
            }
        }

        // If reached the user specified breakpoint address (and break_point not set by
        // previous instruction execution) flag to terminate the loop
        if ((state.pc == (break_addr & ~(0x3UL)) || external_break) && !break_point)
        {
            break_point = LM32_USER_BREAK;                                              //LCOV_EXCL_LINE
            break;                                                                      //LCOV_EXCL_LINE
        }

        // Break when hit the end of memory during a disassemble run
        if (disassemble_run && (state.pc == (mem_offset + num_mem_bytes)))
        {
            break_point = LM32_DISASSEMBLE_BREAK;                                       //LCOV_EXCL_LINE
            break;                                                                      //LCOV_EXCL_LINE
        }

        // If enabled, break if a hardware break or watch point reached
        if (!disable_hw_break)
        {
            if (state.int_flags & (1 << INT_ID_BREAKPOINT))
            {
                break_point = LM32_HW_BREAKPOINT_BREAK;
                break;
            }
            else if (state.int_flags & (1 << INT_ID_WATCHPOINT))
            {
                break_point = LM32_HW_WATCHPOINT_BREAK;
                break;
            }
        }

        // Break on a reset exception
        if (!disable_reset_break && (state.int_flags & (1 << INT_ID_RESET)))
        {
            break_point = LM32_RESET_BREAK;
            break;
        }

        // Break after only 1 instruction execution in single step mode
        if (exec_type == LM32_RUN_SINGLE_STEP)
        {
            break_point = LM32_SINGLE_STEP_BREAK;
            break;
        }
    }
#endif
    
    return break_point;
}


