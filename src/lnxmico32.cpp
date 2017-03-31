//=============================================================
// 
// Copyright (c) 2016 Simon Southwell. All rights reserved.
//
// Date: 24th August 2016
//
// Top level C++ lnxmico32 executable main() entry point.
//
// This file is part of the lnxmico32 ISS linux system model.
//
// lnxmico32 is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// lnxmico32 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with lnxmico32. If not, see <http://www.gnu.org/licenses/>.
//
// $Id: lnxmico32.cpp,v 3.3 2017/03/31 11:48:39 simon Exp $
// $Source: /home/simon/CVS/src/cpu/mico32/src/lnxmico32.cpp,v $
//
//=============================================================

// -------------------------------------------------------------------------
// INCLUDES
// -------------------------------------------------------------------------

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdint.h>

#if !(defined _WIN32) && !(defined _WIN64)
#ifdef CYGWIN
#include <GetOpt.h>
#else
#include <unistd.h>
#endif
#include <termios.h>
#include <sys/time.h>
#else 
extern "C" {
extern int getopt(int nargc, char** nargv, char* ostr);
extern char* optarg;
}
#include <Windows.h>
#endif

#include "lm32_cpu.h"
#include "lm32_get_config.h"
#include "lm32_gdb.h"
#include "lnxmico32.h"
#include "lnxtimer.h"
#include "lnxuart.h"

// -------------------------------------------------------------------------
// DEFINES
// -------------------------------------------------------------------------

#define LM32_MEM_WR_PAGE_BITS 10
#define LM32_MEM_WR_BUF_SIZE (LM32_RAM_SIZE/(1 << LM32_MEM_WR_PAGE_BITS))

#if !(defined _WIN32) && !(defined _WIN64) && !(defined CYGWIN)
#define LM32_TIME_PRINT_STR "%ld"
#else
#define LM32_TIME_PRINT_STR "%lld"
#endif

// -------------------------------------------------------------------------
// LOCAL STATICS
// -------------------------------------------------------------------------

// Mico32 ISS model object pointer. Instantiated here so that it is available to 
// callbacks as well main().
static lm32_cpu* cpu;

static bool mem_wr [LM32_MEM_WR_BUF_SIZE];

static lm32_time_t wakeup_time_save = 0;
static lm32_config_t* p_cfg         = NULL;

// -------------------------------------------------------------------------
// CONSTANTS
// -------------------------------------------------------------------------

// HW setup parameters
static const uint8_t cpu_config[LM32_CPU_CONFIG_LEN] = {
     0, 0, 0, LM32_CPU_CONFIG_LEN,
     0, 0, 0, LM32_HW_TAG_CPU,
    'L', 'M', '3', '2', 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,
    (LM32_CPU_FREQUENCY_HZ >> 24) & 0xff,
    (LM32_CPU_FREQUENCY_HZ >> 16) & 0xff,
    (LM32_CPU_FREQUENCY_HZ >>  8) & 0xff,
    (LM32_CPU_FREQUENCY_HZ >>  0) & 0xff
};

static const uint8_t ddr_config[LM32_DDR_CONFIG_LEN] = {
     0, 0, 0, LM32_DDR_CONFIG_LEN,
     0, 0, 0, LM32_HW_TAG_DDR_SDRAM,
    'd', 'd', 'r', '_', 's', 'd', 'r', 'a',
    'm', 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,
     (LM32_RAM_BASE_ADDR >> 24) & 0xff,
     (LM32_RAM_BASE_ADDR >> 16) & 0xff,
     (LM32_RAM_BASE_ADDR >>  8) & 0xff,
     (LM32_RAM_BASE_ADDR >>  0) & 0xff,
     (LM32_RAM_SIZE >> 24)      & 0xff,
     (LM32_RAM_SIZE >> 16)      & 0xff,
     (LM32_RAM_SIZE >>  8)      & 0xff,
     (LM32_RAM_SIZE >>  0)      & 0xff
};

