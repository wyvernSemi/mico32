//=============================================================
// 
// Copyright (c) 2013-2016 Simon Southwell. All rights reserved.
//
// Date: 6th May 2013
//
// This is the model user definitions for the mico32 ISS
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
// $Id: lm32_cpu_hdr.h,v 2.6 2016-09-03 07:44:06 simon Exp $
// $Source: /home/simon/CVS/src/cpu/mico32/src/lm32_cpu_hdr.h,v $
//
//=============================================================

#ifndef _LM32_CPU_HDR_H_
#define _LM32_CPU_HDR_H_

// -------------------------------------------------------------------------
// INCLUDES
// -------------------------------------------------------------------------

#include <stdio.h>
#include <stdint.h>

// -------------------------------------------------------------------------
// DEFINES
// -------------------------------------------------------------------------

// Execution types
#define LM32_FOREVER                 (-1)
#define LM32_ONCE                    1

#define LM32_RUN_FROM_RESET          0
#define LM32_RUN_CONTINUE            1
#define LM32_RUN_SINGLE_STEP         2
#define LM32_RUN_TICK                3

// Break point types
#define LM32_USER_BREAK              1
#define LM32_LOCK_BREAK              2
#define LM32_DISASSEMBLE_BREAK       3
#define LM32_HW_BREAKPOINT_BREAK     4
#define LM32_HW_WATCHPOINT_BREAK     5
#define LM32_SINGLE_STEP_BREAK       6
#define LM32_TICK_BREAK              7
#define LM32_RESET_BREAK             8

// Default values for configuration
#define LM32_DEFAULT_FNAME           "test.elf"
#define LM32_NO_BREAK_ADDR           -1
#define LM32_DEFAULT_BREAKADDR       LM32_NO_BREAK_ADDR

// Exit code types
#define LM32_NO_ERROR                0
#define LM32_USER_ERROR              1
#define LM32_INSTR_ERROR             2
#define LM32_INTERNAL_ERROR          3

// Verbosity levels
#define LM32_VERBOSITY_LVL_OFF       0
#define LM32_VERBOSITY_LVL_1         1

// Memory access types
#define LM32_MEM_WR_ACCESS_BYTE      0
#define LM32_MEM_WR_ACCESS_HWORD     1
#define LM32_MEM_WR_ACCESS_WORD      2
#define LM32_MEM_WR_ACCESS_INSTR     3
#define LM32_MEM_RD_ACCESS_BYTE      4
#define LM32_MEM_RD_ACCESS_HWORD     5
#define LM32_MEM_RD_ACCESS_WORD      6
#define LM32_MEM_RD_INSTR            7
#define LM32_MEM_RNW_MASK            0x4

#define LM32_MEM_DISABLE_CYCLE_COUNT true

// JTAG access types
#define LM32_JTX_WR                  0
#define LM32_JTX_RD                  1
#define LM32_JRX_RD                  2

// External memory callback return values
#define LM32_EXT_MEM_NOT_PROCESSED   (-1)

// Bit field definitions for CFG register
#define LM32_CFG_M                   0
#define LM32_CFG_D                   1
#define LM32_CFG_S                   2
#define LM32_CFG_U                   3
#define LM32_CFG_X                   4
#define LM32_CFG_CC                  5
#define LM32_CFG_DC                  6
#define LM32_CFG_IC                  7
#define LM32_CFG_G                   8
#define LM32_CFG_H                   9
#define LM32_CFG_R                  10
#define LM32_CFG_J                  11
#define LM32_CFG_INT                12
#define LM32_CFG_BP                 18
#define LM32_CFG_WP                 22
#define LM32_CFG_REV                26

#define LM32_CFG_INT_MASK           0x3f
#define LM32_CFG_BP_MASK            0x07
#define LM32_CFG_WP_MASK            0x07

#define LM32_MULT_ENABLE            (1 << LM32_CFG_M)
#define LM32_DIV_ENABLE             (1 << LM32_CFG_D)
#define LM32_SHIFT_ENABLE           (1 << LM32_CFG_S)
#define LM32_SEXT_ENABLE            (1 << LM32_CFG_X)
#define LM32_COUNT_ENABLE           (1 << LM32_CFG_CC)
#define LM32_DCACHE_ENABLE          (1 << LM32_CFG_DC)
#define LM32_ICACHE_ENABLE          (1 << LM32_CFG_IC)
#define LM32_SWDEBUG_ENABLE         (1 << LM32_CFG_G)
#define LM32_HWDEBUG_ENABLE         (1 << LM32_CFG_H)
#define LM32_JTAG_ENABLE            (1 << LM32_CFG_J)

