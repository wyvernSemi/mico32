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
// $Id: lm32_gdb.cpp,v 3.1 2017/03/31 11:48:52 simon Exp $
// $Source: /home/simon/CVS/src/cpu/mico32/src/lm32_gdb.cpp,v $
//
//=============================================================

// -------------------------------------------------------------------------
// INCLUDES
// -------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

#include "lm32_gdb.h"
#include "lm32_cpu.h"

// -------------------------------------------------------------------------
// LOCAL CONSTANTS
// -------------------------------------------------------------------------

const static char ack_char       = GDB_ACK_CHAR;
const static char hexchars[]     = HEX_CHAR_MAP;

// -------------------------------------------------------------------------
// STATIC VARIABLES
// -------------------------------------------------------------------------

static char ip_buf[IP_BUFFER_SIZE];
static char op_buf[OP_BUFFER_SIZE];

// -------------------------------------------------------------------------
// -------------------------------------------------------------------------

static int gen_register_reply(lm32_cpu* cpu, const char* cmd, char *buf, unsigned char &checksum, const int sigval = SIGHUP)
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
            if (idx != single_reg)
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
// -------------------------------------------------------------------------

static int set_regs (lm32_cpu* cpu, const char* cmd, const int cmdlen, char* buf, unsigned char &checksum)
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

        // Convert the two character hex value to a number
        val  = CHAR2NIB(cmd[cdx]) << 4; cdx++;
        val |= CHAR2NIB(cmd[cdx]);      cdx++;

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
// -------------------------------------------------------------------------

static int read_mem(lm32_cpu* cpu, const char* cmd, const int cmdlen, char *buf, unsigned char &checksum)
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
    for (int idx = 0; idx < len; idx++)
    {
        unsigned val = cpu->lm32_read_mem(addr++, LM32_MEM_RD_ACCESS_BYTE);
        
        checksum += buf[bdx++] = HIHEXCHAR(val);
        checksum += buf[bdx++] = LOHEXCHAR(val);

        //fprintf(stderr, "read_mem: addr = 0x%08x len = %d val = 0x%02x\n", addr, len, val);
    }

    return bdx;
}

// -------------------------------------------------------------------------
// -------------------------------------------------------------------------

static int write_mem (lm32_cpu* cpu, const char* cmd, const int cmdlen, char *buf, unsigned char &checksum, const bool is_binary = false)
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
    while (cdx < cmdlen && cmd[cdx] != ':')
    {
        len <<= 4; 
        len  |= CHAR2NIB(cmd[cdx]);
        cdx++;
    }

    // Skip colon
    cdx++;

    // Get hex characters byte values and put into memory
    for (int idx = 0; idx < len && cdx < cmdlen; idx++)
    {
        int val;

        if (is_binary)
        {
            val = cmd[cdx++];
            
            // Some binary data is escaped (with '}' character) and the following is the data 
            // XORed with a pattern (0x20). '#', '$', and '}' are all escaped. Replies
            // containing '*' (0x2a) must be escaped. See 'Debugging with GDB' manual, Appendix E.1
            if (val == GDB_BIN_ESC)
            {
                val = cmd[cdx++] ^ GDB_BIN_XOR_VAL;
            }
        }
        else
        {
            // Get byte value from hex
            val  = CHAR2NIB(cmd[cdx]) << 4; cdx++;
            val |= CHAR2NIB(cmd[cdx]);      cdx++;
        }

        //fprintf(stderr, "WR%04x (%d, %d): 0x%02x\n", addr, len, cmdlen, val);

        // Write byte to memory (don't update cycle count)
        cpu->lm32_write_mem(addr++, val, LM32_MEM_WR_ACCESS_BYTE, true);
    }

    // Acknowledge the command
    BUFOK(buf, bdx, checksum);

    return bdx;
}


// -------------------------------------------------------------------------
// -------------------------------------------------------------------------

static int set_hw_bp (lm32_cpu* cpu, const char *cmd)
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

    //fprintf(stderr, "BP: 0x%08x\n", cpu_state.bp0);

    // Write back the updated CPU state
    cpu->lm32_set_cpu_state(cpu_state);

    return LM32GDB_OK;
}

