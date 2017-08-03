//=============================================================
// 
// Copyright (c) 2017 Simon Southwell. All rights reserved.
//
// Date: 21st March 2017
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
// $Id: lm32_gdb.cpp,v 3.10 2017/07/31 14:02:45 simon Exp $
// $Source: /home/simon/CVS/src/cpu/mico32/src/lm32_gdb.cpp,v $
//
//=============================================================

// -------------------------------------------------------------------------
// INCLUDES
// -------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>

// For Windows, need to link with Ws2_32.lib
#if defined (_WIN32) || defined (_WIN64)
// -------------------------------------------------------------------------
// INCLUDES (windows)
// -------------------------------------------------------------------------

# undef   UNICODE
# define  WIN32_LEAN_AND_MEAN
  
# include <windows.h>
# include <winsock2.h>
# include <ws2tcpip.h>
#else
// -------------------------------------------------------------------------
// INCLUDES (Linux)
// -------------------------------------------------------------------------
 
# include <string.h>
# include <unistd.h>
# include <sys/types.h> 
# include <sys/socket.h>
# include <netinet/in.h>
# include <termios.h>
#endif

#include "lm32_gdb.h"
#include "lm32_cpu.h"

// -------------------------------------------------------------------------
// LOCAL CONSTANTS
// -------------------------------------------------------------------------

static char ack_char       = GDB_ACK_CHAR;
static char hexchars[]     = HEX_CHAR_MAP;

// -------------------------------------------------------------------------
// STATIC VARIABLES
// -------------------------------------------------------------------------

static char ip_buf[IP_BUFFER_SIZE];
static char op_buf[OP_BUFFER_SIZE];

// -------------------------------------------------------------------------
// lm32gdb_skt_init()
//
// Does any required socket initialisation, prior to opening a TCP socket.
// Current, only windows requires any handling.
//
// -------------------------------------------------------------------------

inline static int lm32gdb_skt_init(void)
{
#if defined (_WIN32) || defined (_WIN64)     
    WSADATA wsaData; 
    
    // Initialize Winsock (windows only). Use windows socket spec. verions up to 2.2.
    if (int status = WSAStartup(MAKEWORD(VER_MAJOR, VER_MINOR), &wsaData))
    {
        fprintf(stderr, "WSAStartup failed with error: %d\n", status);
        return LM32GDB_ERR;
    }
#endif

    return LM32GDB_OK;
}

// -------------------------------------------------------------------------
// lm32_skt_cleanup()
//
// Does any open TCP socket cleanup before exiting the program. Current, 
// only windows requires any handling.
//
// -------------------------------------------------------------------------

inline static void lm32gdb_skt_cleanup(void)
{
#if defined (_WIN32) || defined (_WIN64)     
    WSACleanup();
#endif    
}

// -------------------------------------------------------------------------
// lm32gdb_read()
//
// Read a byte from the PTY (fd) and place in the buffer (buf). Return true
// on successful read, else return false. Compile dependent for windows
// (ReadFile) and linux (read)
//
// -------------------------------------------------------------------------

static inline bool lm32gdb_read (void* fdin, char* buf, bool tcp_connection)
{
    int status = LM32GDB_OK;
    long fd = (long)fdin;

    if (tcp_connection)
    {
        

        // Read from the connection (up to 255 bytes plus null termination).
        if (recv((lm32gdb_skt_t)fd, buf, 1, 0) < 0)
        {
            fprintf(stderr, "ERROR reading from socket\n");
            lm32gdb_skt_cleanup();
            status = LM32GDB_ERR;
        }
    }
    else
    {

#if !(defined(_WIN32) || defined(_WIN64))
        if (read(fd, buf, 1) != 1)
        {
            status = LM32GDB_ERR;
        }
#else
        DWORD n;

        if (!ReadFile((PTY_HDL)fd, buf, 1, &n, NULL))
        {
            fprintf(stderr, "ReadFile returned an error %d\n", GetLastError());
            status = LM32GDB_ERR;
        }
#endif        
    }


    return status == LM32GDB_OK; 
}

// -------------------------------------------------------------------------
// lm32gdb_write()
//
// Write a byte to the PTY (fd) from the buffer (buf). Return true
// on successful read, else return false. Compile dependent for windows
// (WriteFile) and linux (write)
//
// -------------------------------------------------------------------------

static inline bool lm32gdb_write (void* fd, char* buf, bool tcp_connection)
{
    int status = LM32GDB_OK;

    if (tcp_connection)
    {
        if (send((lm32gdb_skt_t)fd, buf, 1, 0) < 0) 
        {
            fprintf(stderr, "ERROR writing to socket\n");
            status = LM32GDB_ERR;
        }
    }
    else
    {
#if !(defined(_WIN32) || defined(_WIN64))
        int n;
        if (write((PTY_HDL)fd, buf, 1) != 1)
        {
            status = LM32GDB_ERR;
        }
#else
        DWORD n;
        if (!WriteFile((PTY_HDL)fd, buf, 1, &n, NULL))
        {
            fprintf(stderr, "WriteFile returned an error %d\n", GetLastError());
            status = LM32GDB_ERR;
        }
#endif
    }

    return status == LM32GDB_OK;
}

// -------------------------------------------------------------------------
// lm32gdb_gen_register_reply()
//
// Generate a register reply to a GDB command into buffer (buf). The format
// generated is either a stop reply type (for commands '?', 'c' or 's')
//
//   "T AA n1:r1;n2:r2;..."
//
// or an orders set replay (for command 'p')
//
//   "r1r2r3...."
//
// The checksum for the generated characters is calculated and returned in
// checksum.
//
// -------------------------------------------------------------------------

