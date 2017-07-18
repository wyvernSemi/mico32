# ----------------------------------------------------------------
# Tests conditional branching instructions of the MICO32 processor
# ----------------------------------------------------------------

        .file            "test.s"
        .text
        .align 4
_start: .global           _start
        .global           main

        .equ FAIL_VALUE,  0x0bad 
        .equ PASS_VALUE,  0x0900d
        .equ RESULT_ADDR, 0xfffc
        .equ SIGN_EXTD16, 0xffff
        
        .equ TESTVAL1,    0x99999999
        .equ TESTVAL2,    0x2823654a
        .equ TESTVAL3,    0xa39874e2

main:
        xor      r0, r0, r0

        # By default, set the result to bad
        ori      r30, r0, 0
        ori      r31, r0, RESULT_ADDR
        sw 	 (r31+0), r30

# -------------- BE tests ----------------

       # BE positive jump pass
       ori      r1, r0, TESTVAL1 & 0xffff
       orhi     r1, r1, (TESTVAL1>>16) & 0xffff
       or       r2, r0, r1
       be       r1, r2, 8
       be       r0, r0, _finish
       
       # BE negative jump pass
       ori      r1, r0, TESTVAL1 & 0xffff
       orhi     r1, r1, (TESTVAL1>>16) & 0xffff
       or       r2, r0, r1
       bi       _jmp2
_jmp1: bi       _ok1
       nop
       nop
_jmp2:
       be       r1, r2, _jmp1
       be       r0, r0, _finish
_ok1:
       
       # BE jump fail
       ori      r1, r0, TESTVAL1 & 0xffff
       orhi     r1, r1, (TESTVAL1>>16) & 0xffff
       or       r2, r0, r1
       xori     r2, r1, 0x4000
       be       r1, r2, 8
       bi       8
       be       r0, r0, _finish
       
# -------------- BNE tests ---------------

       # BNE positive jump pass
       ori      r1, r0, TESTVAL1 & 0xffff
       orhi     r1, r1, (TESTVAL1>>16) & 0xffff
       addi     r2, r1, 1
       bne      r1, r2, 8
       be       r0, r0, _finish
       
       # BNE negative jump pass
       ori      r1, r0, TESTVAL1 & 0xffff
       orhi     r1, r1, (TESTVAL1>>16) & 0xffff
       addi     r2, r1, -1
       bi       _jmp12
_jmp11: bi       _ok11
       nop
       nop
_jmp12:
       bne      r1, r2, _jmp11
       be       r0, r0, _finish
_ok11:

       # BNE fail
       ori      r1, r0, TESTVAL1 & 0xffff
       orhi     r1, r1, (TESTVAL1>>16) & 0xffff
       or       r2, r1, r0
       bne      r1, r2, 8
       bi       8
       be       r0, r0, _finish
       
# -------------- BG tests ----------------

       # BG positive jmp, positive numbers pass
       ori      r1, r0, TESTVAL2 & 0xffff
       orhi     r1, r1, (TESTVAL2>>16) & 0xffff
       addi     r2, r1, -1
       bg       r1, r2, 8
       be       r0, r0, _finish

       # BG negative jmp, positive numbers pass
       ori      r1, r0, TESTVAL2 & 0xffff
       orhi     r1, r1, (TESTVAL2>>16) & 0xffff
       addi     r2, r1, -20872
       bi       _jmp22
_jmp21: bi       _ok21
       nop
       nop
_jmp22:
       bg       r1, r2, _jmp21
       be       r0, r0, _finish
