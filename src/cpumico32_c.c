//=============================================================
// 
// Copyright (c) 2013 - 2016Simon Southwell. All rights reserved.
//
// Date: 9th April 2013
//
// Top level C cpumico32 executable main() entry point. Used
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
// $Id: cpumico32_c.c,v 3.0 2016/09/07 13:15:36 simon Exp $
// $Source: /home/simon/CVS/src/cpu/mico32/src/cpumico32_c.c,v $
//
//=============================================================

// -------------------------------------------------------------------------
// INCLUDES
// -------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifndef WIN32
#ifdef CYGWIN
#include <GetOpt.h>
#else
#include <unistd.h>
#endif
#else 
extern "C" {
extern int getopt(int nargc, char** nargv, char* ostr);
extern char* optarg;
}
#endif

#include "lm32_cpu_c.h"

// -------------------------------------------------------------------------
// INCLUDES
// -------------------------------------------------------------------------

// Prototype for configuration parser
extern lm32_config_t* lm32_get_config(int argc, char** argv);

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
// LOCAL STATICS
// -------------------------------------------------------------------------

// Mico32 ISS model object pointer. Instantiated here so that it is available to
// callbacks as well main().
static lm32c_hdl* cpu_hdl;

// A default value for the execution type. Defined here so it can be overridden
// by the callback functions (in test mode only).
static int exec_type                   = LM32_RUN_FROM_RESET;

// Variables used to communicate between memory mailbox and interrupt callback
static lm32_time_t next_interrupt_time = MAX_INT_TIME;
static uint32_t interrupt_pattern      = 0;

// -------------------------------------------------------------------------
// Prototypes for callback functions
// -------------------------------------------------------------------------

uint32_t ext_interrupt  (const lm32_time_t time, lm32_time_t *wakeup_time);
int      ext_mem_access (const uint32_t byte_addr, uint32_t *data, const int type, const int cache_hit, const lm32_time_t time);
void     jtag_access    (uint32_t *data, const int type, const lm32_time_t time);

// -------------------------------------------------------------------------
// main()
//
// Entry point for cpumico32. Processes command line options and configures
// the model.
//
// -------------------------------------------------------------------------

