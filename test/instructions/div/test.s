# ----------------------------------------------------------------
# Tests divide and modulus instruction of the MICO32 processor
# ----------------------------------------------------------------

        .file   "test.s"
        .text
        .align 4
_start: .global _start
        .global main

        .equ FAIL_VALUE,  0x0bad 
        .equ PASS_VALUE,  0x0900d
        .equ RESULT_ADDR, 0xfffc

        .equ TESTVAL1,    0x73872947
        .equ TESTVAL2,    0x00020098
        .equ TESTVAL3,    0x56000278
        .equ TESTVAL4,    0xffff8989
        .equ TESTVAL5,    0xfffffab0
        .equ TESTVAL6,    0x000000f6
        .equ TESTVAL7,    0x00034728
        .equ TESTVAL8,    0x00034729

        .equ TESTVAL11,   0x500a91d8
        .equ TESTVAL12,   0x001a8027
        .equ TESTVAL13,   0xf08ab301
        .equ TESTVAL14,   0x000078a5
        .equ TESTVAL15,   0xf08ab301
        .equ TESTVAL16,   0xf08ab302

        .equ RESULT1,     0x000039b2
        .equ RESULT2,     0xffff4628
        .equ RESULT3,     0xfffffffb
        .equ RESULT4,     0x00000000

        .equ RESULT11,    0x00000305
        .equ RESULT12,    0x0001fe6a
        .equ RESULT13,    0x00000000

        .equ RESULT21,    0x0000e797
        .equ RESULT22,    0x00000f10
        .equ RESULT23,    0xffffff7e
        .equ RESULT24,    0x00034728

        .equ RESULT33,    0x0000001e

main:
        xor      r0, r0, r0

        # By default, set the result to fail
        ori      r30, r0, 0
        ori      r31, r0, RESULT_ADDR
        sw       (r31+0), r30

# -------------- DIV tests ---------------

#ifdef LM32_ENABLE_DIV_INSTR
        # DIV positive numbers, no underflow
        ori      r1, r0, TESTVAL1 & 0xffff
        orhi     r1, r1, (TESTVAL1 >> 16) & 0xffff
        ori      r2, r0, TESTVAL2 & 0xffff
        orhi     r2, r2, (TESTVAL2 >> 16) & 0xffff
        ori      r3, r0, RESULT1 & 0xffff
        orhi     r3, r3, (RESULT1 >> 16) & 0xffff
        #div      r4, r1, r2
        .word    0x9c222000
        bne      r4, r3, _finish

        # DIV pos/neg numbers, no underflow
        ori      r1, r0, TESTVAL3 & 0xffff
        orhi     r1, r1, (TESTVAL3 >> 16) & 0xffff
        ori      r2, r0, TESTVAL4 & 0xffff
        orhi     r2, r2, (TESTVAL4 >> 16) & 0xffff
        ori      r3, r0, RESULT2 & 0xffff
        orhi     r3, r3, (RESULT2 >> 16) & 0xffff
        #div      r4, r1, r2
        .word    0x9c222000
        bne      r4, r3, _finish

        # DIV neg/pos numbers, no underflow
        ori      r1, r0, TESTVAL5 & 0xffff
        orhi     r1, r1, (TESTVAL5 >> 16) & 0xffff
        ori      r2, r0, TESTVAL6 & 0xffff
        orhi     r2, r2, (TESTVAL6 >> 16) & 0xffff
        ori      r3, r0, RESULT3 & 0xffff
        orhi     r3, r3, (RESULT3 >> 16) & 0xffff
        #div      r4, r1, r2
        .word    0x9c222000
        bne      r4, r3, _finish

        # DIV underflow
        ori      r1, r0, TESTVAL7 & 0xffff
        orhi     r1, r1, (TESTVAL7 >> 16) & 0xffff
        ori      r2, r0, TESTVAL8 & 0xffff
        orhi     r2, r2, (TESTVAL8 >> 16) & 0xffff
        ori      r3, r0, RESULT4 & 0xffff
        orhi     r3, r3, (RESULT4 >> 16) & 0xffff
        #div      r4, r1, r2
        .word    0x9c222000
        bne      r4, r3, _finish