static const uint8_t tim0_config[LM32_TIM_CONFIG_LEN] = {
     0, 0, 0, LM32_TIM_CONFIG_LEN,
     0, 0, 0, LM32_HW_TAG_TIMER,
    't', 'i', 'm', 'e', 'r', '0', 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,
     (LM32_TIMER0_BASE_ADDR >> 24) & 0xff,
     (LM32_TIMER0_BASE_ADDR >> 16) & 0xff,
     (LM32_TIMER0_BASE_ADDR >>  8) & 0xff,
     (LM32_TIMER0_BASE_ADDR >>  0) & 0xff,
     1, 1, 1, 32, 
     0, 0, 0, 20,
     LM32_TIMER0_IRQ, 0, 0, 0
};

static const uint8_t uart0_config[LM32_URT_CONFIG_LEN] = {
     0, 0, 0, LM32_URT_CONFIG_LEN,
     0, 0, 0, LM32_HW_TAG_UART,
    'u', 'a', 'r', 't', '0', 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,
     (LM32_UART0_BASE_ADDR >> 24) & 0xff,
     (LM32_UART0_BASE_ADDR >> 16) & 0xff,
     (LM32_UART0_BASE_ADDR >>  8) & 0xff,
     (LM32_UART0_BASE_ADDR >>  0) & 0xff,
     (LM32_UART_BAUD_RATE >> 24) & 0xff,
     (LM32_UART_BAUD_RATE >> 16) & 0xff,
     (LM32_UART_BAUD_RATE >>  8) & 0xff,
     (LM32_UART_BAUD_RATE >>  0) & 0xff,
     8, 1, 1, 1, 1, 4, 4, 
     LM32_UART0_IRQ
};

static const uint8_t uart1_config[LM32_URT_CONFIG_LEN] = {
     0, 0, 0, LM32_URT_CONFIG_LEN,
     0, 0, 0, LM32_HW_TAG_UART,
    'u', 'a', 'r', 't', '1', 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,
     (LM32_UART1_BASE_ADDR >> 24) & 0xff,
     (LM32_UART1_BASE_ADDR >> 16) & 0xff,
     (LM32_UART1_BASE_ADDR >>  8) & 0xff,
     (LM32_UART1_BASE_ADDR >>  0) & 0xff,
     (LM32_UART_BAUD_RATE >> 24) & 0xff,
     (LM32_UART_BAUD_RATE >> 16) & 0xff,
     (LM32_UART_BAUD_RATE >>  8) & 0xff,
     (LM32_UART_BAUD_RATE >>  0) & 0xff,
     8, 1, 1, 1, 1, 4, 4, 
     LM32_UART1_IRQ
};

static const uint8_t trail_config[LM32_TRL_CONFIG_LEN] = {
    0, 0, 0, LM32_TRL_CONFIG_LEN,
    0, 0, 0, LM32_HW_TAG_EOL
};

static double tv_diff;
#if !(defined _WIN32) && !(defined _WIN64)
static struct timeval tv_start, tv_stop;

#else
LARGE_INTEGER freq, start, stop;
#endif

// -------------------------------------------------------------------------
// Terminal control utility functions for enabling/diabling input echoing
// -------------------------------------------------------------------------

static void pre_run_setup()
{
#if !(defined _WIN32) && !(defined _WIN64)
    // For non-windows systems, turn off echoing of input key presses
    struct termios t;

    tcgetattr(STDIN_FILENO, &t);
    t.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &t);

    // Log time just before running (LINUX only)
    (void)gettimeofday(&tv_start, NULL);
#else
    
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&start);
#endif 
}

static void post_run_setup()
{
#if !(defined _WIN32) && !(defined _WIN64)
    // For non-windows systems, turn off echoing of input key presses
    struct termios t;

    tcgetattr(STDIN_FILENO, &t);
    t.c_lflag |= ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &t);

    // Get time just after running, and calculate run time (LINUX only)
    (void)gettimeofday(&tv_stop, NULL);
    tv_diff = ((float)(tv_stop.tv_sec - tv_start.tv_sec)*1e6) + ((float)(tv_stop.tv_usec - tv_start.tv_usec));
#else
    QueryPerformanceCounter(&stop);
    tv_diff = (double)(stop.QuadPart - start.QuadPart)*1e6/(double)freq.QuadPart;
#endif 
}

// -------------------------------------------------------------------------
// load_system_state()
//
// Load the state previously stored in a .sav file (if the specified file
// exists). The function expects the state to be in the order CPU, Timers,
// Uarts, and a set of RAM pages.
//
// -------------------------------------------------------------------------

