//=============================================================
// 
// Copyright (c) 2017 Simon Southwell. All rights reserved.
//
// Date: 11th May 2017
//
// This file is part of the cpumico32 instruction set simulator.
//
// This code is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// The code is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this code. If not, see <http://www.gnu.org/licenses/>.
//
// $Id: lm32_tlb.h,v 3.4 2017/05/17 13:07:23 simon Exp $
// $Source: /home/simon/CVS/src/cpu/mico32/src/lm32_tlb.h,v $
//
//=============================================================

#ifndef _LM32_TLB_H_
#define _LM32_TLB_H_

// -------------------------------------------------------------------------
// INCLUDES
// -------------------------------------------------------------------------
#include <stdint.h>

// -------------------------------------------------------------------------
// DEFINITIONS
// -------------------------------------------------------------------------

#define LM32_TLB_RO_BIT          0
#define LM32_TLB_NO_CACHE_BIT    1
#define LM32_TLB_PTAG_BIT        2
#define LM32_TLB_PPFN_BIT        12
                                 
#define LM32_TLB_PAGE_BITS       12
#define LM32_TLB_PAGE_SIZE       (1 << LM32_TLB_PAGE_BITS)
#define LM32_TLB_ENTRIES_BITS    10
#define LM32_TLB_NUM_ENTRIES     (1 << LM32_TLB_ENTRIES_BITS)

#define LM32_TLB_GET_PPFN(_e)    (((_e) >> LM32_TLB_PPFN_BIT) & ((1 << 20)-1))
#define LM32_TLB_GET_PTAG(_e)    (((_e) >> LM32_TLB_PTAG_BIT) & ((1 << 10)-1))
#define LM32_TLB_GET_NOCACHE(_e) ((_e) & (1 << LM32_TLB_NO_CACHE_BIT))
#define LM32_TLB_GET_RO(_e)      ((_e) & (1 << LM32_TLB_RO_BIT))

#define LM32_TLB_PTAG_MASK       (~((1 << (LM32_TLB_PAGE_BITS+LM32_TLB_ENTRIES_BITS))-1))
#define LM32_TLB_PPFN_MASK       (~((1 << LM32_TLB_PAGE_BITS)-1))

#define LM32_TLB_VADDR_CMD_MASK  0x7

// -------------------------------------------------------------------------
// TYPE DEFINITIONS
// -------------------------------------------------------------------------

typedef struct 
{
    uint32_t entry[LM32_TLB_NUM_ENTRIES];
    bool     valid[LM32_TLB_NUM_ENTRIES];
}  lm32_tbl_t;


// Document and RTL disagree on VADDR command codes. By default follow RTL 
// as tests match this. Define LM32_MMU_AS_PER_DOC to make act like doc.
#ifdef LM32_MMU_AS_PER_DOC
typedef enum {
    NOP       = 0,
    ITLB_IVLD = 2,
    DTLB_IVLD = 3,
    DTLB_FLSH = 4,
    ITLB_FLSH = 5
} lm32_tlb_vaddr_action_e;
#else
typedef enum {
    NOP       = 0,
    ITLB_FLSH = 2,
    DTLB_FLSH = 3,
    ITLB_IVLD = 4,
    DTLB_IVLD = 5
} lm32_tlb_vaddr_action_e;
#endif

typedef enum
{
    HIT,
    MISS,
    FAULT
} lm32_tlb_status_e;

#endif
