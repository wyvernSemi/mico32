# ----------------------------------------------------------------
# Tests SETXB and SEXTH  instructions of the MICO32 processor
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

        .equ TESTVAL1,    0xffffff7f
        .equ TESTVAL2,    0x00000080
        .equ TESTVAL3,    0xffff7fff
        .equ TESTVAL4,    0x00008000

        .equ RESULT1,     0x0000007f  
        .equ RESULT2,     0xffffff80
        .equ RESULT3,     0x00007fff  
        .equ RESULT4,     0xffff8000

main:
        xor      r0, r0, r0

        # By default, set the result to bad
        ori      r30, r0, 0
        ori      r31, r0, RESULT_ADDR
        sw       (r31+0), r30

# ------------- SEXTB tests --------------

        # SETXB positive
        ori      r9, r0, TESTVAL1 & 0xffff
        orhi     r9, r9, (TESTVAL1 >> 16) & 0xffff
        ori      r10, r0, RESULT1 & 0xffff
        orhi     r10, r10, (RESULT1 >> 16) & 0xffff
        sextb    r11, r9
        bne      r11, r10, _finish

        # SETXB negative
        ori      r9, r0, TESTVAL2 & 0xffff
        orhi     r9, r9, (TESTVAL2 >> 16) & 0xffff
        ori      r10, r0, RESULT2 & 0xffff
        orhi     r10, r10, (RESULT2 >> 16) & 0xffff
        sextb    r11, r9
        bne      r11, r10, _finish

# ------------- SEXTH tests --------------
        
        # SETXH positive
        ori      r9, r0, TESTVAL3 & 0xffff
        orhi     r9, r9, (TESTVAL3 >> 16) & 0xffff
        ori      r10, r0, RESULT3 & 0xffff
        orhi     r10, r10, (RESULT3 >> 16) & 0xffff
        sexth    r11, r9
        bne      r11, r10, _finish

        # SETXH negative
        ori      r9, r0, TESTVAL4 & 0xffff
        orhi     r9, r9, (TESTVAL4 >> 16) & 0xffff
        ori      r10, r0, RESULT4 & 0xffff
        orhi     r10, r10, (RESULT4 >> 16) & 0xffff
        sexth    r11, r9
        bne      r11, r10, _finish


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