static int lm32gdb_gen_register_reply(lm32_cpu* cpu, const char* cmd, char *buf, unsigned char &checksum, const int sigval = SIGHUP)
{
    int      bdx    = 0;
    int      cdx    = 1;
    int      regnum;
    unsigned val;

    bool single_reg = cmd[0] == 'p';
    bool stop_reply = cmd[0] == '?' || cmd[0] == 'c' || cmd[0] == 's';

    // Retrieve the current CPU state
    lm32_cpu::lm32_state cpu_state = cpu->lm32_get_cpu_state();

    // If retrieving a single register, get register number and skip the '=' character
    if (single_reg)
    {
        regnum  = CHAR2NIB(cmd[cdx]) << 4; cdx++;
        regnum |= CHAR2NIB(cmd[cdx]);      cdx++;

        // Skip '=' character
        cdx++;
    }

    // If a 'Stop' reply (e.g. from '?' command), format is "T AA n1:r1;n2:r2;...",
    // where AA is the signal value, nx is the register number, and rx is the register's 
    // value. Otherwise it is just r1r2...
    if (stop_reply)
    {
        buf[bdx++] = 'T';
        BUFBYTE(buf, bdx, sigval);
    }

    // Run through all the registers...
    for (int idx = 0; idx < NUM_REGS; idx++)
    {
        if (idx < LM32_NUM_OF_REGISTERS)
        {
            val = cpu_state.r[idx];
        }
        else
        {
            switch (idx)
            {
               case LM32_REG_PC  : val = cpu_state.pc;   break;
               case LM32_REG_EID : val = sigval;         break;
               case LM32_REG_EBA : val = cpu_state.eba;  break;
               case LM32_REG_DEBA: val = cpu_state.deba; break;
               case LM32_REG_IE  : val = cpu_state.ie;   break;
               case LM32_REG_IM  : val = cpu_state.im;   break;
               case LM32_REG_IP  : val = cpu_state.ip;   break;
            }
        }

        if (stop_reply)
        {
            // Add the register number as a 2 character hex value
            BUFBYTE(buf, bdx, idx);
            buf[bdx++] = ':';
        }
        else if (single_reg)
        {
            if (idx != regnum)
            {
                continue;
            }
        }

        // Add the register 32 bit word value as hex characters 
        BUFWORD(buf, bdx, val);

        if (stop_reply)
        {
            buf[bdx++] = ';';
        }
    }

    // Put a null terminating character at the end (in case we wish to 
    // print the string for debug purposes)
    buf[bdx] = 0;

    // Calculate the checksum
    for (int idx = 0; idx < bdx; idx++)
    {
        checksum += buf[idx];
    }

    // Return number of characters placed in the buffer
    return bdx;
}

// -------------------------------------------------------------------------
// lm32gdb_set_registers()
//
// Sets one or more of the CPU's registers based on the command (cmd). If
// the command is 'P', a register number is extracted, and that register is 
// updated with the command data. Otherwise the command data is treated as
// an ordered list ("r1r2r3...."),
//
// -------------------------------------------------------------------------

static int lm32gdb_set_regs (lm32_cpu* cpu, const char* cmd, const int cmdlen, char* buf, unsigned char &checksum)
{
    int bdx        = 0;
    int cdx        = 1;
    int start_reg  = 0;
    int end_reg    = NUM_REGS;

    // Retrieve the current CPU state
    lm32_cpu::lm32_state cpu_state = cpu->lm32_get_cpu_state();

    // If accessing a single register, get the register number and set 
    // the loop for just this register
    if (cmd[0] == 'P')
    {
        start_reg  = CHAR2NIB(cmd[cdx]) << 4; cdx++;
        start_reg |= CHAR2NIB(cmd[cdx]);      cdx++;

        end_reg = start_reg;

        // Skip '=' character
        cdx++;
    }

    // Run through the registers in order until all done, or command 
    // buffer run out of characters
    for (int rdx = start_reg; rdx < end_reg && cdx < cmdlen; rdx++)
    {
        int val = 0;

        // Convert the 8 character (for 32 bits) hex nibbles to a number
        for (int cdx = 0; cdx < 4*2; cdx++)
        {
            val  <<= 4;
            val |= CHAR2NIB(cmd[cdx]); cdx++;
        }

        // Update the general purpose registers in the retrieved state structure
        if (rdx < LM32_NUM_OF_REGISTERS)
        {
            cpu_state.r[rdx] = val;
        }
        // Update the special registers n the retrieved state structure
        else
        {
            switch (rdx)
            {
               case LM32_REG_PC  : cpu_state.pc   = val; break;
               case LM32_REG_EBA : cpu_state.eba  = val; break;
               case LM32_REG_DEBA: cpu_state.deba = val; break;
               case LM32_REG_IE  : cpu_state.ie   = val; break;
               case LM32_REG_IM  : cpu_state.im   = val; break;
               case LM32_REG_IP  : cpu_state.ip   = val; break;
            }
        }
    }

    // Write back the updated CPU state
    cpu->lm32_set_cpu_state(cpu_state);

    // Acknowledge the command
    BUFOK(buf, bdx, checksum);

    return bdx;
}

// -------------------------------------------------------------------------
// lm32gdb_read_mem()
//
// Read from cpu memory in reply to a read type command (in cmd), and place
// reply in buf. Format of the command is
//
//   M addr,length
//
// Reply format is 'XX...', as set of hex character pairs for each byte, for
// as many as specified in length, starting from addr in memory. The 
// checksum is calculated for the returned characters, and returned in
// checksum.
//
// -------------------------------------------------------------------------

