//=============================================================
// 
// Copyright (c) 2013-2016 Simon Southwell. All rights reserved.
//
// Date: 9th April 2013
//
// This file is the main internal header for the ISS.
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
// $Id: lm32_cpu_mico32.h,v 3.2 2017/05/13 10:45:19 simon Exp $
// $Source: /home/simon/CVS/src/cpu/mico32/src/lm32_cpu_mico32.h,v $
//
//=============================================================

#ifndef _LM32_CPU_MICO32_H_
#define _LM32_CPU_MICO32_H_

// -------------------------------------------------------------------------
// INCLUDES
// -------------------------------------------------------------------------

#include <cstdlib>
#include <cstdio>
#include <stdint.h>

// -------------------------------------------------------------------------
// DEFINES
// -------------------------------------------------------------------------

// General bit position masks
#define MASK_BIT31              (0x80000000)
#define MASK_SIGN_BIT8          (0x00000080)
#define MASK_SIGN_BIT16         (0x00008000)
#define MASK_SIGN_BIT18         (0x00020000)
#define MASK_SIGN_BIT28         (0x08000000)
#define MASK_SIGN_BIT32         (MASK_BIT31)

// Opcode masks
#define MASK_OPCODE             (0xfc000000)
#define OPCODE_START_BIT        (26)

#define MASK_INSTR_ADDR         (0xfffffffc)
#define BYTE_MASK               (0x000000ff)
#define HWORD_MASK              (0x0000ffff)
#define WORD_MASK               (0xffffffff)
#define BIT5_MASK               (0x0000001f)
#define BIT18_MASK              (0x0003ffff)
#define BIT28_MASK              (0x0fffffff)

#define MASK_Rx_REG0            (0x03e00000)
#define MASK_Rx_REG1            (0x001f0000)
#define MASK_Rx_REG2            (0x0000f800)
#define Rx_REG0_START_BIT       (21)
#define Rx_REG1_START_BIT       (16)
#define Rx_REG2_START_BIT       (11)

#define MASK_RI_REG0            (MASK_Rx_REG0)
#define MASK_RI_REG1            (MASK_Rx_REG1)
#define MASK_RI_IMM             (0x0000ffff)
#define RI_REG0_START_BIT       (Rx_REG0_START_BIT)
#define RI_REG1_START_BIT       (Rx_REG1_START_BIT)
#define RI_IMM_START_BIT        (0)

#define MASK_RR_REG0            (MASK_Rx_REG0)
#define MASK_RR_REG1            (MASK_Rx_REG1)
#define MASK_RR_REG2            (MASK_Rx_REG2)
#define RR_REG0_START_BIT       (Rx_REG0_START_BIT)
#define RR_REG1_START_BIT       (Rx_REG1_START_BIT)
#define RR_REG2_START_BIT       (Rx_REG2_START_BIT)

#define MASK_CR_CSR             (MASK_Rx_REG0)
#define MASK_CR_REG1            (MASK_Rx_REG1)
#define MASK_CR_REG2            (MASK_Rx_REG2)
#define MASK_CSR_START_BIT      (Rx_REG0_START_BIT)
#define MASK_REG1_START_BIT     (Rx_REG1_START_BIT)
#define MASK_REG2_START_BIT     (Rx_REG2_START_BIT)

#define MASK_CR_FORMAT          (0xfc000000)
#define RCSR_INSTR              (0x90000000)
#define WCSR_INSTR              (0xd0000000)

#define MASK_I_IMM              (0x03ffffff)
#define I_IMM_START_BIT         (0)
#define MASK_IMM_FORMAT         (0xfc000000)
#define IMM_FORMAT1             (0xf8000000)
#define IMM_FORMAT0             (0xe0000000)

#define MEM_SIZE_BITS           (31)
#define MAXCODEMEM              (0x1ULL << MEM_SIZE_BITS)

#define INT_ID_RESET            (0x0)
#define INT_ID_BREAKPOINT       (0x1)
#define INT_ID_IBUSERROR        (0x2)
#define INT_ID_WATCHPOINT       (0x3)
#define INT_ID_DBUSERROR        (0x4)
#define INT_ID_DIVZERO          (0x5)
#define INT_ID_EXTINT           (0x6)
#define INT_ID_SYSCALL          (0x7)
#ifdef LM32_MMU
#define INT_ID_ITLB_MISS        (0x8)
#define INT_ID_DTLB_MISS        (0x9)
#define INT_ID_DTLB_FAULT       (0xa)
#define INT_ID_PRIV_ACCESS      (0xb)
#define INT_ID_MAX_NUM          INT_ID_PRIV_ACCESS
#else
#define INT_ID_MAX_NUM          INT_ID_SYSCALL
#endif
#define INT_ID_NUM              (INT_ID_MAX_NUM+1)

