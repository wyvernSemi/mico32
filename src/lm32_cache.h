//=============================================================
// 
// Copyright (c) 2013 Simon Southwell. All rights reserved.
//
// Date: 5th July 2013
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
// $Id: lm32_cache.h,v 3.0 2016/09/07 13:15:36 simon Exp $
// $Source: /home/simon/CVS/src/cpu/mico32/src/lm32_cache.h,v $
//
//=============================================================

#ifndef _LM32_CACHE_H_
#define _LM32_CACHE_H_

// -------------------------------------------------------------------------
// INCLUDES
// -------------------------------------------------------------------------

#include <cstdio>
#include <cstdlib>

#include "lm32_cpu_hdr.h"

// -------------------------------------------------------------------------
// TYPEDEFS
// -------------------------------------------------------------------------

typedef struct {
    uint32_t addr;
    int      valid;
} lm32_cache_line_t;

typedef lm32_cache_line_t lm32_cache_set_t [LM32_MAX_WAYS][LM32_MAX_SETS];

// -------------------------------------------------------------------------
// Class definition for LM32 cache
// -------------------------------------------------------------------------

class lm32_cache {

public:
    // Constructor 
         lm32_cache            (const int      bytes_per_line = LM32_CACHE_DEFAULT_LINE,
                                const int      num_of_ways    = LM32_CACHE_DEFAULT_WAYS,
                                const int      num_of_sets    = LM32_CACHE_DEFAULT_SETS,
                                const uint32_t base_addr      = LM32_CACHE_DEFAULT_BASE,
                                const uint32_t limit          = LM32_CACHE_DEFAULT_DLIMIT);

    // Cache access method
    int  lm32_cache_access     (const uint32_t in_addr);

    // Invalidate cache method
    void lm32_cache_invalidate (void);

    // Return configured line width (in bytes)
    inline int get_line_width (void) { return line_width; };
    

private:

    // Internal cache state
    int              line_width;                        // Number of bytes in a cache line (4, 8 or 16)
    int              ways;                              // Size of associativity, or ways (1 or 2)
    int              sets;                              // Size of set lines (128, 256, 512 or 1K)
    int              next_way_update[LM32_MAX_SETS];    // Flag per line in a set to indicate which way is next for update
    uint32_t         base_addr;                         // Base address of cached region
    uint32_t         limit;                             // Upper limit of cached region
    lm32_cache_set_t m;                                 // Cache line sets
};

#endif