static int lm32gdb_read_mem(lm32_cpu* cpu, const char* cmd, const int cmdlen, char *buf, unsigned char &checksum)
{
    int      bdx  = 0;
    int      cdx  = 0;
    unsigned addr = 0;
    unsigned len  = 0;

    // Skip command character
    cdx++;

    // Get address
    while (cdx < cmdlen && cmd[cdx] != ',')
    {
        addr <<= 4; 
        addr  |= CHAR2NIB(cmd[cdx]);
        cdx++;
    }

    // Skip comma
    cdx++;

    // Get length
    while (cdx < cmdlen)
    {
        len <<= 4; 
        len  |= CHAR2NIB(cmd[cdx]);
        cdx++;
    }

    // Get memory bytes and put values as hex characters in buffer
    for (unsigned idx = 0; idx < len; idx++)
    {
        unsigned val = cpu->lm32_read_mem(addr++, LM32_MEM_RD_GDB_BYTE);
        
        checksum += buf[bdx++] = HIHEXCHAR(val);
        checksum += buf[bdx++] = LOHEXCHAR(val);
    }

    return bdx;
}

// -------------------------------------------------------------------------
// lm32gdb_write_mem()
//
// Write to cpu memory in reply to a write type command (in cmd), and place
// a reply in buf. Format of the command is
//
//   M addr,length:XX...
//
// Where XX... is a set of hex character pairs for each byte to be written,
// for length bytes, starting at addr. The data may be in binary format
// (flagged by is_binary), in which case the XX data are single raw bytes,
// some of which are 'escaped' (see commants in function). As the memory
// write command's data can be large, the passed in cmd buffer does not 
// contain the data after the ':' delimiter. This is read directly from
// the serial port and placed into the cpu's memory. A reply is placed in
// buf ("OK", or "EIO" if an error), with a calculated checksum returned
// in checksum.
//
// -------------------------------------------------------------------------

static int lm32gdb_write_mem (void* fd, lm32_cpu* cpu, const char* cmd, const int cmdlen, char *buf, unsigned char &checksum, 
                              const bool tcp_connection, const bool is_binary)
{
    int      bdx          = 0;
    int      cdx          = 0;
    unsigned addr         = 0;
    unsigned len          = 0;
    bool     io_status_ok = true;

    // Skip command character
    cdx++;

    // Get address
    while (cdx < cmdlen && cmd[cdx] != ',')
    {
        addr <<= 4; 
        addr  |= CHAR2NIB(cmd[cdx]);
        cdx++;
    }

    // Skip comma
    cdx++;

    // Get length
    while (cdx < cmdlen && cmd[cdx] != ':')
    {
        len <<= 4; 
        len  |= CHAR2NIB(cmd[cdx]);
        cdx++;
    }

    // Skip colon
    cdx++;

    // Get hex characters byte values and put into memory
    for (unsigned int idx = 0; idx < len; idx++)
    {
        int val;
        char ipbyte[2];

        if (is_binary)
        {
            io_status_ok |= lm32gdb_read(fd, ipbyte, tcp_connection);

            val = ipbyte[0];
            
            // Some binary data is escaped (with '}' character) and the following is the data 
            // XORed with a pattern (0x20). '#', '$', and '}' are all escaped. Replies
            // containing '*' (0x2a) must be escaped. See 'Debugging with GDB' manual, Appendix E.1
            if (val == GDB_BIN_ESC)
            {
                io_status_ok |= lm32gdb_read(fd, ipbyte, tcp_connection);
                
                val = ipbyte[0] ^ GDB_BIN_XOR_VAL;
            }
        }
        else
        {
            io_status_ok |= lm32gdb_read(fd, &ipbyte[0], tcp_connection);
            io_status_ok |= lm32gdb_read(fd, &ipbyte[1], tcp_connection);

            // Get byte value from hex
            val  = CHAR2NIB(ipbyte[0]) << 4;
            val |= CHAR2NIB(ipbyte[1]);
        }

        if (io_status_ok)
        {
            // Write byte to memory (don't update cycle count)
            cpu->lm32_write_mem(addr++, val, LM32_MEM_WR_ACCESS_BYTE, true);
#ifdef LM32GDB_DEBUG
            fprintf(stderr, "%02X", val & 0xff);
#endif

        }
        else
        {
            // On an error, break out of the loop
            break;
        }
    }

    // Acknowledge the command
    if (io_status_ok)
    {
        BUFOK(buf, bdx, checksum);
    }
    else
    {
        BUFERR(EIO, buf, bdx, checksum);
    }

    return bdx;
}

// -------------------------------------------------------------------------
// lm32gdb_set_hw_bp()
//
// Sets a hardware breakpoint using the CPU's hardware breakpoint registers,
// as specified in the command (cmd). The command format is
//
//   Z1,addr,kind
//
// For the lm32, kind is always 4. A free hardware breakpoint is searched
// for in the BP registers, starting at BP0 and working up. When a free
// register is found it is updated with addr, activated, and LM32GDB_OK
// returned. If no free BP is found LM32GDB_ERR is returned.
//
// -------------------------------------------------------------------------

static int lm32gdb_set_hw_bp (lm32_cpu* cpu, const char *cmd)
{
    uint32_t *bp;
    unsigned cdx  = 3;
    uint32_t addr = 0;

    // Get the address from the command buffer
    while (cmd[cdx] != ',')
    {
        addr <<= 4;
        addr |= CHAR2NIB(cmd[cdx]); cdx++;
    }

    // Retrieve the current CPU state
    lm32_cpu::lm32_state cpu_state = cpu->lm32_get_cpu_state();

    // Look for a free BP register
    if (!(cpu_state.bp0 & 0x00000001))
    {
        bp = &cpu_state.bp0;
    }
    else if (!(cpu_state.bp1 & 0x00000001))
    {
        bp = &cpu_state.bp1;
    }
    else if (!(cpu_state.bp2 & 0x00000001))
    {
        bp = &cpu_state.bp2;
    }
    else if (!(cpu_state.bp3 & 0x00000001))
    {
        bp = &cpu_state.bp3;
    }
    else
    {
        // No free register found, so return an error
        return LM32GDB_ERR;
    }

    // Add the breakpoint address to the selected register, and mark as active 
    *bp = (addr & 0xfffffffc) | 0x00000001;

    // Write back the updated CPU state
    cpu->lm32_set_cpu_state(cpu_state);

    return LM32GDB_OK;
}