int main (int argc, char** argv)
{
    FILE*          lfp          = stdout;
    lm32_config_t* p_cfg;
    lm32c_state    cpu_state;
    int            rtn_status;
    int            load_code    = TRUE;
    int            idx;

    // Process command line  and .ini file options
    p_cfg = lm32_get_config(argc, argv);

    // Open the logfile, if "stdout" not specified as the filename
    if (strcmp(p_cfg->log_fname, (char *)"stdout"))
    {
        if ((lfp = fopen(p_cfg->log_fname, "rb")) == NULL)
        {
            fprintf(stderr, "***ERROR: could not open log file %s for reading\n", p_cfg->log_fname);
            exit(LM32_USER_ERROR);
        }
    }

    // Generate a CPU object
    cpu_hdl = lm32c_cpu_init (LM32_VERBOSITY_LVL_OFF,
                              p_cfg->disable_reset_break,
                              p_cfg->disable_lock_break,
                              p_cfg->disable_hw_break,
                              p_cfg->disassemble_run,
                              p_cfg->mem_size,
                              p_cfg->mem_wait_states,
                              p_cfg->cfg_word,
                              lfp,
                              &p_cfg->dcache_cfg,
                              &p_cfg->icache_cfg);

    // Set the verbosity level
    lm32c_set_verbosity_level(cpu_hdl, p_cfg->verbose);

    // If an entry point address specified, fudge a reset vector at addres 0
    // to jump to the specified address
    if (p_cfg->entry_point_addr != -1)
    {
        lm32c_write_mem(cpu_hdl, 0, 0x98000000,                                    LM32_MEM_WR_ACCESS_WORD);
        lm32c_write_mem(cpu_hdl, 4, 0xe0000000 | (p_cfg->entry_point_addr-4) >> 2, LM32_MEM_WR_ACCESS_WORD);
    }

#ifndef LM32_NO_CALLBACK

    // If test mode enabled, register the callacks here, and do some
    // quick external access checks
    if (p_cfg->test_mode)
    {
        lm32c_register_int_callback(cpu_hdl, ext_interrupt);
        lm32c_register_ext_mem_callback(cpu_hdl, ext_mem_access);
        lm32c_register_jtag_callback(cpu_hdl, jtag_access);

        // Do a quick test of the BP register access functions
        for (idx = 0; idx < 4; idx++)
        {
            uint32_t reg_val;
            reg_val = lm32c_set_hw_debug_reg(cpu_hdl, 0xffffffff, LM32_CSR_ID_BP0+idx);
            if (reg_val != 0xfffffffd)
            {
                fprintf(stderr, "***ERROR: failed external access test\n");
                exit(LM32_INTERNAL_ERROR);
            }
        }

        // Do a quick test of the WP register access functions
        for (idx = 0; idx < 4; idx++)
        {
            uint32_t reg_val;
            reg_val = lm32c_set_hw_debug_reg(cpu_hdl, 0xffffffff, LM32_CSR_ID_WP0+idx);
            if (reg_val != 0xffffffff)
            {
                fprintf(stderr, "***ERROR: failed external access test\n");
                exit(LM32_INTERNAL_ERROR);
            }
        }

        // Check we can get the internal state over the external access method
        cpu_state = lm32c_get_cpu_state(cpu_hdl);
        if (cpu_state.cfg != (p_cfg->cfg_word | (1 << LM32_CFG_J)))
        {
            fprintf(stderr, "***ERROR: failed external access of internal state test\n");
            exit(LM32_INTERNAL_ERROR);
        }
    }
#endif

    // Run the program in the specified file on the ISS. Re-enter if the returned status
    // was for a watch- or breakpoint or single stepping, as this is a test program exit.
    do
    {

        rtn_status = lm32c_run_program(cpu_hdl, p_cfg->filename, p_cfg->num_run_instructions, p_cfg->user_break_addr, exec_type, load_code);
        load_code = FALSE;

    } while (rtn_status == LM32_HW_BREAKPOINT_BREAK || rtn_status == LM32_HW_WATCHPOINT_BREAK || 
             rtn_status == LM32_SINGLE_STEP_BREAK   || rtn_status == LM32_TICK_BREAK ||
             rtn_status == LM32_RESET_BREAK);

    // Dump registers after completion, if specified to do so
    if (p_cfg->dump_registers)
    {
        lm32c_dump_registers(cpu_hdl);
    }

    // Dump the number of executed instructions
    if (p_cfg->dump_num_exec_instr)
    {
        fprintf(lfp, "\nNumber of executed instructions = %ld\n",  lm32c_get_num_instructions(cpu_hdl));
    }

    // Dump RAM, if specified to do so and within range
    if (p_cfg->ram_dump_addr >= 0 && p_cfg->ram_dump_addr < p_cfg->mem_size)
    {

        // If a single word (or less), dump a whole word
        if (p_cfg->ram_dump_bytes <= 4)
        {
           fprintf(lfp, "\nRAM 0x%04x = 0x%08x\n", p_cfg->ram_dump_addr, lm32c_read_mem(cpu_hdl, p_cfg->ram_dump_addr, LM32_MEM_RD_ACCESS_WORD));

        // Dump multiple addressed formatted over several lines
        }
        else
        {
            fprintf(lfp, "\nRAM 0x%04x = \n", p_cfg->ram_dump_addr);

            for (idx = 0; idx < p_cfg->ram_dump_bytes; idx += 4)
            {
                if ((p_cfg->ram_dump_addr+idx) < p_cfg->mem_size)
                {
                    fprintf(lfp, "0x%08x ", lm32c_read_mem(cpu_hdl, p_cfg->ram_dump_addr+idx, LM32_MEM_RD_ACCESS_WORD));
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
    lm32c_set_configuration(cpu_hdl, p_cfg->cfg_word & ~((uint32_t)(LM32_DCACHE_ENABLE | LM32_ICACHE_ENABLE)));

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
        cfg = lm32c_get_configuration(cpu_hdl);

        if (type == LM32_MEM_WR_ACCESS_WORD)
        {
            switch (byte_addr & PERIPH_OFFSET_MASK)
            {
            case COMMS_INT_PATTERN_OFFSET:
                interrupt_pattern = *data;
                return 0;
            case COMMS_TIME_OFFSET:
                next_interrupt_time = lm32c_get_current_time(cpu_hdl) + *data;
                return 0;
            case COMMS_NUM_INT_OFFSET:
                lm32c_set_configuration(cpu_hdl, ((*data & 0x1f) << LM32_CFG_INT) | (cfg & ~(LM32_CFG_INT_MASK << LM32_CFG_INT)));
                return 0;
            case COMMS_MULT_EN_OFFSET:
                lm32c_set_configuration(cpu_hdl, ((*data & 0x1) << LM32_CFG_M) | (cfg & ~(1 << LM32_CFG_M)));
                return 0;
            case COMMS_DIV_EN_OFFSET:
                lm32c_set_configuration(cpu_hdl, ((*data & 0x1) << LM32_CFG_D) | (cfg & ~(1 << LM32_CFG_D)));
                return 0;
            case COMMS_SHIFT_EN_OFFSET:
                lm32c_set_configuration(cpu_hdl, ((*data & 0x1) << LM32_CFG_S) | (cfg & ~(1 << LM32_CFG_S)));
                return 0;
            case COMMS_SEXT_EN_OFFSET:
                lm32c_set_configuration(cpu_hdl, ((*data & 0x1) << LM32_CFG_X) | (cfg & ~(1 << LM32_CFG_X)));
                return 0;
            case COMMS_COUNT_EN_OFFSET:
                lm32c_set_configuration(cpu_hdl, ((*data & 0x1) << LM32_CFG_CC) | (cfg & ~(1 << LM32_CFG_CC)));
                return 0;
            case COMMS_BRKPT_EN_OFFSET:
                lm32c_set_configuration(cpu_hdl, ((*data & LM32_CFG_BP_MASK) << LM32_CFG_BP) | (cfg & ~(LM32_CFG_BP_MASK << LM32_CFG_BP)));
                return 0;
            case COMMS_WDOG_EN_OFFSET:
                lm32c_set_configuration(cpu_hdl, ((*data & LM32_CFG_WP_MASK) << LM32_CFG_WP) | (cfg & ~(LM32_CFG_WP_MASK << LM32_CFG_WP)));
                return 0;
            case COMMS_RESET_OFFSET:
                lm32c_reset_cpu(cpu_hdl);

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
                *data = (uint32_t)(lm32c_get_num_instructions(cpu_hdl) & 0xffffffffULL);
                break;
            case COMMS_INSTR_HI_OFFSET:
                *data = (uint32_t)((lm32c_get_num_instructions(cpu_hdl) >> 32ULL) & 0xffffffffULL);
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
uint32_t ext_interrupt (lm32_time_t time, lm32_time_t *wakeup_time) {

    uint32_t tmp;

    // If the time has expired to generate an interrupt, then fire the
    // interrupts with the pre-defined pattern
    if (time >= next_interrupt_time) {

        // Ensure that the next time is far off in the future
        next_interrupt_time = MAX_INT_TIME;

        // Save the interrupt pattern before clearing the state
        tmp = interrupt_pattern;
        interrupt_pattern = 0;

        // Send the interrupt pattern back to the MCU
        return tmp;
    } else 
        return 0;
}

// -------------------------------------------------------------------------
// jtag_access()
//
// Test callback function to emulate JTAG interface accesses. Configured as
// a simple loopback.
//
// -------------------------------------------------------------------------

void jtag_access (uint32_t *data, int type, lm32_time_t time) {

    static uint32_t jtag_reg = 0;

    switch(type) {
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

