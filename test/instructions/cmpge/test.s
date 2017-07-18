# ----------------------------------------------------------------
# Tests compare-greater-than-or-equal instructions of the MICO32 
# processor
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
        sw       (r31+0), r30

# ------------- CMPGE tests --------------

        # CMPGE positive pass
        ori      r20, r0, TESTVAL1 & 0xffff
        orhi     r20, r20, (TESTVAL1 >> 16) & 0xffff
        ori      r21, r0, TESTVAL2 & 0xffff
        orhi     r21, r20, (TESTVAL2 >> 16) & 0xffff
        ori      r23, r0, 1
        cmpge    r22, r21, r20
        be       r22, r23, _ok1
        be       r0, r0, _finish
_ok1:

        # CMPGE positive fail
        ori      r20, r0, TESTVAL1 & 0xffff
        orhi     r20, r20, (TESTVAL1 >> 16) & 0xffff
        ori      r21, r0, TESTVAL2 & 0xffff
        orhi     r21, r20, (TESTVAL2 >> 16) & 0xffff
        ori      r23, r0, 0
        cmpge    r22, r20, r21
        be       r22, r23, _ok2
        be       r0, r0, _finish
_ok2:

        # CMPGE equal positive pass (different regs)
        ori      r20, r0, TESTVAL1 & 0xffff
        orhi     r20, r20, (TESTVAL1 >> 16) & 0xffff
        ori      r21, r0, TESTVAL1 & 0xffff
        orhi     r21, r20, (TESTVAL1 >> 16) & 0xffff
        ori      r23, r0, 1
        cmpge    r22, r20, r21
        be       r22, r23, _ok3
        be       r0, r0, _finish
_ok3:

        # CMPGE equal positive pass (same reg)
        ori      r20, r0, TESTVAL1 & 0xffff
        orhi     r20, r20, (TESTVAL1 >> 16) & 0xffff
        ori      r23, r0, 1
        cmpge    r22, r20, r20
        be       r22, r23, _ok4
        be       r0, r0, _finish
_ok4:

        # CMPGE positive/negative pass
        ori      r20, r0, TESTVAL1 & 0xffff
        orhi     r20, r20, (TESTVAL1 >> 16) & 0xffff
        ori      r21, r0, TESTVAL5 & 0xffff
        orhi     r21, r0, (TESTVAL5 >> 16) & 0xffff
        ori      r23, r0, 1
        cmpge    r22, r20, r21
        be       r22, r23, _ok5
        be       r0, r0, _finish
_ok5:

        # CMPGE positive/negative fail
        ori      r20, r0, TESTVAL1 & 0xffff
        orhi     r20, r20, (TESTVAL1 >> 16) & 0xffff
        ori      r21, r0, TESTVAL5 & 0xffff
        orhi     r21, r0, (TESTVAL5 >> 16) & 0xffff
        ori      r23, r0, 0
        cmpge    r22, r21, r20
        be       r22, r23, _ok6
        be       r0, r0, _finish
_ok6:

        # CMPGE negative pass
        ori      r20, r0, TESTVAL6 & 0xffff
        orhi     r20, r20, (TESTVAL6 >> 16) & 0xffff
        ori      r21, r0, TESTVAL5 & 0xffff
        orhi     r21, r0, (TESTVAL5 >> 16) & 0xffff
        ori      r23, r0, 1
        cmpge    r22, r20, r21
        be       r22, r23, _ok7
        be       r0, r0, _finish
_ok7:

        # CMPGE negative fail
        ori      r20, r0, TESTVAL6 & 0xffff
        orhi     r20, r20, (TESTVAL6 >> 16) & 0xffff
        ori      r21, r0, TESTVAL5 & 0xffff
        orhi     r21, r0, (TESTVAL5 >> 16) & 0xffff
        ori      r23, r0, 0
        cmpge    r22, r21, r20
        be       r22, r23, _ok8
        be       r0, r0, _finish
_ok8:

        # CMPGE equal negative pass (different regs)
        ori      r20, r0, TESTVAL5 & 0xffff
        orhi     r20, r20, (TESTVAL5 >> 16) & 0xffff
        ori      r21, r0, TESTVAL5 & 0xffff
        orhi     r21, r20, (TESTVAL5 >> 16) & 0xffff
        ori      r23, r0, 1
        cmpge    r22, r20, r21
        be       r22, r23, _ok9
        be       r0, r0, _finish