static void load_system_state()
{
    FILE* sfp;

    // Attempt to open a .sav file for reading
    sfp = fopen(p_cfg->save_fname, "rb");

    if (sfp != NULL)
    {
        fprintf(stdout, "Loading %s...", p_cfg->save_fname);

        // Load the CPU state
        lm32_cpu::lm32_state saved_state;
        uint8_t* p_saved_state_byte = (uint8_t*) &saved_state;

        for (int idx = 0; idx < sizeof(lm32_cpu::lm32_state); idx++)
        {
            p_saved_state_byte[idx] = getc(sfp) & 0xff;
        }
        cpu->lm32_set_cpu_state(saved_state);

        // Load the timer state
        lm32_timer_state_t timer_state;
        p_saved_state_byte = (uint8_t*)&timer_state;

        for(int idx = 0; idx < sizeof(lm32_timer_state_t); idx++)
        {
            p_saved_state_byte[idx] = getc(sfp) & 0xff;
        }
        lm32_set_timer_state(timer_state);

        // Load the uart state
        lm32_uart_state_t uart_state;
        p_saved_state_byte = (uint8_t*)&uart_state;

        for(int idx = 0; idx < sizeof(lm32_uart_state_t); idx++)
        {
            p_saved_state_byte[idx] = getc(sfp) & 0xff;
        }
        lm32_set_uart_state(uart_state);

        int c;
        while ((c = getc(sfp)) != EOF)
        {
            // Load the address (MSB)
            unsigned addr = 0;
            addr |= (c         & 0xff) << 24;
            addr |= (getc(sfp) & 0xff) << 16;
            addr |= (getc(sfp) & 0xff) <<  8;
            addr |= (getc(sfp) & 0xff) <<  0;
            
            // Load the bytes of the page
            for(int idx = 0; idx < (1 << LM32_MEM_WR_PAGE_BITS); idx++)
            {
                uint32_t data = getc(sfp) & 0xff;
                cpu->lm32_write_mem(addr++, data, LM32_MEM_WR_ACCESS_BYTE, true);
            }
        }
        
        fprintf(stdout, "Done.\n");
        
        // Close the file
        fclose(sfp);
    }
}

// -------------------------------------------------------------------------
// save_system_state()
//
// Save the systems's state to a .sav file. The function will save the state 
// to be in the order CPU, Timers, Uarts, and a set of RAM pages.
//
// -------------------------------------------------------------------------

static void save_system_state()
{
    // Save updated memory
    FILE* sfp;
    
    if ((sfp = fopen(p_cfg->save_fname, "wb")) != NULL)
    {
        fprintf(stdout, "\nSaving %s...", p_cfg->save_fname);

        // Save the CPU state
        lm32_cpu::lm32_state state = cpu->lm32_get_cpu_state();

        state.wakeup_time_ext_int = wakeup_time_save;

        uint8_t* p_state_byte = (uint8_t*)&state;

        for(int idx = 0; idx < sizeof(lm32_cpu::lm32_state); idx++)
        {
            putc(p_state_byte[idx], sfp);
        }

        // Save the timer(s) state
        lm32_timer_state_t timer_state = lm32_get_timer_state();
        p_state_byte = (uint8_t*)&timer_state;
        for(int idx = 0; idx < sizeof(lm32_timer_state_t); idx++)
        {
            putc(p_state_byte[idx], sfp);
        }

        // Save the UART(s) state
        lm32_uart_state_t uart_state = lm32_get_uart_state();
        p_state_byte = (uint8_t*)&uart_state;
        for(int idx = 0; idx < sizeof(lm32_uart_state_t); idx++)
        {
            putc(p_state_byte[idx], sfp);
        }

        // Save the memory blocks that have been written to
        for (int idx = 0; idx < (LM32_MEM_WR_BUF_SIZE); idx++)
        {
            if (mem_wr[idx])
            {
                // Save the page address (MSB)
                uint32_t addr = LM32_RAM_BASE_ADDR + (idx << LM32_MEM_WR_PAGE_BITS);
                putc((addr >> 24) & 0xff, sfp);
                putc((addr >> 16) & 0xff, sfp);
                putc((addr >>  8) & 0xff, sfp);
                putc((addr >>  0) & 0xff, sfp);
        
                // Store the bytes from the accessed page
                for (int sdx = 0; sdx < (1 << LM32_MEM_WR_PAGE_BITS); sdx++)
                {
                    unsigned data = cpu->lm32_read_mem(addr++, LM32_MEM_RD_ACCESS_BYTE);
                    putc(data, sfp);
                }
            }
        }

        fprintf(stdout, "Done.\n");
        
        fclose(sfp);
    }
    else
    {
        fprintf(stderr, "Warning: unable to open %s for writing. State will not be saved\n", p_cfg->save_fname);
    }
}

