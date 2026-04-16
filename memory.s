@add bias global 
@next = inc bias 
@back = dec bias 
@overflow check 
@set mem ptr 
@ the real C method doesnt touch hardware at all so from here we  will attempt to build out all logic from real asm hardware interaction 


@ALLEGEDLY IN THEORY memory.S IS NOW A STANDALONE FILE THAT SHOULD PLUG AND PLAY 
@NEEDS NEXT PAGE AND BACK PAGE


.syntax unified
.thumb

.extern MEM_BUFFER
.extern PTR 
.extern PAGE_SIZE

.global MEMORY_CLEAR_ALL
.global MEMORY_SET_LOCATION
.global MEMORY_READ
.global MEMORY_WRITE
.global MEMORY_RESET_POINTER
.global MEMORY_NEXT_PAGE
.global MEMORY_BACK_PAGE
.global READ_PINS_ASSIGN_VALUES
.global MEMORY_STEP

.bss
.align 2

@ there is a sexy way of doing this by storing each value in a 1 byte location then masking to check for logic but little old me is too scared to implement that
LOGIC_PIN_ONE:    .space 1 @ GPIO PIN 8
LOGIC_PIN_TWO:    .space 1 @ GPIO PIN 9
LOGIC_PIN_THREE:    .space 1 @ GPIO PIN 10 
LOGIC_PIN_FOUR:    .space 1 @ GPIO PIN 11
BUS_VALUE:    .space 1 @ idk if one byte is enough for this value

.text
.thumb_func
MEMORY_CLEAR_ALL:
    ldr r0, =MEM_BUFFER
    movs r1, #8
    movs r3, #0x00

    INIT_FILL_LOOP_8: @8x32=256 cannot mov{s} big numbers gg
        movs r2, #32

        INIT_FILL_LOOP_32:
            strb r3, [r0]
            adds r0, r0, #1
            subs r2, r2, #1
            cmp r2, #0
        bne INIT_FILL_LOOP_32

        subs r1, r1, #1
        cmp r1, #0 
    bne INIT_FILL_LOOP_8

    ldr r1, =PTR
    strb r3, [r1]

bx lr

.thumb_func
MEMORY_SET_LOCATION: 
@add bias multiplication logic
@wrap around for 0-255
    uxtb r0, r0 
    ldr r1, =PTR
    strb r0, [r1]
bx lr

.thumb_func
MEMORY_READ:
    ldr r1, =PTR
    ldrb r1, [r1]

    ldr r2, =MEM_BUFFER
    adds r2, r2, r1

    ldrb r0, [r2]
bx lr


.thumb_func
MEMORY_WRITE: 
@ig whatever the pointer is at the time we write to
    ldr r1, =PTR
    ldrb r1, [r1]

    ldr r2, =MEM_BUFFER
    adds r2, r2, r1

    ldr r0, =BUS_VALUE
    ldrb r0, [r0]

    strb r0, [r2]
bx lr 

.thumb_func 
MEMORY_RESET_POINTER:
    movs r1, #0x00
    ldr r0, =PTR 
    strb r1, [r0]
bx lr 

.thumb_func
MEMORY_NEXT_PAGE:
    ldr r0, =PTR
    ldrb r1, [r0]

    adds r1, r1, #16

    movs r2, #0xFF
    ands r1, r2

    strb r1, [r0]
bx lr


.thumb_func
MEMORY_BACK_PAGE:
    ldr r0, =PTR
    ldrb r1, [r0]

    subs r1, r1, #16

    movs r2, #0xFF
    ands r1, r2

    strb r1, [r0]
bx lr



@ SIO block base address 0xD0000004
@ GPIO_IN offset 4 bytes 
@ pin must be set for SIO GPIO function
@ there are 5 GPIO pins 

.thumb_func
READ_PINS_ASSIGN_VALUES:
    ldr r0, =0xD0000004
    ldr r0, [r0]
    
    lsrs r1, r0, #8 
    movs r3, #1
    ands r1, r3
    ldr r2, =LOGIC_PIN_ONE
    strb r1, [r2]

    lsrs r1, r0, #9
    movs r3, #1
    ands r1, r3
    ldr r2, =LOGIC_PIN_TWO
    strb r1, [r2]

    lsrs r1, r0, #10
    movs r3, #1
    ands r1, r3
    ldr r2, =LOGIC_PIN_THREE
    strb r1, [r2]

    lsrs r1, r0, #11
    movs r3, #1
    ands r1, r3
    ldr r2, =LOGIC_PIN_FOUR
    strb r1, [r2]

    movs r1, r0
    movs r3, #0xFF
    ands r1, r3
    ldr r2, =BUS_VALUE
    strb r1, [r2]
bx lr

.thumb_func
MEMORY_STEP:
    push {lr}
    bl READ_PINS_ASSIGN_VALUES
    
    ldr r0, =LOGIC_PIN_THREE
    ldrb r0, [r0]
    ldr r1, =LOGIC_PIN_FOUR
    ldrb r1, [r1]

    movs r2, r1
    ands r2, r0
        bne CASE_ONE

    cmp r0, #0 
        bne CASE_TWO

    cmp r1, #0
        bne CASE_THREE

    ldr r1, =LOGIC_PIN_ONE  
    ldrb r1, [r1]
    ldr r0, =LOGIC_PIN_TWO
    ldrb r0, [r0]

    movs r2, r1
    ands r2, r0
        bne CASE_FOUR   

@ if it where up to me these next two checks would only check one reg but the C code checks one pin and the NOT of another 
@ so i will faithfully translate that

    cmp r0, #0
        beq CHK_CASE_SIX

    cmp r1, #0
        beq CASE_FIVE
        b END_FUNCTION_RETURN_ZERO

    CHK_CASE_SIX:
    cmp r1, #0
        bne CASE_SIX
        b END_FUNCTION_RETURN_ZERO

    CASE_ONE:
        bl MEMORY_RESET_POINTER
        b END_FUNCTION_RETURN_ZERO

    CASE_TWO:
        bl MEMORY_NEXT_PAGE
        b END_FUNCTION_RETURN_ZERO
    CASE_THREE:
        bl MEMORY_BACK_PAGE
        b END_FUNCTION_RETURN_ZERO

    CASE_FOUR:
        ldr r0, =BUS_VALUE
        ldrb r0, [r0]
        bl MEMORY_SET_LOCATION
        b END_FUNCTION_RETURN_ZERO

    CASE_FIVE:
        ldr r0, =BUS_VALUE
        ldrb r0, [r0]
        bl MEMORY_WRITE
        b END_FUNCTION_RETURN_ZERO

    CASE_SIX: 
        bl MEMORY_READ
        b END_FUNCTION_RETURN_R0
    
    END_FUNCTION_RETURN_R0:
pop {pc}

    END_FUNCTION_RETURN_ZERO:
    movs r0, #0 @ since this will be returning to C we need to have it return zero
pop {pc}