#define LM32_NUM_BP_0               0
#define LM32_NUM_BP_1               (1 << LM32_CFG_BP)
#define LM32_NUM_BP_2               (2 << LM32_CFG_BP)
#define LM32_NUM_BP_3               (3 << LM32_CFG_BP)
#define LM32_NUM_BP_4               (4 << LM32_CFG_BP)

#define LM32_NUM_WP_0               0
#define LM32_NUM_WP_1               (1 << LM32_CFG_WP)
#define LM32_NUM_WP_2               (2 << LM32_CFG_WP)
#define LM32_NUM_WP_3               (3 << LM32_CFG_WP)
#define LM32_NUM_WP_4               (4 << LM32_CFG_WP)

// Default capabilities
#define LM32_DEFAULT_CONFIG         (LM32_MULT_ENABLE    | \
                                     LM32_DIV_ENABLE     | \
                                     LM32_SHIFT_ENABLE   | \
                                     LM32_SEXT_ENABLE    | \
                                     LM32_COUNT_ENABLE   | \
                                     LM32_DCACHE_ENABLE  | \
                                     LM32_ICACHE_ENABLE  | \
                                     LM32_SWDEBUG_ENABLE | \
                                     LM32_HWDEBUG_ENABLE | \
                                     (32 << LM32_CFG_INT)| \
                                     LM32_NUM_BP_4       | \
                                     LM32_NUM_WP_4)
                                     
// Mask for writing to CFG register. Only bits set are configurable.
#define LM32_CONFIG_WRITE_MASK       (0x03fffbf7)

// Hardware debug register IDs
#define LM32_CSR_ID_BP0              (0x10)
#define LM32_CSR_ID_BP1              (0x11)
#define LM32_CSR_ID_BP2              (0x12)
#define LM32_CSR_ID_BP3              (0x13)
#define LM32_CSR_ID_WP0              (0x18)
#define LM32_CSR_ID_WP1              (0x19)
#define LM32_CSR_ID_WP2              (0x1a)
#define LM32_CSR_ID_WP3              (0x1b)

// Watchpoint control values
#define LM32_WP_DISABLED             (0x00 << 5)
#define LM32_WP_BREAK_ON_READ        (0x01 << 5)
#define LM32_WP_BREAK_ON_WRITE       (0x02 << 5)
#define LM32_WP_BREAK_ALWAYS         (0x03 << 5)

// Reset values for base addresses
#define LM32_EBA_RESET               0
#define LM32_DEBA_RESET              0

// Default size of internal memory
#define LM32_DEFAULT_MEM_SIZE        (0x10000)

// Basic configurations definitions
#define LM32_NUM_OF_REGISTERS        (32)
#define LM32_NUM_OPCODES             (64)

// Cache definitions
#define LM32_INVALIDATE_DCACHE       1
#define LM32_INVALIDATE_ICACHE       2

#define LM32_CACHE_DEFAULT_BASE      0x00000000UL
#define LM32_CACHE_DEFAULT_DLIMIT    0x0fffffffUL
#define LM32_CACHE_DEFAULT_ILIMIT    0x7fffffffUL
#define LM32_CACHE_DEFAULT_SETS      512
#define LM32_CACHE_DEFAULT_WAYS      1
#define LM32_CACHE_DEFAULT_LINE      4

#define LM32_MAX_NUM_CACHES          2
#define LM32_MAX_BYTES               16
#define LM32_MAX_WAYS                2
#define LM32_MAX_SETS                1024

#define LM32_CACHE_MISS              0
#define LM32_CACHE_HIT_WAY1          1
#define LM32_CACHE_HIT_WAY2          2

#define LM32_EXT_TERMINATE_REQ       -1

#define PF_X            0x1             /* Executable. */
#define PF_W            0x2             /* Writable. */
#define PF_R            0x4             /* Readable. */

