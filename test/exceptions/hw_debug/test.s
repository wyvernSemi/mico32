        .section        .boot,  "ax",   @progbits

        .global _start  
_start:         

        .equ FAIL_VALUE,          0x0bad 
        .equ PASS_VALUE,          0x0900d
        .equ RESULT_ADDR,         0xfffc

        .equ WTCH_ADDR0,          0x3000
        .equ WTCH_ADDR1,          0x3101
        .equ WTCH_ADDR2,          0x3202
        .equ WTCH_ADDR3,          0x3303

        .equ WP_DISABLE,          0x00
        .equ WP_READONLY,         0x01
        .equ WP_WRITEONLY,        0x02
        .equ WP_READWRITE,        0x03

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
        calli _break
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

        ret

_break:
        # Copy  Exception number in r2 to r20
        or  r20, r0, r2

        # Check expected exception type (in r12)
        bne r20, r12, _finish

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

# ---------- break point tests -----------

        # Enable interrupts
        ori  r1, r0, 1
        wcsr IE, r1

        # Load r12 with the expected exception number
        ori      r12, r0, BRK_EXCEPTION

        # Load r1 with a break address, and set enable bit
        ori   r1, r0, _break0 + 1

        # Program bp0
        wcsr bp0, r1

        # Read it back again and check
        rcsr r2, bp0
        bne r2, r1, _finish

        # Load r1 with a break address, and set enable bit
        ori   r1, r0, _break1 + 1

        # Program bp1
        wcsr bp1, r1

        # Read it back again and check
        rcsr r2, bp1
        bne r2, r1, _finish

        # Load r1 with a break address, and set enable bit
        ori   r1, r0, _break2 + 1

        # Program bp2
        wcsr bp2, r1

        # Read it back again and check
        rcsr r2, bp2
        bne r2, r1, _finish

        # Load r1 with a break address, and set enable bit
        ori   r1, r0, _break3 + 1

        # Program bp3
        wcsr bp3, r1

        # Read it back again and check
        rcsr r2, bp3
        bne r2, r1, _finish

        # clear r20
        xor r20, r20, r20
        nop
        nop
_break0:
        nop

        # Check we visited the exception code
        bne  r20, r12, _finish

        # clear r20
        xor r20, r20, r20
        nop

_break1:
        nop

        # Check we visited the exception code
        bne  r20, r12, _finish

        # clear r20
        xor r20, r20, r20
        nop

_break2:
        nop

        # Check we visited the exception code
        bne  r20, r12, _finish

        # clear r20
        xor r20, r20, r20
        nop

_break3:
        nop

        # Check we visited the exception code
        bne  r20, r12, _finish

        # Load r1 with a break address, and clear enable bit
        ori   r1, r0, _break4
        
        # clear r20
        xor r20, r20, r20
        nop
_break4:
        nop
        nop
        nop
        # Check we didn't visit the exception code
        bne  r20, r0, _finish

