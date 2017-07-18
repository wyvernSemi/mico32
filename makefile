##########################################################
# 
# Copyright (c) 2012 Simon Southwell. All rights reserved.
#
# Date: 9th April 2013  
#
# Makefile for C 'cpumico32' instruction set simulator
# 
# This file is part of the cpumico32 instruction set simulator.
#
# cpumico32 is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# cpumico32 is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with cpumico32. If not, see <http://www.gnu.org/licenses/>.
#
# $Id: makefile,v 3.7 2017/07/18 09:33:48 simon Exp $
# $Source: /home/simon/CVS/src/cpu/mico32/makefile,v $
# 
##########################################################

##########################################################
# Definitions
##########################################################

BASENAME=cpumico32
LIBBASENAME=libmico32
LNXTARGET=lnxmico32

OSTYPE:=$(shell uname -o)
OSTYPE_S:=$(shell uname -s)

# If BUILDC is defined (as something---doesn't matter what), then we're
# doing a build of the C top level program.
ifneq ($(BUILDC),)
  TARGET   = ${BASENAME}_c
  COVEXCL  = ${TARGET}.c lm32_cpu_disassembler.cpp lm32_cpu_c.cpp lm32_get_config.cpp
else
  TARGET   = ${BASENAME}
  COVEXCL  = ${TARGET}.cpp lm32_cpu_disassembler.cpp lm32_cpu_c.cpp lm32_get_config.cpp
endif

LIBTARGET  = ${LIBBASENAME}.a
LIBSOTARGET= ${LIBBASENAME}.so

LIBOBJS    = lm32_cpu.o lm32_cpu_inst.o lm32_cpu_elf.o lm32_cpu_disassembler.o lm32_cpu_c.o lm32_cache.o lm32_tlb.o
OBJECTS    = ${TARGET}.o lm32_get_config.o lm32_gdb.o

LCOVINFO   = lm32.info
COVLOGFILE = cov.log
COVDIR     = cov_html

SRCDIR     = ./src
OBJDIR     = ./obj
TESTDIR    = ./test
SIMDIR     = ./HDL/test
SYNTHDIR   = ./HDL/synth/altera

CC         = g++
CC_C       = gcc

# GCC in CYGWIN gives tedious warnings that all code is relocatable, and 
# so -fPIC not required. So shut it up. Also define _GNU_SOURCE for tty code.
ifeq (${OSTYPE}, Cygwin)
  COPTS    = -g -D_GNU_SOURCE -DLM32_MMU
else
  COPTS    = -g -fPIC -DLM32_MMU
endif

ifeq (${OSTYPE_S},windows32)
  TGTDIR   = ../../../msvc/Release
else
  TGTDIR   = ../../../
endif

COVOPTS=
#COVOPTS=-coverage

.SILENT:

##########################################################
# Dependency definitions
##########################################################

all : ${TARGET} ${LNXTARGET}

${OBJDIR}/${TARGET}.o             : ${SRCDIR}/lm32_cpu.h ${SRCDIR}/lm32_cpu_hdr.h

${OBJDIR}/lm32_cpu.o              : ${SRCDIR}/lm32_cpu.h     ${SRCDIR}/lm32_cpu_hdr.h ${SRCDIR}/lm32_cpu_mico32.h ${SRCDIR}/lm32_cpu_elf.h   ${SRCDIR}/lm32_cache.h ${SRCDIR}/lm32_cpu.cpp
${OBJDIR}/lm32_cpu_elf.o          : ${SRCDIR}/lm32_cpu.h     ${SRCDIR}/lm32_cpu_hdr.h ${SRCDIR}/lm32_cpu_mico32.h ${SRCDIR}/lm32_cpu_elf.cpp ${SRCDIR}/lm32_cpu_elf.h
${OBJDIR}/lm32_cpu_inst.o         : ${SRCDIR}/lm32_cpu.h     ${SRCDIR}/lm32_cpu_hdr.h ${SRCDIR}/lm32_cpu_mico32.h ${SRCDIR}/lm32_cpu_inst.cpp
${OBJDIR}/lm32_cache.o            : ${SRCDIR}/lm32_cpu_hdr.h ${SRCDIR}/lm32_cache.cpp ${SRCDIR}/lm32_cache.h
${OBJDIR}/lm32_tlb.o              : ${SRCDIR}/lm32_cpu_hdr.h ${SRCDIR}/lm32_tlb.cpp   ${SRCDIR}/lm32_tlb.h
${OBJDIR}/lm32_cpu_disassembler.o : ${SRCDIR}/lm32_cpu.h     ${SRCDIR}/lm32_cpu_hdr.h ${SRCDIR}/lm32_cpu_mico32.h ${SRCDIR}/lm32_cpu_disassembler.cpp
${OBJDIR}/lm32_cpu_c.o            : ${SRCDIR}/lm32_cpu.h     ${SRCDIR}/lm32_cpu_hdr.h ${SRCDIR}/lm32_cpu.h ${SRCDIR}/lm32_cpu_c.cpp ${SRCDIR}/lm32_cpu_c.h
${OBJDIR}/lm32_gdb.o              : ${SRCDIR}/lm32_cpu.h     ${SRCDIR}/lm32_cpu_hdr.h ${SRCDIR}/lm32_gdb.cpp ${SRCDIR}/lm32_gdb.h