// -------------------------------------------------------------------------
// lm32gdb_clear_hw_bp()
//
// Clears a hardware breakpoint from the CPU's hardware breakpoint registers,
// as specified in the command (cmd). The command format is
//
//   z1,addr,kind
//
// For the lm32, kind is always 4. A hardware breakpoint is searched for,
// in the BP registers, that is active and matches addr, starting at BP0 
// and working up. When a matching register is found it is set to 0, and 
// deactivated, and LM32GDB_OK returned. If no matching BP is found 
// LM32GDB_ERR is returned.
// 
// -------------------------------------------------------------------------

static int lm32gdb_clear_hw_bp (lm32_cpu* cpu, const char* cmd)
{
    uint32_t *bp;
    unsigned cdx  = 3;
    uint32_t addr = 0;

    // Get the address from the command buffer
    while (cmd[cdx] != ',')
    {
        addr <<= 4;
        addr |= CHAR2NIB(cmd[cdx]);
        cdx++;
    }

    // Retrieve the current CPU state
    lm32_cpu::lm32_state cpu_state = cpu->lm32_get_cpu_state();

    // Look for an active BP register that matches the address
    if (((addr & 0xfffffffc) | 1) == cpu_state.bp0)
    {
        bp = &cpu_state.bp0;
    }
    else if (((addr & 0xfffffffc) | 1) == cpu_state.bp1)
    {
        bp = &cpu_state.bp1;
    }
    else if (((addr & 0xfffffffc) | 1) == cpu_state.bp2)
    {
        bp = &cpu_state.bp2;
    }
    else if (((addr & 0xfffffffc) | 1) == cpu_state.bp3)
    {
        bp = &cpu_state.bp3;
    }
    else
    {
        // No matching register found, so return an error
        return LM32GDB_ERR;
    }

    // Mark as inactive the selected register
    *bp = 0;

    // Write back the updated CPU state
    cpu->lm32_set_cpu_state(cpu_state);

    return LM32GDB_OK;
}

// -------------------------------------------------------------------------
// lm32gdb_set_hw_wp()
//
// Sets a hardware watchpoint using the CPU's hardware breakpoint registers,
// as specified in the command (cmd). The command format is
//
//   Zn,addr,kind
//
// where n is one of 2, 3 or 4 for writes, reads or both. For the lm32, kind
// is always 4. A free hardware watchpoint is searched for in the DC 
// register's C[0:3] fields, starting at C0 and working up.  When a free
// watchpoint is found the DC field is updated with the relevant state,
// depending on the command's 'n' value, the associated watchpoint register
// (WP[0:3]) is updated with the addr value, and LM32GDB_OK returned. If no
// free WP is found LM32GDB_ERR is returned.
//
// -------------------------------------------------------------------------

static int lm32gdb_set_hw_wp (lm32_cpu* cpu, const char *cmd)
{
    unsigned wp_type;
    unsigned cdx      = 1;
    uint32_t addr     = 0;

    // Retrieve the current CPU state
    lm32_cpu::lm32_state cpu_state = cpu->lm32_get_cpu_state();

    switch(cmd[cdx++])
    {
    case '2':
        wp_type = LM32_WP_BREAK_ON_WRITE;
        break;
    
    case '3':
        wp_type = LM32_WP_BREAK_ON_READ;
        break;
    
    case '4':
        wp_type = LM32_WP_BREAK_ALWAYS;
        break;
    }

    // Skip the comma
    cdx++;

    // Get the address from the command buffer
    while (cmd[cdx] != ',')
    {
        addr <<= 4;
        addr |= CHAR2NIB(cmd[cdx]);
        cdx++;
    }

    // Look for a free watchpoint
    if ((cpu_state.dc & LM32_WP0_DC_MASK) == LM32_WP_DISABLED)
    {
        cpu_state.dc &= ~LM32_WP0_DC_MASK;
        cpu_state.dc |= wp_type << LM32_WP0_BIT;
        cpu_state.wp0 = addr;
    }
    else if ((cpu_state.dc & LM32_WP1_DC_MASK) == LM32_WP_DISABLED)
    {
        cpu_state.dc &= ~LM32_WP1_DC_MASK;
        cpu_state.dc |= wp_type << LM32_WP1_BIT;
        cpu_state.wp1 = addr;
    }
    else if ((cpu_state.dc & LM32_WP2_DC_MASK) == LM32_WP_DISABLED)
    {
        cpu_state.dc &= ~LM32_WP2_DC_MASK;
        cpu_state.dc |= wp_type << LM32_WP2_BIT;
        cpu_state.wp2 = addr;
    }
    else if ((cpu_state.dc & LM32_WP3_DC_MASK) == LM32_WP_DISABLED)
    {
        cpu_state.dc &= ~LM32_WP3_DC_MASK;
        cpu_state.dc |= wp_type << LM32_WP3_BIT;
        cpu_state.wp3 = addr;
    }
    else
    {
        // No matching register found, so return an error
        return LM32GDB_ERR;
    }

    // Write back the updated CPU state
    cpu->lm32_set_cpu_state(cpu_state);

    return LM32GDB_OK;
}


// -------------------------------------------------------------------------
// lm32gdb_clear_hw_wp()
//
// Clears a hardware watchpoint from the CPU's hardware breakpoint
// registers, as specified in the command (cmd). The command format is
//
//   zn,addr,kind
//
// where n is one of 2, 3 or 4 for writes, reads or both. For the lm32, kind
// is always 4. A matching hardware watchpoint is searched for in the DC 
// register's C[0:3] fields (against the command's 'n' value) and the WP
// registers, starting at C0/WP0 and working up. When a matching watchpoint
// is found, the DC field is set to be disabled and LM32GDB_OK returned. If
// no matching WP is found LM32GDB_ERR is returned.
//
// -------------------------------------------------------------------------

