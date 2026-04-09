.syntax unified
.thumb

.extern MEM_BUFFER
.extern PTR 
.extern PAGE_SIZE

.global MEMORY_INIT
.global MEMORY_SET_LOCATION
.global MEMORY_READ
.global MEMORY_WRITE


.thumb_func
MEMORY_INIT:
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
    uxtb r0, r0 @wrap around for 0-255
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
MEMORY_WRITE: @ig whatever the pointer is at the time we write to
    ldr r1, =PTR
    ldrb r1, [r1]

    ldr r2, =MEM_BUFFER
    add r2, r2, r1

    strb r0, [r2]
bx lr 

.thumb_func 
MEMORY_RESET_POINTER:
    movs r1, #0x00
    ldr r0, =PTR 
    strb r1, [r0]
bx lr 