// -------------------------------------------------------------------------
// ext_mem_access()
//
// Intercept any memory accesses that match external locations.
//
// -------------------------------------------------------------------------

static int ext_mem_access (const uint32_t byte_addr, uint32_t *data, const int type, const int cache_hit, const lm32_time_t time)
{
    // By default, memory access is not intercepted
    int processing_time  = LM32_EXT_MEM_NOT_PROCESSED;

    // Get the address aligned to a 4K page boundary
    uint32_t page_addr   = byte_addr & ~(LM32_PAGE_MASK);
    uint32_t offset_addr = byte_addr & (LM32_PAGE_MASK);

    // Default the register access time to a single cycle
    processing_time = 1;

    switch(page_addr)
    {
    case LM32_TIMER0_BASE_ADDR:
        if (type == LM32_MEM_WR_ACCESS_WORD)
            lm32_timer_write(offset_addr, *data);
        else
            lm32_timer_read(offset_addr, data);
        break;
    case LM32_UART0_BASE_ADDR:
    case LM32_UART1_BASE_ADDR:
        {
            unsigned cntx = (page_addr == LM32_UART1_BASE_ADDR) ? LM32_UART1_CNTX : LM32_UART0_CNTX;
        
            if (type == LM32_MEM_WR_ACCESS_WORD)
                lm32_uart_write(offset_addr, *data, cntx);
            else
                lm32_uart_read(offset_addr, data, cntx);
            break;
        }
    default:
        processing_time = LM32_EXT_MEM_NOT_PROCESSED;
        if (byte_addr >=0x08000000 && byte_addr < 0x0c000000 &&
            (type == LM32_MEM_WR_ACCESS_WORD || type == LM32_MEM_WR_ACCESS_HWORD || type == LM32_MEM_WR_ACCESS_BYTE))
        {
            mem_wr[(byte_addr & 0x07ffffff) >> LM32_MEM_WR_PAGE_BITS] = true;
        }
        break;
    }

    return processing_time;
}

// -------------------------------------------------------------------------
// ext_interrupt()
//
// Callback function to generate external interrupts, based on peripheral
// requests.
//
// -------------------------------------------------------------------------

static uint32_t ext_interrupt (const lm32_time_t time, lm32_time_t *wakeup_time)
{
    uint32_t rtn_interrupt = 0;
    bool terminate = false;

    // Tick timer 0, and check interrupt status
    if (lm32_timer_tick(time))
    {
        // Timer is requesting an interrupt
        rtn_interrupt |= 1 << LM32_TIMER0_IRQ;
    }

    // Tick UART0 and check interrupt status---keyboard is connected to this UART
    if (lm32_uart_tick(time, terminate, LM32_KEYBOARD_CONNECTED, LM32_UART0_CNTX))
    {
        // Timer is requesting an interrupt
        rtn_interrupt |= 1 << LM32_UART0_IRQ;
    }

    // Tick UART1 and check interrupt status
    if (lm32_uart_tick(time, terminate, LM32_KEYBOARD_NOT_CONNECTED, LM32_UART1_CNTX))
    {
        // Timer is requesting an interrupt
        rtn_interrupt |= 1 << LM32_UART1_IRQ;
    }

    // Set the next time to call.
    if (terminate)
    {
        wakeup_time_save = (time + LM32_INTERRUPT_GRANULARITY);
        *wakeup_time = LM32_EXT_TERMINATE_REQ;
    }
    else
    {
        *wakeup_time = (time + LM32_INTERRUPT_GRANULARITY);
    }

    // Send the interrupt pattern back to the MCU
    return rtn_interrupt;
}

// -------------------------------------------------------------------------
// load_binary_data()
//
// Load binary data from a file to memory 
//
// -------------------------------------------------------------------------

