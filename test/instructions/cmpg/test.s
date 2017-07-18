# ----------------------------------------------------------------
# Tests compare-greater-than instructions of the MICO32 processor
# ----------------------------------------------------------------

        .file "test.s"
        .text
        .align 4
_start: .global _start
        .global main

        .equ FAIL_VALUE,  0x0bad 
        .equ PASS_VALUE,  0x0900d
        .equ RESULT_ADDR, 0xfffc
        .equ SIGN_EXTD16, 0xffff

        .equ TESTVAL1,    0x12345678
        .equ TESTVAL2,    0x76543210
        .equ TESTVAL3,    TESTVAL2+1
        .equ TESTVAL4,    TESTVAL2-1
        .equ TESTVAL5,    0x80000000
        .equ TESTVAL6,    TESTVAL5+1
        .equ TESTVAL7,    0x000023ad
        .equ TESTVAL8,    TESTVAL7-1
        .equ TESTVAL9,    TESTVAL7+1
        .equ TESTVAL10,   0xffff803d
        .equ TESTVAL11,   TESTVAL10-1
        .equ TESTVAL12,   TESTVAL10+1
        .equ TESTVAL20,   0x0000803d
        .equ TESTVAL21,   (TESTVAL20+1) & 0xffff
        .equ TESTVAL22,   (TESTVAL20-1) & 0xffff

main:
        xor      r0, r0, r0

        # By default, set the result to bad
        ori      r30, r0, 0
        ori      r31, r0, RESULT_ADDR
        sw 	 (r31+0), r30

# -------------- CMPG tests --------------

        # CMPG positive pass
        ori      r20, r0, TESTVAL1 & 0xffff
        orhi     r20, r20, (TESTVAL1 >> 16) & 0xffff
        ori      r21, r0, TESTVAL2 & 0xffff
        orhi     r21, r20, (TESTVAL2 >> 16) & 0xffff
        ori      r26, r0, 1
        cmpg     r22, r21, r20
        be       r22, r26, _ok1
        be       r0, r0, _finish
_ok1:

        # CMPG positive fail
        ori      r20, r0, TESTVAL1 & 0xffff
        orhi     r20, r20, (TESTVAL1 >> 16) & 0xffff
        ori      r21, r0, TESTVAL2 & 0xffff
        orhi     r21, r20, (TESTVAL2 >> 16) & 0xffff
        ori      r27, r0, 0
        cmpg     r22, r20, r21
        be       r22, r27, _ok2
        be       r0, r0, _finish
_ok2:

        # CMPG equal positive fail (different regs)
        ori      r20, r0, TESTVAL1 & 0xffff
        orhi     r20, r20, (TESTVAL1 >> 16) & 0xffff
        ori      r21, r0, TESTVAL1 & 0xffff
        orhi     r21, r20, (TESTVAL1 >> 16) & 0xffff
        ori      r23, r0, 0
        cmpg     r22, r20, r21
        be       r22, r23, _ok3
        be       r0, r0, _finish
_ok3:

        # CMPG equal positive fail (same reg)
        ori      r20, r0, TESTVAL1 & 0xffff
        orhi     r20, r20, (TESTVAL1 >> 16) & 0xffff
        ori      r23, r0, 0
        cmpg     r22, r20, r20
        be       r22, r23, _ok4
        be       r0, r0, _finish
_ok4:

        # CMPG positive/negative pass
        ori      r20, r0, TESTVAL1 & 0xffff
        orhi     r20, r20, (TESTVAL1 >> 16) & 0xffff
        ori      r21, r0, TESTVAL5 & 0xffff
        orhi     r21, r0, (TESTVAL5 >> 16) & 0xffff
        ori      r23, r0, 1
        cmpg     r22, r20, r21
        be       r22, r23, _ok5
        be       r0, r0, _finish
_ok5:

        # CMPG positive/negative fail
        ori      r20, r0, TESTVAL1 & 0xffff
        orhi     r20, r20, (TESTVAL1 >> 16) & 0xffff
        ori      r21, r0, TESTVAL5 & 0xffff
        orhi     r21, r0, (TESTVAL5 >> 16) & 0xffff
        ori      r23, r0, 0
        cmpg     r22, r21, r20
        be       r22, r23, _ok6
        be       r0, r0, _finish
_ok6:

        # CMPG negative pass
        ori      r20, r0, TESTVAL6 & 0xffff
        orhi     r20, r20, (TESTVAL6 >> 16) & 0xffff
        ori      r21, r0, TESTVAL5 & 0xffff
        orhi     r21, r0, (TESTVAL5 >> 16) & 0xffff
        ori      r23, r0, 1
        cmpg     r22, r20, r21
        be       r22, r23, _ok7
        be       r0, r0, _finish
