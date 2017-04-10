//=============================================================
// 
// Copyright (c) 2013-2017 Simon Southwell. All rights reserved.
//
// Date: 9th April 2013
//
// Top level C++ cpumico32 executable main() entry point. Used
// mainly for self-test of model, and as an example API inter-
// facing program.
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
// $Id: cpumico32.cpp,v 3.5 2017/04/10 13:19:29 simon Exp $
// $Source: /home/simon/CVS/src/cpu/mico32/src/cpumico32.cpp,v $
//
//=============================================================

// -------------------------------------------------------------------------
// INCLUDES
// -------------------------------------------------------------------------

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdint.h>

#include "lm32_cpu.h"
#include "lm32_get_config.h"
#include "lm32_gdb.h"

#if !defined _WIN32 && !defined _WIN64
#include <unistd.h>
#else 
extern "C" {
extern int getopt(int nargc, char** nargv, char* ostr);
extern char* optarg;
}
#endif

// -------------------------------------------------------------------------
// DEFINES
// -------------------------------------------------------------------------

#define COMMS_BASE_ADDRESS        0x20000000
#define COMMS_INT_PATTERN_OFFSET  0x00000000
#define COMMS_TIME_OFFSET         0x00000004
#define COMMS_NUM_INT_OFFSET      0x00000008
#define COMMS_MULT_EN_OFFSET      0x0000000C
#define COMMS_DIV_EN_OFFSET       0x00000010
#define COMMS_SHIFT_EN_OFFSET     0x00000014
#define COMMS_SEXT_EN_OFFSET      0x00000018
#define COMMS_COUNT_EN_OFFSET     0x0000001C
#define COMMS_BRKPT_EN_OFFSET     0x00000020
#define COMMS_WDOG_EN_OFFSET      0x00000024
#define COMMS_RESET_OFFSET        0x00000028
#define COMMS_INSTR_LO_OFFSET     0x0000002c
#define COMMS_INSTR_HI_OFFSET     0x00000030
#define MAX_INT_TIME              0x7fffffffffffffffULL

#define PERIPH_PAGE_SIZE          4096
#define PERIPH_OFFSET_MASK        (PERIPH_PAGE_SIZE-1)
#define PERIPH_PAGE_MASK          (~PERIPH_OFFSET_MASK)

// -------------------------------------------------------------------------
// LOCAL PROTOTYPES
// -------------------------------------------------------------------------

// Prototypes for callback functions
uint32_t ext_interrupt  (lm32_time_t time, lm32_time_t *wakeup_time);
int      ext_mem_access (uint32_t byte_addr, uint32_t *data, int type, int cache_hit, lm32_time_t time);
void     jtag_access    (uint32_t *data, int type, lm32_time_t time);

// -------------------------------------------------------------------------
// LOCAL STATICS
// -------------------------------------------------------------------------

// Mico32 ISS model object pointer. Instantiated here so that it is available to
// callbacks as well main().
static lm32_cpu* cpu;

// A default value for the execution type. Defined here so it can be overridden
// by the callback functions (in test mode only).
static int exec_type                   = LM32_RUN_FROM_RESET;

// Variables used to communicate between memory mailbox and interrupt callback
static lm32_time_t next_interrupt_time = MAX_INT_TIME;
static uint32_t interrupt_pattern      = 0;

