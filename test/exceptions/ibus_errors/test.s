        .section        .boot,  "ax",   @progbits

        .global _start  
_start:         

        .equ FAIL_VALUE,          0x0bad 
        .equ PASS_VALUE,          0x0900d
        .equ RESULT_ADDR,         0xfffc

        .equ COMMS_BASE_ADDR,       0x20000000
        .equ COMMS_MULT_EN_OFFSET,  0x0000000C
        .equ COMMS_DIV_EN_OFFSET,   0x00000010
        .equ COMMS_SHIFT_EN_OFFSET, 0x00000014
        .equ COMMS_SEXT_EN_OFFSET,  0x00000018

        .equ INSTR_RESVRD1,   0xA8000000
        .equ INSTR_RESVRD2,   0xCC000000
        .equ TEST_ADDR1,      0x02000000

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

# -------- instr bus error tests ---------

        # Enable interrupts
        ori  r1, r0, 1
        wcsr IE, r1

        ### Out of range exception

        # Load r12 with the expected exception number
        ori      r12, r0, IBUS_EXCEPTION
        
#ifndef WY_RTL 
        # Load a real return address into r11
        ori       r11, r0, _ok1

        # Branch to an out of range address
        ori       r13, r0, TEST_ADDR1 & 0xffff
        orhi      r13, r13, (TEST_ADDR1 >> 16) & 0xffff
        b         r13
_ok1:
        # Check that ea had bad address
        addi      r13, r13, 4
        bne       r14, r13, _finish
#endif
        ### Reserved instruction exception
        
        # Clear r20
        or        r20, r0, r0
        
        # Load return address into r11
        ori       r11, r0, _ok2

        # execute a reserved instruction
        .word     INSTR_RESVRD1
_ok2:
        # Check we visited the exception code
        bne r20, r12, _finish

        # Clear r20
        or        r20, r0, r0
        
        # Load return address into r11
        ori       r11, r0, _ok3

        # execute second reserved instruction
        .word     INSTR_RESVRD2
_ok3:
        # Check we visited the exception code
        bne r20, r12, _finish
        
#ifndef WY_RTL
    ### Disable multiplier and check mul instruction cause exception

        # Clear r20
        or        r20, r0, r0

        # Disable multiplier

        ori      r1, r0, (COMMS_BASE_ADDR+COMMS_MULT_EN_OFFSET) & 0xffff
        orhi     r1, r1, ((COMMS_BASE_ADDR+COMMS_MULT_EN_OFFSET) >> 16) & 0xffff
        sw       (r1+0), r0

        # Read back CFG register via external interface, and check
        # multiply bit clear
        lw       r2, (r1+0)
        andi     r2, r2, 1
        bne      r2, r0, _finish

        # Load return address into r11
        ori       r11, r0, _ok4

        # execute the disabled instruction
        mul       r1, r1, r1
_ok4:
        # Check we visited the exception code
        bne r20, r12, _finish

        # Reenable multiplier
        ori      r2, r0, 1
        sw       (r1+0), r2

        # Clear r20
        or        r20, r0, r0

        # execute the enabled instruction
        mul       r1, r1, r1

        # Check we didn't visit the exception code
        bne r20, r0, _finish

    ### Disable multiplier and check muli instruction cause exception

        # Clear r20
        or        r20, r0, r0

        # Disable multiplier

        ori      r1, r0, (COMMS_BASE_ADDR+COMMS_MULT_EN_OFFSET) & 0xffff
        orhi     r1, r1, ((COMMS_BASE_ADDR+COMMS_MULT_EN_OFFSET) >> 16) & 0xffff
        sw       (r1+0), r0

        # Load return address into r11
        ori       r11, r0, _ok5

        # execute the disabled instruction
        muli      r1, r1, 1
_ok5:
        # Check we visited the exception code
        bne r20, r12, _finish

        # Reenable multiplier
        ori      r2, r0, 1
        sw       (r1+0), r2

        # Clear r20
        or        r20, r0, r0

        # execute the enabled instruction
        muli      r1, r1, 1

        # Check we didn't visit the exception code
        bne r20, r0, _finish

     ### Disable divider and check divu/modu instructions cause exception

        # Disable divider
        ori      r1, r0, (COMMS_BASE_ADDR+COMMS_DIV_EN_OFFSET) & 0xffff
        orhi     r1, r1, ((COMMS_BASE_ADDR+COMMS_DIV_EN_OFFSET) >> 16) & 0xffff
        sw       (r1+0), r0

        # Load return address into r11
        ori       r11, r0, _ok6

        # Clear r20
        or        r20, r0, r0

        # execute the disabled instruction
        divu      r1, r1, r1
_ok6:
        # Check we visited the exception code
        bne r20, r12, _finish

        # Load return address into r11
        ori       r11, r0, _ok7

        # Clear r20
        or        r20, r0, r0

        # execute the disabled instruction
        modu      r1, r1, r1
_ok7:
        # Check we visited the exception code
        bne r20, r12, _finish

        # Load return address into r11
        ori       r11, r0, _ok7a

        # Clear r20
        or        r20, r0, r0

        # execute the disabled instruction
        #div      r4, r1, r2
        .word    0x9c222000
