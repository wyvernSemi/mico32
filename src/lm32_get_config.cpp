//=============================================================
// 
// Copyright (c) 2013-2017 Simon Southwell. All rights reserved.
//
// Date: 16th July 2013
//
// The file contains functions for setting the configuration
// of the lm32 ISS, as called by cpumico32.
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
// $Id: lm32_get_config.cpp,v 3.10 2017/04/20 09:01:29 simon Exp $
// $Source: /home/simon/CVS/src/cpu/mico32/src/lm32_get_config.cpp,v $
//
//=============================================================

// -------------------------------------------------------------------------
// INCLUDES
// -------------------------------------------------------------------------

#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#if !(defined _WIN32) && !(defined _WIN64)
#include <unistd.h>
#else 
extern "C" {
extern int getopt(int nargc, char** nargv, char* ostr);
extern char* optarg;
extern int optind;
}
#endif

#include "lm32_cpu_hdr.h"

// -------------------------------------------------------------------------
// DEFINES
// -------------------------------------------------------------------------

// Define the getopt sub-strings for the different groups of arguments
#define LM32_COMMON_ARGS               "hl:r:R:DIc:i:"
#define LM32_CPUMICO32_ARGS            "f:m:o:e:T"
#define LM32_LNXMICO32_ARGS            "s:SL"
#define LM32_NON_FAST_ARGS             "n:vxb:dw:"
#define LM32_LNX_NON_FAST_ARGS         "V:"
#define LM32_DBG_ARGS                  "gtG:"

// Construct the getopt arguments specification string based on the compile options
# ifdef LNXMICO32
#  ifdef LM32_FAST_COMPILE
#   define LM32_GETOPT_ARG_STR LM32_COMMON_ARGS LM32_LNXMICO32_ARGS
#  else
#   define LM32_GETOPT_ARG_STR LM32_COMMON_ARGS LM32_LNXMICO32_ARGS LM32_NON_FAST_ARGS LM32_DBG_ARGS LM32_LNX_NON_FAST_ARGS 
#  endif
# else
#  ifdef LM32_FAST_COMPILE
#   define LM32_GETOPT_ARG_STR LM32_COMMON_ARGS LM32_CPUMICO32_ARGS
#  else
#   define LM32_GETOPT_ARG_STR LM32_COMMON_ARGS LM32_CPUMICO32_ARGS LM32_NON_FAST_ARGS LM32_DBG_ARGS
#  endif
#endif

#define MAX_SECT_STR_SIZE  50
#define MAX_ENTRY_STR_SIZE 50
#define MAX_VALUE_STR_SIZE 50
#define MAXLINESIZE        150
#define MAX_CFG_ENTRIES    50

#define LM32_SAVE_FILE_NAME "lnxmico32.sav"

#define LM32_CFG_INI_PARAM_WARNING   {fprintf(stderr, "Warning: unrecognised parameter %s in section %s of INI file.\n", \
                                                    cfg_entries[cdx].entry, cfg_entries[cdx].section);}

#define LM32_CFG_INI_SECTION_WARNING {fprintf(stderr, "Warning: unrecognised section %s in INI file.\n", \
                                                    cfg_entries[cdx].section);}

// -------------------------------------------------------------------------
// TYPEDEFS
// -------------------------------------------------------------------------

typedef struct {
    char section [MAX_SECT_STR_SIZE];
    char entry   [MAX_ENTRY_STR_SIZE];
    char value   [MAX_VALUE_STR_SIZE];
} ini_entry_t;

// -------------------------------------------------------------------------
// LOCAL STATICS
// -------------------------------------------------------------------------

static ini_entry_t   cfg_entries[MAX_CFG_ENTRIES];
static int           ini_entry_idx = 0;
static lm32_config_t lm32_cpu_cfg;

// -------------------------------------------------------------------------
// lm32_parse_ini_file()
//
// Function to parse an INI file to populate the cfg_entries array with
// parameter entries for configuring cpumico32
//
// -------------------------------------------------------------------------

