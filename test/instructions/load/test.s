# ----------------------------------------------------------------
# Tests load instructions
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

        .equ ALLPOSBYTE_VAL,  0x7f01654d
        .equ ALLNEGBYTE_VAL,  0xa780bed1
        .equ MIXEDBYTE_VAL,   0x39a177ee

        .equ RESULT1,  (ALLPOSBYTE_VAL >> 24) & 0xff
        .equ RESULT2,  (ALLPOSBYTE_VAL >> 16) & 0xff
        .equ RESULT3,  (ALLPOSBYTE_VAL >>  8) & 0xff
        .equ RESULT4,  (ALLPOSBYTE_VAL >>  0) & 0xff
        .equ RESULT5,  -89
        .equ RESULT6,  -128
        .equ RESULT7,  -66
        .equ RESULT8,  -47
        .equ RESULT9,  0x39
        .equ RESULT10, 0xa1
        .equ RESULT11, 0x77
        .equ RESULT12, 0xee
        .equ RESULT13, (ALLPOSBYTE_VAL >> 16) & 0xffff
        .equ RESULT14, (ALLPOSBYTE_VAL >>  0) & 0xffff
        .equ RESULT15, (ALLNEGBYTE_VAL >> 16) & 0xffff
        .equ RESULT16, (ALLNEGBYTE_VAL >>  0) & 0xffff
        .equ RESULT17, (ALLNEGBYTE_VAL >> 16) & 0xffff
        .equ RESULT18, (ALLNEGBYTE_VAL >>  0) & 0xffff
        .equ RESULT19, MIXEDBYTE_VAL

        .data
val1:   .word ALLPOSBYTE_VAL
val2:   .word ALLNEGBYTE_VAL
val3:   .word MIXEDBYTE_VAL
val4:   .word 0x00000000

        .text
main:
        xor      r0, r0, r0

        # By default, set the result to bad
        ori      r30, r0, 0
        ori      r31, r0, RESULT_ADDR
        sw       (r31+0), r30

        # Set up

# -------------- LB tests ----------------

        # LB  with positive numbers and immediate offset

        # byte offset 0 (as negative offset from next word)
        ori r10, r0, val2
        lb r11, (r10+(-4))
        ori r12, r0, RESULT1
        be r11, r12, _ok1
        be r0, r0, _finish
_ok1:

        # byte offset 1
        ori r10, r0, val1
        lb r11, (r10+1)
        ori r12, r0, RESULT2
        be r11, r12, _ok2
        be r0, r0, _finish
_ok2:

        # byte offset 2 (as negative offset from next word)
        ori r10, r0, val2
        lb r11, (r10+(-2))
        ori r12, r0, RESULT3
        be r11, r12, _ok3
        be r0, r0, _finish
_ok3:

        # byte offset 2
        ori r10, r0, val1
        lb r11, (r10+3)
        ori r12, r0, RESULT4
        be r11, r12, _ok4
        be r0, r0, _finish
_ok4:
        # LB with positive numbers and register offset

        # byte offset 1
        ori r10, r0, val1+1
        lb r11, (r10+0)
        ori r12, r0, RESULT2
        be r11, r12, _ok5
        be r0, r0, _finish
_ok5:

        # byte offset 2
        ori r10, r0, val1+2
        lb r11, (r10+0)
        ori r12, r0, RESULT3
        be r11, r12, _ok6
        be r0, r0, _finish
_ok6:

        # byte offset 3
        ori r10, r0, val1+3
        lb r11, (r10+0)
        ori r12, r0, RESULT4
        be r11, r12, _ok7
        be r0, r0, _finish
_ok7:

        # LB with negative numbers and immediate offset

        # byte offset 0
        ori r10, r0, val2
        lb r11, (r10+0)
        ori r12, r0, RESULT5
        sexth r12, r12
        be r11, r12, _ok8
        be r0, r0, _finish
