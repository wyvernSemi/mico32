# ----------------------------------------------------------------
# Tests shift right instructions of the MICO32 processor
# ----------------------------------------------------------------

        .file   "test.s"
        .text
        .align 4
_start: .global _start
        .global main

        .equ FAIL_VALUE,  0x0bad 
        .equ PASS_VALUE,  0x0900d
        .equ RESULT_ADDR, 0xfffc
        .equ SIGN_EXTD16, 0xffff

        .equ TESTVAL1,    0x7fffffff
        .equ TESTVAL2,    13
        .equ TESTVAL3,    0x80000000
        .equ TESTVAL4,    22

        .equ RESULT1,     0x0003ffff
        .equ RESULT2,     0xfffffe00
        .equ RESULT3,     0x00000200

main:
        xor      r0, r0, r0

        # By default, set the result to bad
        ori      r30, r0, 0
        ori      r31, r0, RESULT_ADDR
        sw       (r31+0), r30

# -------------- SR tests ----------------

        # SR positive
        ori      r1, r0, TESTVAL1 & 0xffff
        orhi     r1, r1, (TESTVAL1 >> 16) & 0xffff
        ori      r2, r0, TESTVAL2
        ori      r3, r0, RESULT1 & 0xffff
        orhi     r3, r3, (RESULT1 >> 16) & 0xffff
        sr       r4, r1, r2
        bne      r4, r3, _finish

        # SR negative
        ori      r1, r0, TESTVAL3 & 0xffff
        orhi     r1, r1, (TESTVAL3 >> 16) & 0xffff
        ori      r2, r0, TESTVAL4
        ori      r3, r0, RESULT2 & 0xffff
        orhi     r3, r3, (RESULT2 >> 16) & 0xffff
        sr       r4, r1, r2
        bne      r4, r3, _finish

# -------------- SRI tests ---------------

        # SRI positive
        ori      r1, r0, TESTVAL1 & 0xffff
        orhi     r1, r1, (TESTVAL1 >> 16) & 0xffff
        ori      r3, r0, RESULT1 & 0xffff
        orhi     r3, r3, (RESULT1 >> 16) & 0xffff
        sri      r4, r1, TESTVAL2
        bne      r4, r3, _finish

        # SRI negative
        ori      r1, r0, TESTVAL3 & 0xffff
        orhi     r1, r1, (TESTVAL3 >> 16) & 0xffff
        ori      r3, r0, RESULT2 & 0xffff
        orhi     r3, r3, (RESULT2 >> 16) & 0xffff
        sri      r4, r1, TESTVAL4
        bne      r4, r3, _finish

# -------------- SRUI tests --------------

        # SRUI positive
        ori      r1, r0, TESTVAL1 & 0xffff
        orhi     r1, r1, (TESTVAL1 >> 16) & 0xffff
        ori      r3, r0, RESULT1 & 0xffff
        orhi     r3, r3, (RESULT1 >> 16) & 0xffff
        srui     r4, r1, TESTVAL2
        bne      r4, r3, _finish

        # SRUI negative
        ori      r1, r0, TESTVAL3 & 0xffff
        orhi     r1, r1, (TESTVAL3 >> 16) & 0xffff
        ori      r3, r0, RESULT3 & 0xffff
        orhi     r3, r3, (RESULT3 >> 16) & 0xffff
        srui     r4, r1, TESTVAL4
        bne      r4, r3, _finish

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