_ok21:

       # BG fail, positive numbers 
       ori      r1, r0, TESTVAL2 & 0xffff
       orhi     r1, r1, (TESTVAL2>>16) & 0xffff
       addi     r2, r1, 1
       bg       r1, r2, 8
       be       r0, r0, 8
       be       r0, r0, _finish

       # BG fail, equal positive numbers 
       ori      r1, r0, TESTVAL2 & 0xffff
       orhi     r1, r1, (TESTVAL2>>16) & 0xffff
       addi     r2, r1, 0
       bg       r1, r2, 8
       be       r0, r0, 8
       be       r0, r0, _finish

       # BG positive jmp, negative numbers pass
       ori      r1, r0, TESTVAL3 & 0xffff
       orhi     r1, r1, (TESTVAL3>>16) & 0xffff
       addi     r2, r1, -1
       bg       r1, r2, 8
       be       r0, r0, _finish

       # BG negative jmp, negative numbers pass
       ori      r1, r0, TESTVAL3 & 0xffff
       orhi     r1, r1, (TESTVAL3>>16) & 0xffff
       addi     r2, r1, -20872
       bi       _jmp32
_jmp31: bi       _ok31
       nop
       nop
_jmp32:
       bg       r1, r2, _jmp31
       be       r0, r0, _finish
_ok31:

       # BG fail, negative numbers 
       ori      r1, r0, TESTVAL3 & 0xffff
       orhi     r1, r1, (TESTVAL3>>16) & 0xffff
       addi     r2, r1, 1
       bg       r1, r2, 8
       be       r0, r0, 8
       be       r0, r0, _finish

       # BG fail, equal negative numbers 
       ori      r1, r0, TESTVAL3 & 0xffff
       orhi     r1, r1, (TESTVAL3>>16) & 0xffff
       addi     r2, r1, 0
       bg       r1, r2, 8
       be       r0, r0, 8
       be       r0, r0, _finish

       # BG positive jmp, pos/neg numbers pass
       ori      r1, r0, TESTVAL2 & 0xffff
       orhi     r1, r1, (TESTVAL2>>16) & 0xffff
       ori      r2, r0, TESTVAL3 & 0xffff
       orhi     r2, r1, (TESTVAL3>>16) & 0xffff
       bg       r1, r2, 8
       be       r0, r0, _finish

       # BG negative jmp, pos/neg numbers pass
       ori      r1, r0, TESTVAL2 & 0xffff
       orhi     r1, r1, (TESTVAL2>>16) & 0xffff
       ori      r2, r0, TESTVAL3 & 0xffff
       orhi     r2, r1, (TESTVAL3>>16) & 0xffff
       bi       _jmp42
_jmp41: bi       _ok41
       nop
       nop
_jmp42:
       bg       r1, r2, _jmp41
       be       r0, r0, _finish
_ok41:

       # BG fail, pos/neg numbers 
       ori      r1, r0, TESTVAL2 & 0xffff
       orhi     r1, r1, (TESTVAL2>>16) & 0xffff
       ori      r2, r0, TESTVAL3 & 0xffff
       orhi     r2, r1, (TESTVAL3>>16) & 0xffff
       bg       r2, r1, 8
       be       r0, r0, 8
       be       r0, r0, _finish

# -------------- BGU tests ---------------

       # BGU positive jmp, positive numbers pass
       ori      r1, r0, TESTVAL2 & 0xffff
       orhi     r1, r1, (TESTVAL2>>16) & 0xffff
       addi     r2, r1, -1
       bgu      r1, r2, 8
       be       r0, r0, _finish

       # BGU negative jmp, positive numbers pass
       ori      r1, r0, TESTVAL2 & 0xffff
       orhi     r1, r1, (TESTVAL2>>16) & 0xffff
       addi     r2, r1, -20872
       bi       _jmp52
_jmp51: bi       _ok51
       nop
       nop
_jmp52:
       bgu      r1, r2, _jmp51
       be       r0, r0, _finish
_ok51:

       # BGU fail, positive numbers 
       ori      r1, r0, TESTVAL2 & 0xffff
       orhi     r1, r1, (TESTVAL2>>16) & 0xffff
       addi     r2, r1, 1
       bgu      r1, r2, 8
       be       r0, r0, 8
       be       r0, r0, _finish

       # BGU fail, equal positive numbers 
       ori      r1, r0, TESTVAL2 & 0xffff
       orhi     r1, r1, (TESTVAL2>>16) & 0xffff
       addi     r2, r1, 0
       bgu      r1, r2, 8
       be       r0, r0, 8
       be       r0, r0, _finish

       # BGU positive jmp, negative numbers pass
       ori      r1, r0, TESTVAL3 & 0xffff
       orhi     r1, r1, (TESTVAL3>>16) & 0xffff
       addi     r2, r1, -1
       bgu      r1, r2, 8
       be       r0, r0, _finish

       # BGU negative jmp, negative numbers pass
       ori      r1, r0, TESTVAL3 & 0xffff
       orhi     r1, r1, (TESTVAL3>>16) & 0xffff
       addi     r2, r1, -20872
       bi       _jmp62
