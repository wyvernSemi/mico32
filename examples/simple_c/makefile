##########################################################
# 
# Copyright (c) 2017 Simon Southwell. All rights reserved.
#
# Date: 12th April 2017  
#
# Makefile for C lnxmico32 ISS test program
# 
# This file is part of the lnxmico32 instruction set simulator.
#
# lnxmico32 is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# lnxmico32 is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with lnxmico32. If not, see <http://www.gnu.org/licenses/>.
#
# $Id: makefile,v 1.3 2017/07/31 14:26:29 simon Exp $
# $Source: /home/simon/CVS/src/cpu/mico32/examples/simple_c/makefile,v $
# 
##########################################################

##########################################################
# Definitions
##########################################################

# Get the type of OS we're running in
OSTYPE = $(shell uname)

ifeq (${OSTYPE}, Linux)
  #
  # As lm32-elf-gcc calls 'as' (as opposed to lm32-elf-as), this clashes with
  # the Linux GNU 'as' if the Lattice Mico GNU tools path isn't first, so
  # locally set it to be the first in the search path. Modify LM32GCCBIN to 
  # local system as required, either here or override on the command line
  #
  LM32GCCBIN   = /usr/local/latticemicosystem/3.8_x64/lm/micosystem/gtools/lm32/lm32-elf/bin

  # Put the LM32 tool chain first in search path
  export PATH := $(LM32GCCBIN):$(PATH)
endif

BASENAME = simple_c

LIBDIR   = ../../drivers
LIBS     = lm32drivers

LDSCRIPT = ${LIBDIR}/lnxram.ld

SRC      = ${BASENAME}.c                                # Program source files
TARGET   = ${BASENAME:%=%.elf}

CC       = lm32-elf-gcc
LD       = lm32-elf-ld

#
# Compile C with symbols for debugging
#
COPTS    = -g

# If compiling for hardware, comment out line below, or redefine LIBOPTS on command line
LIBOPTS = "-DCRT_DISABLE_CPU_RUN_INDICATOR=1"

#
# Tell the assembler that there is a multiplier, divider, sign-extender and shifter
#
ASOPTS   = -mbarrel-shift-enabled -msign-extend-enabled -mmultiply-enabled -mdivide-enabled

#
# Linker options. -whole-archive should be placed at the end
# followed by any static library archives that musn't have
# objects removed.
#
LDOPTS   = -T ${LDSCRIPT} -L${LIBDIR} -whole-archive -llm32drivers

BUILDDIR = obj

# Comment out to see print out of executed  commands
.SILENT:

##########################################################
# Dependency definitions
##########################################################

all: ${BUILDDIR} ${LIBS} ${TARGET}

##########################################################
# Compilation rules
##########################################################

#
# Rule for linking objects to executable. Put ${LDOPTS} last.
#
${TARGET}: ${LDSCRIPT} ${SRC:%.c=${BUILDDIR}/%.o} 
	${LD} ${SRC:%.c=${BUILDDIR}/%.o} -o $@ ${LDOPTS} 

#
# Rule for main program file compilation
#
${BUILDDIR}/%.o: %.c
	${CC} ${COPTS} -Xassembler -a=${@:%.o=%.lst} ${ASOPTS} -I ${LIBDIR} -c $< -o $@

${LIBS}:    
	${MAKE} -C ${LIBDIR} CPPOPTS=${LIBOPTS}
    
.PHONY:  ${BUILDDIR} 
${BUILDDIR}:
	@mkdir -p ${BUILDDIR}

clean:
	rm -rf ${BUILDDIR} *.elf 
	${MAKE} -C ${LIBDIR} clean