static int load_binary_data(const char *fname, const uint32_t address)
{
    FILE     *fp;

    // Open file for reading
    if ((fp = fopen(fname, "rb")) == NULL) 
    {
        fprintf(stderr, "***ERROR: could not open file %s for reading\n", fname);
        exit(LM32_USER_ERROR);
    }

    uint32_t byte_addr = address;

    // Load data to memory
    for (int c; (c = fgetc(fp)) != EOF;)
    {
        cpu->lm32_write_mem(byte_addr++, c & 0xff, LM32_MEM_WR_ACCESS_BYTE, true);
    }

    fclose(fp);

    // Return length
    return byte_addr - address;
}

// -------------------------------------------------------------------------
// load_string_to_mem()
//
// Load a string to memory
//
// -------------------------------------------------------------------------

static void load_string_to_mem(const char *str, const uint32_t address)
{
    unsigned len = strlen(str);
    unsigned byte_addr, idx;

    for (byte_addr = address, idx = 0; idx < len; byte_addr++, idx++)
    {
        cpu->lm32_write_mem(byte_addr, str[idx], LM32_MEM_WR_ACCESS_BYTE, true);
    }

    // Terminate string
    cpu->lm32_write_mem(byte_addr, 0, LM32_MEM_WR_ACCESS_BYTE, true);
}

// -------------------------------------------------------------------------
// load_hwsetup_to_mem()
//
// Load hardware setup date (in the defined const arrays) to memory.
//
// -------------------------------------------------------------------------

static void load_hwsetup_to_mem(const uint32_t address)
{
    uint32_t wr_addr = address;

    // Load CPU setup
    for (int idx = 0; idx < LM32_CPU_CONFIG_LEN; idx++)
    {
        cpu->lm32_write_mem(wr_addr++, cpu_config[idx], LM32_MEM_WR_ACCESS_BYTE, LM32_MEM_DISABLE_CYCLE_COUNT);
    }

    // Load DDR setup
    for (int idx = 0; idx < LM32_DDR_CONFIG_LEN; idx++)
    {
        cpu->lm32_write_mem(wr_addr++, ddr_config[idx], LM32_MEM_WR_ACCESS_BYTE, LM32_MEM_DISABLE_CYCLE_COUNT);
    }

    // Load timer0 setup
    for (int idx = 0; idx < LM32_TIM_CONFIG_LEN; idx++)
    {
        cpu->lm32_write_mem(wr_addr++, tim0_config[idx], LM32_MEM_WR_ACCESS_BYTE, LM32_MEM_DISABLE_CYCLE_COUNT);
    }

    // Load uart0 setup
    for (int idx = 0; idx < LM32_URT_CONFIG_LEN; idx++)
    {
        cpu->lm32_write_mem(wr_addr++, uart0_config[idx], LM32_MEM_WR_ACCESS_BYTE, LM32_MEM_DISABLE_CYCLE_COUNT);
    }
    
    // load uart1 setup---this UART isn't used, but the kernel crashes if UART1 is missing from the setup
    for (int idx = 0; idx < LM32_URT_CONFIG_LEN; idx++)
    {
       cpu->lm32_write_mem(wr_addr++, uart1_config[idx], LM32_MEM_WR_ACCESS_BYTE, LM32_MEM_DISABLE_CYCLE_COUNT);
    }

    // load trailer
    for (int idx = 0; idx < LM32_TRL_CONFIG_LEN; idx++)
    {
        cpu->lm32_write_mem(wr_addr++, trail_config[idx], LM32_MEM_WR_ACCESS_BYTE, LM32_MEM_DISABLE_CYCLE_COUNT);
    }
}