_ok7:

        # CMPG negative fail
        ori      r20, r0, TESTVAL6 & 0xffff
        orhi     r20, r20, (TESTVAL6 >> 16) & 0xffff
        ori      r21, r0, TESTVAL5 & 0xffff
        orhi     r21, r0, (TESTVAL5 >> 16) & 0xffff
        ori      r23, r0, 0
        cmpg     r22, r21, r20
        be       r22, r23, _ok8
        be       r0, r0, _finish
_ok8:

        # CMPG equal negative fail (different regs)
        ori      r20, r0, TESTVAL5 & 0xffff
        orhi     r20, r20, (TESTVAL5 >> 16) & 0xffff
        ori      r21, r0, TESTVAL5 & 0xffff
        orhi     r21, r20, (TESTVAL5 >> 16) & 0xffff
        ori      r23, r0, 0
        cmpg     r22, r20, r21
        be       r22, r23, _ok9
        be       r0, r0, _finish
_ok9:

# -------------- CMPGI tests --------------

        # CMPGI positive pass
        ori      r20, r0, TESTVAL7
        ori      r23, r0, 1
        cmpgi    r22, r20, TESTVAL8
        be       r22, r23, _ok11
        be       r0, r0, _finish
_ok11:

        # CMPGI positive fail
        ori      r20, r0, TESTVAL7
        ori      r23, r0, 0
        cmpgi    r22, r20, TESTVAL9
        be       r22, r23, _ok12
        be       r0, r0, _finish
_ok12:

        # CMPGI equal positive fail 
        ori      r20, r0, TESTVAL7
        ori      r23, r0, 0
        cmpgi    r22, r20, TESTVAL7
        be       r22, r23, _ok13
        be       r0, r0, _finish
_ok13:

        # CMPGI positive/hegative pass
        ori      r20, r0, TESTVAL7
        ori      r23, r0, 1
        cmpgi    r22, r20, TESTVAL10
        be       r22, r23, _ok14
        be       r0, r0, _finish
_ok14:

        # CMPGI positive/hegative fail
        ori      r20, r0, TESTVAL10
        sexth    r20, r20
        ori      r23, r0, 0
        cmpgi    r22, r20, TESTVAL7
        be       r22, r23, _ok15
        be       r0, r0, _finish
_ok15:

        # CMPGI negative pass
        ori      r20, r0, TESTVAL10
        sexth    r20, r20
        ori      r23, r0, 1
        cmpgi    r22, r20, TESTVAL11
        be       r22, r23, _ok16
        be       r0, r0, _finish
_ok16:

        # CMPGI negative fail
        ori      r20, r0, TESTVAL10
        sexth    r20, r20
        ori      r23, r0, 0
        cmpgi    r22, r20, TESTVAL12
        be       r22, r23, _ok17
        be       r0, r0, _finish
_ok17:

        # CMPGI equal negative fail
        ori      r20, r0, TESTVAL10
        sexth    r20, r20
        ori      r23, r0, 0
        cmpgi    r22, r20, TESTVAL10
        be       r22, r23, _ok18
        be       r0, r0, _finish
_ok18:

# -------------- CMPGU tests --------------

        # CMPGU positive pass
        ori      r20, r0, TESTVAL1 & 0xffff
        orhi     r20, r20, (TESTVAL1 >> 16) & 0xffff
        ori      r21, r0, TESTVAL2 & 0xffff
        orhi     r21, r20, (TESTVAL2 >> 16) & 0xffff
        ori      r23, r0, 1
        cmpgu    r22, r21, r20
        be       r22, r23, _ok21
        be       r0, r0, _finish
_ok21:

        # CMPGU positive fail
        ori      r20, r0, TESTVAL1 & 0xffff
        orhi     r20, r20, (TESTVAL1 >> 16) & 0xffff
        ori      r21, r0, TESTVAL2 & 0xffff
        orhi     r21, r20, (TESTVAL2 >> 16) & 0xffff
        ori      r23, r0, 0
        cmpgu    r22, r20, r21
        be       r22, r23, _ok22
        be       r0, r0, _finish
_ok22:

        # CMPGU equal positive fail (different regs)
        ori      r20, r0, TESTVAL1 & 0xffff
        orhi     r20, r20, (TESTVAL1 >> 16) & 0xffff
        ori      r21, r0, TESTVAL1 & 0xffff
        orhi     r21, r20, (TESTVAL1 >> 16) & 0xffff
        ori      r23, r0, 0
        cmpgu    r22, r20, r21
        be       r22, r23, _ok23
        be       r0, r0, _finish
