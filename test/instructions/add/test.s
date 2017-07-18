# ----------------------------------------------------------------
# Tests ADD and ADDI instructions of the MICO32 processor
# ----------------------------------------------------------------

        .file   "test.s"
        .text
        .align 4
_start: .global _start
        .global main

        .equ FAIL_VALUE,  0x0bad 
        .equ PASS_VALUE,  0x0900d
        .equ RESULT_ADDR, 0xfffc

        .equ TEST_VAL1,   55
        .equ TEST_VAL2,   66
        .equ TEST_VAL3,  -55
        .equ TEST_VAL4,  -66
        .equ TEST_VAL5,  -64
        .equ TEST_VAL6,   1234
        .equ TEST_VAL7,   789
        .equ TEST_VAL8,  -789
        .equ TEST_VAL9,  -251
        .equ TEST_VAL10, -1234


        .equ RESULT_1,    121
        .equ RESULT_2,    11
        .equ RESULT_3,    2
        .equ RESULT_4,   -11
        .equ RESULT_5,    2023
        .equ RESULT_6,    445
        .equ RESULT_7,    538
        .equ RESULT_8,   -445

main:
        xor      r0, r0, r0

        # By default, set the result to fail
        ori      r30, r0, 0
        ori      r31, r0, RESULT_ADDR
        sw       (r31+0), r30

# -------------- ADD tests ---------------

        # test positive ADD with non-wrap (RESULT_1 = TEST_VAL1 + TESTVAL2)
        ori      r1, r0, TEST_VAL1
        ori      r2, r0, TEST_VAL2
        ori      r4, r0, RESULT_1
        add      r3, r1, r2
        be       r3, r4, _ok1
        be       r0, r0, _finish
_ok1:
       # test negative ADD with non-wrap (RESULT_2 = TEST_VAL2 + TESTVAL3)
        ori      r1, r0, TEST_VAL3
        sexth    r1, r1                         # Sign extend
        ori      r2, r0, TEST_VAL2
        ori      r4, r0, RESULT_2
        add      r3, r2, r1
        be       r3, r4, _ok2
        be       r0, r0, _finish
_ok2:       
        # test positive ADD with wrap (RESULT_3 = TEST_VAL5 + TESTVAL2)
        ori      r1, r0, TEST_VAL5
        sexth    r1, r1                         # Sign extend
        ori      r2, r0, TEST_VAL2
        ori      r4, r0, 2
        add      r3, r1, r2
        be       r3, r4, _ok3
        be       r0, r0, _finish
_ok3:
        # test negative ADD with wrap ((RESULT_4 = TEST_VAL4 + TESTVAL1)
        ori      r1, r0, TEST_VAL1
        ori      r2, r0, TEST_VAL4
        sexth    r2, r2                         # Sign extend
        ori      r4, r0, RESULT_4
        sexth    r4, r4
        add      r3, r2, r1
        be       r3, r4, _ok4
        be       r0, r0, _finish
_ok4:

# -------------- ADDI tests --------------

        # test positive ADDI with non-overflow
        ori      r1, r0, TEST_VAL6
        ori      r2, r0, RESULT_5
        addi     r3, r1, TEST_VAL7
        be       r3, r2, _ok5
        be       r0, r0, _finish
_ok5:
       # test negative ADDI with non-overflow
        ori      r1, r0, TEST_VAL8
        sexth    r1, r1                         # Sign extend
        ori      r2, r0, RESULT_6
        addi     r3, r1, TEST_VAL6
        be       r3, r2, _ok6
        be       r0, r0, _finish
_ok6:       
        # test positive ADDI with overflow
        ori      r1, r0, TEST_VAL9
        sexth    r1, r1                         # Sign extend
        ori      r2, r0, RESULT_7
        addi     r3, r1, TEST_VAL7
        be       r3, r2, _ok7
        be       r0, r0, _finish
_ok7:
        # test negative ADDI with overflow
        ori      r1, r0, TEST_VAL7
        ori      r2, r0, RESULT_8
        sexth    r2, r2                         # Sign extend
        addi     r3, r1, TEST_VAL10
        be       r3, r2, _ok8
        be       r0, r0, _finish
_ok8:


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