_ok9:

# ------------- CMPGEI tests --------------

        # CMPGEI positive pass
        ori      r20, r0, TESTVAL7
        ori      r23, r0, 1
        cmpgei   r22, r20, TESTVAL8
        be       r22, r23, _ok11
        be       r0, r0, _finish
_ok11:

        # CMPGEI positive fail
        ori      r20, r0, TESTVAL7
        ori      r23, r0, 0
        cmpgei   r22, r20, TESTVAL9
        be       r22, r23, _ok12
        be       r0, r0, _finish
_ok12:

        # CMPGEI equal positive pass
        ori      r20, r0, TESTVAL7
        ori      r23, r0, 1
        cmpgei   r22, r20, TESTVAL7
        be       r22, r23, _ok13
        be       r0, r0, _finish
_ok13:

        # CMPGEI positive/hegative pass
        ori      r20, r0, TESTVAL7
        ori      r23, r0, 1
        cmpgei   r22, r20, TESTVAL10
        be       r22, r23, _ok14
        be       r0, r0, _finish
_ok14:

        # CMPGEI positive/hegative fail
        ori      r20, r0, TESTVAL10
        sexth    r20, r20
        ori      r23, r0, 0
        cmpgei   r22, r20, TESTVAL7
        be       r22, r23, _ok15
        be       r0, r0, _finish
_ok15:

        # CMPGEI negative pass
        ori      r20, r0, TESTVAL10
        sexth    r20, r20
        ori      r23, r0, 1
        cmpgei   r22, r20, TESTVAL11
        be       r22, r23, _ok16
        be       r0, r0, _finish
_ok16:

        # CMPGEI negative fail
        ori      r20, r0, TESTVAL10
        sexth    r20, r20
        ori      r23, r0, 0
        cmpgei   r22, r20, TESTVAL12
        be       r22, r23, _ok17
        be       r0, r0, _finish
_ok17:

        # CMPGEI equal negative pass
        ori      r20, r0, TESTVAL10
        sexth    r20, r20
        ori      r23, r0, 1
        cmpgei   r22, r20, TESTVAL10
        be       r22, r23, _ok18
        be       r0, r0, _finish
_ok18:

# ------------- CMPGEU tests --------------

        # CMPGEU positive pass
        ori      r20, r0, TESTVAL1 & 0xffff
        orhi     r20, r20, (TESTVAL1 >> 16) & 0xffff
        ori      r21, r0, TESTVAL2 & 0xffff
        orhi     r21, r20, (TESTVAL2 >> 16) & 0xffff
        ori      r23, r0, 1
        cmpgeu   r22, r21, r20
        be       r22, r23, _ok21
        be       r0, r0, _finish
_ok21:

        # CMPGEU positive fail
        ori      r20, r0, TESTVAL1 & 0xffff
        orhi     r20, r20, (TESTVAL1 >> 16) & 0xffff
        ori      r21, r0, TESTVAL2 & 0xffff
        orhi     r21, r20, (TESTVAL2 >> 16) & 0xffff
        ori      r23, r0, 0
        cmpgeu   r22, r20, r21
        be       r22, r23, _ok22
        be       r0, r0, _finish
_ok22:

        # CMPGEU equal positive pass (different regs)
        ori      r20, r0, TESTVAL1 & 0xffff
        orhi     r20, r20, (TESTVAL1 >> 16) & 0xffff
        ori      r21, r0, TESTVAL1 & 0xffff
        orhi     r21, r20, (TESTVAL1 >> 16) & 0xffff
        ori      r23, r0, 1
        cmpgeu   r22, r20, r21
        be       r22, r23, _ok23
        be       r0, r0, _finish
_ok23:

        # CMPGEU equal positive pass (same reg)
        ori      r20, r0, TESTVAL1 & 0xffff
        orhi     r20, r20, (TESTVAL1 >> 16) & 0xffff
        ori      r23, r0, 1
        cmpgeu   r22, r20, r20
        be       r22, r23, _ok24
        be       r0, r0, _finish