static void lm32_parse_ini_file(const char *ini_fname, ini_entry_t* p_cfg_entries)
{
    FILE *ini_fp;
    int c;
    int idx, wdx;

    char line[MAXLINESIZE];
    char curr_section[MAX_SECT_STR_SIZE];

    // Make sure this buffer is initialised with something, to prevent warnings
    curr_section[0] = '\0';

    // Open ini file for reading, if one specified
    if (!strcmp(ini_fname, ""))
    {
        return;
    }
    else if ((ini_fp = fopen(ini_fname, "rb")) == NULL)
    {
        fprintf(stderr, "***ERROR: could not open file %s for reading\n", ini_fname);
        exit(LM32_USER_ERROR);
    }

    while (fgets(line, MAXLINESIZE, ini_fp) != NULL)
    {    
        // Strip white space
        for (idx=0,wdx=0; (c = line[idx]) != 0; idx++)
        {
            if (!isspace(c))
            {
                line[wdx++] = line[idx];
            }
        }
        line[wdx] = 0;

        // Strip any trailing comments
        for (idx = 0; idx < wdx; idx++)
        {
           if (line[idx] == ';')
           {
               break;
           }
        }
        line[idx] = 0;

        // If section, make current section
        if (line[0] == '[')
        {
            line[strlen(line)-1] = 0;
            strncpy(curr_section, &line[1], MAX_SECT_STR_SIZE);

        // else if entry, extract name and value and create an entry
        } 
        else if (isalpha(line[0]))
        {
#           pragma warning(suppress: 6053) // This suppresses a warning about no string terminator, but strncpy guarantees a terminated string
            strncpy(p_cfg_entries[ini_entry_idx].section, curr_section, MAX_SECT_STR_SIZE);

            // Find the equals sign
            for(idx = 0; idx < MAXLINESIZE; idx++)
            {
                if (line[idx] == '=')
                {
                    break;
                }
            }

            line[idx] = '\0';

            strncpy(p_cfg_entries[ini_entry_idx].entry, line, MAX_ENTRY_STR_SIZE);
            strncpy(p_cfg_entries[ini_entry_idx].value, &line[idx+1], MAX_VALUE_STR_SIZE);

            ini_entry_idx++;
        }
    }
}

// -------------------------------------------------------------------------
// lm32_get_config()
//
// Function that returns the configuration (in a lm32_config_t structure 
// pointer) values for configuring the lm32 ISS. It sets the config 
// structure to default values, before reading a specified .ini file
// and updating the default values with any valid entries found.
//
// -------------------------------------------------------------------------