_jmp61: bi       _ok61
       nop
       nop
_jmp62:
       bgu      r1, r2, _jmp61
       be       r0, r0, _finish
_ok61:

       # BGU fail, negative numbers 
       ori      r1, r0, TESTVAL3 & 0xffff
       orhi     r1, r1, (TESTVAL3>>16) & 0xffff
       addi     r2, r1, 1
       bgu      r1, r2, 8
       be       r0, r0, 8
       be       r0, r0, _finish

       # BGU fail, equal negative numbers 
       ori      r1, r0, TESTVAL3 & 0xffff
       orhi     r1, r1, (TESTVAL3>>16) & 0xffff
       addi     r2, r1, 0
       bgu      r1, r2, 8
       be       r0, r0, 8
       be       r0, r0, _finish

       # BGU positive jmp, pos/neg numbers pass
       ori      r1, r0, TESTVAL2 & 0xffff
       orhi     r1, r1, (TESTVAL2>>16) & 0xffff
       ori      r2, r0, TESTVAL3 & 0xffff
       orhi     r2, r1, (TESTVAL3>>16) & 0xffff
       bgu      r2, r1, 8
       be       r0, r0, _finish

       # BGU negative jmp, pos/neg numbers pass
       ori      r1, r0, TESTVAL2 & 0xffff
       orhi     r1, r1, (TESTVAL2>>16) & 0xffff
       ori      r2, r0, TESTVAL3 & 0xffff
       orhi     r2, r1, (TESTVAL3>>16) & 0xffff
       bi       _jmp72
_jmp71: bi       _ok71
       nop
       nop
_jmp72:
       bgu      r2, r1, _jmp71
       be       r0, r0, _finish
_ok71:

       # BGU fail, pos/neg numbers 
       ori      r1, r0, TESTVAL2 & 0xffff
       orhi     r1, r1, (TESTVAL2>>16) & 0xffff
       ori      r2, r0, TESTVAL3 & 0xffff
       orhi     r2, r1, (TESTVAL3>>16) & 0xffff
       bgu      r1, r2, 8
       be       r0, r0, 8
       be       r0, r0, _finish

# -------------- BGE tests ---------------

       # BGE positive jmp, positive numbers pass
       ori      r1, r0, TESTVAL2 & 0xffff
       orhi     r1, r1, (TESTVAL2>>16) & 0xffff
       addi     r2, r1, -1
       bge      r1, r2, 8
       be       r0, r0, _finish

       # BGE negative jmp, positive numbers pass
       ori      r1, r0, TESTVAL2 & 0xffff
       orhi     r1, r1, (TESTVAL2>>16) & 0xffff
       addi     r2, r1, -20872
       bi       _jmp82
_jmp81: bi       _ok81
       nop
       nop
_jmp82:
       bge      r1, r2, _jmp81
       be       r0, r0, _finish
_ok81:

       # BGE fail, positive numbers 
       ori      r1, r0, TESTVAL2 & 0xffff
       orhi     r1, r1, (TESTVAL2>>16) & 0xffff
       addi     r2, r1, 1
       bge      r1, r2, 8
       be       r0, r0, 8
       be       r0, r0, _finish

       # BGE positive jmp, negative numbers pass
       ori      r1, r0, TESTVAL3 & 0xffff
       orhi     r1, r1, (TESTVAL3>>16) & 0xffff
       addi     r2, r1, -1
       bge      r1, r2, 8
       be       r0, r0, _finish

       # BGE negative jmp, negative numbers pass
       ori      r1, r0, TESTVAL3 & 0xffff
       orhi     r1, r1, (TESTVAL3>>16) & 0xffff
       addi     r2, r1, -20872
       bi       _jmp92
