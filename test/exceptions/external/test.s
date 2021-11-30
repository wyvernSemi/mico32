        .section        .boot,  "ax",   @progbits

        .global _start  
_start:         

        .equ FAIL_VALUE,          0x0bad 
        .equ PASS_VALUE,          0x0900d
        .equ RESULT_ADDR,         0xfffc

        .equ COMMS_BASE_ADDR,      0x20000000
        .equ COMMS_PATTERN_OFFSET, 0x00000000
        .equ COMMS_TIME_OFFSET,    0x00000004
        .equ COMMS_NUM_INT_OFFSET, 0x00000008
        .equ COMMS_RESET_OFFSET,   0x00000028

        .equ LM32_RUN_FROM_RESET,         0
        .equ LM32_RUN_CONTINUE,           1
        .equ LM32_RUN_SINGLE_STEP,        2
        .equ LM32_RUN_TICK,               3

        .equ RESTART_ADDR,         0x7000
        .equ RESTART_ID,           0x0000b00b

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
        lw  r1, (r0+RESTART_ADDR)
        ori r2, r0, RESTART_ID
#ifndef WY_RTL       
        be  r2, r1, _restart
#else
        nop
#endif
        bi main
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
        ret
_raise:
        # Copy  Exception number in r2 to r20
        or  r20, r0, r2

        # Copy interrupt pending register to r21
        rcsr r21, IP

        # Check interrupts are disabled and EIE set
        rcsr r2, IE
        ori  r1, r0, 2
        bne  r2, r1, _finish

        # Check IP is that set just before interrupt
        rcsr r2, IP
        bne r2, r3, _finish

        # Clear interrupt and check (value in r3)
        wcsr IP, r3
        rcsr r2, IP
        bne  r2, r0, _finish

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

# ----------- interrupt tests ------------

        # Enable interrupts
        ori  r1, r0, 1
        wcsr IE, r1

        # r3 is our interrupt input, and will be used for checking in the exception code.
        # Initilised to the first interrupt input
        ori  r3, r0, 1

        # Set r2 to point to external mailbox
        orhi r2, r0, (COMMS_BASE_ADDR >> 16) & 0xffff

_loop1:
        ori r20, r0, 0

        # Mask interrupt 
        wcsr IM, r0

        # Set a maximum loop count to wait for interrupt (r4) 
        ori  r4, r0, 20

        # Generate an interrupt immediately
        sw  (r2+COMMS_PATTERN_OFFSET), r3
        sw  (r2+COMMS_TIME_OFFSET),    r0

_loop2:
        # Wait for timeout if r4 reaches 0, and check no exeption number in r20
        addi r4, r4, -1
        be   r4, r0, _ok1
        be   r20, r0, _loop2
        be   r0, r0, _finish
_ok1:
        # Set a maximum loop count to wait for interrupt (r4) 
        ori  r4, r0, 10

        # Unmask interrupts and wait for interrupt which should be pending
        wcsr IM, r3

_loop3:
        # Wait for exception number (returned in r20) to be non-zero (or timeout if r4 reaches 0)
        addi r4, r4, -1
        be   r4, r0, _finish
        be   r20, r0, _loop3

        # Check exception was an external interrupt
        ori r4, r0, INT_EXCEPTION
        bne r20, r4, _finish

        # Loop for each interrupt
        sli r3, r3, 1
        bne r3, r0, _loop1

#ifndef WY_RTL
# --------- interrupt num tests -----------

        ori r20, r0, 0

        # Set the number of interrupts to be 1
        ori r1, r0, 1
        sw  (r2+COMMS_NUM_INT_OFFSET), r1

        # Set the interrupt input to be outside of supported interrupts
        ori r3, r0, 2

        # Unmask interrupts 
        wcsr IM, r3

        # Set a maximum loop count to wait for interrupt (r4) 
        ori  r4, r0, 20

        # Generate an interrupt immediately
        sw  (r2+COMMS_PATTERN_OFFSET), r3
        sw  (r2+COMMS_TIME_OFFSET),    r0

_loop4:
        # Wait for timeout if r4 reaches 0, and check no exeption number in r20
        addi r4, r4, -1
        be   r4, r0, _ok2
        be   r20, r0, _loop4
        be   r0, r0, _finish
_ok2:

        # Set the interrupt input to be inside of supported interrupts
        ori r3, r0, 1

        # Unmask interrupts 
        wcsr IM, r3

        # Set a maximum loop count to wait for interrupt (r4) 
        ori  r4, r0, 20

        # Generate an interrupt immediately
        sw  (r2+COMMS_PATTERN_OFFSET), r3
        sw  (r2+COMMS_TIME_OFFSET),    r0

_loop5:
        # Wait for exception number (returned in r20) to be non-zero (or timeout if r4 reaches 0)
        addi r4, r4, -1
        be   r4, r0, _finish
        be   r20, r0, _loop5

        # Test external reset

        # Set r14 to be the single set value that's to be set after reset
        ori r14, r0, LM32_RUN_SINGLE_STEP

_loop6:
        # Store a marker in memory to tell exception code to jump 
        # back here.
        ori r1, r0, RESTART_ID
        sw (r0+RESTART_ADDR), r1


        # Clear r1
        ori r1, r0, 0

        # Set r2 to point to external mailbox
        orhi r2, r0, (COMMS_BASE_ADDR >> 16) & 0xffff

        # Reset the chip (and set to exec_type in r14)
        sw  (r2+COMMS_RESET_OFFSET), r14

_restart:

        # Check that R1 has the ID
        ori r2, r0, RESTART_ID
        bne r1, r2, _finish

        # If r14 is LM32_RUN_SINGLE_STEP set to LM32_RUN_TICK and jump back to reset once more
        ori r15, r0, LM32_RUN_SINGLE_STEP
        bne r14, r15, _ok3
        ori r14, r0, LM32_RUN_TICK
        
        # Enable interrupts
        ori  r1, r0, 1
        wcsr IE, r1

        be  r0, r0, _loop6
_ok3:
#else
        be  r0, r0, _ok3
_restart:
        be r0, r0, _finish
_ok3:        
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

