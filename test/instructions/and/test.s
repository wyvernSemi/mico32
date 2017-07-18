# ----------------------------------------------------------------
# Tests AND, ANDI and ANDHI  instructions of the MICO32 processor
# ----------------------------------------------------------------

        .file            "test.s"
        .text
        .align 4
_start: .global _start
        .global main

        .equ FAIL_VALUE,  0x0bad 
        .equ PASS_VALUE,  0x0900d
        .equ RESULT_ADDR, 0xfffc
        .equ SIGN_EXTD16, 0xffff

        .equ TESTVAL1,    0x38753490
        .equ TESTVAL2,    0xa73ef875
        .equ TESTVAL3,    0x00007ea9
        .equ TESTVAL4,    0x08328473

        .equ RESULT1,     0x20343010
        .equ RESULT2,     0x00000421
        .equ RESULT3,     0x08200000

main:
        xor      r0, r0, r0

        # By default, set the result to bad
        ori      r30, r0, 0
        ori      r31, r0, RESULT_ADDR
        sw 	 (r31+0), r30

# -------------- AND tests ---------------

        ori      r12, r0,  TESTVAL1 & 0xffff
        orhi     r12, r12, (TESTVAL1 >> 16) & 0xffff
        ori      r13, r0,  TESTVAL2 & 0xffff
        orhi     r13, r13, (TESTVAL2 >> 16) & 0xffff
        ori      r14, r0,  RESULT1 & 0xffff
        orhi     r14, r14, (RESULT1 >> 16) & 0xffff
        and      r15, r12, r13
        bne      r15, r14, _finish

# -------------- ANDI tests --------------

        ori      r13, r0,  TESTVAL4 & 0xffff
        orhi     r13, r13, (TESTVAL4 >> 16) & 0xffff
        ori      r14, r0,  RESULT2 & 0xffff
        orhi     r14, r14, (RESULT2 >> 16) & 0xffff
        andi     r15, r13, TESTVAL3
        bne      r15, r14, _finish

# -------------- ANDHI tests -------------

        ori      r13, r0,  TESTVAL4 & 0xffff
        orhi     r13, r13, (TESTVAL4 >> 16) & 0xffff
        ori      r14, r0,  RESULT3 & 0xffff
        orhi     r14, r14, (RESULT3 >> 16) & 0xffff
        andhi    r15, r13, TESTVAL3
        bne      r15, r14, _finish

_good:
        ori      r30, r0, PASS_VALUE
        be       r0, r0, _store_result

_finish:
        ori      r30, r0, FAIL_VALUE
_store_result:
        ori      r31, r0, RESULT_ADDR
        sw       (r31+0), r30
_end:
        be       r0, r0, _end
        
        .end