_ok23:

        # CMPGU equal positive fail (same reg)
        ori      r20, r0, TESTVAL1 & 0xffff
        orhi     r20, r20, (TESTVAL1 >> 16) & 0xffff
        ori      r23, r0, 0
        cmpgu    r22, r20, r20
        be       r22, r23, _ok24
        be       r0, r0, _finish
_ok24:

        # CMPGU positive/negative pass
        ori      r20, r0, TESTVAL1 & 0xffff
        orhi     r20, r20, (TESTVAL1 >> 16) & 0xffff
        ori      r21, r0, TESTVAL5 & 0xffff
        orhi     r21, r0, (TESTVAL5 >> 16) & 0xffff
        ori      r23, r0, 1
        cmpgu    r22, r21, r20
        be       r22, r23, _ok25
        be       r0, r0, _finish
_ok25:

        # CMPGU positive/negative fail
        ori      r20, r0, TESTVAL1 & 0xffff
        orhi     r20, r20, (TESTVAL1 >> 16) & 0xffff
        ori      r21, r0, TESTVAL5 & 0xffff
        orhi     r21, r0, (TESTVAL5 >> 16) & 0xffff
        ori      r23, r0, 0
        cmpgu    r22, r20, r21
        be       r22, r23, _ok26
        be       r0, r0, _finish
_ok26:

        # CMPGU negative pass
        ori      r20, r0, TESTVAL6 & 0xffff
        orhi     r20, r20, (TESTVAL6 >> 16) & 0xffff
        ori      r21, r0, TESTVAL5 & 0xffff
        orhi     r21, r0, (TESTVAL5 >> 16) & 0xffff
        ori      r23, r0, 1
        cmpgu    r22, r20, r21
        be       r22, r23, _ok27
        be       r0, r0, _finish
_ok27:

        # CMPGU negative fail
        ori      r20, r0, TESTVAL6 & 0xffff
        orhi     r20, r20, (TESTVAL6 >> 16) & 0xffff
        ori      r21, r0, TESTVAL5 & 0xffff
        orhi     r21, r0, (TESTVAL5 >> 16) & 0xffff
        ori      r23, r0, 0
        cmpgu    r22, r21, r20
        be       r22, r23, _ok28
        be       r0, r0, _finish
_ok28:

        # CMPGU equal negative fail (different regs)
        ori      r20, r0, TESTVAL5 & 0xffff
        orhi     r20, r20, (TESTVAL5 >> 16) & 0xffff
        ori      r21, r0, TESTVAL5 & 0xffff
        orhi     r21, r20, (TESTVAL5 >> 16) & 0xffff
        ori      r23, r0, 0
        cmpgu    r22, r21, r20
        be       r22, r23, _ok29
        be       r0, r0, _finish
_ok29:

# -------------- CMPGUI tests --------------

        # CMPGUI positive pass
        ori      r20, r0, TESTVAL7
        ori      r23, r0, 1
        cmpgui   r22, r20, TESTVAL8
        be       r22, r23, _ok31
        be       r0, r0, _finish
_ok31:

        # CMPGUI positive fail
        ori      r20, r0, TESTVAL7
        ori      r23, r0, 0
        cmpgui   r22, r20, TESTVAL9
        be       r22, r23, _ok32
        be       r0, r0, _finish
_ok32:

        # CMPGUI equal positive fail 
        ori      r20, r0, TESTVAL7
        ori      r23, r0, 0
        cmpgui   r22, r20, TESTVAL7
        be       r22, r23, _ok33
        be       r0, r0, _finish
_ok33:

        # CMPGUI positive/negative pass
        ori      r20, r0, TESTVAL20
        ori      r23, r0, 1
        cmpgui   r22, r20, TESTVAL7
        be       r22, r23, _ok34
        be       r0, r0, _finish
_ok34:

        # CMPGUI positive/negative fail
        ori      r20, r0, TESTVAL7
        ori      r23, r0, 0
        cmpgui   r22, r20, TESTVAL20
        be       r22, r23, _ok35
        be       r0, r0, _finish
_ok35:

        # CMPGUI negative pass
        ori      r20, r0, TESTVAL20
        ori      r23, r0, 1
        cmpgui   r22, r20, TESTVAL22
        be       r22, r23, _ok36
        be       r0, r0, _finish
_ok36:

        # CMPGUI negative fail
        ori      r20, r0, TESTVAL20
        ori      r23, r0, 0
        cmpgui   r22, r20, TESTVAL21
        be       r22, r23, _ok37
        be       r0, r0, _finish
_ok37:

        # CMPGUI equal negative fail
        ori      r20, r0, TESTVAL20
        ori      r23, r0, 0
        cmpgui   r22, r20, TESTVAL20
        be       r22, r23, _ok38
        be       r0, r0, _finish
_ok38:


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