// -------------------------------------------------------------------------
// main()
//
// Entry point for cpumico32. Processes command line options and configures
// the model.
//
// -------------------------------------------------------------------------
int main (int argc, char** argv)
{
    FILE     *lfp             = stdout;
    lm32_config_t* p_cfg;

    // Process command line and .ini file options
    p_cfg = lm32_get_config(argc, argv);

    // Open the logfile, if "stdout" not specified as the filename
    if (strcmp(p_cfg->log_fname, (char *)"stdout"))
    {
        if ((lfp = fopen(p_cfg->log_fname, "wb")) == NULL)
        {
            fprintf(stderr, "***ERROR: could not open log file %s for writing\n", p_cfg->log_fname);
            exit(LM32_USER_ERROR);
        }
    }

    // Generate a CPU object
    cpu = new lm32_cpu(LM32_VERBOSITY_LVL_OFF,
                       p_cfg->disable_reset_break ? true : false,
                       p_cfg->disable_lock_break  ? true : false,
                       p_cfg->disable_hw_break    ? true : false,
                       p_cfg->disable_int_break   ? true : false,
                       p_cfg->disassemble_run     ? true : false,
                       p_cfg->mem_size,
                       p_cfg->mem_offset,
                       p_cfg->mem_wait_states,
                       p_cfg->entry_point_addr,
                       p_cfg->cfg_word,
                       lfp,
                       &p_cfg->dcache_cfg,
                       &p_cfg->icache_cfg);

    // Set the verbosity level
    cpu->lm32_set_verbosity_level(p_cfg->verbose);

    // By convention, r0 is always 0. Use lm32_set_gp_reg() to ensure this
    // for coverage.
    cpu->lm32_set_gp_reg(0, 0);

    // If test mode enabled, register the callacks here, and do some
    // quick external access checks
    if (p_cfg->test_mode)
    {
        cpu->lm32_register_int_callback(ext_interrupt);
        cpu->lm32_register_ext_mem_callback(ext_mem_access);
        cpu->lm32_register_jtag_callback(jtag_access);

        // Do a quick test of the BP register access functions
        for (int idx = 0; idx < 4; idx++)
        {
            uint32_t reg_val;
            reg_val = cpu->lm32_set_hw_debug_reg(0xffffffff, LM32_CSR_ID_BP0+idx);
            if (reg_val != 0xfffffffd)
            {
                fprintf(stderr, "***ERROR: failed external access test\n");
                exit(LM32_INTERNAL_ERROR);
            }
            reg_val = cpu->lm32_set_hw_debug_reg(0, LM32_CSR_ID_BP0+idx);
        }

        // Do a quick test of the WP register access functions
        for (int idx = 0; idx < 4; idx++)
        {
            uint32_t reg_val;
            reg_val = cpu->lm32_set_hw_debug_reg(0xffffffff, LM32_CSR_ID_WP0+idx);
            if (reg_val != 0xffffffff)
            {
                fprintf(stderr, "***ERROR: failed external access test\n");
                exit(LM32_INTERNAL_ERROR);
            }
            reg_val = cpu->lm32_set_hw_debug_reg(0, LM32_CSR_ID_WP0+idx);
        }

        // Check we can get the internal state over the external access method
        lm32_cpu::lm32_state cpu_state = cpu->lm32_get_cpu_state();
        if (cpu_state.cfg != (p_cfg->cfg_word | (1 << LM32_CFG_J))) 
        {
            fprintf(stderr, "***ERROR: failed external access of internal state test\n");
            exit(LM32_INTERNAL_ERROR);
        }
    }

    if (!p_cfg->gdb_run)
    {
        // Run the program in the specified file on the ISS. Re-enter if the returned status
        // was for a watch- or breakpoint or single stepping, as this is a test program exit.
        int rtn_status;
        bool load_code = true;
        do
        {
    
            rtn_status = cpu->lm32_run_program(p_cfg->filename, p_cfg->num_run_instructions, p_cfg->user_break_addr, exec_type, load_code);
            load_code = false;
    
        } while (rtn_status == LM32_HW_BREAKPOINT_BREAK || rtn_status == LM32_HW_WATCHPOINT_BREAK || 
                 rtn_status == LM32_SINGLE_STEP_BREAK   || rtn_status == LM32_TICK_BREAK ||
                 rtn_status == LM32_RESET_BREAK);
    }
    else
    {
        // Start procssing commands from GDB
        if (lm32gdb_process_gdb(cpu, p_cfg->com_port_num))
        {
            fprintf(stderr, "***ERROR in opening PTY\n");
            return -1;
        }
    }

    // Dump registers after completion, if specified to do so
    if (p_cfg->dump_registers)
    {
        cpu->lm32_dump_registers();
    }

    // Dump the number of executed instructions
    if (p_cfg->dump_num_exec_instr)
    {
        fprintf(lfp, "\nNumber of executed instructions = %ld\n",  cpu->lm32_get_num_instructions());
    }

    // Dump RAM, if specified to do so and within range
    if (p_cfg->ram_dump_addr >= 0 && p_cfg->ram_dump_addr < (int)p_cfg->mem_size)
    {
        // If a single word (or less), dump a whole word
        if (p_cfg->ram_dump_bytes <= 4)
        {
           fprintf(lfp, "\nRAM 0x%04x = 0x%08x\n", p_cfg->ram_dump_addr, cpu->lm32_read_mem(p_cfg->ram_dump_addr, LM32_MEM_RD_ACCESS_WORD));

        // Dump multiple address locations, formatted over several lines
        }
        else
        {
            fprintf(lfp, "\nRAM 0x%04x = \n", p_cfg->ram_dump_addr);

            for (int idx = 0; idx < p_cfg->ram_dump_bytes; idx += 4)
            {
                if ((p_cfg->ram_dump_addr+idx) < (int)p_cfg->mem_size)
                {
                    fprintf(lfp, "0x%08x ", cpu->lm32_read_mem(p_cfg->ram_dump_addr+idx, LM32_MEM_RD_ACCESS_WORD));
                }
                // Ran off the end of memory
                else
                {
                    fprintf(lfp, "0x???????? ");
                }

                if ((idx%16) == 12)
                {
                    fprintf(lfp, "\n");
                }
            }
        }
    } 
    else 
    {
        if (p_cfg->ram_dump_addr != -1)
        {
            fprintf(lfp, "RAM 0x%04x = ????\n", p_cfg->ram_dump_addr);
        }
    }

    // Disable the caches, to exercise their deletion
    cpu->lm32_set_configuration(p_cfg->cfg_word & ~((uint32_t)(LM32_DCACHE_ENABLE | LM32_ICACHE_ENABLE)));
}