#if defined( __CYGWIN__) || defined(_WIN32) || defined(__x86_64__) || defined(_M_X64) || defined(__i386) || defined(_M_IX86)
#define SWAP(_ARG) (((_ARG >> 24)&0xff) | ((_ARG >> 8)&0xff00) | ((_ARG << 8)&0xff0000) | ((_ARG << 24)&0xff000000))
#define SWAPHALF(_ARG) (uint32_t)(((_ARG>>8)&0xff) | ((_ARG<<8)&0xff00))
#else
#define SWAP(_ARG) _ARG
#define SWAPHALF(_ARG) _ARG
#endif

// -------------------------------------------------------------------------
// TYPEDEFS
// -------------------------------------------------------------------------

// Define a type for time
typedef  int64_t lm32_time_t;

// Define the type of the callback functions. These must be used
// by any function registered with the ISS.

// External interrupt callback is passed current time value. The function can
// optionally return a future time in 'wakeup_time' which will delay the
// subsequent recall to at, or beyond, that time. Any time less will result
// in the function be called in the next time slot. The return value is a
// 32 bit bitmap of requesting external interrupts.

typedef uint32_t (*p_lm32_intcallback_t) (const lm32_time_t time, lm32_time_t *wakeup_time);

// Memory access callback is passed a 32 bit address, a pointer to a 32 bit data
// word, a type // and the current time. The type is one of the 6 values defined
// above. If a write type 'data' will point to the value to be written, else it
// should be updated with the read value to be returned, if the callback intercepts
// the address. If the callback matches against the provided address, the function
// must return a non-zero value. If the address did not match one processed by the
// function, then -1 must be returned. By default the ISS will assume a single
// cycle access for intercepted addresses (i.e. 0 wait states). If the value returned
// is greater than 0, the cycle count will be incremented by the value returned.

typedef int      (*p_lm32_memcallback_t) (const uint32_t byte_addr, uint32_t *data, const int type, const int cache_hit, const lm32_time_t time);

// JTAG callback types
typedef void     (*p_lm32_jtagcallback_t) (uint32_t *data, const int type, const lm32_time_t time);

// Decode table entry structure type definition
typedef struct {
    const char* instr_name;            // Instruction name string for disassembly
    unsigned    instr_fmt;             // Instruction format
    lm32_time_t result_cycles;         // Number of cycles for a returned result (if any)
    lm32_time_t issue_cycles;          // Number of cycles to issue instruction (and not taken conditional branches)
    lm32_time_t issue_not_takencycles; // Number of cycles to issue instruction on taken branches (conditional branches only
    unsigned    signed_imm;            // Flag indicating whether immediate is signed extended
} lm32_decode_table_t, *p_lm32_decode_table_t;

// Current decoded instruction type, combining opcode and a pointer to its
// entry in the decode table
typedef struct {
    uint32_t    opcode;                // Full instruction opcode
    uint32_t    reg0_csr;              // reg0 (doubles as CSR field in CR format) 5 bit index
    uint32_t    reg1;                  // reg1 (doubles as reg field in CR format) 5 bit index 
    uint32_t    reg2;                  // reg2 5 bit index (0 in CR format)
    uint32_t    imm;                   // 26 bits for bi (0x38) and calli (0x3e) else 16 bits
    const lm32_decode_table_t* decode;
} lm32_decode_t, *p_lm32_decode_t;

// Cache configuration structures
typedef struct {
    uint32_t cache_base_addr;
    uint32_t cache_limit;
    int      cache_num_sets;
    int      cache_num_ways;
    int      cache_bytes_per_line;
} lm32_cache_config_t;

// Configuration structure for use externally to the model (not used internally)
typedef struct {
    char*               filename;
    char*               log_fname;
    int32_t             entry_point_addr;
    int                 test_mode;
    int                 verbose;
    int32_t             ram_dump_addr;
    int                 ram_dump_bytes;
    int                 dump_registers;
    int                 dump_num_exec_instr;
    int                 disassemble_run;
    int                 user_break_addr;
    int                 num_run_instructions;
    int                 disable_reset_break;
    int                 disable_hw_break;
    int                 disable_lock_break;
    uint32_t            mem_size;
    uint32_t            mem_offset;
    int                 mem_wait_states;
    int32_t             disassemble_start;
    uint32_t            cfg_word;
    lm32_cache_config_t dcache_cfg;
    lm32_cache_config_t icache_cfg;
    char*               save_fname;
    bool                load_state_file;
    bool                save_state_file;
} lm32_config_t;

#endif