#endif

        # DIVU positive numbers, no underflow
        ori      r1, r0, TESTVAL11 & 0xffff
        orhi     r1, r1, (TESTVAL11 >> 16) & 0xffff
        ori      r2, r0, TESTVAL12 & 0xffff
        orhi     r2, r2, (TESTVAL12 >> 16) & 0xffff
        ori      r3, r0, RESULT11 & 0xffff
        orhi     r3, r3, (RESULT11 >> 16) & 0xffff
        divu     r4, r1, r2
        bne      r4, r3, _finish

        # DIVU neg/pos numbers, no underflow
        ori      r1, r0, TESTVAL13 & 0xffff
        orhi     r1, r1, (TESTVAL13 >> 16) & 0xffff
        ori      r2, r0, TESTVAL14 & 0xffff
        orhi     r2, r2, (TESTVAL14 >> 16) & 0xffff
        ori      r3, r0, RESULT12 & 0xffff
        orhi     r3, r3, (RESULT12 >> 16) & 0xffff
        divu     r4, r1, r2
        bne      r4, r3, _finish

        # DIVU underflow
        ori      r1, r0, TESTVAL15 & 0xffff
        orhi     r1, r1, (TESTVAL15 >> 16) & 0xffff
        ori      r2, r0, TESTVAL16 & 0xffff
        orhi     r2, r2, (TESTVAL16 >> 16) & 0xffff
        ori      r3, r0, RESULT13 & 0xffff
        orhi     r3, r3, (RESULT13 >> 16) & 0xffff
        divu     r4, r1, r2
        bne      r4, r3, _finish

#ifdef LM32_ENABLE_DIV_INSTR
        # MOD positive numbers, no underflow
        ori      r1, r0, TESTVAL1 & 0xffff
        orhi     r1, r1, (TESTVAL1 >> 16) & 0xffff
        ori      r2, r0, TESTVAL2 & 0xffff
        orhi     r2, r2, (TESTVAL2 >> 16) & 0xffff
        ori      r3, r0, RESULT21 & 0xffff
        orhi     r3, r3, (RESULT21 >> 16) & 0xffff
        #mod      r4, r1, r2
        .word    0xd4222000
        bne      r4, r3, _finish
        
        # MOD pos/neg numbers, no underflow
        ori      r1, r0, TESTVAL3 & 0xffff
        orhi     r1, r1, (TESTVAL3 >> 16) & 0xffff
        ori      r2, r0, TESTVAL4 & 0xffff
        orhi     r2, r2, (TESTVAL4 >> 16) & 0xffff
        ori      r3, r0, RESULT22 & 0xffff
        orhi     r3, r3, (RESULT22 >> 16) & 0xffff
        #mod      r4, r1, r2
        .word    0xd4222000
        bne      r4, r3, _finish

        # MOD neg/pos numbers, no underflow
        ori      r1, r0, TESTVAL5 & 0xffff
        orhi     r1, r1, (TESTVAL5 >> 16) & 0xffff
        ori      r2, r0, TESTVAL6 & 0xffff
        orhi     r2, r2, (TESTVAL6 >> 16) & 0xffff
        ori      r3, r0, RESULT23 & 0xffff
        orhi     r3, r3, (RESULT23 >> 16) & 0xffff
        #mod      r4, r1, r2
        .word    0xd4222000
        bne      r4, r3, _finish

        # MOD underflow
        ori      r1, r0, TESTVAL7 & 0xffff
        orhi     r1, r1, (TESTVAL7 >> 16) & 0xffff
        ori      r2, r0, TESTVAL8 & 0xffff
        orhi     r2, r2, (TESTVAL8 >> 16) & 0xffff
        ori      r3, r0, RESULT24 & 0xffff
        orhi     r3, r3, (RESULT24 >> 16) & 0xffff
        #mod      r4, r1, r2
        .word    0xd4222000
        bne      r4, r3, _finish
#endif
        # MODU positive numbers, no underflow
        ori      r1, r0, TESTVAL1 & 0xffff
        orhi     r1, r1, (TESTVAL1 >> 16) & 0xffff
        ori      r2, r0, TESTVAL2 & 0xffff
        orhi     r2, r2, (TESTVAL2 >> 16) & 0xffff
        ori      r3, r0, RESULT21 & 0xffff
        orhi     r3, r3, (RESULT21 >> 16) & 0xffff
        modu     r4, r1, r2
        bne      r4, r3, _finish
        
        # MODU neg/pos numbers, no underflow
        ori      r1, r0, TESTVAL5 & 0xffff
        orhi     r1, r1, (TESTVAL5 >> 16) & 0xffff
        ori      r2, r0, TESTVAL6 & 0xffff
        orhi     r2, r2, (TESTVAL6 >> 16) & 0xffff
        ori      r3, r0, RESULT33 & 0xffff
        orhi     r3, r3, (RESULT33 >> 16) & 0xffff
        modu     r4, r1, r2
        bne      r4, r3, _finish
        
        # MODU underflow
        ori      r1, r0, TESTVAL7 & 0xffff
        orhi     r1, r1, (TESTVAL7 >> 16) & 0xffff
        ori      r2, r0, TESTVAL8 & 0xffff
        orhi     r2, r2, (TESTVAL8 >> 16) & 0xffff
        ori      r3, r0, RESULT24 & 0xffff
        orhi     r3, r3, (RESULT24 >> 16) & 0xffff
        modu     r4, r1, r2
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

