//=============================================================
// 
// Copyright (c) 2017 Simon Southwell. All rights reserved.
//
// Date: 21st April 2017
//
// This file is part of the lnxmico32 instruction set simulator.
//
// The code is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This code is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this code. If not, see <http://www.gnu.org/licenses/>.
//
// $Id: simple_c.c,v 1.1 2017/04/21 15:12:38 simon Exp $
// $Source%$
//
//=============================================================

#include "small_printf.h"

#define NEWLINE 0x0a

// Function to get a decimal number from the input.
static int get_count_ip()
{
    int loops, c;

    sml_printf("Enter loop count: ");

    // Fetch bytes from UART until new line
    while((c = inbyte()) != NEWLINE)
    {
        // Echo user input
        sml_printf("%c", c);

        // Process numerical characters
        if (c >= '0' && c <= '9')
        {
            loops *= 10;
            loops += c - '0';
        }
        // Terminate on non-numerical characters
        else
        {
            break;
        }
    }

    // Whether loop terminated with newline or not, output a new line
    outbyte('\n');

    // Return calculated input value
    return loops;
}


// Example program to (labouriously) add number from 1 to n
main (int argc, char** argv)
{
    static int count = 0;
    int        loops = 0;
    int        idx;

    // Get number of loops from user
    loops = get_count_ip();

    // Add natural numbers from 1 to loops
    for (idx = 1; idx <= loops; idx++)
    {
        count += idx;
    }

    // Print out result.
    sml_printf("n = %d : sigma n = %d\n", loops, count);

    return 0;
}