static int lm32gdb_clear_hw_wp (lm32_cpu* cpu, const char *cmd)
{
    unsigned wp_type;
    unsigned cdx      = 1;
    uint32_t addr     = 0;

    // Retrieve the current CPU state
    lm32_cpu::lm32_state cpu_state = cpu->lm32_get_cpu_state();

    switch(cmd[cdx++])
    {
    case '2':
        wp_type = LM32_WP_BREAK_ON_WRITE;
        break;
    
    case '3':
        wp_type = LM32_WP_BREAK_ON_READ;
        break;
    
    case '4':
        wp_type = LM32_WP_BREAK_ALWAYS;
        break;
    }

    // Skip the comma
    cdx++;

    // Get the address from the command buffer
    while (cmd[cdx] != ',')
    {
        addr <<= 4;
        addr |= CHAR2NIB(cmd[cdx]);
        cdx++;
    }

    // Look for a matching watchpoint and clear state in DC reg if found
    if ((cpu_state.dc & LM32_WP0_DC_MASK) == (wp_type << LM32_WP0_BIT) && cpu_state.wp0 == addr)
    {
        cpu_state.dc &= ~LM32_WP0_DC_MASK;
    }
    else if ((cpu_state.dc & LM32_WP1_DC_MASK) == (wp_type << LM32_WP0_BIT)  && cpu_state.wp1 == addr)
    {
        cpu_state.dc &= ~LM32_WP1_DC_MASK;
    }
    else if ((cpu_state.dc & LM32_WP2_DC_MASK) == (wp_type << LM32_WP0_BIT)  && cpu_state.wp2 == addr)
    {
        cpu_state.dc &= ~LM32_WP2_DC_MASK;
    }
    else if ((cpu_state.dc & LM32_WP3_DC_MASK) == (wp_type << LM32_WP0_BIT)  && cpu_state.wp3 == addr)
    {
        cpu_state.dc &= ~LM32_WP3_DC_MASK;
    }
    else
    {
        // No matching register found, so return an error
        return LM32GDB_ERR;
    }

    // Write back the updated CPU state
    cpu->lm32_set_cpu_state(cpu_state);

    return LM32GDB_OK;
}

// -------------------------------------------------------------------------
// lm32gdb_run_cpu()
//
// Executes the CPU dependent on the particular GDB command (cmd)---either
// continue (c) or single step (s). The default is to run from the current
// PC value, but the command can have an optional address which, if present
// updates the PC value before execution. The cpu's lm32_run_program()
// method is called with the relevant type, which executes until returning
// with a  termination 'reason' value. The LM32 reason is mapped to a
// signal type and, for break- and watchpoints, the interrupt flags cleared.
// The signal is then returned.
//
// Note that the CPU internal int_flags state for BPs and WPs is cleared
// here *before* the CPU can act upon it, allowing non-intrusive debugging,
// and obviating the need for handlers in the code being debugged.
//
// -------------------------------------------------------------------------

static int lm32gdb_run_cpu (lm32_cpu* cpu, const char* cmd, const int cmdlen, const int type)
{
    int  status;
    int  reason      = SIGHUP;
    bool state_valid = false;

    lm32_cpu::lm32_state cpu_state;

    // If there's an address, fetch it and update PC
    if (cmdlen > 1)
    {
        int cdx = 1;

        // Retrieve the current CPU state
        cpu_state = cpu->lm32_get_cpu_state();
        cpu_state.pc = 0;
        state_valid = true;
        
        // Get the address from the command buffer
        while (cdx < cmdlen)
        {
            cpu_state.pc <<= 4;
            cpu_state.pc |= CHAR2NIB(cmd[cdx]);
            cdx++;
        }
        // Write back the updated CPU state
        cpu->lm32_set_cpu_state(cpu_state);
    }

    // Continue execution
    status = cpu->lm32_run_program(NULL, LM32_FOREVER, LM32_NO_BREAK_ADDR, type, false);
    
    // Inspect the returned status, and process accordingly 
    switch(status)
    {
    case LM32_HW_BREAKPOINT_BREAK:

        // Fetch CPU state and clear breakpoint interrupt
        cpu_state = cpu->lm32_get_cpu_state();
        cpu_state.int_flags &= ~(1 << INT_ID_BREAKPOINT);

        // Write back the updated CPU state
        cpu->lm32_set_cpu_state(cpu_state);

        reason = SIGTRAP;
        break;

    case LM32_HW_WATCHPOINT_BREAK:
        
        // Fetch CPU state and clear watchpoint interrupt
        cpu_state = cpu->lm32_get_cpu_state();
        cpu_state.int_flags &= ~(1 << INT_ID_WATCHPOINT);

        // Write back the updated CPU state
        cpu->lm32_set_cpu_state(cpu_state);
        
        reason = SIGTRAP;
        break;    

    case LM32_SINGLE_STEP_BREAK:
        reason = SIGTRAP;
        break;

    case LM32_BUS_ERROR_BREAK:
        reason = SIGSEGV;
        break;

    case LM32_DIV_ZERO_BREAK:
        reason = SIGFPE;
        break;

    case LM32_INT_BREAK:
        reason = SIGINT;
        break;

    case LM32_LOCK_BREAK:
        reason = SIGTERM;
        break;

    case LM32_RESET_BREAK:
        reason = 0;
        break;
    }

    return reason;        
}

// -------------------------------------------------------------------------
// lm32gdb_proc_hw_bp()
//
// Processes hardware breakpoint (and watchpoint) commands, as passed in
// in cmd. The relevent set/clear bp/wp  function is selected, and the
// returned status inspected. A replay is generated and placed in op_buf
// with either "OK" for a good status, or "Enn" for a bad status (with
// nn being the hex characters for ENOSPC---no space). The replay checksum
// is returned in checksum.
//
// -------------------------------------------------------------------------