_ok24:

        # CMPGEU positive/negative pass
        ori      r20, r0, TESTVAL1 & 0xffff
        orhi     r20, r20, (TESTVAL1 >> 16) & 0xffff
        ori      r21, r0, TESTVAL5 & 0xffff
        orhi     r21, r0, (TESTVAL5 >> 16) & 0xffff
        ori      r23, r0, 1
        cmpgeu   r22, r21, r20
        be       r22, r23, _ok25
        be       r0, r0, _finish
_ok25:

        # CMPGEU positive/negative fail
        ori      r20, r0, TESTVAL1 & 0xffff
        orhi     r20, r20, (TESTVAL1 >> 16) & 0xffff
        ori      r21, r0, TESTVAL5 & 0xffff
        orhi     r21, r0, (TESTVAL5 >> 16) & 0xffff
        ori      r23, r0, 0
        cmpgeu   r22, r20, r21
        be       r22, r23, _ok26
        be       r0, r0, _finish
_ok26:

        # CMPGEU negative pass
        ori      r20, r0, TESTVAL6 & 0xffff
        orhi     r20, r20, (TESTVAL6 >> 16) & 0xffff
        ori      r21, r0, TESTVAL5 & 0xffff
        orhi     r21, r0, (TESTVAL5 >> 16) & 0xffff
        ori      r23, r0, 1
        cmpgeu   r22, r20, r21
        be       r22, r23, _ok27
        be       r0, r0, _finish
_ok27:

        # CMPGEU negative fail
        ori      r20, r0, TESTVAL6 & 0xffff
        orhi     r20, r20, (TESTVAL6 >> 16) & 0xffff
        ori      r21, r0, TESTVAL5 & 0xffff
        orhi     r21, r0, (TESTVAL5 >> 16) & 0xffff
        ori      r23, r0, 0
        cmpgeu   r22, r21, r20
        be       r22, r23, _ok28
        be       r0, r0, _finish
_ok28:

        # CMPGEU equal negative pass (different regs)
        ori      r20, r0, TESTVAL5 & 0xffff
        orhi     r20, r20, (TESTVAL5 >> 16) & 0xffff
        ori      r21, r0, TESTVAL5 & 0xffff
        orhi     r21, r20, (TESTVAL5 >> 16) & 0xffff
        ori      r23, r0, 1
        cmpgeu   r22, r21, r20
        be       r22, r23, _ok29
        be       r0, r0, _finish
_ok29:

# -------------- CMPGUI tests --------------

        # CMPGEUI positive pass
        ori      r20, r0, TESTVAL7
        ori      r23, r0, 1
        cmpgeui  r22, r20, TESTVAL8
        be       r22, r23, _ok31
        be       r0, r0, _finish
_ok31:

        # CMPGEUI positive fail
        ori      r20, r0, TESTVAL7
        ori      r23, r0, 0
        cmpgeui  r22, r20, TESTVAL9
        be       r22, r23, _ok32
        be       r0, r0, _finish
_ok32:

        # CMPGEUI equal positive pass 
        ori      r20, r0, TESTVAL7
        ori      r23, r0, 1
        cmpgeui  r22, r20, TESTVAL7
        be       r22, r23, _ok33
        be       r0, r0, _finish
_ok33:

        # CMPGEUI positive/negative pass
        ori      r20, r0, TESTVAL20
        ori      r23, r0, 1
        cmpgeui  r22, r20, TESTVAL7
        be       r22, r23, _ok34
        be       r0, r0, _finish
_ok34:

        # CMPGEUI positive/negative fail
        ori      r20, r0, TESTVAL7
        ori      r23, r0, 0
        cmpgeui  r22, r20, TESTVAL20
        be       r22, r23, _ok35
        be       r0, r0, _finish
_ok35:

        # CMPGEUI negative pass
        ori      r20, r0, TESTVAL20
        ori      r23, r0, 1
        cmpgeui  r22, r20, TESTVAL22
        be       r22, r23, _ok36
        be       r0, r0, _finish
_ok36:

        # CMPGEUI negative fail
        ori      r20, r0, TESTVAL20
        ori      r23, r0, 0
        cmpgeui  r22, r20, TESTVAL21
        be       r22, r23, _ok37
        be       r0, r0, _finish
_ok37:

        # CMPGEUI equal negative pass
        ori      r20, r0, TESTVAL20
        ori      r23, r0, 1
        cmpgeui  r22, r20, TESTVAL20
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