_ok8:

        # byte offset 1
        ori r10, r0, val2
        lb r11, (r10+1)
        ori r12, r0, RESULT6
        sexth r12, r12
        be r11, r12, _ok9
        be r0, r0, _finish
_ok9:

        # byte offset 2
        ori r10, r0, val2
        lb r11, (r10+2)
        ori r12, r0, RESULT7
        sexth r12, r12
        be r11, r12, _ok10
        be r0, r0, _finish
_ok10:

        # byte offset 3
        ori r10, r0, val2
        lb r11, (r10+3)
        ori r12, r0, RESULT8
        sexth r12, r12
        be r11, r12, _ok11
        be r0, r0, _finish
_ok11:

# -------------- LBU tests ---------------

        # LBU with mixed numbers 
        ori r10, r0, val3
        lbu r11, (r10+0)
        ori r12, r0, RESULT9
        be r11, r12, _ok12
        be r0, r0, _finish
_ok12:

        # LBU with mixed numbers 
        ori r10, r0, val4
        lbu r11, (r10+(-3))
        ori r12, r0, RESULT10
        be r11, r12, _ok13
        be r0, r0, _finish
_ok13:

        # LBU with mixed numbers 
        ori r10, r0, val3+2
        lbu r11, (r10+0)
        ori r12, r0, RESULT11
        be r11, r12, _ok14
        be r0, r0, _finish
_ok14:

        # LBU with mixed numbers 
        ori r10, r0, val3+3
        lbu r11, (r10+0)
        ori r12, r0, RESULT12
        be r11, r12, _ok15
        be r0, r0, _finish
_ok15:

# -------------- LH tests ----------------

        # LH with positive numbers
        ori r10, r0, val2
        lh  r11, (r10+(-4))
        ori r12, r0, RESULT13
        be r11, r12, _ok16
        be r0, r0, _finish
_ok16:

        # LH with positive numbers
        ori r10, r0, val1
        lh  r11, (r10+2)
        ori r12, r0, RESULT14
        be r11, r12, _ok17
        be r0, r0, _finish
_ok17:

        # LH with negative numbers
        ori r10, r0, val2
        lh  r11, (r10+0)
        ori r12, r0, RESULT15
        sexth r12, r12
        be r11, r12, _ok18
        be r0, r0, _finish
_ok18:

        # LH with negative numbers
        ori r10, r0, val2+2
        lh  r11, (r10+0)
        ori r12, r0, RESULT16
        sexth r12, r12
        be r11, r12, _ok19
        be r0, r0, _finish
_ok19:

# -------------- LHU tests ---------------

        # LHU with negative numbers
        ori r10, r0, val2
        lhu r11, (r10+0)
        ori r12, r0, RESULT17
        be r11, r12, _ok20
        be r0, r0, _finish
_ok20:

        # LHU with negative numbers
        ori r10, r0, val3
        lhu r11, (r10+(-2))
        ori r12, r0, RESULT18
        be r11, r12, _ok21
        be r0, r0, _finish
_ok21:

# -------------- LW tests ----------------

        # LW
        ori r10, r0, val3
        lw r11, (r10+0)
        ori r12, r0,  RESULT19 & 0xffff
        orhi r12, r12, (RESULT19 >> 16) & 0xffff
        be r11, r12, _ok22
        be r0, r0, _finish
_ok22:

        # LW
        or   r11, r0, r0
        ori r10, r0, val2
        lw r11, (r10+4)
        ori r12, r0,  RESULT19 & 0xffff
        orhi r12, r12, (RESULT19 >> 16) & 0xffff
        be r11, r12, _ok23
        be r0, r0, _finish
_ok23:

        # LW
        or   r11, r0, r0
        ori r10, r0, val4
        lw r11, (r10+(-4))
        ori r12, r0,  RESULT19 & 0xffff
        orhi r12, r12, (RESULT19 >> 16) & 0xffff
        be r11, r12, _ok24
        be r0, r0, _finish
_ok24:

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

