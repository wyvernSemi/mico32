//=============================================================
// 
// Copyright (c) 2013 Simon Southwell
//
// Date: 13th April 2013
//
// ELF executable reader method for the lm32_cpu class
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
// $Id: lm32_cpu_elf.cpp,v 2.4 2016-09-03 07:44:09 simon Exp $
// $Source: /home/simon/CVS/src/cpu/mico32/src/lm32_cpu_elf.cpp,v $
//
//=============================================================

// -------------------------------------------------------------------------
// INCLUDES
// -------------------------------------------------------------------------

#include <cstdio>
#include <stdint.h>

#include "lm32_cpu.h"
#include "lm32_cpu_elf.h"
#include "lm32_cpu_mico32.h"

void lm32_cpu::read_elf (const char * const filename)
{
    int         i, c;
    uint32_t    pcount, bytecount = 0;
    uint32_t    word;
    pElf32_Ehdr h;
    pElf32_Phdr h2[ELF_MAX_NUM_PHDR];
    char        buf[sizeof(Elf32_Ehdr)];
    char        buf2[sizeof(Elf32_Phdr)*ELF_MAX_NUM_PHDR];
    const char* ptr;
    FILE*       elf_fp;


    // Open program file ready for loading
    if ((elf_fp = fopen(filename, "rb")) == NULL)
    {
        fprintf(stderr, "*** ReadElf(): Unable to open file %s for reading\n", filename); //LCOV_EXCL_LINE
        exit(LM32_USER_ERROR);                                                            //LCOV_EXCL_LINE
    }

    // Read elf header
    h = (pElf32_Ehdr) buf;
    for (i = 0; i < sizeof(Elf32_Ehdr); i++)
    {
        buf[i] = fgetc(elf_fp);
        bytecount++;
        if (buf[i] == EOF) 
        {
            fprintf(stderr, "*** ReadElf(): unexpected EOF\n");                           //LCOV_EXCL_LINE
            exit(LM32_USER_ERROR);                                                        //LCOV_EXCL_LINE
        } 
    }

    //LCOV_EXCL_START
    // Check some things
    ptr= ELF_IDENT;
    for (i = 0; i < 4; i++) 
    {
        if (h->e_ident[i] != ptr[i])
        {
            fprintf(stderr, "*** ReadElf(): not an ELF file\n");
            exit(LM32_USER_ERROR);
        }
    }

    if (SWAPHALF(h->e_type) != ET_EXEC)
    {
        fprintf(stderr, "*** ReadElf(): not an executable ELF file\n");
        exit(LM32_USER_ERROR);
    }

    if (SWAPHALF(h->e_machine) != EM_LATTICEMICO32 && SWAPHALF(h->e_machine) != EM_LATTICEMICO32_OLD)
    {
        fprintf(stderr, "*** ReadElf(): not a Mico32 ELF file\n");
        exit(LM32_USER_ERROR);
    }

    if (SWAPHALF(h->e_phnum) > ELF_MAX_NUM_PHDR)
    {
        fprintf(stderr, "*** ReadElf(): Number of Phdr (%d) exceeds maximum supported (%d)\n", SWAPHALF(h->e_phnum), ELF_MAX_NUM_PHDR);
        exit(LM32_USER_ERROR);
    }
    //LCOV_EXCL_STOP

    // Read program headers
    for (pcount=0 ; pcount < SWAPHALF(h->e_phnum); pcount++)
    {
        for (i = 0; i < sizeof(Elf32_Phdr); i++)
        {
            c = fgetc(elf_fp);
            if (c == EOF)
            {
                fprintf(stderr, "*** ReadElf(): unexpected EOF\n");                         //LCOV_EXCL_LINE
                exit(LM32_USER_ERROR);                                                      //LCOV_EXCL_LINE
            } 
            buf2[i+(pcount * sizeof(Elf32_Phdr))] = c;
            bytecount++;
        }
    }

    // Load text/data segments
    for (pcount=0 ; pcount < SWAPHALF(h->e_phnum); pcount++)
    {
        h2[pcount] = (pElf32_Phdr) &buf2[pcount * sizeof(Elf32_Phdr)];

        // Gobble bytes until section start
        for (; bytecount < SWAP(h2[pcount]->p_offset); bytecount++)
        {
            c = fgetc(elf_fp);
            if (c == EOF) {
                fprintf(stderr, "*** ReadElf(): unexpected EOF\n");                         //LCOV_EXCL_LINE
                exit(LM32_USER_ERROR);                                                      //LCOV_EXCL_LINE
            }
        }

        // Check we can load the segment to memory
        if ((SWAP(h2[pcount]->p_vaddr) + SWAP(h2[pcount]->p_memsz)) >= (1 << MEM_SIZE_BITS))
        {
            fprintf(stderr, "*** ReadElf(): segment memory footprint outside of internal memory range\n"); //LCOV_EXCL_LINE
            exit(LM32_USER_ERROR);                                                                         //LCOV_EXCL_LINE
        }

        // For p_filesz bytes ...
        i = 0;
        word = 0;
        for (; bytecount < (SWAP(h2[pcount]->p_offset) + SWAP(h2[pcount]->p_filesz)); bytecount++)
        {
            if ((c = fgetc(elf_fp)) == EOF)
            {
                fprintf(stderr, "*** ReadElf(): unexpected EOF\n");                         //LCOV_EXCL_LINE
                exit(LM32_USER_ERROR);                                                      //LCOV_EXCL_LINE
            }

            // Big endian
            word |= (c << ((3-(bytecount&3)) * 8));

            if ((bytecount&3) == 3)
            {
                load_mem_word(SWAP(h2[pcount]->p_vaddr) + i, word, SWAP(h2[pcount]->p_flags));
                i+=4;
                word = 0;
            }
        }
    }
}