#define IE_IE_MASK              (0x00000001)
#define IE_EIE_MASK             (0x00000002)
#define IE_BIE_MASK             (0x00000004)

#define CSR_ID_IE               (0x0)
#define CSR_ID_IM               (0x1)
#define CSR_ID_IP               (0x2)
#define CSR_ID_ICC              (0x3)
#define CSR_ID_DCC              (0x4)
#define CSR_ID_CC               (0x5)
#define CSR_ID_CFG              (0x6)
#define CSR_ID_EBA              (0x7)
#define CSR_ID_CFG2             (0xA)

#define CSR_ID_DC               (0x8)
#define CSR_ID_DEBA             (0x9)
#define CSR_ID_JTX              (0xE)
#define CSR_ID_JRX              (0xF)
#define CSR_ID_BP0              LM32_CSR_ID_BP0
#define CSR_ID_BP1              LM32_CSR_ID_BP1
#define CSR_ID_BP2              LM32_CSR_ID_BP2
#define CSR_ID_BP3              LM32_CSR_ID_BP3
#define CSR_ID_WP0              LM32_CSR_ID_WP0
#define CSR_ID_WP1              LM32_CSR_ID_WP1
#define CSR_ID_WP2              LM32_CSR_ID_WP2
#define CSR_ID_WP3              LM32_CSR_ID_WP3

#define CSR_NAMES {"IE  ", "IM  ", "IP  ", "ICC ", "DCC ", "CC  ", "CFG ", "EBA ", \
                   "DC  ", "DEBA", "CFG2", "??? ", "??? ", "??? ", "JTX ", "JRX ", \
                   "BP0 ", "BP1 ", "BP2 ", "BP3 ", "??? ", "??? ", "??? ", "??? ", \
                   "WP0 ", "WP1 ", "WP2 ", "WP3 ", "??? ", "??? ", "??? ", "??? "}

#define BREAK_INSTR             (0xAC000002)
#define SCALL_INSTR             (0xAC000007)

#define R0_REG_IDX              (0)
#define GP_REG_IDX              (26)
#define FP_REG_IDX              (27)
#define SP_REG_IDX              (28)
#define RA_REG_IDX              (29)
#define EA_REG_IDX              (30)
#define BA_REG_IDX              (31)
#define NULL_REG_IDX            (0xffff)

#define DEBUG_SINGLE_STEP       (0x00000001)
#define DEBUG_REMAP_ALL         (0x00000002)

#define GET_DC_C0(_dc)          (((_dc) & 0x0000000c) >> 2)
#define GET_DC_C1(_dc)          (((_dc) & 0x00000030) >> 4)
#define GET_DC_C2(_dc)          (((_dc) & 0x000000c0) >> 6)
#define GET_DC_C3(_dc)          (((_dc) & 0x00000300) >> 8)

#define WP_DISABLE              (0)
#define WP_RDONLY               (1)
#define WP_WRONLY               (2)
#define WP_RW                   (3)
#define WP_RD_MASK              (0x1)
#define WP_WR_MASK              (0x2)

#define INSTR_FMT_RI            (0)
#define INSTR_FMT_RR            (1)
#define INSTR_FMT_CR            (2)
#define INSTR_FMT_IM            (3)
#define INSTR_FMT_BR            (4)
#define INSTR_FMT_BK            (5)
#define INSTR_FMT_RC            (6)

#define BREAK_LOCK_INSTR_R0     (0x44000000)
#define BREAK_LOCK_INSTR_BI     (0xe0000000)
#define INSTR_NOP               (0x34000000)
#define INSTR_BREAK             (0xac000002)
#define INSTR_SCALL             (0xac000007)

#define MEM_UNUSED              0
#define MEM_INSTRUCTION_WR      1
#define MEM_DATA_RD             2 
#define MEM_DATA_WR             4

#define INSTR_SE_FALSE          0
#define INSTR_SE_TRUE           1
#define INSTR_SE_DONT_CARE      2
#define INSTR_SE_FALSE_HEX      4