extern "C" lm32_config_t* lm32_get_config(int argc, char** argv, const char* default_ini_fname)
{
    int    cdx;
    int    option;
    const char* ini_fname = default_ini_fname;

    // Set some defaults
    lm32_cpu_cfg.filename                        = (char*)LM32_DEFAULT_FNAME;
    lm32_cpu_cfg.log_fname                       = (char*)"stdout";
    lm32_cpu_cfg.entry_point_addr                = 0;
    lm32_cpu_cfg.test_mode                       = 0;
    lm32_cpu_cfg.verbose                         = LM32_VERBOSITY_LVL_OFF;
    lm32_cpu_cfg.ram_dump_addr                   = -1;
    lm32_cpu_cfg.ram_dump_bytes                  = 0;
    lm32_cpu_cfg.dump_registers                  = 0;
    lm32_cpu_cfg.dump_num_exec_instr             = 0;
    lm32_cpu_cfg.disassemble_run                 = 0;
    lm32_cpu_cfg.user_break_addr                 = -1;
    lm32_cpu_cfg.num_run_instructions            = LM32_FOREVER;
    lm32_cpu_cfg.disable_reset_break             = 0;
    lm32_cpu_cfg.disable_hw_break                = 0;
    lm32_cpu_cfg.disable_int_break               = 1;
    lm32_cpu_cfg.disable_lock_break              = 0;
    lm32_cpu_cfg.mem_size                        = LM32_DEFAULT_MEM_SIZE;
    lm32_cpu_cfg.mem_offset                      = 0; 
    lm32_cpu_cfg.mem_wait_states                 = 0;
    lm32_cpu_cfg.disassemble_start               = 0;
    lm32_cpu_cfg.cfg_word                        = LM32_DEFAULT_CONFIG;
    lm32_cpu_cfg.save_fname                      = (char*)LM32_SAVE_FILE_NAME;
    lm32_cpu_cfg.save_state_file                 = false;
    lm32_cpu_cfg.load_state_file                 = false;
    lm32_cpu_cfg.gdb_run                         = false;
#if !(defined _WIN32) && !(defined _WIN64)    
    lm32_cpu_cfg.com_port_num                    = LM32_DEFAULT_TCP_PORT;
#else
    lm32_cpu_cfg.com_port_num                    = LM32_DEFAULT_COM_PORT;
#endif    
    lm32_cpu_cfg.use_tcp_skt                     = false;

    lm32_cpu_cfg.dcache_cfg.cache_base_addr      = LM32_CACHE_DEFAULT_BASE;
    lm32_cpu_cfg.dcache_cfg.cache_limit          = LM32_CACHE_DEFAULT_DLIMIT;
    lm32_cpu_cfg.dcache_cfg.cache_num_sets       = LM32_CACHE_DEFAULT_SETS;
    lm32_cpu_cfg.dcache_cfg.cache_num_ways       = LM32_CACHE_DEFAULT_WAYS;
    lm32_cpu_cfg.dcache_cfg.cache_bytes_per_line = LM32_CACHE_DEFAULT_LINE;

    lm32_cpu_cfg.icache_cfg.cache_base_addr      = LM32_CACHE_DEFAULT_BASE;
    lm32_cpu_cfg.icache_cfg.cache_limit          = LM32_CACHE_DEFAULT_ILIMIT;
    lm32_cpu_cfg.icache_cfg.cache_num_sets       = LM32_CACHE_DEFAULT_SETS;
    lm32_cpu_cfg.icache_cfg.cache_num_ways       = LM32_CACHE_DEFAULT_WAYS;
    lm32_cpu_cfg.icache_cfg.cache_bytes_per_line = LM32_CACHE_DEFAULT_LINE;


    // Process the command line options *only* for the INI filename, as we
    // want the command line options to override the INI options
    while ((option = getopt(argc, argv, LM32_GETOPT_ARG_STR)) != EOF)
    {
        switch(option)
        {
        case 'i':
            ini_fname = optarg;
            break;

        case 'h':
        case '?':
            fprintf(stderr,
                    "Usage: %s [-h] "
#ifndef LM32_FAST_COMPILE
                    "[-g] [-t] [-G <port #>] [-v] [-x] [-d] "
#endif
                    "[-D] [-I] "
                    "\n         "
#ifndef LM32_FAST_COMPILE
                    "[-n <num>] [-b <addr>] "
#endif
                    "[-r <addr>]"
#ifndef LNXMICO32
                    "\n        "
#endif
                    " [-R <num>] "
#ifndef LNXMICO32
                    " [-f <filename>] [-m <num>] [-o < addr>] [-e <addr>]"
#endif
                    "\n"
                    "         [-l <filename>] [-c <num>] "
#ifndef LM32_FAST_COMPILE
                    "[-w <wait states>] "
#endif
                    "[-i <filename>]"
#ifndef LNXMICO32
                    " [-T]"
#else
# ifndef LM32_FAST_COMPILE
                    " [-V <num>]"
# endif
                    " [-s <filename>] [-S] [-L]"
#endif
                    "\n\n"
                    "    -h Display this help message\n"
#ifndef LM32_FAST_COMPILE
                    "    -g Start up in GDB remote debug mode (default: off)\n"
                    "    -t Specify TCP socket connection for GDB remote debug (default: COM/pty connection)\n"
                    "    -G Specify TCP "
#if (defined _WIN32) || (defined _WIN64)
                    "COM "
#endif                    
                    "port to use for GDB remote debug (default: %d)\n"
                    "    -n Specify number of instructions to run (default: run forever)\n"
                    "    -b Specify address for breakpoint (default: none)\n"
#endif
#ifndef LNXMICO32
                    "    -f Specify executable ELF file (default: %s)\n"
#endif
                    "    -l Specify log file output (default: stdout)\n"
#ifndef LNXMICO32
                    "    -m Specify size of internal memory in bytes (default: %d)\n"
                    "    -o Internal memory offset (default 0x00000000)\n"
                    "    -e specify an entry point address (default 0x00000000)\n"
#endif
#ifndef LM32_FAST_COMPILE
                    "    -v Specify verbose output (default: off)\n"
                    "    -x Enable disassemble mode (default: disabled)\n"
                    "    -d Disable breaking on lock condition (default: enabled)\n"
#endif
                    "    -r Address to dump value from internal ram after completion (default: no dump)\n"
                    "    -R Number of bytes to dump from RAM if -r specified (default 4)\n"
                    "    -D Dump registers after completion (default: no dump)\n"
                    "    -I Dump number of instructions executed (default: no dump)\n"
                    "    -c Set configuration word value to enable/disable features\n"
#ifndef LM32_FAST_COMPILE
                    "    -w Set the number of wait states for internal memory (default 0)\n"
#endif
                    "    -i Specify a .ini filename to use for configuration (default none)\n"
#ifndef LNXMICO32
                    "    -T Enable internal callback functions for test (default disabled)\n"
#else
# ifndef LM32_FAST_COMPILE
                    "    -V Enable verbose output from cycle specified (default disabled)\n"
# endif
                    "    -s Specify .sav filename (default lnxmico32.sav)\n"
                    "    -S Save state on exit (default no save)\n"
                    "    -L Load saved state before execution (default no load)\n"
#endif
                    "\n"
                    , argv[0]
#ifndef LM32_FAST_COMPILE
#if !(defined _WIN32) && !(defined _WIN64)
                    , LM32_DEFAULT_TCP_PORT
#else                    
                    , LM32_DEFAULT_COM_PORT
#endif                    
#endif                    
#ifndef LNXMICO32
                    , LM32_DEFAULT_FNAME, LM32_DEFAULT_MEM_SIZE
#endif
                        );
            exit(LM32_NO_ERROR);
        }
    }

    // Parse the INI file, and populate the config table
    lm32_parse_ini_file(ini_fname, cfg_entries);

    // Run through the structure setting entries
    for (cdx = 0; cdx < ini_entry_idx; cdx++)
    {
#ifndef LNXMICO32
        if (!strcmp(cfg_entries[cdx].section, (char*)"program"))
        {
            if (!strcmp(cfg_entries[cdx].entry, (char*)"filename"))
            {
                lm32_cpu_cfg.filename = cfg_entries[cdx].value;
            }

            else if (!strcmp(cfg_entries[cdx].entry,(char*)"entry_point_addr"))
            {
                lm32_cpu_cfg.entry_point_addr = strtol(cfg_entries[cdx].value, NULL, 0);
            }
            else
            {
                LM32_CFG_INI_PARAM_WARNING;
            }
        }
        else
#endif
        if (!strcmp(cfg_entries[cdx].section, (char*)"configuration"))
        {
            if (!strcmp(cfg_entries[cdx].entry, (char*)"cfg_word"))
            {
                lm32_cpu_cfg.cfg_word = (uint32_t)strtol(cfg_entries[cdx].value, NULL, 0);
            }
            else
            {
                LM32_CFG_INI_PARAM_WARNING;
            }
        }
        else if (!strcmp(cfg_entries[cdx].section, (char*)"debug"))
        {
            if (!strcmp(cfg_entries[cdx].entry, (char*)"log_fname"))
            {
                lm32_cpu_cfg.log_fname = cfg_entries[cdx].value;
            }
#ifndef LNXMICO32
            else if (!strcmp(cfg_entries[cdx].entry, (char*)"test_mode"))
            {
                lm32_cpu_cfg.test_mode = (!strcmp(cfg_entries[cdx].value, "true")) ? 1 : 0;
            }
#endif
#ifndef LM32_FAST_COMPILE
            else if (!strcmp(cfg_entries[cdx].entry, (char*)"verbose"))
            {
                lm32_cpu_cfg.verbose = (!strcmp(cfg_entries[cdx].value, "true")) ? 1 : 0;
            }
#endif
            else if (!strcmp(cfg_entries[cdx].entry, (char*)"ram_dump_addr"))
            {
                lm32_cpu_cfg.ram_dump_addr = (int32_t)strtol(cfg_entries[cdx].value, NULL, 0);
            }
            else if (!strcmp(cfg_entries[cdx].entry, (char*)"ram_dump_bytes"))
            {
                lm32_cpu_cfg.ram_dump_bytes = (int)strtol(cfg_entries[cdx].value, NULL, 0);
            }
            else if (!strcmp(cfg_entries[cdx].entry, (char*)"dump_registers"))
            {
                lm32_cpu_cfg.dump_registers = (!strcmp(cfg_entries[cdx].value, "true")) ? 1 : 0;
            }
            else if (!strcmp(cfg_entries[cdx].entry, (char*)"dump_num_exec_instr"))
            {
                lm32_cpu_cfg.dump_num_exec_instr = (!strcmp(cfg_entries[cdx].value, "true")) ? 1 : 0;
            }
#ifndef LM32_FAST_COMPILE
            else if (!strcmp(cfg_entries[cdx].entry, (char*)"disassemble_run"))
            {
                lm32_cpu_cfg.disassemble_run = (!strcmp(cfg_entries[cdx].value, "true")) ? 1 : 0;
            }
#endif
            else
            {
                LM32_CFG_INI_PARAM_WARNING;
            }
        }
#ifndef LM32_FAST_COMPILE
        else if (!strcmp(cfg_entries[cdx].section, (char*)"breakpoints"))
        {
            if (!strcmp(cfg_entries[cdx].entry, (char*)"user_break_addr"))
            {
                lm32_cpu_cfg.user_break_addr = (int32_t)strtol(cfg_entries[cdx].value, NULL, 0);
            }
            else if (!strcmp(cfg_entries[cdx].entry, (char*)"num_run_instructions"))
            {
                lm32_cpu_cfg.num_run_instructions = (int32_t)strtol(cfg_entries[cdx].value, NULL, 0);
            }
            else if (!strcmp(cfg_entries[cdx].entry, (char*)"disable_reset_break"))
            {
                lm32_cpu_cfg.disable_reset_break = (!strcmp(cfg_entries[cdx].value, "true")) ? 1 : 0;
            }
            else if (!strcmp(cfg_entries[cdx].entry, (char*)"disable_hw_break"))
            {
                lm32_cpu_cfg.disable_hw_break = (!strcmp(cfg_entries[cdx].value, "true")) ? 1 : 0;
            }
            else if (!strcmp(cfg_entries[cdx].entry, (char*)"disable_lock_break"))
            {
                lm32_cpu_cfg.disable_lock_break = (!strcmp(cfg_entries[cdx].value, "true")) ? 1 : 0;
            }
            else
            {
                LM32_CFG_INI_PARAM_WARNING;
            }
        }
#endif
        else if (!strcmp(cfg_entries[cdx].section, (char*)"memory"))
        {
#ifndef LM32_FAST_COMPILE
            if (!strcmp(cfg_entries[cdx].entry, (char*)"mem_wait_states"))
            {
                lm32_cpu_cfg.mem_wait_states = (uint32_t)strtol(cfg_entries[cdx].value, NULL, 0);
            }
            else
#endif
#ifndef LNXMICO32
            if (!strcmp(cfg_entries[cdx].entry, (char*)"mem_size"))
            {
                lm32_cpu_cfg.mem_size = (uint32_t)strtol(cfg_entries[cdx].value, NULL, 0);
            }
            else if (!strcmp(cfg_entries[cdx].entry, (char*)"mem_offset"))
            {
                lm32_cpu_cfg.mem_offset = (uint32_t)strtol(cfg_entries[cdx].value, NULL, 0);
            }
            else
#endif
            {
                LM32_CFG_INI_PARAM_WARNING;
            }
        } 
#ifndef LM32_FAST_COMPILE
        else if (!strcmp(cfg_entries[cdx].section, (char*)"dcache"))
        {
            if (!strcmp(cfg_entries[cdx].entry, (char*)"cache_base_addr"))
            {
                lm32_cpu_cfg.dcache_cfg.cache_base_addr = (uint32_t)strtol(cfg_entries[cdx].value, NULL, 0);
            }
            else if (!strcmp(cfg_entries[cdx].entry, (char*)"cache_limit"))
            {
                lm32_cpu_cfg.dcache_cfg.cache_limit = (uint32_t)strtol(cfg_entries[cdx].value, NULL, 0);
            }
            else if (!strcmp(cfg_entries[cdx].entry, (char*)"cache_num_sets"))
            {
                lm32_cpu_cfg.dcache_cfg.cache_num_sets = strtol(cfg_entries[cdx].value, NULL, 0);
            }
            else if (!strcmp(cfg_entries[cdx].entry, (char*)"cache_num_ways"))
            {
                lm32_cpu_cfg.dcache_cfg.cache_num_ways = strtol(cfg_entries[cdx].value, NULL, 0);
            }
            else if (!strcmp(cfg_entries[cdx].entry, (char*)"cache_bytes_per_line"))
            {
                lm32_cpu_cfg.dcache_cfg.cache_bytes_per_line = strtol(cfg_entries[cdx].value, NULL, 0);
            }
            else
            {
                LM32_CFG_INI_PARAM_WARNING;
            }

        } 
        else if (!strcmp(cfg_entries[cdx].section, (char*)"icache"))
        {
            if (!strcmp(cfg_entries[cdx].entry, (char*)"cache_base_addr"))
            {
                lm32_cpu_cfg.icache_cfg.cache_base_addr = (uint32_t)strtol(cfg_entries[cdx].value, NULL, 0);
            }
            else if (!strcmp(cfg_entries[cdx].entry, (char*)"cache_limit"))
            {
                lm32_cpu_cfg.icache_cfg.cache_limit = (uint32_t)strtol(cfg_entries[cdx].value, NULL, 0);
            }
            else if (!strcmp(cfg_entries[cdx].entry, (char*)"cache_num_sets"))
            {
                lm32_cpu_cfg.icache_cfg.cache_num_sets = strtol(cfg_entries[cdx].value, NULL, 0);
            }
            else if (!strcmp(cfg_entries[cdx].entry, (char*)"cache_num_ways"))
            {
                lm32_cpu_cfg.icache_cfg.cache_num_ways = strtol(cfg_entries[cdx].value, NULL, 0);
            }
            else if (!strcmp(cfg_entries[cdx].entry, (char*)"cache_bytes_per_line"))
            {
                lm32_cpu_cfg.icache_cfg.cache_bytes_per_line = strtol(cfg_entries[cdx].value, NULL, 0);
            }
            else
            {
                LM32_CFG_INI_PARAM_WARNING;
            }
        }
#endif
#ifdef LNXMICO32
        else if (!strcmp(cfg_entries[cdx].section, (char*)"state"))
        {
            if (!strcmp(cfg_entries[cdx].entry, (char*)"save_file_name"))
            {
                lm32_cpu_cfg.save_fname = cfg_entries[cdx].value;
            }
            else if (!strcmp(cfg_entries[cdx].entry, (char*)"load_state"))
            {
                lm32_cpu_cfg.load_state_file = (!strcmp(cfg_entries[cdx].value, "true")) ? 1 : 0;
            }
            else if (!strcmp(cfg_entries[cdx].entry, (char*)"save_state"))
            {
                lm32_cpu_cfg.save_state_file = (!strcmp(cfg_entries[cdx].value, "true")) ? 1 : 0;
            }
            else
            {
                LM32_CFG_INI_PARAM_WARNING;
            }
        }
#endif
        else
        {
            LM32_CFG_INI_SECTION_WARNING;
        }
    }

    // Reset the arguments to the beginning
    optind = 1;

    // Process the command line options into a temporary configuration
    while ((option = getopt(argc, argv, LM32_GETOPT_ARG_STR)) != EOF)
    {
        switch(option) 
        {
        // Do nothing for the INI file specification, as already processed
        case 'i':
            break;

#ifndef LNXMICO32
        case 'f':
            lm32_cpu_cfg.filename = optarg;
            break;
#endif
        case 'I':
            lm32_cpu_cfg.dump_num_exec_instr = 1;
            break;

        case 'D':
            lm32_cpu_cfg.dump_registers = 1;
            break;

#ifndef LM32_FAST_COMPILE
        case 'g':
            lm32_cpu_cfg.gdb_run = true;
            break;

        case 't':
            lm32_cpu_cfg.use_tcp_skt = true;
            lm32_cpu_cfg.gdb_run = true;
            break;

        // In windows, need a means to specify COM port to use, as not created by the program
        // (like a pseudo terminal in Linux), or the TCP socket port number, if -t selected.
        case 'G':
            lm32_cpu_cfg.gdb_run = true;
            lm32_cpu_cfg.com_port_num = strtol(optarg, NULL, 0);
            break;

        case 'n':
            lm32_cpu_cfg.num_run_instructions = strtol(optarg, NULL, 0);
            if (lm32_cpu_cfg.num_run_instructions < LM32_FOREVER)
                lm32_cpu_cfg.num_run_instructions = LM32_FOREVER;
            break;

        case 'b':
            lm32_cpu_cfg.user_break_addr = (uint32_t)strtol(optarg, NULL, 0);
            break;

        case 'x':
            lm32_cpu_cfg.disassemble_run = 1;
            break;

        case 'd':
            lm32_cpu_cfg.disable_lock_break = 1;
            break;

        case 'w':
            lm32_cpu_cfg.mem_wait_states = (int)strtol(optarg, NULL, 0);
            break;

        case 'v':
            lm32_cpu_cfg.verbose = 1;
            break;
#endif
        case 'r':
            // Silently word align the user specified dump address
            lm32_cpu_cfg.ram_dump_addr = (uint32_t)strtol(optarg, NULL, 0) & 0xfffffffc;
            break;

        case 'R':
            lm32_cpu_cfg.ram_dump_bytes = (uint32_t)strtol(optarg, NULL, 0);

            // Round up number of bytes to be word aligned
            if (lm32_cpu_cfg.ram_dump_bytes & 0x3)
                lm32_cpu_cfg.ram_dump_bytes += 4 - (lm32_cpu_cfg.ram_dump_bytes & 0x3);

            break;

        case 'l':
            lm32_cpu_cfg.log_fname = optarg;
            break;

        case 'c':
            lm32_cpu_cfg.cfg_word = strtol(optarg, NULL, 0);
            break;

        case 'T':
            lm32_cpu_cfg.test_mode = 1;
            break;

#ifndef LNXMICO32              
        case 'm':
            lm32_cpu_cfg.mem_size = strtol(optarg, NULL, 0);
            break;

        case 'o':
            lm32_cpu_cfg.mem_offset = strtol(optarg, NULL, 0);
            break;

        case 'e':
            // Silently word align the user specified entry point address
            lm32_cpu_cfg.entry_point_addr = (uint32_t)strtol(optarg, NULL, 0) & 0xfffffffc;
            break;
#else                    
        case 's':
            lm32_cpu_cfg.save_fname = optarg;
            break;
        case 'S':
            lm32_cpu_cfg.save_state_file = true;
            break;
        case 'L':
            lm32_cpu_cfg.load_state_file = true;
            break;
# ifndef LM32_FAST_COMPILE
        case 'V':
            lm32_cpu_cfg.disassemble_start = (int)strtol(optarg, NULL, 0);
            lm32_cpu_cfg.verbose = 1;
            break;
# endif
#endif
        }
    }

    // Return a pointer the updated configuration structure
    return &lm32_cpu_cfg;
}

#ifdef __TEST

int main (int argc, char** argv)
{
    lm32_config_t* p_cfg;

    p_cfg = lm32_get_config(argc, argv);

    return 0;
}

#endif