_jmp91: bi       _ok91
       nop
       nop
_jmp92:
       bge      r1, r2, _jmp91
       be       r0, r0, _finish
_ok91:

       # BGE fail, negative numbers 
       ori      r1, r0, TESTVAL3 & 0xffff
       orhi     r1, r1, (TESTVAL3>>16) & 0xffff
       addi     r2, r1, 1
       bge      r1, r2, 8
       be       r0, r0, 8
       be       r0, r0, _finish

       # BGE positive jmp, pos/neg numbers pass
       ori      r1, r0, TESTVAL2 & 0xffff
       orhi     r1, r1, (TESTVAL2>>16) & 0xffff
       ori      r2, r0, TESTVAL3 & 0xffff
       orhi     r2, r1, (TESTVAL3>>16) & 0xffff
       bge      r1, r2, 8
       be       r0, r0, _finish

       # BGE negative jmp, pos/neg numbers pass
       ori      r1, r0, TESTVAL2 & 0xffff
       orhi     r1, r1, (TESTVAL2>>16) & 0xffff
       ori      r2, r0, TESTVAL3 & 0xffff
       orhi     r2, r1, (TESTVAL3>>16) & 0xffff
       bi       _jmp102
_jmp101: bi       _ok101
       nop
       nop
_jmp102:
       bge      r1, r2, _jmp101
       be       r0, r0, _finish
_ok101:

       # BGE fail, pos/neg numbers 
       ori      r1, r0, TESTVAL2 & 0xffff
       orhi     r1, r1, (TESTVAL2>>16) & 0xffff
       ori      r2, r0, TESTVAL3 & 0xffff
       orhi     r2, r1, (TESTVAL3>>16) & 0xffff
       bge      r2, r1, 8
       be       r0, r0, 8
       be       r0, r0, _finish

       # BGE positive jmp, equal positive numbers pass
       ori      r1, r0, TESTVAL2 & 0xffff
       orhi     r1, r1, (TESTVAL2>>16) & 0xffff
       addi     r2, r1, 0
       bge      r1, r2, 8
       be       r0, r0, _finish

       # BGE negative jmp, equal positive numbers pass
       ori      r1, r0, TESTVAL2 & 0xffff
       orhi     r1, r1, (TESTVAL2>>16) & 0xffff
       addi     r2, r1, 0
       bi       _jmp112
_jmp111: bi       _ok111
       nop
       nop
_jmp112:
       bge      r1, r2, _jmp111
       be       r0, r0, _finish
_ok111:

       # BGE positive jmp, equal negative numbers pass
       ori      r1, r0, TESTVAL3 & 0xffff
       orhi     r1, r1, (TESTVAL3>>16) & 0xffff
       addi     r2, r1, 0
       bge      r1, r2, 8
       be       r0, r0, _finish

       # BGE negative jmp, equal negative numbers pass
       ori      r1, r0, TESTVAL3 & 0xffff
       orhi     r1, r1, (TESTVAL3>>16) & 0xffff
       addi     r2, r1, 0
       bi       _jmp122
_jmp121: bi       _ok121
       nop
       nop
_jmp122:
       bge      r1, r2, _jmp121
       be       r0, r0, _finish
_ok121:

# -------------- BGEU tests --------------

       # BGEU positive jmp, positive numbers pass
       ori      r1, r0, TESTVAL2 & 0xffff
       orhi     r1, r1, (TESTVAL2>>16) & 0xffff
       addi     r2, r1, -1
       bgeu     r1, r2, 8
       be       r0, r0, _finish

       # BGEU negative jmp, positive numbers pass
       ori      r1, r0, TESTVAL2 & 0xffff
       orhi     r1, r1, (TESTVAL2>>16) & 0xffff
       addi     r2, r1, -31999
       bi       _jmp132
_jmp131: bi       _ok131
       nop
       nop
_jmp132:
       bgeu     r1, r2, _jmp131
       be       r0, r0, _finish
