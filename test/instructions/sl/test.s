# ----------------------------------------------------------------
# Tests shift instructions of the MICO32 processor
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

        .equ TESTVAL1,    0xffffffff
        .equ TESTVAL2,    3
        .equ TESTVAL3,    7
        .equ TESTVAL4,    12
        .equ TESTVAL5,    31
        .equ TESTVAL6,    33

        .equ RESULT1,     0xfffffff8
        .equ RESULT2,     0xffffff80
        .equ RESULT3,     0xfffff000
        .equ RESULT4,     0x80000000
        .equ RESULT5,     0xfffffffe

main:
        xor      r0, r0, r0

        # By default, set the result to bad
        ori      r30, r0, 0
        ori      r31, r0, RESULT_ADDR
        sw       (r31+0), r30

# -------------- SL tests ----------------

        # SL small
        ori      r1, r0, TESTVAL1 & 0xffff
        orhi     r1, r1, (TESTVAL1 >> 16) & 0xffff
        ori      r2, r0, TESTVAL2
        ori      r3, r0, RESULT1 & 0xffff
        orhi     r3, r3, (RESULT1 >> 16) & 0xffff
        sl       r4, r1, r2
        bne      r4, r3, _finish

        # SL medium
        ori      r1, r0, TESTVAL1 & 0xffff
        orhi     r1, r1, (TESTVAL1 >> 16) & 0xffff
        ori      r2, r0, TESTVAL3
        ori      r3, r0, RESULT2 & 0xffff
        orhi     r3, r3, (RESULT2 >> 16) & 0xffff
        sl       r4, r1, r2
        bne      r4, r3, _finish

        # SL medium
        ori      r1, r0, TESTVAL1 & 0xffff
        orhi     r1, r1, (TESTVAL1 >> 16) & 0xffff
        ori      r2, r0, TESTVAL4
        ori      r3, r0, RESULT3 & 0xffff
        orhi     r3, r3, (RESULT3 >> 16) & 0xffff
        sl       r4, r1, r2
        bne      r4, r3, _finish

        # SL large
        ori      r1, r0, TESTVAL1 & 0xffff
        orhi     r1, r1, (TESTVAL1 >> 16) & 0xffff
        ori      r2, r0, TESTVAL5
        ori      r3, r0, RESULT4 & 0xffff
        orhi     r3, r3, (RESULT4 >> 16) & 0xffff
        sl       r4, r1, r2
        bne      r4, r3, _finish

        # SL wrap
        ori      r1, r0, TESTVAL1 & 0xffff
        orhi     r1, r1, (TESTVAL1 >> 16) & 0xffff
        ori      r2, r0, TESTVAL6
        ori      r3, r0, RESULT5 & 0xffff
        orhi     r3, r3, (RESULT5 >> 16) & 0xffff
        sl       r4, r1, r2
        bne      r4, r3, _finish

# -------------- SLI tests ---------------

        # SLI small
        ori      r1, r0, TESTVAL1 & 0xffff
        orhi     r1, r1, (TESTVAL1 >> 16) & 0xffff
        ori      r3, r0, RESULT1 & 0xffff
        orhi     r3, r3, (RESULT1 >> 16) & 0xffff
        sli      r4, r1, TESTVAL2
        bne      r4, r3, _finish

        # SL medium
        ori      r1, r0, TESTVAL1 & 0xffff
        orhi     r1, r1, (TESTVAL1 >> 16) & 0xffff
        ori      r3, r0, RESULT2 & 0xffff
        orhi     r3, r3, (RESULT2 >> 16) & 0xffff
        sli      r4, r1, TESTVAL3
        bne      r4, r3, _finish

        # SL medium
        ori      r1, r0, TESTVAL1 & 0xffff
        orhi     r1, r1, (TESTVAL1 >> 16) & 0xffff
        ori      r3, r0, RESULT3 & 0xffff
        orhi     r3, r3, (RESULT3 >> 16) & 0xffff
        sli      r4, r1, TESTVAL4
        bne      r4, r3, _finish

        # SL large
        ori      r1, r0, TESTVAL1 & 0xffff
        orhi     r1, r1, (TESTVAL1 >> 16) & 0xffff
        ori      r3, r0, RESULT4 & 0xffff
        orhi     r3, r3, (RESULT4 >> 16) & 0xffff
        sli      r4, r1, TESTVAL5
        bne      r4, r3, _finish

        # SL wrap
        ori      r1, r0, TESTVAL1 & 0xffff
        orhi     r1, r1, (TESTVAL1 >> 16) & 0xffff
        ori      r3, r0, RESULT5 & 0xffff
        orhi     r3, r3, (RESULT5 >> 16) & 0xffff
        sli      r4, r1, TESTVAL6
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

