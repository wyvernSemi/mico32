# ----------------------------------------------------------------
# Tests OR/NOR instructions of the MICO32 processor
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

        .equ TESTVAL1,    0x5555eeee
        .equ TESTVAL2,    0x5a5a1010
        .equ TESTVAL3,    0x00003333
        .equ TESTVAL4,    0x12345678

        .equ RESULT1,     0x5f5ffefe
        .equ RESULT2,     0x1234777b
        .equ RESULT3,     0x33375678
        .equ RESULT4,     0xa0a00101
        .equ RESULT5,     0xedcb8884

main:
        xor      r0, r0, r0

        # By default, set the result to bad
        ori      r30, r0, 0
        ori      r31, r0, RESULT_ADDR
        sw       (r31+0), r30

# -------------- OR tests ----------------

        ori      r12, r0,  TESTVAL1 & 0xffff
        orhi     r12, r12, (TESTVAL1 >> 16) & 0xffff
        ori      r13, r0,  TESTVAL2 & 0xffff
        orhi     r13, r13, (TESTVAL2 >> 16) & 0xffff
        ori      r14, r0,  RESULT1 & 0xffff
        orhi     r14, r14, (RESULT1 >> 16) & 0xffff
        or       r15, r12, r13
        bne      r15, r14, _finish

# -------------- ORI tests ---------------

        ori      r13, r0,  TESTVAL4 & 0xffff
        orhi     r13, r13, (TESTVAL4 >> 16) & 0xffff
        ori      r14, r0,  RESULT2 & 0xffff
        orhi     r14, r14, (RESULT2 >> 16) & 0xffff
        ori      r15, r13, TESTVAL3
        bne      r15, r14, _finish

# -------------- ORHI tests --------------

        ori      r13, r0,  TESTVAL4 & 0xffff
        orhi     r13, r13, (TESTVAL4 >> 16) & 0xffff
        ori      r14, r0,  RESULT3 & 0xffff
        orhi     r14, r14, (RESULT3 >> 16) & 0xffff
        orhi     r15, r13, TESTVAL3
        bne      r15, r14, _finish

# -------------- NOR tests ---------------

        ori      r12, r0,  TESTVAL1 & 0xffff
        orhi     r12, r12, (TESTVAL1 >> 16) & 0xffff
        ori      r13, r0,  TESTVAL2 & 0xffff
        orhi     r13, r13, (TESTVAL2 >> 16) & 0xffff
        ori      r14, r0,  RESULT4 & 0xffff
        orhi     r14, r14, (RESULT4 >> 16) & 0xffff
        nor      r15, r12, r13
        bne      r15, r14, _finish

# -------------- NORI tests --------------

        ori      r13, r0,  TESTVAL4 & 0xffff
        orhi     r13, r13, (TESTVAL4 >> 16) & 0xffff
        ori      r14, r0,  RESULT5 & 0xffff
        orhi     r14, r14, (RESULT5 >> 16) & 0xffff
        nori     r15, r13, TESTVAL3
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