# ---------- watch point tests -----------

        # Load r12 with the expected exception number
        ori      r12, r0, WTCH_EXCEPTION

        # Configure watchpoint 0
        ori  r1, r0, WTCH_ADDR0
        wcsr wp0, r1

        # Read it back again and check
        rcsr r2, wp0
        bne r2, r1, _finish

        # Configure watchpoint 1
        ori  r1, r0, WTCH_ADDR1
        wcsr wp1, r1

        # Read it back again and check
        rcsr r2, wp1
        bne r2, r1, _finish

        # Configure watchpoint 2
        ori  r1, r0, WTCH_ADDR2
        wcsr wp2, r1

        # Read it back again and check
        rcsr r2, wp2
        bne r2, r1, _finish

        # Configure watchpoint 3
        ori  r1, r0, WTCH_ADDR3
        wcsr wp3, r1

        # Read it back again and check
        rcsr r2, wp3
        bne r2, r1, _finish

    ### Configure DC to set watchpoint to fire on reads and test
        ori  r1, r0, (WP_READONLY << 2) | (WP_READONLY << 4) | (WP_READONLY << 6) | (WP_READONLY << 8)
        wcsr dc, r1

    ### Watchpoint 0
        # clear r20
        xor r20, r20, r20

        # Read wp0 location
        ori r1, r0, WTCH_ADDR0 & 0xffff
        lw  r2, (r1 + 0)

        # Check we visited the exception code
        bne  r20, r12, _finish

        # clear r20
        xor r20, r20, r20

        # Do a write to the location
        ori r1, r0, WTCH_ADDR0 & 0xffff
        sw  (r1 + 0), r2

        # Check we didn't visit the exception code
        bne  r20, r0, _finish

        # Disable watchpoint
        ori  r1, r0, (WP_DISABLE << 2) | (WP_READONLY << 4) | (WP_READONLY << 6) | (WP_READONLY << 8)
        wcsr dc, r1

        # clear r20
        xor r20, r20, r20

        # Read wp0 location
        ori r1, r0, WTCH_ADDR0 & 0xffff
        lw  r2, (r1 + 0)

        # Check we didn't visit the exception code
        bne  r20, r0, _finish

    ### Watchpoint 1
        # clear r20
        xor r20, r20, r20

        # Read wp1 location
        ori r1, r0, WTCH_ADDR1 & 0xffff
        lb  r2, (r1 + 0)

        # Check we visited the exception code
        bne  r20, r12, _finish

        # clear r20
        xor r20, r20, r20

        # Do a write to the location
        ori r1, r0, WTCH_ADDR1 & 0xffff
        sb  (r1 + 0), r2

        # Check we didn't visit the exception code
        bne  r20, r0, _finish

        # Disable watchpoint
        ori  r1, r0, (WP_READONLY << 2) | (WP_DISABLE << 4) | (WP_READONLY << 6) | (WP_READONLY << 8)
        wcsr dc, r1

        # clear r20
        xor r20, r20, r20

        # Read wp1 location
        ori r1, r0, WTCH_ADDR1 & 0xffff
        lb  r2, (r1 + 0)

        # Check we didn't visit the exception code
        bne  r20, r0, _finish

    ### Watchpoint 2
        # clear r20
        xor r20, r20, r20

        # Read wp2 location
        ori r1, r0, WTCH_ADDR2 & 0xffff
        lh  r2, (r1 + 0)

        # Check we visited the exception code
        bne  r20, r12, _finish

        # clear r20
        xor r20, r20, r20

        # Do a write to the location
        ori r1, r0, WTCH_ADDR2 & 0xffff
        sh  (r1 + 0), r2

        # Check we didn't visit the exception code
        bne  r20, r0, _finish

        # Disable watchpoint
        ori  r1, r0, (WP_READONLY << 2) | (WP_READONLY << 4) | (WP_DISABLE << 6) | (WP_READONLY << 8)
        wcsr dc, r1

        # clear r20
        xor r20, r20, r20

        # Read wp2 location
        ori r1, r0, WTCH_ADDR2 & 0xffff
        lh  r2, (r1 + 0)

        # Check we didn't visit the exception code
        bne  r20, r0, _finish

    ### Watchpoint 3
        # clear r20
        xor r20, r20, r20

        # Read wp2 location
        ori r1, r0, WTCH_ADDR3 & 0xffff
        lb  r2, (r1 + 0)

        # Check we visited the exception code
        bne  r20, r12, _finish

        # clear r20
        xor r20, r20, r20

        # Do a write to the location
        ori r1, r0, WTCH_ADDR3 & 0xffff
        sb  (r1 + 0), r2

        # Check we didn't visit the exception code
        bne  r20, r0, _finish

        # Disable watchpoint
        ori  r1, r0, (WP_READONLY << 2) | (WP_READONLY << 4) | (WP_READONLY << 6) | (WP_DISABLE << 8)
        wcsr dc, r1

        # clear r20
        xor r20, r20, r20

        # Read wp3 location
        ori r1, r0, WTCH_ADDR3 & 0xffff
        lb  r2, (r1 + 0)

        # Check we didn't visit the exception code
        bne  r20, r0, _finish

    ### Configure DC to set watchpoint to fire on writes and test
        ori  r1, r0, (WP_WRITEONLY << 2) | (WP_WRITEONLY << 4) | (WP_WRITEONLY << 6) | (WP_WRITEONLY << 8)
        wcsr dc, r1

    ### Watchpoint 0
        # clear r20
        xor r20, r20, r20

        # write wp0 location
        ori r1, r0, WTCH_ADDR0 & 0xffff
        sw  (r1 + 0), r2

        # Check we visited the exception code
        bne  r20, r12, _finish

        # clear r20
        xor r20, r20, r20

        # Do a read to the location
        ori r1, r0, WTCH_ADDR0 & 0xffff
        lw  r2, (r1 + 0)

        # Check we didn't visit the exception code
        bne  r20, r0, _finish

        # Disable watchpoint
        ori  r1, r0, (WP_DISABLE << 2) | (WP_WRITEONLY << 4) | (WP_WRITEONLY << 6) | (WP_WRITEONLY << 8)
        wcsr dc, r1

        # clear r20
        xor r20, r20, r20

        # Write wp0 location
        ori r1, r0, WTCH_ADDR0 & 0xffff
        sw  (r1 + 0), r2

        # Check we didn't visit the exception code
        bne  r20, r0, _finish

    ### Watchpoint 1
        # clear r20
        xor r20, r20, r20

        # write wp1 location
        ori r1, r0, WTCH_ADDR1 & 0xffff
        sb  (r1 + 0), r2

        # Check we visited the exception code
        bne  r20, r12, _finish

        # clear r20
        xor r20, r20, r20

        # Do a read to the location
        ori r1, r0, WTCH_ADDR1 & 0xffff
        lb  r2, (r1 + 0)

        # Check we didn't visit the exception code
        bne  r20, r0, _finish

        # Disable watchpoint
        ori  r1, r0, (WP_WRITEONLY << 2) | (WP_DISABLE << 4) | (WP_WRITEONLY << 6) | (WP_WRITEONLY << 8)
        wcsr dc, r1

        # clear r20
        xor r20, r20, r20

        # Write wp1 location
        ori r1, r0, WTCH_ADDR1 & 0xffff
        sb  (r1 + 0), r2

        # Check we didn't visit the exception code
        bne  r20, r0, _finish

    ### Watchpoint 2
        # clear r20
        xor r20, r20, r20

        # write wp2 location
        ori r1, r0, WTCH_ADDR2 & 0xffff
        sh  (r1 + 0), r2

        # Check we visited the exception code
        bne  r20, r12, _finish

        # clear r20
        xor r20, r20, r20

        # Do a read to the location
        ori r1, r0, WTCH_ADDR2 & 0xffff
        lh  r2, (r1 + 0)

        # Check we didn't visit the exception code
        bne  r20, r0, _finish

        # Disable watchpoint
        ori  r1, r0, (WP_WRITEONLY << 2) | (WP_WRITEONLY << 4) | (WP_DISABLE << 6) | (WP_WRITEONLY << 8)
        wcsr dc, r1

        # clear r20
        xor r20, r20, r20

        # Write wp2 location
        ori r1, r0, WTCH_ADDR2 & 0xffff
        sh  (r1 + 0), r2

        # Check we didn't visit the exception code
        bne  r20, r0, _finish

    ### Watchpoint 3
        # clear r20
        xor r20, r20, r20

        # write wp3 location
        ori r1, r0, WTCH_ADDR3 & 0xffff
        sb  (r1 + 0), r2

        # Check we visited the exception code
        bne  r20, r12, _finish

        # clear r20
        xor r20, r20, r20

        # Do a read to the location
        ori r1, r0, WTCH_ADDR3 & 0xffff
        lb  r2, (r1 + 0)

        # Check we didn't visit the exception code
        bne  r20, r0, _finish

        # Disable watchpoint
        ori  r1, r0, (WP_WRITEONLY << 2) | (WP_WRITEONLY << 4) | (WP_WRITEONLY << 6) | (WP_DISABLE << 8)
        wcsr dc, r1

        # clear r20
        xor r20, r20, r20

        # Write wp3 location
        ori r1, r0, WTCH_ADDR3 & 0xffff
        sb  (r1 + 0), r2

        # Check we didn't visit the exception code
        bne  r20, r0, _finish

    ### Configure DC to set watchpoint to fire on reads or writes and test
        ori  r1, r0, (WP_READWRITE << 2) | (WP_READWRITE << 4) | (WP_READWRITE << 6) | (WP_READWRITE << 8)
        wcsr dc, r1

    ### Watchpoint 0
        # clear r20
        xor r20, r20, r20

        # write wp0 location
        ori r1, r0, WTCH_ADDR0 & 0xffff
        sw  (r1 + 0), r2

        # Check we visited the exception code
        bne  r20, r12, _finish

        # clear r20
        xor r20, r20, r20

        # Do a read to the location
        ori r1, r0, WTCH_ADDR0 & 0xffff
        lw  r2, (r1 + 0)

        # Check we visited the exception code
        bne  r20, r12, _finish

        # Disable watchpoint
        ori  r1, r0, (WP_DISABLE << 2) | (WP_READWRITE << 4) | (WP_READWRITE << 6) | (WP_READWRITE << 8)
        wcsr dc, r1

        # clear r20
        xor r20, r20, r20

        # Write wp0 location
        ori r1, r0, WTCH_ADDR0 & 0xffff
        sw  (r1 + 0), r2

        # Check we didn't visit the exception code
        bne  r20, r0, _finish

        # Read wp0 location
        ori r1, r0, WTCH_ADDR0 & 0xffff
        lw  r2, (r1 + 0)

        # Check we didn't visit the exception code
        bne  r20, r0, _finish

    ### Watchpoint 1
        # clear r20
        xor r20, r20, r20

        # write wp1 location
        ori r1, r0, WTCH_ADDR1 & 0xffff
        sb  (r1 + 0), r2

        # Check we visited the exception code
        bne  r20, r12, _finish

        # clear r20
        xor r20, r20, r20

        # Do a read to the location
        ori r1, r0, WTCH_ADDR1 & 0xffff
        lb  r2, (r1 + 0)

        # Check we visited the exception code
        bne  r20, r12, _finish

        # Disable watchpoint
        ori  r1, r0, (WP_READWRITE << 2) | (WP_DISABLE << 4) | (WP_READWRITE << 6) | (WP_READWRITE << 8)
        wcsr dc, r1

        # clear r20
        xor r20, r20, r20

        # Write wp1 location
        ori r1, r0, WTCH_ADDR1 & 0xffff
        sb  (r1 + 0), r2

        # Check we didn't visit the exception code
        bne  r20, r0, _finish

        # Read wp1 location
        ori r1, r0, WTCH_ADDR1 & 0xffff
        lb  r2, (r1 + 0)

        # Check we didn't visit the exception code
        bne  r20, r0, _finish

    ### Watchpoint 2
        # clear r20
        xor r20, r20, r20

        # write wp2 location
        ori r1, r0, WTCH_ADDR2 & 0xffff
        sh  (r1 + 0), r2

        # Check we visited the exception code
        bne  r20, r12, _finish

        # clear r20
        xor r20, r20, r20

        # Do a read to the location
        ori r1, r0, WTCH_ADDR2 & 0xffff
        lh  r2, (r1 + 0)

        # Check we visited the exception code
        bne  r20, r12, _finish

        # Disable watchpoint
        ori  r1, r0, (WP_READWRITE << 2) | (WP_READWRITE << 4) | (WP_DISABLE << 6) | (WP_READWRITE << 8)
        wcsr dc, r1

        # clear r20
        xor r20, r20, r20

        # Write wp2 location
        ori r1, r0, WTCH_ADDR2 & 0xffff
        sh  (r1 + 0), r2

        # Check we didn't visit the exception code
        bne  r20, r0, _finish

        # Read wp2 location
        ori r1, r0, WTCH_ADDR2 & 0xffff
        lh  r2, (r1 + 0)

        # Check we didn't visit the exception code
        bne  r20, r0, _finish

    ### Watchpoint 3
        # clear r20
        xor r20, r20, r20

        # write wp3 location
        ori r1, r0, WTCH_ADDR3 & 0xffff
        sb  (r1 + 0), r2

        # Check we visited the exception code
        bne  r20, r12, _finish

        # clear r20
        xor r20, r20, r20

        # Do a read to the location
        ori r1, r0, WTCH_ADDR3 & 0xffff
        lb  r2, (r1 + 0)

        # Check we visited the exception code
        bne  r20, r12, _finish

        # Disable watchpoint
        ori  r1, r0, (WP_READWRITE << 2) | (WP_READWRITE << 4) | (WP_READWRITE << 6) | (WP_DISABLE << 8)
        wcsr dc, r1

        # clear r20
        xor r20, r20, r20

        # Write wp3 location
        ori r1, r0, WTCH_ADDR3 & 0xffff
        sb  (r1 + 0), r2

        # Check we didn't visit the exception code
        bne  r20, r0, _finish

        # Read wp3 location
        ori r1, r0, WTCH_ADDR3 & 0xffff
        lb  r3, (r1 + 0)

        # Check we didn't visit the exception code
        bne  r20, r0, _finish

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