// -------------------------------------------------------------------------
// Test callback functions used as part of the verification test suite 
// -------------------------------------------------------------------------

    
// -------------------------------------------------------------------------
// ext_mem_access()
//
// External interrupt callback function for test purposes. Implements a mailbox
// where software running on the ISS can set up an interrupt at some future time
// and configure the number of supported interrupts, as well as other MCU
// configurable parameters, such as supported instructions and number of
// h/w break- and watchpoints
//
// -------------------------------------------------------------------------

int ext_mem_access (uint32_t byte_addr, uint32_t *data, int type, int cache_hit, lm32_time_t time)
{
    int cfg;

    // Trap on certain addresses to allow a program to specify a time (relative to 
    // current time) that an interrupt should occur, and a pattern to place
    // on the external interrupt inputs, or to reconfigure the ISS
    if ((byte_addr & PERIPH_PAGE_MASK) == COMMS_BASE_ADDRESS)
    {
        // Fetch the current configuration in anticipation of an update to a field
        cfg = cpu->lm32_get_configuration();

        if (type == LM32_MEM_WR_ACCESS_WORD)
        {
            switch (byte_addr & PERIPH_OFFSET_MASK)
            {
            case COMMS_INT_PATTERN_OFFSET:
                interrupt_pattern = *data;
                return 0;
            case COMMS_TIME_OFFSET:
                next_interrupt_time = cpu->lm32_get_current_time() + *data;
                return 0;
            case COMMS_NUM_INT_OFFSET:
                cpu->lm32_set_configuration(((*data & 0x1f) << LM32_CFG_INT) | (cfg & ~(LM32_CFG_INT_MASK << LM32_CFG_INT)));
                return 0;
            case COMMS_MULT_EN_OFFSET:
                cpu->lm32_set_configuration(((*data & 0x1) << LM32_CFG_M) | (cfg & ~(1 << LM32_CFG_M)));
                return 0;
            case COMMS_DIV_EN_OFFSET:
                cpu->lm32_set_configuration(((*data & 0x1) << LM32_CFG_D) | (cfg & ~(1 << LM32_CFG_D)));
                return 0;
            case COMMS_SHIFT_EN_OFFSET:
                cpu->lm32_set_configuration(((*data & 0x1) << LM32_CFG_S) | (cfg & ~(1 << LM32_CFG_S)));
                return 0;
            case COMMS_SEXT_EN_OFFSET:
                cpu->lm32_set_configuration(((*data & 0x1) << LM32_CFG_X) | (cfg & ~(1 << LM32_CFG_X)));
                return 0;
            case COMMS_COUNT_EN_OFFSET:
                cpu->lm32_set_configuration(((*data & 0x1) << LM32_CFG_CC) | (cfg & ~(1 << LM32_CFG_CC)));
                return 0;
            case COMMS_BRKPT_EN_OFFSET:
                cpu->lm32_set_configuration(((*data & LM32_CFG_BP_MASK) << LM32_CFG_BP) | (cfg & ~(LM32_CFG_BP_MASK << LM32_CFG_BP)));
                return 0;
            case COMMS_WDOG_EN_OFFSET:
                cpu->lm32_set_configuration(((*data & LM32_CFG_WP_MASK) << LM32_CFG_WP) | (cfg & ~(LM32_CFG_WP_MASK << LM32_CFG_WP)));
                return 0;
            case COMMS_RESET_OFFSET:
                cpu->lm32_reset_cpu();

                // After a reset, switch to defined type, as set by the test program
                exec_type = *data & 0x3;
                return 0;
            default:
                return LM32_EXT_MEM_NOT_PROCESSED;
            }

        } 
        else if (type == LM32_MEM_RD_ACCESS_WORD)
        {
            switch (byte_addr & PERIPH_OFFSET_MASK)
            {
            case COMMS_INSTR_LO_OFFSET:
                *data = (uint32_t)(cpu->lm32_get_num_instructions() & 0xffffffffULL);
                break;
            case COMMS_INSTR_HI_OFFSET:
                *data = (uint32_t)((cpu->lm32_get_num_instructions() >> 32ULL) & 0xffffffffULL);
                break;
            // For all the configuration offsets, return the whole CFG register value
            case COMMS_NUM_INT_OFFSET:
            case COMMS_MULT_EN_OFFSET:
            case COMMS_DIV_EN_OFFSET:
            case COMMS_SHIFT_EN_OFFSET:
            case COMMS_SEXT_EN_OFFSET:
            case COMMS_COUNT_EN_OFFSET:
            case COMMS_BRKPT_EN_OFFSET:
            case COMMS_WDOG_EN_OFFSET:
                *data = cpu->lm32_get_configuration();
                break;
            default:
                *data= 0xdeadbeef;
                break;
            }
            return 0;
        } 
        else
        {
            return LM32_EXT_MEM_NOT_PROCESSED;
        }
    } 
    else
    {
        return LM32_EXT_MEM_NOT_PROCESSED;
    }
}

