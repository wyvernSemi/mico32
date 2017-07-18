# ----------------------------------------------------------------
# Tests compare instructions of the MICO32 processor
# ----------------------------------------------------------------

        .file "test.s"
        .text
        .align 4
_start: .global _start
        .global main

        .equ FAIL_VALUE,  0x0bad 
        .equ PASS_VALUE,  0x900d
        .equ RESULT_ADDR, 0xfffc
        .equ SIGN_EXTD16, 0xffff

        .equ TESTVAL1,    0x76543210
        .equ TESTVAL2,    0xf308a543
        .equ TESTVAL3,    0x092aeb54
        .equ TESTVAL4,    0x00007fff
        .equ TESTVAL5,    0xffff8000
        .equ TESTVAL6,    0xffffabcd

main:
        xor      r0, r0, r0

        # By default, set the result to fail
        ori      r30, r0, 0
        ori      r31, r0, RESULT_ADDR
        sw 	 (r31+0), r30

# -------------- CMPE tests --------------

        # CMPE different registers and pass
        ori      r17, r0,  TESTVAL1 & 0xffff
        orhi     r17, r17, (TESTVAL1 >> 17) & 0xffff
        ori      r18, r0,  TESTVAL1 & 0xffff
        orhi     r18, r18, (TESTVAL1 >> 17) & 0xffff
        ori      r20, r0, 1
        cmpe     r19, r18, r17
        be       r19, r20, _ok1
        be       r0, r0, _finish
_ok1:

        # CMPE same registers and pass
        ori      r18, r0,  TESTVAL2 & 0xffff
        orhi     r18, r18, (TESTVAL2 >> 17) & 0xffff
        ori      r20, r0, 1
        cmpe     r19, r18, r18
        be       r19, r20, _ok2
        be       r0, r0, _finish
_ok2:

        # CMPE different registers and fail
        ori      r17, r0,  TESTVAL3 & 0xffff
        orhi     r17, r17, (TESTVAL3 >> 17) & 0xffff
        ori      r18, r0,  TESTVAL3 & 0xffff
        orhi     r18, r18, (TESTVAL3 >> 17) & 0xffff
        addi     r18, r18, 0x2000
        ori      r20, r0, 0
        cmpe     r19, r18, r17
        be       r19, r20, _ok3
        be       r0, r0, _finish
_ok3:

# -------------- CMPIE tests -------------

        # CMPEI positive number and pass
        ori      r18, r0,  TESTVAL4
        ori      r20, r0, 1
        cmpei    r19, r18, TESTVAL4
        be       r19, r20, _ok4
        be       r0, r0, _finish
_ok4:

        # CMPEI positive number and fail
        ori      r18, r0,  TESTVAL4
        ori      r20, r0, 0
        cmpei    r19, r18, TESTVAL4-1
        be       r19, r20, _ok5
        be       r0, r0, _finish
_ok5:

        # CMPEI negative number and pass
        ori      r18, r0,  TESTVAL5 & 0xffff
        orhi     r18, r18, (TESTVAL5 >> 16) & 0xffff
        ori      r20, r0, 1
        cmpei    r19, r18, TESTVAL5
        be       r19, r20, _ok6
        be       r0, r0, _finish
_ok6:

        # CMPEI negative number and fail
        ori      r18, r0,  TESTVAL6 & 0xffff
        orhi     r18, r18, (TESTVAL6 >> 16) & 0xffff
        ori      r20, r0, 0
        cmpei    r19, r18, TESTVAL6+1
        be       r19, r20, _ok7
        be       r0, r0, _finish
_ok7:

# -------------- CMPNE tests -------------

        # CMPNE different registers and fail
        ori      r17, r0,  TESTVAL1 & 0xffff
        orhi     r17, r17, (TESTVAL1 >> 17) & 0xffff
        ori      r18, r0,  TESTVAL1 & 0xffff
        orhi     r18, r18, (TESTVAL1 >> 17) & 0xffff
        ori      r20, r0, 0
        cmpne    r19, r18, r17
        be       r19, r20, _ok11
        be       r0, r0, _finish
_ok11:

        # CMPNE same registers and fail
        ori      r18, r0,  TESTVAL2 & 0xffff
        orhi     r18, r18, (TESTVAL2 >> 17) & 0xffff
        ori      r20, r0, 0
        cmpne    r19, r18, r18
        be       r19, r20, _ok12
        be       r0, r0, _finish
_ok12:

        # CMPNE different registers and pass
        ori      r17, r0,  TESTVAL3 & 0xffff
        orhi     r17, r17, (TESTVAL3 >> 17) & 0xffff
        ori      r18, r0,  TESTVAL3 & 0xffff
        orhi     r18, r18, (TESTVAL3 >> 17) & 0xffff
        addi     r18, r18, 0x2000
        ori      r20, r0, 1
        cmpne     r19, r18, r17
        be       r19, r20, _ok13
        be       r0, r0, _finish
_ok13:

# ------------- CMPNEI tests -------------

        # CMPNEI positive number and fail
        ori      r18, r0,  TESTVAL4
        ori      r20, r0, 0
        cmpnei   r19, r18, TESTVAL4
        be       r19, r20, _ok14
        be       r0, r0, _finish
_ok14:

        # CMPNEI positive number and pass
        ori      r18, r0,  TESTVAL4
        ori      r20, r0, 1
        cmpnei   r19, r18, TESTVAL4-1
        be       r19, r20, _ok15
        be       r0, r0, _finish
_ok15:

        # CMPNEI negative number and fail
        ori      r18, r0,  TESTVAL5 & 0xffff
        orhi     r18, r18, (TESTVAL5 >> 16) & 0xffff
        ori      r20, r0, 0
        cmpnei   r19, r18, TESTVAL5
        be       r19, r20, _ok16
        be       r0, r0, _finish
_ok16:

        # CMPEI negative number and pass
        ori      r18, r0,  TESTVAL6 & 0xffff
        orhi     r18, r18, (TESTVAL6 >> 16) & 0xffff
        ori      r20, r0, 1
        cmpnei   r19, r18, TESTVAL6+1
        be       r19, r20, _ok17
        be       r0, r0, _finish
_ok17:


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

