.syntax unified
.thumb

.extern STRING_BUFFER
.extern STRING_LEN 
.extern STRING_MAX_SIZE

.global CLEAR_STRING_BUFFER
.global SB_APPEND_CHAR
.global SB_DETERMINE_ASCII

.text 
.thumb_func
CLEAR_STRING_BUFFER:
push {r4, lr}
    ldr r0, =STRING_BUFFER
    movs r1, #0
    movs r2, #8

CLEAR_SB_LOOP_8:
    movs r3, #32

CLEAR_SB_LOOP_32:
    strb r1, [r0]
    adds r0, r0, #1
    subs r3, r3, #1
    bne CLEAR_SB_LOOP_32

    subs r2, r2, #1
    bne CLEAR_SB_LOOP_8

    ldr r0, =STRING_LEN
    movs r1, #0
    strb r1, [r0]
pop {r4, pc}


@ check STRING_LEN -> iff LEN > 255 break 
@ slide to LEN + 1 -> append 
@ ++LEN all values are 0 @ init so no need to add null term 
@ r0 should hold the char to append after its been verified as a char or a "?"
SB_APPEND_CHAR:
push {r4, lr}
    ldr r3, =STRING_LEN
    ldrb r1, [r3]

    cmp r1, #255 
    beq END_SB_APPEND

    ldr r2, =STRING_BUFFER
    adds r2, r2, r1 
    strb r0, [r2]

    adds r1, r1, #1
    strb r1, [r3]

    ldr r2, =STRING_BUFFER
    adds r2, r2, r1
    movs r0, #0
    strb r0, [r2]

    END_SB_APPEND:
pop {r4, pc}

@ this ought to pass r0 as either a valid char or a "?"
@ valid chars 32 - 126
SB_DETERMINE_ASCII:
push {r4, lr}
    cmp r0, #32
    blo END_SB_DETERMINE_ASCII_NONPRINTABLE

    cmp r0, #126
    bhi END_SB_DETERMINE_ASCII_NONPRINTABLE

pop {r4, pc}

    END_SB_DETERMINE_ASCII_NONPRINTABLE:
    movs r0, #63
pop {r4, pc}