// Macros to sign extend
#define SIGN_EXT8(_val)  ((int32_t)((_val) | (((_val) & MASK_SIGN_BIT8 ) ? ~BYTE_MASK  : 0)))
#define SIGN_EXT16(_val) ((int32_t)((_val) | (((_val) & MASK_SIGN_BIT16) ? ~HWORD_MASK : 0)))
#define SIGN_EXT18(_val) ((int32_t)((_val) | (((_val) & MASK_SIGN_BIT18) ? ~BIT18_MASK : 0)))
#define SIGN_EXT28(_val) ((int32_t)((_val) | (((_val) & MASK_SIGN_BIT28) ? ~BIT28_MASK : 0)))

// Define the decode table entries, in order of opcode
#define DECODE_TABLE {                                                          \
    {"srui     ",   INSTR_FMT_RI,  2,  1, 0, INSTR_SE_FALSE    },  /* 0x00 */   \
    {"nori     ",   INSTR_FMT_RI,  1,  1, 0, INSTR_SE_FALSE_HEX},               \
    {"muli     ",   INSTR_FMT_RI,  3,  1, 0, INSTR_SE_TRUE     },               \
    {"sh       ",   INSTR_FMT_RI,  0,  1, 0, INSTR_SE_TRUE     },               \
    {"lb       ",   INSTR_FMT_RI,  3,  1, 0, INSTR_SE_TRUE     },               \
    {"sri      ",   INSTR_FMT_RI,  2,  1, 0, INSTR_SE_FALSE    },               \
    {"xori     ",   INSTR_FMT_RI,  1,  1, 0, INSTR_SE_FALSE_HEX},               \
    {"lh       ",   INSTR_FMT_RI,  3,  1, 0, INSTR_SE_TRUE     },               \
    {"andi     ",   INSTR_FMT_RI,  1,  1, 0, INSTR_SE_FALSE_HEX},  /* 0x08 */   \
    {"xnori    ",   INSTR_FMT_RI,  1,  1, 0, INSTR_SE_FALSE_HEX},               \
    {"lw       ",   INSTR_FMT_RI,  3,  1, 0, INSTR_SE_TRUE     },               \
    {"lhu      ",   INSTR_FMT_RI,  3,  1, 0, INSTR_SE_TRUE     },               \
    {"sb       ",   INSTR_FMT_RI,  0,  1, 0, INSTR_SE_TRUE     },               \
    {"addi     ",   INSTR_FMT_RI,  1,  1, 0, INSTR_SE_TRUE     },               \
    {"ori      ",   INSTR_FMT_RI,  1,  1, 0, INSTR_SE_FALSE_HEX},               \
    {"sli      ",   INSTR_FMT_RI,  2,  1, 0, INSTR_SE_FALSE    },               \
    {"lbu      ",   INSTR_FMT_RI,  3,  1, 0, INSTR_SE_TRUE     },  /* 0x10 */   \
    {"be       ",   INSTR_FMT_RI,  0,  4, 1, INSTR_SE_TRUE     },               \
    {"bg       ",   INSTR_FMT_RI,  0,  4, 1, INSTR_SE_TRUE     },               \
    {"bge      ",   INSTR_FMT_RI,  0,  4, 1, INSTR_SE_TRUE     },               \
    {"bgeu     ",   INSTR_FMT_RI,  0,  4, 1, INSTR_SE_TRUE     },               \
    {"bgu      ",   INSTR_FMT_RI,  0,  4, 1, INSTR_SE_TRUE     },               \
    {"sw       ",   INSTR_FMT_RI,  0,  1, 0, INSTR_SE_TRUE     },               \
    {"bne      ",   INSTR_FMT_RI,  0,  4, 1, INSTR_SE_TRUE     },               \
    {"andhi    ",   INSTR_FMT_RI,  1,  1, 0, INSTR_SE_FALSE_HEX},  /* 0x18 */   \
    {"cmpei    ",   INSTR_FMT_RI,  2,  1, 0, INSTR_SE_TRUE     },               \
    {"cmpgi    ",   INSTR_FMT_RI,  2,  1, 0, INSTR_SE_TRUE     },               \
    {"cmpgei   ",   INSTR_FMT_RI,  2,  1, 0, INSTR_SE_TRUE     },               \
    {"cmpgeui  ",   INSTR_FMT_RI,  2,  1, 0, INSTR_SE_FALSE    },               \
    {"cmpgui   ",   INSTR_FMT_RI,  2,  1, 0, INSTR_SE_FALSE    },               \
    {"orhi     ",   INSTR_FMT_RI,  1,  1, 0, INSTR_SE_FALSE_HEX},               \
    {"cmpnei   ",   INSTR_FMT_RI,  2,  1, 0, INSTR_SE_TRUE     },               \
    {"sru      ",   INSTR_FMT_RR,  2,  1, 0, INSTR_SE_DONT_CARE},  /* 0x20 */   \
    {"nor      ",   INSTR_FMT_RR,  1,  1, 0, INSTR_SE_DONT_CARE},               \
    {"mul      ",   INSTR_FMT_RR,  3,  1, 0, INSTR_SE_DONT_CARE},               \
    {"divu     ",   INSTR_FMT_RR, 34, 34, 0, INSTR_SE_DONT_CARE},               \
    {"rcsr     ",   INSTR_FMT_CR,  1,  1, 0, INSTR_SE_DONT_CARE},               \
    {"sr       ",   INSTR_FMT_RR,  2,  1, 0, INSTR_SE_DONT_CARE},               \
    {"xor      ",   INSTR_FMT_RR,  1,  1, 0, INSTR_SE_DONT_CARE},               \
    {"div      ",   INSTR_FMT_RR, 34, 34, 0, INSTR_SE_DONT_CARE},               \
    {"and      ",   INSTR_FMT_RR,  1,  1, 0, INSTR_SE_DONT_CARE},  /* 0x28 */   \
    {"xnor     ",   INSTR_FMT_RR,  1,  1, 0, INSTR_SE_DONT_CARE},               \
    {"reserved ",   INSTR_FMT_RI,  1,  1, 0, INSTR_SE_DONT_CARE},               \
    {"raise    ",   INSTR_FMT_BK,  0,  4, 0, INSTR_SE_DONT_CARE},               \
    {"sextb    ",   INSTR_FMT_RC,  1,  1, 0, INSTR_SE_DONT_CARE},               \
    {"add      ",   INSTR_FMT_RR,  1,  1, 0, INSTR_SE_DONT_CARE},               \
    {"or       ",   INSTR_FMT_RR,  1,  1, 0, INSTR_SE_DONT_CARE},               \
    {"sl       ",   INSTR_FMT_RR,  2,  1, 0, INSTR_SE_DONT_CARE},               \
    {"b        ",   INSTR_FMT_BR,  0,  4, 0, INSTR_SE_DONT_CARE},  /* 0x30 */   \
    {"modu     ",   INSTR_FMT_RR, 34, 34, 0, INSTR_SE_DONT_CARE},               \
    {"sub      ",   INSTR_FMT_RR,  1,  1, 0, INSTR_SE_DONT_CARE},               \
    {"reserved ",   INSTR_FMT_RI,  1,  1, 0, INSTR_SE_DONT_CARE},               \
    {"wcsr     ",   INSTR_FMT_CR,  1,  1, 0, INSTR_SE_DONT_CARE},               \
    {"mod      ",   INSTR_FMT_RR, 34, 34, 0, INSTR_SE_DONT_CARE},               \
    {"call     ",   INSTR_FMT_BR,  1,  4, 0, INSTR_SE_DONT_CARE},               \
    {"sexth    ",   INSTR_FMT_RC,  1,  1, 0, INSTR_SE_DONT_CARE},               \
    {"bi       ",   INSTR_FMT_IM,  0,  4, 0, INSTR_SE_TRUE     },  /* 0x38 */   \
    {"cmpe     ",   INSTR_FMT_RR,  2,  1, 0, INSTR_SE_DONT_CARE},               \
    {"cmpg     ",   INSTR_FMT_RR,  2,  1, 0, INSTR_SE_DONT_CARE},               \
    {"cmpge    ",   INSTR_FMT_RR,  2,  1, 0, INSTR_SE_DONT_CARE},               \
    {"cmpgeu   ",   INSTR_FMT_RR,  2,  1, 0, INSTR_SE_DONT_CARE},               \
    {"cmpgu    ",   INSTR_FMT_RR,  2,  1, 0, INSTR_SE_DONT_CARE},               \
    {"calli    ",   INSTR_FMT_IM,  1,  4, 0, INSTR_SE_TRUE     },               \
    {"cmpne    ",   INSTR_FMT_RR,  2,  1, 0, INSTR_SE_DONT_CARE}                \
}               

// -------------------------------------------------------------------------
// TYPEDEFS
// -------------------------------------------------------------------------

// Forward reference for following type definition
class lm32_cpu;

// Define a "pointer to a function" type to match lm32_cpu instruction member functions
typedef void (lm32_cpu::* pFunc_t) (const p_lm32_decode_t);


#endif    
