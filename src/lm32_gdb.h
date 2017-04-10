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
// $Id: lm32_gdb.h,v 3.3 2017/04/10 13:19:29 simon Exp $
// $Source: /home/simon/CVS/src/cpu/mico32/src/lm32_gdb.h,v $
//
//=============================================================

#ifndef _LM32_GDB_H_
#define _LM32_GDB_H_

// -------------------------------------------------------------------------
// INCLUDES
// -------------------------------------------------------------------------

#include "lm32_cpu.h"

// -------------------------------------------------------------------------
// DEFINES
// -------------------------------------------------------------------------

#if !(defined(_WIN32) || defined(_WIN64))

#include <termios.h>
# define BAUDRATE             B19200
# define PTY_MASTER_DEVICE    "/dev/ptmx"
# define PTY_HDL              int
#else
// Map some signals undefined in windows to linux values
# define SIGTRAP              5
# define SIGHUP               1

# define BAUDRATE             CBR_19200
# define PTY_MASTER_DEVICE    LM32_DEFAULT_COM_PORT
# define PTY_HDL              HANDLE
#endif

#define LM32GDB_OK            0
#define LM32GDB_ERR           -1

#define IP_BUFFER_SIZE        1024
#define OP_BUFFER_SIZE        1024
#define PTY_ERROR             LM32GDB_ERR
#define GDB_ACK_CHAR          '+'
#define GDB_NAK_CHAR          '-'
#define GDB_SOP_CHAR          '$'
#define GDB_EOP_CHAR          '#'
#define GDB_MEM_DELIM_CHAR    ':'
#define GDB_BIN_ESC           0x7d
#define GDB_BIN_XOR_VAL       0x20
#define HEX_CHAR_MAP          {'0', '1', '2', '3', \
                               '4', '5', '6', '7', \
                               '8', '9', 'a', 'b', \
                               'c', 'd', 'e', 'f'}

// -------------------------------------------------------------------------
// MACRO DEFINITIONS
// -------------------------------------------------------------------------                               

#define HIHEXCHAR(_x) hexchars[((_x) & 0xf0) >> 4]
#define LOHEXCHAR(_x) hexchars[ (_x) & 0x0f]

#define CHAR2NIB(_x) (((_x) >= '0' && (_x) <= '9') ? (_x) - '0': \
                      ((_x) >= 'a' && (_x) <= 'f') ? (10 + (_x) - 'a'): \
                                                     (10 + (_x) - 'A'))

#define BUFBYTE(_b,_i,_v) {     \
    _b[_i]     = HIHEXCHAR(_v); \
    _b[(_i)+1] = LOHEXCHAR(_v); \
    _i        += 2;             \
}
#define BUFWORD(_b,_i,_v) {     \
    BUFBYTE(_b,_i,(_v) >> 24);  \
    BUFBYTE(_b,_i,(_v) >> 16);  \
    BUFBYTE(_b,_i,(_v) >>  8);  \
    BUFBYTE(_b,_i,(_v));        \
}


#define BUFOK(_b,_i,_c) {_c += _b[_i] = 'O'; _c += _b[_i+1] = 'K'; _i+=2;}

#define BUFERR(_e,_b,_i,_c) {_c += _b[_i] = 'E'; _c += _b[_i+1] = HIHEXCHAR(_e); _c += _b[_i+2] = LOHEXCHAR(_e); _i+=3;}

// -------------------------------------------------------------------------
// ENUMERATIONS
// -------------------------------------------------------------------------

// Indexes/IDs of LM32 registers that match GDB's expectations
enum lm32gdb_regs_e 
{
  LM32_REG_R0,  LM32_REG_R1,  LM32_REG_R2,  LM32_REG_R3,   
  LM32_REG_R4,  LM32_REG_R5,  LM32_REG_R6,  LM32_REG_R7,
  LM32_REG_R8,  LM32_REG_R9,  LM32_REG_R10, LM32_REG_R11,
  LM32_REG_R12, LM32_REG_R13, LM32_REG_R14, LM32_REG_R15,
  LM32_REG_R16, LM32_REG_R17, LM32_REG_R18, LM32_REG_R19,
  LM32_REG_R20, LM32_REG_R21, LM32_REG_R22, LM32_REG_R23,
  LM32_REG_R24, LM32_REG_R25, LM32_REG_GP,  LM32_REG_FP,
  LM32_REG_SP,  LM32_REG_RA,  LM32_REG_EA,  LM32_REG_BA,
  LM32_REG_PC,  LM32_REG_EID, LM32_REG_EBA, LM32_REG_DEBA, 
  LM32_REG_IE,  LM32_REG_IM,  LM32_REG_IP,
  NUM_REGS
};

// -------------------------------------------------------------------------
// PUBLIC PROTOTYPES
// -------------------------------------------------------------------------

extern int lm32gdb_process_gdb (lm32_cpu* cpu, int port_num = LM32_DEFAULT_COM_PORT);

#endif   