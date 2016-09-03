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

main:
        xor      r0, r0, r0

	# By default, set the result to bad
        ori      r30, r0, FAIL_VALUE
        ori      r31, r0, RESULT_ADDR
        sw 	 (r31+0), r30

        ori      r10, r0, 0x0000
        orhi     r10, r10, 0x8000
	ori      r2, r0, -1
	ori      r2, r0, 1
_loop1:
        add      r3, r1, r2
	bne      r0, r0, _label1
_label1:
	sri      r3, r1, 1
	#muli     r3, r1, 1
	#divu     r1, r1, r1
	rcsr     r4, CC
	bgeu     r10, r4, _loop1

_good:
        ori      r30, r0, PASS_VALUE

_finish:
        ori      r31, r0, RESULT_ADDR
        sw 	 (r31+0), r30
_end:
	be       r0, r0, _end

	.end

