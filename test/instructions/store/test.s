# ----------------------------------------------------------------
# Tests the store instructions (SB,SH and SW) of the MICO32 
# processor
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

        .equ TESTVAL1,    0x55
        .equ TESTVAL2,    0xa8
        .equ TESTVAL3,    0x13
        .equ TESTVAL4,    0x7c
        .equ TESTVAL5,    0x1234
        .equ TESTVAL6,    0x6789
        .equ TESTVAL7,    0xae200917
        .equ TESTVAL8,    0x23946299

        .equ RESULT1,     0x55000000
        .equ RESULT2,     0x55a80000
        .equ RESULT3,     0x55a81300
        .equ RESULT4,     0x55a8137c
        .equ RESULT5,     0x00001234
        .equ RESULT6,     0x67891234
        .equ RESULT7,     TESTVAL7
        .equ RESULT8,     TESTVAL8

        .data
val1:   .word 0x00000000
val2:   .word 0x00000000
val3:   .word 0x00000000
val4:   .word 0x00000000

        .text

main:
        xor      r0, r0, r0

        # By default, set the result to bad
        ori      r30, r0, 0
        ori      r31, r0, RESULT_ADDR
        sw       (r31+0), r30

# -------------- SB tests ----------------

        # SB with 0 offset
        ori      r10, r0, val1
        ori      r11, r0, TESTVAL1
        sb       (r10 + 0), r11
        lw       r12, (r10 + 0)
        ori      r13, r0, (RESULT1 & 0xffff)
        orhi     r13, r13, (RESULT1 >> 16) & 0xffff
        be       r12, r13, 8
        be       r0, r0, _finish

        # SB with 1 offset
        ori      r10, r0, val1+1
        ori      r11, r0, TESTVAL2
        sb       (r10 + 0), r11
        lw       r12, (r10 + -1)
        ori      r13, r0, (RESULT2 & 0xffff)
        orhi     r13, r13, (RESULT2 >> 16) & 0xffff
        be       r12, r13, 8
        be       r0, r0, _finish

        # SB with 2 offset
        ori      r10, r0, val2
        ori      r11, r0, TESTVAL3
        sb       (r10 + -2), r11
        lw       r12, (r10 + -4)
        ori      r13, r0, (RESULT3 & 0xffff)
        orhi     r13, r13, (RESULT3 >> 16) & 0xffff
        be       r12, r13, 8
        be       r0, r0, _finish

        # SB with 3 offset
        ori      r10, r0, val1
        ori      r11, r0, TESTVAL4
        sb       (r10 + 3), r11
        lw       r12, (r10 + 0)
        ori      r13, r0, (RESULT4 & 0xffff)
        orhi     r13, r13, (RESULT4 >> 16) & 0xffff
        be       r12, r13, 8
        be       r0, r0, _finish

# -------------- SH tests ----------------

        # SH with 2 offset
        ori      r10, r0, val3
        ori      r11, r0, TESTVAL5
        sh       (r10 + -2), r11
        lw       r12, (r10 + -4)
        ori      r13, r0, (RESULT5 & 0xffff)
        orhi     r13, r13, (RESULT5 >> 16) & 0xffff
        be       r12, r13, 8
        be       r0, r0, _finish
        
        # SH with 0 offset
        ori      r10, r0, val2
        ori      r11, r0, TESTVAL6
        sh       (r10 + 0), r11
        lw       r12, (r10 + 0)
        ori      r13, r0, (RESULT6 & 0xffff)
        orhi     r13, r13, (RESULT6 >> 16) & 0xffff
        be       r12, r13, 8
        be       r0, r0, _finish
        
# -------------- SW tests ----------------

        # SW with 0 offset
        ori      r10, r0, val3
        ori      r11, r0, TESTVAL7 & 0xffff
        orhi     r11, r11, (TESTVAL7 >> 16) & 0xffff
        sw       (r10 + 0), r11
        lw       r12, (r10 + 0)
        ori      r13, r0, (RESULT7 & 0xffff)
        orhi     r13, r13, (RESULT7 >> 16) & 0xffff
        be       r12, r13, 8
        be       r0, r0, _finish

        # SW with -4 offset
        ori      r10, r0, val4
        ori      r11, r0, TESTVAL8 & 0xffff
        orhi     r11, r11, (TESTVAL8 >> 16) & 0xffff
        sw       (r10 + -4), r11
        lw       r12, (r10 + -4)
        ori      r13, r0, (RESULT8 & 0xffff)
        orhi     r13, r13, (RESULT8 >> 16) & 0xffff
        be       r12, r13, 8
        be       r0, r0, _finish
        
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