_ok7a:
        # Check we visited the exception code
       bne r20, r12, _finish
       # Load return address into r11
       ori       r11, r0, _ok7b

       # Clear r20
       or        r20, r0, r0

       # execute the disabled instruction
       #mod      r4, r1, r2
       .word    0xd4222000
_ok7b:
       # Check we visited the exception code
       bne r20, r12, _finish

        # Reenable divider
        ori      r2, r0, 1
        sw       (r1+0), r2

        # Clear r20
        or        r20, r0, r0

        # execute the enabled instruction
        divu      r1, r1, r1

        # Check we didn't visit the exception code
        bne r20, r0, _finish

        # Clear r20
        or        r20, r0, r0

        # execute the enabled instruction
        modu      r1, r1, r1

        # Check we didn't visit the exception code
        bne r20, r0, _finish


     ### Disable sign extend and check sextb/sexth instructions cause exception

        # Disable sign extender
        ori      r1, r0, (COMMS_BASE_ADDR+COMMS_SEXT_EN_OFFSET) & 0xffff
        orhi     r1, r1, ((COMMS_BASE_ADDR+COMMS_SEXT_EN_OFFSET) >> 16) & 0xffff
        sw       (r1+0), r0

        # Load return address into r11
        ori       r11, r0, _ok8

        # Clear r20
        or        r20, r0, r0

        # execute the disabled instruction
        sextb      r1, r1
_ok8:
        # Check we visited the exception code
        bne r20, r12, _finish

        # Load return address into r11
        ori       r11, r0, _ok9

        # Clear r20
        or        r20, r0, r0

        # execute the disabled instruction
        sexth      r1, r1
_ok9:
        # Check we visited the exception code
        bne r20, r12, _finish

        # Reenable sign extender
        ori      r2, r0, 1
        sw       (r1+0), r2

        # Clear r20
        or        r20, r0, r0

        # execute the enabled instruction
        sextb      r1, r1

        # Check we didn't visit the exception code
        bne r20, r0, _finish

        # Clear r20
        or        r20, r0, r0

        # execute the enabled instruction
        sexth     r1, r1

        # Check we didn't visit the exception code
        bne r20, r0, _finish

     ### Disable barrel shifter and check shift instructions cause exception

        # Disable barrel shifter
        ori      r1, r0, (COMMS_BASE_ADDR+COMMS_SHIFT_EN_OFFSET) & 0xffff
        orhi     r1, r1, ((COMMS_BASE_ADDR+COMMS_SHIFT_EN_OFFSET) >> 16) & 0xffff
        sw       (r1+0), r0

        # Load return address into r11
       ori       r11, r0, _ok10

        # Clear r20
        or        r20, r0, r0

        # execute the disabled instruction
        sr      r1, r1, r1
_ok10:
        # Check we visited the exception code
        bne r20, r12, _finish

        # Load return address into r11
        ori       r11, r0, _ok11

        # Clear r20
        or        r20, r0, r0

        # execute the disabled instruction
        sru     r1, r1, r1
_ok11:
        # Check we visited the exception code
        bne r20, r12, _finish

        # Load return address into r11
        ori       r11, r0, _ok12

        # Clear r20
        or        r20, r0, r0

        # execute the disabled instruction
        sri     r1, r1, 1
_ok12:
        # Check we visited the exception code
        bne r20, r12, _finish

        # Load return address into r11
        ori       r11, r0, _ok13

        # Clear r20
        or        r20, r0, r0

        # execute the disabled instruction
        srui     r1, r1, 1
_ok13:
        # Check we visited the exception code
        bne r20, r12, _finish

        # Load return address into r11
        ori       r11, r0, _ok14

        # Clear r20
        or        r20, r0, r0

        # execute the disabled instruction
        sl      r1, r1, r1
_ok14:
        # Check we visited the exception code
        bne r20, r12, _finish

        # Load return address into r11
        ori       r11, r0, _ok15

        # Clear r20
        or        r20, r0, r0

        # execute the disabled instruction
        sli     r1, r1, 1
_ok15:
        # Check we visited the exception code
        bne r20, r12, _finish

        # Re-enable barrel shifter
        ori      r2, r0, 1
        sw       (r1+0), r2

        # Clear r20
        or        r20, r0, r0

        # execute the enabled instruction
        sr      r1, r1, r1

        # Check we didn't visit the exception code
        bne r20, r0, _finish

        # Clear r20
        or        r20, r0, r0

        # execute the enabled instruction
        sru     r1, r1, r1

        # Check we didn't visit the exception code
        bne r20, r0, _finish

        # Clear r20
        or        r20, r0, r0

        # execute the enabled instruction
        sri     r1, r1, 1

        # Check we didn't visit the exception code
        bne r20, r0, _finish

        # Clear r20
        or        r20, r0, r0

        # execute the enabled instruction
        srui    r1, r1, 1

        # Check we didn't visit the exception code
        bne r20, r0, _finish

        # Clear r20
        or        r20, r0, r0

        # execute the enabled instruction
        sl      r1, r1, r1

        # Check we didn't visit the exception code
        bne r20, r0, _finish

        # Clear r20
        or        r20, r0, r0

        # execute the enabled instruction
        sli     r1, r1, 1

        # Check we didn't visit the exception code
        bne r20, r0, _finish
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