static int lm32gdb_proc_hw_bp(lm32_cpu* cpu, const char* cmd, char* op_buf,  unsigned char &checksum)
{
    int op_idx = 0;

    bool clear_not_set = cmd[0] == 'z';

    // Process hardware breakpoint (z1/Z1) command
    if (cmd[1] == '1')
    {
        // Set free hardware breakpoint or clear matching and active h/w breakpoint
        int status = clear_not_set ? lm32gdb_clear_hw_bp(cpu, cmd) : lm32gdb_set_hw_bp(cpu, cmd);
        
        if (status == LM32GDB_OK)
        {
            BUFOK(op_buf, op_idx, checksum);
        }
        // No match found in BP registers...
        else
        {
            // Return 'No space left on device' errno number
            BUFERR(ENOSPC, op_buf, op_idx, checksum);
        }
    }
    // Process hardware watch points (z2/Z2, z3/Z3 and z4/Z4)
    else if (cmd[1] >= '2' && cmd[1] <= '4')
    {
        // Set free h/w watchpoint or clear matching and active h/w watchpoint
        int status = clear_not_set ? lm32gdb_clear_hw_wp(cpu, cmd) : lm32gdb_set_hw_wp(cpu, cmd);

        // Find a free (inactive) h/w watchpoint, and set to address
        if (status == LM32GDB_OK)
        {
            BUFOK(op_buf, op_idx, checksum);
        }
        // No free h/w watchpoint found...
        else
        {
            // Return 'No space left on device' errno number
            BUFERR(ENOSPC, op_buf, op_idx, checksum);
        }
    }

    return op_idx;
}

// -------------------------------------------------------------------------
// lm32gdb_proc_gdb_cmd()
//
// Processes a single GDB command, as stored in cmd. The command is
// inspected and the appropriate local functions called. Generated replies 
// are added to op_buf, with this function bracketing these with $ and #,
// followed by the two character checksum, returned by the functions (if 
// any). An exception to a reply is for the kill (k) command which has
// no reply. Unsupported commands return a default reply of "$#00".
// The function sends the reply to the PTY (fd) and then return either
// true if a 'detach' command (D) was seen, otherwise false.
//
// -------------------------------------------------------------------------

static bool lm32gdb_proc_gdb_cmd (lm32_cpu* cpu, const char* cmd, const int cmdlen, void* fd, bool tcp_connection)
{
    int           cmd_idx   = 1;
    int           op_idx    = 0;
    unsigned char checksum  = 0;
    bool          rcvd_kill = false;
    bool          detached  = false;
    static int    reason    = 0;

    // Packet start
    op_buf[op_idx++] = GDB_SOP_CHAR;

    // Select on command chcracter
    switch(cmd[0])
    {
    // Reason for halt
    case '?':
        op_idx += lm32gdb_gen_register_reply(cpu, cmd, &op_buf[op_idx], checksum, reason);
        break;

    // Read general purpose registers
    case 'g':
        op_idx += lm32gdb_gen_register_reply(cpu, cmd, &op_buf[op_idx], checksum);
        break;

    // Write general purpose registers
    case 'G':
        // Update registers from command
        op_idx += lm32gdb_set_regs(cpu, cmd, cmdlen, &op_buf[op_idx], checksum);
        break;

    // Read memory 
    case 'm':
        op_idx += lm32gdb_read_mem(cpu, cmd, cmdlen, &op_buf[op_idx], checksum);
        break;

    // Write memory (binary)
    case 'X':
        op_idx += lm32gdb_write_mem(fd, cpu, cmd, cmdlen, &op_buf[op_idx], checksum, tcp_connection, true);
        break;

    // Write memory
    case 'M':
        op_idx += lm32gdb_write_mem(fd, cpu, cmd, cmdlen, &op_buf[op_idx], checksum, tcp_connection, false);
        break;

    // Continue
    case 'c':
        // Continue onwards 
        reason = lm32gdb_run_cpu(cpu, cmd, cmdlen, LM32_RUN_CONTINUE);

        // On a break, return with a stop reply packet
        op_idx += lm32gdb_gen_register_reply(cpu, cmd, &op_buf[op_idx], checksum, reason);
        break;

    // Single step
    case 's':
        reason = lm32gdb_run_cpu(cpu, cmd, cmdlen, LM32_RUN_SINGLE_STEP);

        // On a break, return with a stop reply packet
        op_idx += lm32gdb_gen_register_reply(cpu, cmd, &op_buf[op_idx], checksum, reason);
        break;

    case 'D':
        detached = true;
        BUFOK(op_buf, op_idx, checksum);
        break;

    case 'p':
        op_idx += lm32gdb_gen_register_reply(cpu, cmd, &op_buf[op_idx], checksum, reason);
        break;

    case 'P':
        op_idx += lm32gdb_set_regs(cpu, cmd, cmdlen, &op_buf[op_idx], checksum);
        break;

    case 'z':
    case 'Z':
        op_idx += lm32gdb_proc_hw_bp(cpu, cmd, &op_buf[op_idx], checksum);
        break;

    case 'k':
        rcvd_kill = true;
        detached  = true;
        break;    
    }

    // Packet end
    op_buf[op_idx++] = GDB_EOP_CHAR;

    // Checksum
    op_buf[op_idx++] = HIHEXCHAR(checksum);
    op_buf[op_idx++] = LOHEXCHAR(checksum);
    
    // Terminate buffer withy a NULL character in case we want to print for debug
    op_buf[op_idx]   = 0;

    // Send reply if not 'kill' command (which has no reply)
    if (!rcvd_kill)
    {
        // Output the response for the gdb command to the terminal
        for (int idx = 0; idx < op_idx; idx++)
        {
            if (!lm32gdb_write(fd, &op_buf[idx], tcp_connection))
            {
                fprintf(stderr, "LM32GDB: ERROR writing to host: terminating.\n");
                return true;
            }
        }

#ifdef LM32GDB_DEBUG        
        fprintf(stderr, "\nREPLY: %s\n", op_buf);
#endif        
            
    }

    return detached;
}

