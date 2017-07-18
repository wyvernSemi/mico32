# ----------------------------------------------------------------
# Tests unconditional branching instructions of the MICO32 
# processor
# ----------------------------------------------------------------

        .file  "test.s"
        .text
        .align 4
_start:	.global	_start
        .global main

        .equ FAIL_VALUE,  0x0bad 
        .equ PASS_VALUE,  0x0900d
        .equ RESULT_ADDR, 0xfffc
        .equ SIGN_EXTD16, 0xffff

main:
        xor      r0, r0, r0

        # By default, set the result to bad
        ori      r30, r0, 0
        ori      r31, r0, RESULT_ADDR
        sw 	 (r31+0), r30

# -------------- B tests -----------------

        # B positive jump
        ori      r17, r0, _jmp1
        b        r17
        bi       _finish
        nop
        nop
        nop
        nop
_jmp1:

        # B negative jump
        ori      r17, r0, _jmp2
        bi       _jmp3
_jmp2:
        bi       _jmp4
        bi       _finish
        nop
        nop
        nop
        nop
_jmp3:
        b        r17
        bi       _finish
_jmp4:

# -------------- BI tests ----------------

        # BI positive jump
        bi       _jmp10
        nop
        nop
        nop
        nop
        nop
        bi       _finish
_jmp10:
        
        # BI negative jump
        bi       _jmp11
_jmp12: bi       _jmp13
        bi       _finish
        nop
        nop
        nop
        nop
        nop
_jmp11: bi       _jmp12
        bi       _finish
_jmp13:

# ------------- CALL tests ---------------

        # CALL positive jump
        ori      r18, r0, _call1
        ori      r19, r0, _ret1
        call     r18
_ret1:  bi       _ok1
        bi       _finish

_call1: or       r20, r0, ra
        bne      r19, r20, _finish
        ret
_ok1:

        # CALL negative jump
        bi       _jmp22
_call2: or       r20, r0, ra
        bne      r19, r20, _finish
        ret
_jmp22:
        ori      r18, r0, _call2
        ori      r19, r0, _ret2
        call     r18
_ret2:  bi       _ok2
        bi       _finish
_ok2:


# ------------- CALLI tests --------------

        # CALIL positive jump
        ori      r19, r0, _ret3
        calli    _call3
_ret3:  bi       _ok3
        bi       _finish

_call3: or       r20, r0, ra
        bne      r19, r20, _finish
        ret
_ok3:

        # CALLI negative jump
        bi       _jmp42
_call4: or       r20, r0, ra
        bne      r19, r20, _finish
        ret
_jmp42:
        ori      r19, r0, _ret4
        calli    _call4
_ret4:  bi       _ok4
        bi       _finish
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

