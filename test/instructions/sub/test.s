# ----------------------------------------------------------------
# Tests SUB instruction of the MICO32 processor
# ----------------------------------------------------------------

        .file   "test.s"
        .text
        .align 4
_start: .global _start
        .global main

        .equ FAIL_VALUE,  0x0bad 
        .equ PASS_VALUE,  0x0900d
        .equ RESULT_ADDR, 0xfffc

        .equ TEST_VAL1,   801
        .equ TEST_VAL2,   577
        .equ TEST_VAL3,  -55
        .equ TEST_VAL4,  -66
        .equ TEST_VAL5,  164
        .equ TEST_VAL6,  -2


        .equ RESULT_1,    224
        .equ RESULT_2,    632
        .equ RESULT_3,   -413
        .equ RESULT_4,    53

main:
        xor      r0, r0, r0

        # By default, set the result to fail
        ori      r30, r0, 0
        ori      r31, r0, RESULT_ADDR
        sw       (r31+0), r30

# -------------- SUB tests ---------------

        # test positive SUB with non-wrap (RESULT_1 = TEST_VAL1 - TESTVAL2)
        ori      r1, r0, TEST_VAL1
        ori      r2, r0, TEST_VAL2
        ori      r4, r0, RESULT_1
        sub      r3, r1, r2
        be       r3, r4, _ok1
        be       r0, r0, _finish
_ok1:
       # test negative SUB with non-wrap (RESULT_2 = TEST_VAL2 - TESTVAL3)
        ori      r1, r0, TEST_VAL3
        sexth    r1, r1                         # Sign extend
        ori      r2, r0, TEST_VAL2
        ori      r4, r0, RESULT_2
        sub      r3, r2, r1
        be       r3, r4, _ok2
        be       r0, r0, _finish
_ok2:       
        # test positive SUB with wrap (RESULT_3 = TEST_VAL5 - TESTVAL2)
        ori      r1, r0, TEST_VAL5
        sexth    r1, r1                         # Sign extend
        ori      r2, r0, TEST_VAL2
        ori      r4, r0, RESULT_3
        sexth    r4, r4                         # Sign extend
        sub      r3, r1, r2
        be       r3, r4, _ok3
        be       r0, r0, _finish
_ok3:
        # test negative SUB with wrap ((RESULT_4 = TEST_VAL6 - TESTVAL3)
        ori      r1, r0, TEST_VAL6
        sexth    r1, r1                         # Sign extend
        ori      r2, r0, TEST_VAL3
        sexth    r2, r2                         # Sign extend
        ori      r4, r0, RESULT_4
        sexth    r4, r4
        sub      r3, r1, r2
        be       r3, r4, _ok4
        be       r0, r0, _finish
_ok4:


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
