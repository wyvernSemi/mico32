/* =============================================================
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
// $Id: cpuram.ld,v 1.1 2017/06/29 14:08:26 simon Exp $
// $Source: /home/simon/CVS/src/cpu/mico32/drivers/cpuram.ld,v $
//
//============================================================= */

_uart = 0x80000000;

ENTRY(_start)

MEMORY
{
    rom (rx)  : ORIGIN = 0x00000000, LENGTH = 0x00008000
    ram (rxw) : ORIGIN = 0x00010000, LENGTH = 0x00008000
}

SECTIONS
{
  .boot   : { *(.boot)   } >rom
  .text   : { *(.text)   } >rom
  .rodata : { *(.rodata) } >rom
  .data   : { *(.data) _gp = ALIGN(16) + 0x4000;  } >ram
  .bss    : { _fbss = .; *(.bss) ; . = ALIGN(4); _ebss = .; } >ram
}

PROVIDE(_fstack = ORIGIN(ram) + LENGTH(ram) - 4);