##########################################################
# Compilation rules
##########################################################

ifeq ($(OSTYPE_S),windows32)
${TARGET}:
	runmsbuild.bat ${BASENAME}
else

${TARGET} : ${OBJECTS:%=${OBJDIR}/%} ${LIBTARGET} ${LIBSOTARGET}
	@$(CC) ${OBJECTS:%=${OBJDIR}/%} ${LIBTARGET} ${ARCHOPT} ${COVOPTS} ${LDOPTS} -o ${TARGET}

${LIBTARGET}: ${LIBOBJS:%=${OBJDIR}/%}
	@rm -f ${LIBTARGET}
	@ar rcs $@ ${LIBOBJS:%=${OBJDIR}/%}

${LIBSOTARGET}: ${LIBOBJS:%=${OBJDIR}/%}
	@rm -f ${LIBSOTARGET}
	@$(CC) -shared -Wl,-soname,${LIBSOTARGET} -o ${LIBSOTARGET} ${COVOPTS} ${LIBOBJS:%=${OBJDIR}/%}

${OBJDIR}/%.o : ${SRCDIR}/%.cpp
	@$(CC) ${ARCHOPT} $(COPTS) ${COVOPTS} -c $< -o $@ 

${OBJDIR}/%.o : ${SRCDIR}/%.c
	@$(CC_C) ${ARCHOPT} $(COPTS) ${COVOPTS} -c $< -o $@
endif

${LNXTARGET}:
	@make -f makefile.lnx

##########################################################
# Microsoft Visual C++ 2010
##########################################################

MSVCDIR=./msvc
MSVCCONF="Release"

mscv_dummy:

MSVC:   mscv_dummy
	@MSBuild.exe ${MSVCDIR}/${BASENAME}.sln /nologo /v:q /p:Configuration=${MSVCCONF} /p:OutDir='..\..\'
	@rm -f *.pdb *.ilk

##########################################################
# Test
##########################################################

.PHONY: test sim hwtest

# Rule for running tests on the model
test: ${TARGET} 
	@cd ${TESTDIR}; ./runtest.py -e ${TGTDIR}/${TARGET}

# Rule for running tests on the simulation
sim: 
	@make -C ${SIMDIR} regression

# Rule for running tests on the hardware
hwtest:
	@make -C ${SYNTHDIR} regression

# Rule to run all tests
alltest: test sim hwtest

##########################################################
# coverage
##########################################################

coverage:
	@lcov -c -d ${OBJDIR} -o ${LCOVINFO} > ${COVLOGFILE}
	@lcov -r ${LCOVINFO} ${COVEXCL} -o ${LCOVINFO} >> ${COVLOGFILE}
	@genhtml -o ${COVDIR} ${LCOVINFO} >> ${COVLOGFILE}

##########################################################
# Clean up rules
##########################################################

clean:
	@make -f makefile.lnx clean
	@if [ -d ${SIMDIR} ]; then make -C ${SIMDIR} clean; fi
	@/bin/rm -rf ${TARGET} ${LIBTARGET} ${LIBSOTARGET} \
                 ${OBJDIR}/*.o ${OBJDIR}/*.gc* ${TESTDIR}/instructions/*/test.o \
                 ${TESTDIR}/instructions/*/test.elf ${COVDIR} *.info

cleanmsvc:
	@/bin/rm -rf *.pdb ${MSVCDIR}/*.sdf ${MSVCDIR}/*.suo ${LIBBASENAME}.dll \
                       ${MSVCDIR}/Debug ${MSVCDIR}/Release ${MSVCDIR}/ipch \
                       ${MSVCDIR}/${BASENAME}/Debug \
                       ${MSVCDIR}/${BASENAME}/release \
                       ${MSVCDIR}/${LIBBASENAME}/*.vcxproj.user \
                       ${MSVCDIR}/${LIBBASENAME}/Debug \
                       ${MSVCDIR}/${LIBBASENAME}/release 

sparkle: clean cleanmsvc
	@/bin/rm -f *.exe *.log