// -------------------------------------------------------------------------
// -------------------------------------------------------------------------

static int clear_hw_bp (lm32_cpu* cpu, const char* cmd)
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
// -------------------------------------------------------------------------

static int set_hw_wp (lm32_cpu* cpu, const char *cmd)
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
// -------------------------------------------------------------------------

static int clear_hw_wp (lm32_cpu* cpu, const char *cmd)
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
// -------------------------------------------------------------------------

static int run_cpu (lm32_cpu* cpu, const char* cmd, const int cmdlen, const int type)
{
    int  status;
    int  reason      = SIGHUP;
    bool state_valid = false;

    lm32_cpu::lm32_state cpu_state;

    // If there's an address, fetch it and update PC
    if (cmdlen > 1)
    {
        unsigned cdx = 1;

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
        cpu_state.int_flags &= ~(1 <<INT_ID_WATCHPOINT);

        // Write back the updated CPU state
        cpu->lm32_set_cpu_state(cpu_state);
        
        reason = SIGTRAP;
        break;    

    case LM32_SINGLE_STEP_BREAK:
        reason = SIGTRAP;
        break;

    case LM32_RESET_BREAK:
        reason = 0;
        break;
    }

    return reason;        
}

// -------------------------------------------------------------------------
// -------------------------------------------------------------------------

