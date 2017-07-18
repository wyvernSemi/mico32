# ----------------------------------------------------------------
# Tests XOR/XNOR instructions of the MICO32 processor
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

        .equ TESTVAL1,    0x3287982a
        .equ TESTVAL2,    0x87593e50
        .equ TESTVAL3,    0x0000ffff

        .equ RESULT1,     0xb5dea67a
        .equ RESULT2,     0x8759c1af
        .equ RESULT3,     0x4a215985
        .equ RESULT4,     0x78a63e50

main:
        xor      r0, r0, r0

        # By default, set the result to bad
        ori      r30, r0, 0
        ori      r31, r0, RESULT_ADDR
        sw       (r31+0), r30

# -------------- XOR tests ---------------

        ori      r12, r0,  TESTVAL1 & 0xffff
        orhi     r12, r12, (TESTVAL1 >> 16) & 0xffff
        ori      r13, r0,  TESTVAL2 & 0xffff
        orhi     r13, r13, (TESTVAL2 >> 16) & 0xffff
        ori      r14, r0,  RESULT1 & 0xffff
        orhi     r14, r14, (RESULT1 >> 16) & 0xffff
        xor      r15, r12, r13
        bne      r15, r14, _finish

# -------------- XORI tests ---------------

        ori      r13, r0,  TESTVAL2 & 0xffff
        orhi     r13, r13, (TESTVAL2 >> 16) & 0xffff
        ori      r14, r0,  RESULT2 & 0xffff
        orhi     r14, r14, (RESULT2 >> 16) & 0xffff
        xori     r15, r13, TESTVAL3
        bne      r15, r14, _finish

# -------------- XNOR tests ---------------

        ori      r12, r0,  TESTVAL1 & 0xffff
        orhi     r12, r12, (TESTVAL1 >> 16) & 0xffff
        ori      r13, r0,  TESTVAL2 & 0xffff
        orhi     r13, r13, (TESTVAL2 >> 16) & 0xffff
        ori      r14, r0,  RESULT3 & 0xffff
        orhi     r14, r14, (RESULT3 >> 16) & 0xffff
        xnor     r15, r12, r13
        bne      r15, r14, _finish

# -------------- XNORI tests --------------

        ori      r13, r0,  TESTVAL2 & 0xffff
        orhi     r13, r13, (TESTVAL2 >> 16) & 0xffff
        ori      r14, r0,  RESULT4 & 0xffff
        orhi     r14, r14, (RESULT4 >> 16) & 0xffff
        xnori    r15, r13, TESTVAL3
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

