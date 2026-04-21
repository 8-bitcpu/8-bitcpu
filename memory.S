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
.extern IN_BUS_VALUE

.global MEMORY_CLEAR_ALL
.global MEMORY_SET_LOCATION
.global MEMORY_READ
.global MEMORY_WRITE
.global MEMORY_RESET_POINTER
.global MEMORY_NEXT_PAGE
.global MEMORY_BACK_PAGE


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

    ldr r0, =IN_BUS_VALUE
    ldrb r0, [r0]

    strb r0, [r2]
bx lr 

.thumb_func 
MEMORY_RESET_POINTER:
    movs r1, #0x00
    ldr r0, =PTR 
    strb r1, [r0]
bx lr 


@ dont forget these two need to be changed so they move off biass
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