// -------------------------------------------------------------------------
// lm32_connect_skt()
//
// Opens a TCP socket connection, suitable for GDB remote debugging, on the
// given port number (portno). It listens for a single connection, before
// returning the connection handle established. If any error occurs,
// LM32GDB_ERR is returned instead.
//
// -------------------------------------------------------------------------

static lm32gdb_skt_t lm32gdb_connect_skt (const int portno)
{    
    // Initialise socket environment
    if (lm32gdb_skt_init() < 0)
    {
        return LM32GDB_ERR;
    }
    
    // Create an IPv4 socket byte stream
    lm32gdb_skt_t svrskt;
    if ((svrskt = socket(AF_INET, SOCK_STREAM, IPPROTO_IP)) < 0)
    {
        fprintf(stderr, "ERROR opening socket (%ld)\n", svrskt);
        lm32gdb_skt_cleanup();
        return LM32GDB_ERR;
    }
    
    // Create and zero a server address structure
    struct sockaddr_in serv_addr;
    ZeroMemory((char *) &serv_addr, sizeof(serv_addr));
    
    // Configure the server address structure
    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port        = htons(portno);
    
    // Bind the socket to the address
    int status = bind(svrskt, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
    if (status < 0)
    {
        fprintf(stderr, "ERROR on Binding: %d\n", status);
        lm32gdb_skt_cleanup();
        return LM32GDB_ERR;
    }

    // Advertise the port number
    fprintf(stderr, "LM32GDB: Using TCP port number: %d\n", portno);
    fflush(stderr);
    
    // Listen for connections (blocking)
    if (int status = listen(svrskt, MAXBACKLOG) < 0)
    {
        fprintf(stderr, "ERROR on listening: %d\n", status);
        lm32gdb_skt_cleanup();
        return LM32GDB_ERR;
    }
    
    // Get a client address structure, and length as has to be passed as a pointer to accept()
    struct sockaddr_in cli_addr;  
    socklen_t clilen = sizeof(cli_addr);
    
    // Accept a connection, and get returned handle
    lm32gdb_skt_t cliskt;
    if ((cliskt = accept(svrskt, (struct sockaddr *) &cli_addr,  &clilen)) < 0)
    {         
        fprintf(stderr, "ERROR on accept\n");
        lm32gdb_skt_cleanup();
        return LM32GDB_ERR;
    }
    
    // No longer need the server side (listening) socket
    closesocket(svrskt);
    
    // Return the handle to the connected socket. With this handle can
    // use recv()/send() to read and write (or, Linux only, read()/write()).
    return cliskt;
}

// -------------------------------------------------------------------------
// lm32gdb_create_pty()
//
// Opens a pseudo/virtual serial connection, suitable for GDB remote
// debugging. In the case of Linux, this is done by opening /dev/ptmx,
// which returns a file descriptor. The created pseudo terminal's name 
// is available via ptsname(fd), and this is advertised. For windows
// a serial port is opened, as for a normal COM port, with the assumption
// that a external virtual port pair is already setup (e.g. with com0com).
// The file descriptor/handle is returned in pty_fd, and the function
// returns a 0 value if all is okay, else a non-zero value for an error.
//
// -------------------------------------------------------------------------

static int lm32gdb_create_pty(PTY_HDL* pty_fd, int port_num)
{
#if !(defined(_WIN32) || defined(_WIN64))

    struct termios tio;
    int fd;

    // Create and open a pseudo-terminal for reads and writes (and not this process's console)
    if ((fd = open(PTY_MASTER_DEVICE, O_RDWR | O_NOCTTY)) == PTY_ERROR)
    {
        return PTY_ERROR;
    }

    // Grant slave access to terminal
    grantpt(fd);

    // Unlock it so slaves can open
    unlockpt(fd);

    // Advertise the pseudo-terminal being used
    fprintf(stderr, "LM32GDB: Using pseudo-terminal %s\n", ptsname(fd));

    // Get the serial port parameters
    tcgetattr(fd, &tio);

    // Change settings for our needs
    tio.c_cflag     = BAUDRATE | CS8 | CLOCAL | CREAD;
    tio.c_iflag     = 0;
    tio.c_oflag     = 0;
    tio.c_lflag     = 0;
    tio.c_cc[VMIN]  = 1;
    tio.c_cc[VTIME] = 0;

    // Clear out any rubbish in the input that might have arrived already
    tcflush(fd, TCIFLUSH);

    // Set the input and output BAUD rate in the structure
    cfsetispeed(&tio, BAUDRATE);
    cfsetospeed(&tio, BAUDRATE);

    // Update the attributes of the terminal (immediately)
    tcsetattr(fd, TCSANOW, &tio);

    *pty_fd = (PTY_HDL)fd;

    return fd;

#else

    // For windows, we just open a serial port. This code relies on external use of 
    // virtual com ports, as setup by com0com (https://sourceforge.net/projects/com0com/)
    // The PTY_MASTER_DEVICE number (defined in lm32_gdb.h) must match the *lower* COM 
    // port of a created linked pair (e.g. COM6/COM7), and this program will advertise to 
    // connect to the higher valued port from GDB.

    char comm_port[10];
    DCB  dcb;

    // Create the string for the comm port name
    sprintf(comm_port, "COM%d", port_num);

    //  Open a handle to the specified com port.
    *pty_fd = CreateFile(comm_port,
                         GENERIC_READ | GENERIC_WRITE,
                         0,                               // must be opened with exclusive-access
                         NULL,                            // default security attributes
                         OPEN_EXISTING,                   // must use OPEN_EXISTING
                         0,                               // not overlapped I/O
                         NULL );                          // hTemplate must be NULL for comm devices

    if (*pty_fd == INVALID_HANDLE_VALUE) 
    {
        fprintf (stderr, "LM32GDB: CreateFile failed with error %d.\n", GetLastError());
        return PTY_ERROR;
    }

    // Initialize the DCB structure length field.
    dcb.DCBlength = sizeof(DCB);

    // Build on the current configuration by first retrieving all current
    // settings.
    if (!GetCommState(*pty_fd, &dcb)) 
    {
       fprintf (stderr, "LM32GDB: GetCommState failed with error %d.\n", GetLastError());
       return PTY_ERROR;
    }

    // Fill in some DCB values and set the com state: 
    dcb.BaudRate    = BAUDRATE;      //  baud rate
    dcb.ByteSize    = 8;             //  data size, xmit and rcv
    dcb.Parity      = false;         //  parity bit
    dcb.StopBits    = ONESTOPBIT;    //  stop bit
    dcb.fDtrControl = 0;
    dcb.fRtsControl = 0;

    if (!SetCommState(*pty_fd, &dcb)) 
    {
       fprintf (stderr, "LM32GDB: SetCommState failed with error %d.\n", GetLastError());
       return PTY_ERROR;
    }

    // Advertise the serial port being used. Note: lm32-elf-gdb uses Cygwin, 
    // and needs a Cygwin port for the 'target remote' command, where 
    // COM1 => /dev/ttyS0, COM2 => /dev/ttyS1 etc.

    fprintf(stderr, "LM32GDB: Using serial port /dev/ttyS%d\n", port_num);
    fflush(stderr);

    return LM32GDB_OK;

#endif
}

// -------------------------------------------------------------------------
// lm32gdb_process_gdb()
//
// Top level for the GDB interface of the lm32 CPU ISS. The function is
// called with a pointer to an lm32_cpu object, pre-configured if desired.
// It calls local functions to create a pseudo/virtual serial port for GDB 
// connection, and starts reading characters for this port. It monitors
// for start and end of packets, placing packet contents in ip_buf. Once
// a whole packet is received, it calls lm32gdb_proc_gdb_cmd() to process
// it. This repeats until lm32gdb_proc_gdb_cmd() returns true, flagging
// that the GDB session has detached, when the function cleans up and
// returns. It will return LM32GDB_OK if all is well, else LM32GDB_ERR
// is returned.
//
// -------------------------------------------------------------------------

int lm32gdb_process_gdb (lm32_cpu* cpu, int port_num, bool tcp_connection) 
{
    int   idx      = 0;
    bool  active   = false;
    bool  detached = false;
    bool  waiting  = true;
    char  ipbyte;
    void* pty_fd;

    if (tcp_connection)
    {
        // Create a TCP/IP socket
        if ((pty_fd = (void *)lm32gdb_connect_skt(port_num)) < 0)
        {
            return PTY_ERROR;
        }
    }
    else
    {
        // Create a pseudo-terminal to use
        PTY_HDL fd;
        if (lm32gdb_create_pty(&fd, port_num) == PTY_ERROR)
        {
            return PTY_ERROR;
        }
        pty_fd = (void *) fd;
    }
    
    while (!detached && lm32gdb_read(pty_fd, &ipbyte, tcp_connection))
    {
        // If waiting for first communication, flag that attachment has happened.
        if (waiting)
        {
            waiting = false;
            fprintf(stderr, "LM32GDB: host attached.\n");
            fflush(stderr);
        }

        // If receiving a packet end character (or delimiter for mem writes), process the command an go idle
        if (active && (ipbyte  == GDB_EOP_CHAR     || 
                       idx     == IP_BUFFER_SIZE-1 || 
                       (ipbyte == GDB_MEM_DELIM_CHAR && (ip_buf[0] == 'X' || ip_buf[0] == 'M'))))
        {
            // Acknowledge the packet
            if (!lm32gdb_write(pty_fd, &ack_char, tcp_connection))
            {
                return LM32GDB_ERR;
            }

            // Terminate the buffer string, for ease of debug
            ip_buf[idx] = 0;

            // Process the command
            detached = lm32gdb_proc_gdb_cmd(cpu, ip_buf, idx, pty_fd, tcp_connection);

            // Flag state as inactive
            active = false;

            // Reset input buffer index
            idx    = 0;

#ifdef LM32GDB_DEBUG
            // At termination echo newline char to stdout
            putchar('\n');
            fflush(stdout);
#endif            
        }
        // Wait for a packet start character
        else if (!active && ipbyte == GDB_SOP_CHAR)
        {
            active = true;
        }
        // Get command packet characters, store in buffer [and echo to screen].
        else if (active)
        {
            ip_buf[idx++] = ipbyte;

#ifdef LM32GDB_DEBUG
            // Echo packet data to stdout          
            putchar(ipbyte);
            fflush(stdout);
#endif            
        }
    }

    if (detached)
    {
        fprintf(stderr, "LM32GDB: host detached or received 'kill' from target: terminating.\n");
    }
    else
    {
        fprintf(stderr, "LM32GDB: connection lost to host: terminating.\n");
    }

    // Close socket if a TCP connection
    if (tcp_connection)
    {        
        closesocket((lm32gdb_skt_t)pty_fd);
    }
    return LM32GDB_OK;
}

// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
#ifdef LM32GDB_EXE

int main(int argc, char** argv)
{
    // Create a new cpu object, with verbosity disabled, reset break enabled, lock break disabled,
    // and hw break enabled
    lm32_cpu* local_cpu = new lm32_cpu(false, false, true, false);

    // Start procssing commands from GDB
    if (lm32gdb_process_gdb(local_cpu))
    {
        fprintf(stderr, "***ERROR in opening PTY\n");
        return PTY_ERROR;
    }

    return 0;
}

#endif
