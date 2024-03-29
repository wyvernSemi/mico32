        .section        .boot,  "ax",   @progbits

        .global _start  
_start:         

        .equ FAIL_VALUE,          0x0bad 
        .equ PASS_VALUE,          0x0900d
        .equ RESULT_ADDR,         0xfffc

        .equ TESTADDR1,           0x0000c001
        .equ TESTADDR2,           0x03000000

        .equ RST_EXCEPTION,   0
        .equ BRK_EXCEPTION,   1
        .equ IBUS_EXCEPTION,  2
        .equ WTCH_EXCEPTION,  3
        .equ DBUS_EXCEPTION,  4
        .equ DIV0_EXCEPTION,  5
        .equ INT_EXCEPTION,   6
        .equ SCALL_EXCEPTION, 7

        .equ DC_RE_MASK,      2

/* Exception handlers */
_reset_handler:
        xor r0, r0, r0
        xor r28, r0, r0
        ori  r28, r0, 0xfff0
        bi main
        nop
        nop
        nop
        nop
_breakpoint_handler:
        sw (sp+0), ra
        calli _save_all
        mvi r1, 5
        mvi r2, BRK_EXCEPTION
        calli _break
        bi _restore_all_and_bret
        nop
        nop
_instruction_bus_error_handler:
        sw (sp+0), ra
        calli _save_all
        mvi r1, 11
        mvi r2, IBUS_EXCEPTION
        calli _raise
        bi _restore_all_and_eret
        nop
        nop
_watchpoint_handler:
        sw (sp+0), ra
        calli _save_all
        mvi r1, 5
        mvi r2, WTCH_EXCEPTION
        calli _raise
        bi _restore_all_and_bret
        nop
        nop
_data_bus_error_handler:
        sw (sp+0), ra
        calli _save_all
        mvi r1, 11
        mvi r2, DBUS_EXCEPTION
        calli _raise
        bi _restore_all_and_eret
        nop
        nop
_divide_by_zero_handler:
        sw (sp+0), ra
        calli _save_all
        mvi r1, 8
        mvi r2, DIV0_EXCEPTION
        calli _raise
        bi _restore_all_and_eret
        nop
        nop
_interrupt_handler:
        sw (sp+0), ra
        calli _save_all
        mvi r1, 2
        mvi r2, INT_EXCEPTION
        calli _raise
        bi _restore_all_and_eret
        nop
        nop
_system_call_handler:
        sw (sp+0), ra
        calli _save_all
        mv r1, sp
        mvi r2, SCALL_EXCEPTION
        calli _handle_scall
        bi _restore_all_and_eret
        nop
        nop
_save_all:
        addi sp, sp, -56
        /* Save all caller save registers onto the stack */
        sw (sp+4), r1
        sw (sp+8), r2
        sw (sp+12), r3
        sw (sp+16), r4
        sw (sp+20), r5
        sw (sp+24), r6
        sw (sp+28), r7
        sw (sp+32), r8
        sw (sp+36), r9
        sw (sp+40), r10
        sw (sp+48), ea
        sw (sp+52), ba
        /* ra needs to be moved from initial stack location */
        lw r1, (sp+56)
        sw (sp+44), r1
        ret
/* Restore all registers and return from exception */
_restore_all_and_eret:
        lw r1, (sp+4)
        lw r2, (sp+8)
        lw r3, (sp+12)
        lw r4, (sp+16)
        lw r5, (sp+20)
        lw r6, (sp+24)
        lw r7, (sp+28)
        lw r8, (sp+32)
        lw r9, (sp+36)
        lw r10, (sp+40)
        lw ra, (sp+44)
        lw ea, (sp+48)
        lw ba, (sp+52)
        addi sp, sp, 56
        eret
/* Restore all registers and return from breakpoint */
_restore_all_and_bret:
        lw r1, (sp+4)
        lw r2, (sp+8)
        lw r3, (sp+12)
        lw r4, (sp+16)
        lw r5, (sp+20)
        lw r6, (sp+24)
        lw r7, (sp+28)
        lw r8, (sp+32)
        lw r9, (sp+36)
        lw r10, (sp+40)
        lw ra, (sp+44)
        lw ea, (sp+48)
        lw ba, (sp+52)
        addi sp, sp, 56
        bret

_handle_scall:
        # Copy  Exception number in r2 to r19
        or  r19, r0, r2

        # Check interrupts are disabled and EIE set
        rcsr r2, IE
        ori  r1, r0, 2
        bne  r2, r1, _finish

        ret
_raise:
        # Copy  Exception number in r2 to r20
        or  r20, r0, r2

        # Check expected exception type (in r12)
        bne r20, r12, _finish

        # Check interrupts are disabled and EIE set
        rcsr r2, IE
        ori  r1, r0, 2
        bne  r2, r1, _finish

        # Remember ea in r14
        or   r14, r0, ea

        # load ea's restore stack value with a good return address
        sw   (sp+48), r11

        ret

_break:
        # Copy  Exception number in r2 to r20
        or  r20, r0, r2

        # Check interrupts are disabled and BIE set
        rcsr r2, IE
        ori  r1, r0, 4
        bne  r2, r1, _finish

        ret


main:

        # By default, set the result to bad
        ori  r24, r0, 0
        ori  r25, r0, RESULT_ADDR
        sw  (r25+0), r24

# --------- data bus error tests ---------

        # Enable interrupts
        ori  r1, r0, 1
        wcsr IE, r1

        # Load r12 with the expected exception number
        ori      r12, r0, DBUS_EXCEPTION

        # Put a mis-aligned address in r1
        ori  r1, r0, TESTADDR1 & 0xffff
        orhi r1, r1, (TESTADDR1 >> 16) & 0xffff

        # Load return address into r11
        ori       r11, r0, _ok1

        # Clear r20
        or        r20, r0, r0

        # Misaligned word write
        sw (r1 + 0), r2
_ok1:

        # Check we visited the exception code
        bne  r20, r12, _finish

        # Load return address into r11
        ori  r11, r0, _ok2

        # Clear r20
        or  r20, r0, r0

        # Misaligned word read
        lw  r2,  (r1 + 0)
_ok2:

        # Check we visited the exception code
        bne  r20, r12, _finish

        # Load return address into r11
        ori       r11, r0, _ok3

        # Clear r20
        or        r20, r0, r0

        # Misaligned half-word write 
        sh (r1 + 0), r2
_ok3:

        # Check we visited the exception code
        bne  r20, r12, _finish

        # Load return address into r11
        ori  r11, r0, _ok4

        # Clear r20
        or  r20, r0, r0

        # Misaligned half-word read
        lh  r2,  (r1 + 0)
_ok4:

#ifndef WY_RTL
        # Put an out-of-range address in r1
        ori  r1, r0, TESTADDR2 & 0xffff
        orhi r1, r1, (TESTADDR2 >> 16) & 0xffff

        # Load return address into r11
        ori  r11, r0, _ok5

        # Clear r20
        or  r20, r0, r0

        # Out-of-range access write
        sb  (r1 + 0), r2
_ok5:
        # Check we visited the exception code
        bne  r20, r12, _finish

        # Load return address into r11
        ori  r11, r0, _ok6

        # Clear r20
        or  r20, r0, r0

        # Out-of-range access read
        lb  r2, (r1 + 0)
_ok6:
        # Check we visited the exception code
        bne  r20, r12, _finish
#endif

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

