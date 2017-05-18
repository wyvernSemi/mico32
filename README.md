# mico32

LatticeMico32 instruction set simulator project

Includes a C/C++ model of the lm32 CPU, with the follwoing features:

* All supported core instructions
* All h/w modelled for configurable instructions
  - Multiplier
  - Divider
  - Sign extender
  - Barrel shifter
* Configurable internal memory
* All h/w debug break- and watchpoints modelled
* Cycle count functionality
* Configurable 'hardware', as per the Mico32
* Run-time and static disassembly
* Data and Instruction caches for timing model
* Optional MMU capability, via Data and Instruction TLBs
* Extensibility via callbacks
  - Intercept memory accesses
  - Regular callback with ability for external interrupt generation
  - JTAG register access callback
* Configurable user defined execution break points
  - On a given address
  - After a single step, or clock tick
  - After a fixed number of cycles
  - On 'hardware' debug break point
* Access to internal Memory
* Access to internal state
* Compatible with Lattice Semiconductor GNU tool chain (lm32-elf-xx)
* Both C++ and C linkage interfaces available
* Separate GDB remote debug interface code is included
  - Supports both virtual serial and TCP socket connections

There are build options for MSVC 2010, CYGWIN and Linux, and an embedded Linux system model case study. A Python3 GUI is also bundled for configuring and running cpumico32, and a GDB remote target interface is available for non-intrusive debug.

simon@anita-simulators.org.uk

www.anita-simulators.org.uk/wyvernsemi
