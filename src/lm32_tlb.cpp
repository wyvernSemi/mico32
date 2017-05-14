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
// $Id: lm32_tlb.cpp,v 3.1 2017/05/13 10:45:19 simon Exp $
// $Source: /home/simon/CVS/src/cpu/mico32/src/lm32_tlb.cpp,v $
//
//=============================================================

// -------------------------------------------------------------------------
// INCLUDES
// -------------------------------------------------------------------------
#include <string.h>

#include "lm32_cpu.h"
#include "lm32_tlb.h"

// Only include code if compiling in MMU
#ifdef LM32_MMU

// -------------------------------------------------------------------------
//  tlb_lookup()
// 
//  Lookup physical address from virtual address in selected table.
//  Return HOT, MISS or FAULT as appropriate. Physical address returned
//  in paddr.
// 
// -------------------------------------------------------------------------

lm32_tlb_status_e lm32_cpu::tlb_lookup(uint32_t &paddr, const uint32_t vaddr, const bool is_data, const bool is_write)
{
    // By default, assume we'll miss
    lm32_tlb_status_e status = MISS;

    uint32_t ptag   = vaddr & ~(LM32_TLB_PAGE_SIZE-1);
    uint32_t tlbidx = (vaddr >> LM32_TLB_PAGE_BITS) & (LM32_TLB_NUM_ENTRIES-1);
    lm32_tbl_t* tlb = is_data ? &dtlb : &itlb;
    uint32_t entry  = tlb->entry[tlbidx];

    // Check of the TLB entry is valid...
    if (tlb->valid[tlbidx])
    {
        // Does the entry tag match the virtual address tag?
        if (LM32_TLB_GET_PTAG(entry) == ptag)
        {
            // Is this a write to read-only memory?
            if (is_data && is_write && LM32_TLB_GET_RO(entry))
            {
                status = FAULT;
            }
            else
            {
                status = HIT;
            }
        }
    }

    // Regardless, construct the physical address
    paddr = LM32_TLB_GET_PPFN(entry) | (vaddr & (LM32_TLB_PAGE_SIZE-1));

    // If a TLB exception, update the TLBBADVADDR register with virtual address that caused it.
    if (status != HIT)
    {
        state.tlbbadvaddr = vaddr;
    }

    return status;
}

// -------------------------------------------------------------------------
//  tlb_vaddr_update()
// 
//  Update TLBs when TLBVADDR CSR written, getting index from TLBVADDR.
//  VADDR bits 2 down to 0 select table and operation---either to clear
//  entry, or clear an entire table.
// 
// -------------------------------------------------------------------------


void lm32_cpu::tlb_vaddr_update(uint32_t vaddr)
{
    uint32_t tlbidx = (vaddr >> LM32_TLB_PAGE_BITS) & (LM32_TLB_NUM_ENTRIES-1);

    // Update CSR register
    state.tlbvaddr = vaddr;

    // Action the command
    switch(vaddr & LM32_TLB_VADDR_CMD_MASK)
    {
    case ITLB_IVLD:
        itlb.valid[tlbidx] = false;
        break;
    case DTLB_IVLD:
        dtlb.valid[tlbidx] = false;
        break;
    case DLTB_FLSH:
        memset(dtlb.valid, 0, sizeof(dtlb.valid));
        break;
    case ITLB_FLSH:
        memset(itlb.valid, 0, sizeof(itlb.valid));
        break;
    }
}

// -------------------------------------------------------------------------
//  tlb_paddr_update()
// 
//  Update TLB entry when TLBPADDR CSR written, getting index from TLBVADDR.
//  PADDR bit 0 selects between instruction and data tables.
// 
// -------------------------------------------------------------------------

void lm32_cpu::tlb_paddr_update(uint32_t paddr)
{
    // Extract table index from VADDR
    uint32_t tlbidx = (state.tlbvaddr >> LM32_TLB_PAGE_BITS) & (LM32_TLB_NUM_ENTRIES-1);

    // Select  appropriate table
    lm32_tbl_t* tlb = (paddr & 1) ? &dtlb : &itlb;

    // Update table entry
    tlb->valid[tlbidx] = true;
    tlb->entry[tlbidx] = paddr;
}

#endif