static int proc_hw_bp(lm32_cpu* cpu, const char* cmd, char* op_buf,  unsigned char &checksum)
{
    int op_idx = 0;

    bool clear_not_set = cmd[0] == 'z';

    // Process hardware breakpoint (z1/Z1) command
    if (cmd[1] == '1')
    {
        // Set free hardware breakpoint or clear matching and active h/w breakpoint
        int status = clear_not_set ? clear_hw_bp(cpu, cmd) : set_hw_bp(cpu, cmd);
        
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
        int status = clear_not_set ? clear_hw_wp(cpu, cmd) : set_hw_wp(cpu, cmd);

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
// -------------------------------------------------------------------------

static bool proc_gdb_cmd (lm32_cpu* cpu, const char* cmd, const int cmdlen, const int fd)
{
    int           cmd_idx   = 1;
    int           op_idx    = 0;
    unsigned char checksum  = 0;
    bool          rcvd_kill = false;
    bool          detached  = false;
    static int    reason    = SIGHUP; // TODO: decode reasons and send correct signal

    // Packet start
    op_buf[op_idx++] = GDB_SOP_CHAR;

    // Select on command chcracter
    switch(cmd[0])
    {
    // Reason for halt
    case '?':
        op_idx += gen_register_reply(cpu, cmd, &op_buf[op_idx], checksum, reason);
        break;

    // Read general purpose registers
    case 'g':
        op_idx += gen_register_reply(cpu, cmd, &op_buf[op_idx], checksum);
        break;

    // Write general purpose registers
    case 'G':
        // Update registers from command
        op_idx += set_regs(cpu, cmd, cmdlen, &op_buf[op_idx], checksum);
        break;

    // Read memory 
    case 'm':
        op_idx += read_mem(cpu, cmd, cmdlen, &op_buf[op_idx], checksum);
        break;

    // Write memory (binary)
    case 'X':
        op_idx += write_mem(cpu, cmd, cmdlen, &op_buf[op_idx], checksum, true);
        break;

    // Write memory
    case 'M':
        op_idx += write_mem(cpu, cmd, cmdlen, &op_buf[op_idx], checksum, false);
        break;

    // Continue
    case 'c':
        // Continue onwards 
        reason = run_cpu(cpu, cmd, cmdlen, LM32_RUN_CONTINUE);

        // On a break, return with a stop reply packet
        op_idx += gen_register_reply(cpu, cmd, &op_buf[op_idx], checksum, reason);
        break;

    // Single step
    case 's':
        reason = run_cpu(cpu, cmd, cmdlen, LM32_RUN_SINGLE_STEP);

        // On a break, return with a stop reply packet
        op_idx += gen_register_reply(cpu, cmd, &op_buf[op_idx], checksum, reason);
        break;

    case 'D':
        detached = true;
        BUFOK(op_buf, op_idx, checksum);
        break;

    case 'p':
        op_idx += gen_register_reply(cpu, cmd, &op_buf[op_idx], checksum, reason);
        break;

    case 'P':
        op_idx += set_regs(cpu, cmd, cmdlen, &op_buf[op_idx], checksum);
        break;

    case 'z':
    case 'Z':
        op_idx += proc_hw_bp(cpu, cmd, &op_buf[op_idx], checksum);
        break;

    case 'k':
        rcvd_kill = true;
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
            if (write(fd, &op_buf[idx], 1) == -1)
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
// -------------------------------------------------------------------------

static int create_pty()
{
    struct termios tio;
    int pty_fd;

    // Create and open a pseudo-terminal for reads ad writes (and not this process's console)
    if ((pty_fd = open(PTY_MASTER_DEVICE, O_RDWR | O_NOCTTY)) == PTY_ERROR)
    {
        return PTY_ERROR;
    }

    // Grant slave access to terminal
    grantpt(pty_fd);

    // Unlock it so slaves can open
    unlockpt(pty_fd);

    // Advertise the pseudo-terminal being used
    fprintf(stderr, "LM32GDB: Using pseudo-terminal %s\n", ptsname(pty_fd));

    setenv("LM32GDB_PTS", ptsname(pty_fd), 1);

    // Get the serial port parameters
    tcgetattr(pty_fd, &tio);

    // Change settings for our needs
    tio.c_cflag     = BAUDRATE | CS8 | CLOCAL | CREAD;
    tio.c_iflag     = 0;
    tio.c_oflag     = 0;
    tio.c_lflag     = 0;
    tio.c_cc[VMIN]  = 1;
    tio.c_cc[VTIME] = 0;

    // Clear out any rubbish in the input that might have arrived already
    tcflush(pty_fd, TCIFLUSH);

    // Set the input and output BAUD rate in the structure
    cfsetispeed(&tio, BAUDRATE);
    cfsetospeed(&tio, BAUDRATE);

    // Update the attributes of the terminal (immediately)
    tcsetattr(pty_fd, TCSANOW, &tio);

    return pty_fd;
}

// -------------------------------------------------------------------------
// -------------------------------------------------------------------------

int process_gdb (lm32_cpu* cpu) 
{
    int  idx      = 0;
    bool active   = false;
    bool detached = false;
    bool waiting  = true;
    char ipbyte;
    int  pty_fd;
    int  status;

    // Create a pseudo-terminal to use
    if ((pty_fd = create_pty()) == PTY_ERROR)
    {
        return PTY_ERROR;
    }
    
    while (!detached && (status = read(pty_fd, &ipbyte, 1)) == 1)
    {
        // If waiting for first communication, flag that attachment has happened.
        if (waiting)
        {
            waiting = false;
            fprintf(stderr, "LM32GDB: host attached.\n");
        }

        // If receiving a packet end character, process the command an go idle
        if (active && (ipbyte == GDB_EOP_CHAR || idx == IP_BUFFER_SIZE-1))
        {
            // Acknowledge the packet
            if (write(pty_fd, &ack_char, 1) == -1)
            {
                return LM32GDB_ERR;
            }

            // Terminate the buffer string, for ease of debug
            ip_buf[idx] = 0;

            // Process the command
            detached = proc_gdb_cmd(cpu, ip_buf, idx, pty_fd);

            // Flag state as inactive
            active = false;

            // Reset input buffer index
            idx    = 0;

#ifdef LM32GDB_DEBUG
            // Echo termination char to stdout
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
        fprintf(stderr, "LM32GDB: host detached from target: terminating.\n");
    }
    else
    {
        fprintf(stderr, "LM32GDB: connection lost to host (status = %d): terminating.\n", status);
    }
    return 0;
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
    if (process_gdb(local_cpu))
    {
        fprintf(stderr, "***ERROR in opening PTY\n");
        return PTY_ERROR;
    }

    return 0;
}

#endif
