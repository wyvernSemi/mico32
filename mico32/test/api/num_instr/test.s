# ----------------------------------------------------------------
# Tests ???  instructions of the MICO32 processor
# ----------------------------------------------------------------

  	.file	"test.s"
	.text
	.align 4
_start:	.global	_start
        .global main

	.equ FAIL_VALUE,  0x0bad 
	.equ PASS_VALUE,  0x0900d
	.equ RESULT_ADDR, 0xfffc
	.equ SIGN_EXTD16, 0xffff

        .equ COMMS_BASE_ADDRESS,        0x20000000
        .equ COMMS_INSTR_LO_OFFSET,     0x0000002c
        .equ COMMS_INSTR_HI_OFFSET,     0x00000030
        .equ COMMS_TICK_EXEC_OFFSET,    0x00000034


main:
        xor      r0, r0, r0

	# By default, set the result to bad
        ori      r30, r0, FAIL_VALUE
        ori      r31, r0, RESULT_ADDR
        sw 	 (r31+0), r30

        # Set r1 to be the comms peripheral base address
        orhi     r1, r0, (COMMS_BASE_ADDRESS>>16) & 0xffff

        # Get current instruction count
        lw       r2, (r1+COMMS_INSTR_LO_OFFSET)

        # Execute a number of instrcutions
        nop
        nop
        nop

        # Get the new value of the instrcution count
        lw       r3, (r1+COMMS_INSTR_LO_OFFSET)
        addi     r4, r2, 4
        bne      r3, r4, _finish


_good:
        ori      r30, r0, PASS_VALUE

_finish:
        ori      r31, r0, RESULT_ADDR
        sw 	 (r31+0), r30
_end:
	be       r0, r0, _end

	.end

