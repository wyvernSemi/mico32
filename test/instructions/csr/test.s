# ----------------------------------------------------------------
# Tests RCSR/WCSR  instructions of the MICO32 processor
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

        .equ COMMS_BASE_ADDRESS,        0x20000000
        .equ COMMS_COUNT_EN_OFFSET,     0x0000001C

        #.equ EXP_CFG_VAL, 0x01120837
        .equ EXP_CFG_VAL, 0x01120df7
        .equ TESTVAL1,    0xfe718100
        .equ TESTVAL2,    0xfe718131
        .equ CCRSTVAL,    0x7650

main:
        xor      r0, r0, r0

        # By default, set the result to bad
        ori      r30, r0, 0
        ori      r31, r0, RESULT_ADDR
        sw       (r31+0), r30

# -------------- RCSR tests --------------

        # Read only CFG

        ori      r4, r0, EXP_CFG_VAL & 0xffff
        orhi     r4, r4, (EXP_CFG_VAL >> 16) & 0xffff
        rcsr     r8, CFG
        bne      r4, r8, _finish

        # Write 0 to cfg
        wcsr     CFG, r0

        # Check it's unchanged
        rcsr     r9, CFG
        bne      r8, r9, _finish

        # Read only CFG2
        #rcsr     r8, CFG2
        .word 0x91404000
        bne      r8, r0, _finish

        ori      r9, r0, 0xffff
        orhi     r9, r9, 0xffff
        #wcsr     CFG2, r9
        .word 0xd1490000

        #rcsr     r8, CFG2
        .word 0x91404000
        bne      r8, r0, _finish

        # Unimplemented  ICC
        rcsr     r8, ICC
        bne      r8, r0, _finish
        wcsr     ICC, r9
        rcsr     r8, ICC
        bne      r8, r0, _finish

        # Unimplemented  DCC
        rcsr     r8, DCC
        bne      r8, r0, _finish
        wcsr     DCC, r9
        rcsr     r8, DCC
        bne      r8, r0, _finish

        # Implemented  JTX (loopback)
        wcsr     JTX, r9
        rcsr     r8, JTX
        ori      r9, r0, 0xff
        bne      r8, r9, _finish

        # Implemented JRX (loopback on JTAX)
        wcsr     JRX, r0                # Should have no effect
        rcsr     r8, JRX
        bne      r8, r9, _finish

# ------------ WCSR/RCSR tests -----------

        # CC check that it's running

        # Expecting CC to have moved on 4 cycles between reading
        ori      r7, r0, 4
        rcsr     r8, CC
        nop
        nop
        nop
        rcsr     r9, CC
        sub      r10, r9, r8
        bne      r10, r7, _finish

        # reset it to a value
        ori      r7, r0, CCRSTVAL
        wcsr     cc, r7
        nop
        nop
        nop
        rcsr     r9, CC
        addi     r10, r7, 4
        bne      r10, r9, _finish

        # Configure the model to remove the cycle counter
        ori      r1, r0, COMMS_BASE_ADDRESS & 0xffff
        orhi     r1, r1, (COMMS_BASE_ADDRESS >> 16) & 0xffff
        sw       (r1 + COMMS_COUNT_EN_OFFSET), r0

        # Check that it is 0
        rcsr     r8, CC
        bne      r8, r0, _finish

        ori      r4, r0, TESTVAL1 & 0xffff
        orhi     r4, r4, (TESTVAL1 >> 16) & 0xffff
        wcsr     EBA, r4
        rcsr     r8, EBA
        bne      r8, r4, _finish

        ori      r4, r0, TESTVAL2 & 0xffff
        orhi     r4, r4, (TESTVAL2 >> 16) & 0xffff
        wcsr     IM, r4
        rcsr     r8, IM
        bne      r8, r4, _finish

        ori      r4, r0, TESTVAL2 & 0xffff
        orhi     r4, r4, (TESTVAL2 >> 16) & 0xffff
        wcsr     IE, r4
        rcsr     r8, IE
        andi     r4, r4, 1
        bne      r8, r4, _finish

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