// -------------------------------------------------------------------------
// ext_interrupt()
//
// Test callback function to generate external interrupts as configured by
// the mailbox implemented in the memory callback function above.
//
// -------------------------------------------------------------------------

// External interrupt callback function for test purposes
uint32_t ext_interrupt (lm32_time_t time, lm32_time_t *wakeup_time) 
{
    uint32_t tmp;

    // If the time has expired to generate an interrupt, then fire the
    // interrupts with the pre-defined pattern
    if (time >= next_interrupt_time)
    {
        // Ensure that the next time is far off in the future
        next_interrupt_time = MAX_INT_TIME;

        // Save the interrupt pattern before clearing the state
        tmp = interrupt_pattern;
        interrupt_pattern = 0;

        // Send the interrupt pattern back to the MCU
        return tmp;
    }
    else
    {
        return 0;
    }
}

// -------------------------------------------------------------------------
// jtag_access()
//
// Test callback function to emulate JTAG interface accesses. Configured as
// a simple loopback.
//
// -------------------------------------------------------------------------

void jtag_access (uint32_t *data, int type, lm32_time_t time)
{
    static uint32_t jtag_reg = 0;

    switch(type)
    {
    case LM32_JTX_WR:
        jtag_reg = *data;
        break;
    case LM32_JTX_RD:
        *data = jtag_reg;
        break;
    case LM32_JRX_RD:
        *data = jtag_reg;
        break;
    }
}
