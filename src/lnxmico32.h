//=============================================================
// 
// Copyright (c) 2016 Simon Southwell. All rights reserved.
//
// Date: 24th August 2016  
//
// Top level C++ lnxmico32 executable main() entry point header. 
//
// This file is part of the lnxmico32 ISS linux system model.
//
// lnxmico32 is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// lnxmico32 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with lnxmico32. If not, see <http://www.gnu.org/licenses/>.
//
// $Id: lnxmico32.h,v 3.1 2016-09-15 18:11:12 simon Exp $
// $Source: /home/simon/CVS/src/cpu/mico32/src/lnxmico32.h,v $
//
//=============================================================

#ifndef _LNXMICO32_H_
#define _LNXMICO32_H_

// -------------------------------------------------------------------------
// DEFINES
// -------------------------------------------------------------------------

#define LM32_RAM_BASE_ADDR              0x08000000U
#define LM32_UART0_BASE_ADDR            0x80000000U
#define LM32_UART1_BASE_ADDR            0x81000000U
#define LM32_TIMER0_BASE_ADDR           0x80002000U

#define LM32_PAGE_SIZE                  0x00001000U
#define LM32_PAGE_MASK                  (LM32_PAGE_SIZE - 1)

#define LM32_INIT_RD_BASE_ADDR          0x08400000U
#define LM32_CMDLINE_BASE_ADDR          0x0BFFF000U
#define LM32_HWSETUP_BASE_ADDR          0x0BFFE000U

#define LM32_KERNEL_BASE_ADDR           LM32_RAM_BASE_ADDR

#define LM32_RAM_SIZE                   (64 * 1024 * 1024)
#define LM32_RAM_MASK                   (LM32_RAM_SIZE - 1)
#define LM32_PERIPH_MASK                0xf0000000

#define LM32_CPU_FREQUENCY_MHZ          10
#define LM32_CPU_FREQUENCY_HZ           (LM32_CPU_FREQUENCY_MHZ * 1000 * 1000)

#define LM32_INTERRUPT_GRANULARITY      10000

#define LM32_CMDLINE_STR                "root=/dev/ram0 console=ttyS0,115200 ramdisk_size=16384"
#define LM32_VM_LINUX_FNAME             "vmlinux.bin"
#define LM32_FILE_SYS_FNAME             "romfs.ext2"

#define LM32_UART_BAUD_RATE             115200
#ifndef LM32_FAST_COMPILE
#define LM32_UART_TICKS_PER_BIT         ((LM32_CPU_FREQUENCY_HZ/LM32_UART_BAUD_RATE) * 11)
#else
#define LM32_UART_TICKS_PER_BIT         1
#endif

#define LM32_UART0_IRQ                  0
#define LM32_TIMER0_IRQ                 1
#define LM32_UART1_IRQ                  2

#define LM32_HW_TAG_EOL                 0
#define LM32_HW_TAG_CPU                 1
#define LM32_HW_TAG_DDR_SDRAM           6
#define LM32_HW_TAG_TIMER               8
#define LM32_HW_TAG_UART                9

#define LM32_CPU_CONFIG_LEN            44
#define LM32_DDR_CONFIG_LEN            48
#define LM32_TIM_CONFIG_LEN            56
#define LM32_URT_CONFIG_LEN            56
#define LM32_TRL_CONFIG_LEN             8

#define LM32_UART0_CNTX                 0
#define LM32_UART1_CNTX                 1

#define LM32_KEYBOARD_CONNECTED         true
#define LM32_KEYBOARD_NOT_CONNECTED     false

#endif
