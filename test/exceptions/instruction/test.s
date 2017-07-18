        .section        .boot,  "ax",   @progbits

        .global _start  
_start:         

        .equ FAIL_VALUE,         0x0bad 
        .equ PASS_VALUE,         0x0900d
        .equ RESULT_ADDR,        0xfffc

        .equ RESET_INDICATOR,    0x1964

        .equ BAD_EXCEPTION_ADDR, 0x1200

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
        ori  r20, r0, RESET_INDICATOR
        bi main
        nop
        nop
        nop
_breakpoint_handler:
        sw (sp+0), ra
        # A bret will return to the break instruction, so move it forward here
        addi ba, ba, 4
        calli _save_all
        mvi r1, 5
        mvi r2, BRK_EXCEPTION
        calli _break
        bi _restore_all_and_bret
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
        addi ea, ea, 4
        sw (sp+0), ra
        calli _save_all
        mvi r1, 8
        mvi r2, DIV0_EXCEPTION
        calli _raise
        bi _restore_all_and_eret
        nop
#        nop
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
        # An eret will return to the scall instruction, so move it forward here
        addi ea, ea, 4
        calli _save_all
        mv r1, sp
        mvi r2, SCALL_EXCEPTION
        calli _handle_scall
        bi _restore_all_and_eret
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
        ret
_raise:
        # Copy  Exception number in r2 to r20
        or  r20, r0, r2

        # Check interrupts are disabled and EIE set
        rcsr r2, IE
        ori  r1, r0, 2
        bne  r2, r1, _finish

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
# ------------- reset tests --------------

        # By default, set the result to bad
        ori  r24, r0, 0
        ori  r25, r0, RESULT_ADDR
        sw  (r25+0), r24

        ori  r1, r0, RESET_INDICATOR
        bne  r20, r1, _finish

        # Set DEBA to a non-zero value
        ori  r1, r0, BAD_EXCEPTION_ADDR
        wcsr DEBA, r1

        # Read it back again and check
        rcsr r2, DEBA
        bne r1, r2, _finish

        # Set up a loop counter
        ori  r17, r0, 0
        
# Looping over tests using EBA and DEBA registers
_loop1:

# ---------- divide by 0 tests -----------

        # Enable interrupts
        ori  r2, r0, 1
        wcsr IE, r2

        ori  r3, r0, DIV0_EXCEPTION
        divu r2, r0, r0
        bne  r3, r20, _finish

        # Check interrupts re-enabled
        rcsr r2, IE
        andi r2, r2, 1
        ori  r1, r0, 1
        bne  r2, r1, _finish

        # Check div instruction raises interrupt
        ori  r3, r0, DIV0_EXCEPTION
        #div      r4, r1, r0
        .word    0x9c202000
        bne  r3, r20, _finish

        # Check interrupts re-enabled
        rcsr r2, IE
        andi r2, r2, 1
        ori  r1, r0, 1
        bne  r2, r1, _finish

        # Check modu instruction raises interrupt
        ori  r3, r0, DIV0_EXCEPTION
        modu     r2, r0, r0
        bne  r3, r20, _finish

        # Check interrupts re-enabled
        rcsr r2, IE
        andi r2, r2, 1
        ori  r1, r0, 1
        bne  r2, r1, _finish

        # Check mod instruction raises interrupt
        ori  r3, r0, DIV0_EXCEPTION
        #mod      r4, r1, r0
        .word    0xd4202000
        bne  r3, r20, _finish

        # Check interrupts re-enabled
        rcsr r2, IE
        andi r2, r2, 1
        ori  r1, r0, 1
        bne  r2, r1, _finish

# ---------- system call tests -----------

        # Enable interrupts
        ori  r2, r0, 1
        wcsr IE, r2
        
        ori  r3, r0, SCALL_EXCEPTION
        scall
        bne  r3, r19, _finish

# ---------- break instr tests -----------

        # Skip break test on EBA loop, as always uses DEBA, and it is set to be bad
        rcsr r2, DEBA
        ori r3, r0, BAD_EXCEPTION_ADDR
        be r2, r3, _skip_break

        # Enable interrupts
        ori  r2, r0, 1
        wcsr IE, r2
        
        ori  r3, r0, BRK_EXCEPTION
        break
        bne  r3, r20, _finish
_skip_break:

        # Set DEBA to a zero value and EBA to non-zero
        ori  r1, r0, 0x0
        wcsr DEBA, r1
        ori  r1, r0, BAD_EXCEPTION_ADDR
        wcsr EBA, r1

        # Set DC.RE bit so the DEBA is used
        rcsr r1, DC
        ori  r1, r0, DC_RE_MASK
        wcsr DC, r1

        # Loop back over test with EBA pointing badly and DEBA correct
        # to check DC.RE overrides exception base address
        addi r17, r17, 1
        ori  r1, r0, 2
        bne  r17, r1, _loop1

_good:
        ori      r24, r0, PASS_VALUE
        be       r0, r0, _store_result

_finish:
        ori      r24, r0, FAIL_VALUE
_store_result:
        ori      r25, r0, RESULT_ADDR
        sw       (r25+0), r24
_end:
        be       r0, r0, _end
        
        .end

