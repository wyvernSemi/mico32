# ----------------------------------------------------------------
# Tests ???  instructions of the MICO32 processor
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

        .equ TESTVAL1,    0x1234
        .equ TESTVAL2,    0x5678
        .equ TESTVAL3,    0x12345678
        .equ TESTVAL4,    0x76543210
        .equ TESTVAL5,    0x87654321
        .equ TESTVAL6,    0x90ddebaf
        .equ TESTVAL7,    0x00017012
        .equ TESTVAL8,    0x00002a07
        .equ TESTVAL9,    0x6007a401
        .equ TESTVAL10,   0x00003001
        .equ TESTVAL11,   0x000ab001
        .equ TESTVAL12,  -0x000000ff
        .equ TESTVAL13,   0x050ab001
        .equ TESTVAL14,  -0x00001fff
        .equ TESTVAL15,   0x89abcdef
        .equ TESTVAL16,   0x00000001
        .equ TESTVAL17,   0xf08a111e
        .equ TESTVAL18,   0x00007890

        .equ RESULT1,     0x06260060
        .equ RESULT2,     0x0b88d780
        .equ RESULT3,     0x06552e8f
        .equ RESULT4,     0x3c6d047e
        .equ RESULT5,     0xcec7d401
        .equ RESULT6,     0xf55aaf01
        .equ RESULT7,     0xaf0a9001
        .equ RESULT8,     0x89abcdef
        .equ RESULT9,     0x05afb0e0

main:
        xor      r0, r0, r0

        # By default, set the result to bad
        ori      r30, r0, 0
        ori      r31, r0, RESULT_ADDR
        sw       (r31+0), r30

# -------------- MUL tests ---------------

        # MUL positive no overflow
        ori      r1, r0, TESTVAL1 & 0xffff
        orhi     r1, r1, (TESTVAL1 >> 16) & 0xffff
        ori      r2, r0, TESTVAL2 & 0xffff
        orhi     r2, r2, (TESTVAL2 >> 16) & 0xffff
        ori      r3, r0, RESULT1 & 0xffff
        orhi     r3, r3, (RESULT1 >> 16) & 0xffff
        mul      r4, r1, r2
        bne      r4, r3, _finish

        # MUL positive overflow
        ori      r1, r0, TESTVAL3 & 0xffff
        orhi     r1, r1, (TESTVAL3 >> 16) & 0xffff
        ori      r2, r0, TESTVAL4 & 0xffff
        orhi     r2, r2, (TESTVAL4 >> 16) & 0xffff
        ori      r3, r0, RESULT2 & 0xffff
        orhi     r3, r3, (RESULT2 >> 16) & 0xffff
        mul      r4, r1, r2
        bne      r4, r3, _finish

        # MUL negative overflow
        ori      r1, r0, TESTVAL5 & 0xffff
        orhi     r1, r1, (TESTVAL5 >> 16) & 0xffff
        ori      r2, r0, TESTVAL6 & 0xffff
        orhi     r2, r2, (TESTVAL6 >> 16) & 0xffff
        ori      r3, r0, RESULT3 & 0xffff
        orhi     r3, r3, (RESULT3 >> 16) & 0xffff
        mul      r4, r1, r2
        bne      r4, r3, _finish

# -------------- MULI tests --------------

        # MULI positive no overflow
        ori      r1, r0, TESTVAL7 & 0xffff
        orhi     r1, r1, (TESTVAL7 >> 16) & 0xffff
        ori      r3, r0, RESULT4 & 0xffff
        orhi     r3, r3, (RESULT4 >> 16) & 0xffff
        muli     r4, r1, TESTVAL8
        bne      r4, r3, _finish

        # MULI positive overflow
        ori      r1, r0, TESTVAL9 & 0xffff
        orhi     r1, r1, (TESTVAL9 >> 16) & 0xffff
        ori      r3, r0, RESULT5 & 0xffff
        orhi     r3, r3, (RESULT5 >> 16) & 0xffff
        muli     r4, r1, TESTVAL10
        bne      r4, r3, _finish

        # MULI positive reg, negative imm, no overflow
        ori      r1, r0, TESTVAL11 & 0xffff
        orhi     r1, r1, (TESTVAL11 >> 16) & 0xffff
        ori      r3, r0, RESULT6 & 0xffff
        orhi     r3, r3, (RESULT6 >> 16) & 0xffff
        muli     r4, r1, TESTVAL12
        bne      r4, r3, _finish

        # MULI positive reg, negative imm, overflow
        ori      r1, r0, TESTVAL13 & 0xffff
        orhi     r1, r1, (TESTVAL13 >> 16) & 0xffff
        ori      r3, r0, RESULT7 & 0xffff
        orhi     r3, r3, (RESULT7 >> 16) & 0xffff
        muli     r4, r1, TESTVAL14
        bne      r4, r3, _finish

        # MULI negative reg, positive imm, no overflow
        ori      r1, r0, TESTVAL15 & 0xffff
        orhi     r1, r1, (TESTVAL15 >> 16) & 0xffff
        ori      r3, r0, RESULT8 & 0xffff
        orhi     r3, r3, (RESULT8 >> 16) & 0xffff
        muli     r4, r1, TESTVAL16
        bne      r4, r3, _finish

        # MULI negative reg, positive imm, overflow1G
        ori      r1, r0, TESTVAL17 & 0xffff
        orhi     r1, r1, (TESTVAL17 >> 16) & 0xffff
        ori      r3, r0, RESULT9 & 0xffff
        orhi     r3, r3, (RESULT9 >> 16) & 0xffff
        muli     r4, r1, TESTVAL18
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