// -------------------------------------------------------------------------
// main()
//
// Entry point for lnxmico32. Processes command line options and configures
// the model.
//
// -------------------------------------------------------------------------
int main (int argc, char** argv)
{

    FILE*          lfp = stdout;

    // Process command line and .ini file options
    p_cfg = lm32_get_config(argc, argv, (const char *)"lnx.ini");

    // Open the logfile, if "stdout" not specified as the filename
    if (strcmp(p_cfg->log_fname, (char *)"stdout"))
    {
        if ((lfp = fopen(p_cfg->log_fname, "wb")) == NULL) 
        {
            fprintf(stderr, "***ERROR: could not open log file %s for writeing\n", p_cfg->log_fname);
            exit(LM32_USER_ERROR);
        }
    }

    // Override some CPU defaults that need to be specific to lnxmico32, and invariant
    p_cfg->mem_size         = LM32_RAM_SIZE;
    p_cfg->mem_offset       = LM32_RAM_BASE_ADDR;
    p_cfg->entry_point_addr = LM32_RAM_BASE_ADDR;

    // Generate a CPU object
    cpu = new lm32_cpu(p_cfg->verbose, 
                       p_cfg->disable_reset_break ? true : false,
                       p_cfg->disable_lock_break  ? true : false, 
                       p_cfg->disable_hw_break    ? true : false, 
                       p_cfg->disassemble_run     ? true : false, 
                       p_cfg->mem_size, 
                       p_cfg->mem_offset, 
                       p_cfg->mem_wait_states,
                       p_cfg->entry_point_addr,
                       p_cfg->cfg_word, 
                       lfp,
                       &p_cfg->dcache_cfg,
                       &p_cfg->icache_cfg,
                       p_cfg->disassemble_start);

    // Register the memory and the interrupt callback functions
    cpu->lm32_register_ext_mem_callback(ext_mem_access);
    cpu->lm32_register_int_callback(ext_interrupt);

    if (!p_cfg->gdb_run)
    {
        // Load vmlinux.bin
        int rd_length = load_binary_data(LM32_VM_LINUX_FNAME, LM32_KERNEL_BASE_ADDR);
    
        // Load romfs.ext2
        rd_length     = load_binary_data(LM32_FILE_SYS_FNAME, LM32_INIT_RD_BASE_ADDR);
    
        // Initialise memory tags *after* loading code, so as not to count instructions
        for(int idx = 0; idx < (LM32_MEM_WR_BUF_SIZE); idx++)
        {
            mem_wr[idx] = false;
        }
    
        // Put command line into memory
        load_string_to_mem(LM32_CMDLINE_STR, LM32_CMDLINE_BASE_ADDR);
    
        // Write the hardware setup values to memory
        load_hwsetup_to_mem(LM32_HWSETUP_BASE_ADDR);
    
        // Pre-charge the GP regs 1 to 4 with locations
        cpu->lm32_set_gp_reg(1, LM32_HWSETUP_BASE_ADDR);
        cpu->lm32_set_gp_reg(2, LM32_CMDLINE_BASE_ADDR);
        cpu->lm32_set_gp_reg(3, LM32_INIT_RD_BASE_ADDR);
        cpu->lm32_set_gp_reg(4, LM32_INIT_RD_BASE_ADDR + rd_length);
    
        // If enabled, and a state file exists, load state
        if (p_cfg->load_state_file)
        {
            load_system_state();
        }
    
        // Turn off key input echoing, as the running OS software will do this
        pre_run_setup();
    
        // Run the CPU.
        (void)cpu->lm32_run_program(NULL, p_cfg->num_run_instructions, p_cfg->user_break_addr, LM32_RUN_FROM_RESET, false);
    
        // Turn key input echoing back on
        post_run_setup();
    }
#if !(defined(_WIN32) || defined(_WIN64) || defined (__CYGWIN__))
    else
    {
        // Start procssing commands from GDB
        if (process_gdb(cpu))
        {
            fprintf(stderr, "***ERROR in opening PTY\n");
            return -1;
        }
    }
#endif

    // Dump registers after completion, if specified to do so
    if (p_cfg->dump_registers)
    {
        cpu->lm32_dump_registers();
    }

    // Dump the number of executed instructions
    if (p_cfg->dump_num_exec_instr)
    {
        uint64_t instr_count = cpu->lm32_get_num_instructions();

        fprintf(lfp, "\nNumber of executed instructions = %.1f million (%.1f MIPS)\n",  
	              (float)instr_count/1e6, (float)instr_count/tv_diff);
    }

    // Dump RAM, if specified to do so and within range
    if (p_cfg->ram_dump_addr >= (int)p_cfg->mem_offset && p_cfg->ram_dump_addr < (int)(p_cfg->mem_offset + p_cfg->mem_size))
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
                if ((p_cfg->ram_dump_addr+idx) < (int)(p_cfg->mem_offset + p_cfg->mem_size))
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

    // If enabled, save the system state
    if (p_cfg->save_state_file)
    {
        save_system_state();
    }

    return 0;
}