_ok131:

       # BGEU fail, positive numbers 
       ori      r1, r0, TESTVAL2 & 0xffff
       orhi     r1, r1, (TESTVAL2>>16) & 0xffff
       addi     r2, r1, 1
       bgeu     r1, r2, 8
       be       r0, r0, 8
       be       r0, r0, _finish

       # BGEU positive jmp, negative numbers pass
       ori      r1, r0, TESTVAL3 & 0xffff
       orhi     r1, r1, (TESTVAL3>>16) & 0xffff
       addi     r2, r1, -1
       bgeu     r1, r2, 8
       be       r0, r0, _finish

       # BGEU negative jmp, negative numbers pass
       ori      r1, r0, TESTVAL3 & 0xffff
       orhi     r1, r1, (TESTVAL3>>16) & 0xffff
       addi     r2, r1, -20872
       bi       _jmp142
_jmp141: bi       _ok141
       nop
       nop
_jmp142:
       bgeu     r1, r2, _jmp141
       be       r0, r0, _finish
_ok141:

       # BGEU fail, negative numbers 
       ori      r1, r0, TESTVAL3 & 0xffff
       orhi     r1, r1, (TESTVAL3>>16) & 0xffff
       addi     r2, r1, 1
       bgeu     r1, r2, 8
       be       r0, r0, 8
       be       r0, r0, _finish

       # BGEU positive jmp, pos/neg numbers pass
       ori      r1, r0, TESTVAL2 & 0xffff
       orhi     r1, r1, (TESTVAL2>>16) & 0xffff
       ori      r2, r0, TESTVAL3 & 0xffff
       orhi     r2, r1, (TESTVAL3>>16) & 0xffff
       bgeu     r2, r1, 8
       be       r0, r0, _finish

       # BGEU negative jmp, pos/neg numbers pass
       ori      r1, r0, TESTVAL2 & 0xffff
       orhi     r1, r1, (TESTVAL2>>16) & 0xffff
       ori      r2, r0, TESTVAL3 & 0xffff
       orhi     r2, r1, (TESTVAL3>>16) & 0xffff
       bi       _jmp152
_jmp151: bi       _ok151
       nop
       nop
_jmp152:
       bgeu     r2, r1, _jmp151
       be       r0, r0, _finish
_ok151:

       # BGEU fail, pos/neg numbers 
       ori      r1, r0, TESTVAL2 & 0xffff
       orhi     r1, r1, (TESTVAL2>>16) & 0xffff
       ori      r2, r0, TESTVAL3 & 0xffff
       orhi     r2, r1, (TESTVAL3>>16) & 0xffff
       bgeu     r1, r2, 8
       be       r0, r0, 8
       be       r0, r0, _finish

       # BGEU positive jmp, equal positive numbers pass
       ori      r1, r0, TESTVAL2 & 0xffff
       orhi     r1, r1, (TESTVAL2>>16) & 0xffff
       addi     r2, r1, 0
       bgeu     r1, r2, 8
       be       r0, r0, _finish

       # BGEU negative jmp, equal positive numbers pass
       ori      r1, r0, TESTVAL2 & 0xffff
       orhi     r1, r1, (TESTVAL2>>16) & 0xffff
       addi     r2, r1, 0
       bi       _jmp162
_jmp161: bi       _ok161
       nop
       nop
_jmp162:
       bgeu     r1, r2, _jmp161
       be       r0, r0, _finish
_ok161:

       # BGEU positive jmp, equal negative numbers pass
       ori      r1, r0, TESTVAL3 & 0xffff
       orhi     r1, r1, (TESTVAL3>>16) & 0xffff
       addi     r2, r1, 0
       bgeu     r1, r2, 8
       be       r0, r0, _finish

       # BGEU negative jmp, equal negative numbers pass
       ori      r1, r0, TESTVAL3 & 0xffff
       orhi     r1, r1, (TESTVAL3>>16) & 0xffff
       addi     r2, r1, 0
       bi       _jmp172
_jmp171: bi       _ok171
       nop
       nop
_jmp172:
       bgeu     r1, r2, _jmp171
       be       r0, r0, _finish
_ok171:



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

