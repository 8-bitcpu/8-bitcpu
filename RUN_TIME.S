@ base versions of these methods will be written with the intention of upgrading
@ them later to report to the OLED on the pico

.syntax unified
.thumb

.extern MEM_BUFFER
.extern PTR 
.extern PAGE_SIZE
.extern run_preload_cli

.global READ_ALL_PINS
.global VERIFY_PIO_PINS_STABLE
.global VERIFY_CONTROL_PINS_STABLE
.global OUTPUT_BUS_ENABLE
.global OUTPUT_BUS_DISABLE
.global OUTPUT_BUS_WRITE
.global HANDLE_PRELOAD
.global HANDLE_HALT
.global HANDLE_IN
.global HANDLE_OUT
.global RUNTIME_STEP

.global IN_BUS_VALUE
.global GPIO_STATE
.global CONTROL_PINS_STABLE
.global PIO_PINS_STABLE
.global PREV_PIO_PINS_STABLE

.bss
.align 2

GPIO_STATE: .space 4
PREV_GPIO_STATE: .space 4

CONTROL_PINS_STABLE: .space 1
PREV_CONTROL_PINS_STABLE: .space 1

PIO_PINS_STABLE: .space 1
PREV_PIO_PINS_STABLE: .space 1

IN_BUS_VALUE: .space 1
PRE_LOAD_PIN: .space 1

.text

.thumb_func
READ_ALL_PINS:
    ldr r0, =0xD0000004
    ldr r1, [r0]

    ldr r2, =GPIO_STATE
    str r1, [r2]
bx lr

.thumb_func
VERIFY_PIO_PINS_STABLE:
push {r4, lr}
    bl READ_ALL_PINS

    ldr r2, =GPIO_STATE
    ldr r0, [r2]
    lsrs r0, r0, #20
    movs r2, #0x7
    ands r0, r2

    movs r3, #255
    DUMB_LOOP:
        subs r3, #1
        bne DUMB_LOOP

    bl READ_ALL_PINS

        ldr r2, =GPIO_STATE
        ldr r1, [r2]
        lsrs r1, r1, #20
        movs r2, #0x7
        ands r1, r2

        cmp r0, r1
    bne USE_SECOND_SAMPLE

    P_STABLE_YES:
        ldr r0, =PIO_PINS_STABLE
        strb r1, [r0]
pop {r4, pc}

    USE_SECOND_SAMPLE:
        ldr r0, =PIO_PINS_STABLE
        strb r1, [r0]
pop {r4, pc}


.thumb_func
VERIFY_CONTROL_PINS_STABLE:
push {r4, lr}
    bl READ_ALL_PINS

    ldr r2, =GPIO_STATE
    ldr r0, [r2]
    lsrs r0, r0, #8
    movs r2, #0xF
    ands r0, r2

    movs r3, #255
    DUMB_LOOP2:
        subs r3, #1
    bne DUMB_LOOP2

    bl READ_ALL_PINS

        ldr r2, =GPIO_STATE
        ldr r1, [r2]
        lsrs r1, r1, #8
        movs r2, #0xF
        ands r1, r2

        cmp r0, r1
    bne USE_SECOND_CONTROL_SAMPLE

    C_STABLE_YES:
        ldr r0, =CONTROL_PINS_STABLE
        strb r1, [r0]
pop {r4, pc}

    USE_SECOND_CONTROL_SAMPLE:
        ldr r0, =CONTROL_PINS_STABLE
        strb r1, [r0]
pop {r4, pc}

READ_IN_BUS_VALUE:
    ldr r0, =GPIO_STATE
    ldr r1, [r0]
    movs r2, #0xFF
    ands r1, r2
    ldr r0, =IN_BUS_VALUE
    strb r1, [r0]
    movs r0, r1 @ little hacky but the bus value should just be kept in r0 for better integration later
bx lr


.thumb_func
OUTPUT_BUS_ENABLE:
    ldr r0, =0xD0000024
    ldr r1, =0x000FF000
    str r1, [r0]
bx lr

.thumb_func
OUTPUT_BUS_DISABLE:
    ldr r0, =0xD0000028
    ldr r1, =0x000FF000
    str r1, [r0]
bx lr

.thumb_func
OUTPUT_BUS_WRITE:
    movs r2, #0xFF
    ands r0, r2
    lsls r0, r0, #12

    ldr r1, =0xD0000018     @ GPIO_OUT_CLR
    ldr r2, =0x000FF000
    str r2, [r1]

    ldr r1, =0xD0000014     @ GPIO_OUT_SET
    str r0, [r1]

bx lr

@ r4 is saved here because C needs 8 byte offset or it wont play nice
.thumb_func
HANDLE_PRELOAD:
push {r4,lr}
    bl CLEAR_STRING_BUFFER
    bl OUTPUT_BUS_DISABLE
    bl run_preload_cli
pop {r4 ,pc}

@ Halt means next and back pins are high 
@its possible this method can report to OLED "HALT DETECTED" kinda deal
.thumb_func
HANDLE_HALT:
push {r4, lr}
    bl OUTPUT_BUS_DISABLE
    bl MEMORY_CLEAR_ALL @this clears the memory & resets the pointer 
    bl CLEAR_STRING_BUFFER

pop {r4, pc}


.thumb_func
HANDLE_IN:
push {r4, lr}
    bl MEMORY_READ
    bl SB_DETERMINE_ASCII
    bl SB_APPEND_CHAR
pop {r4, pc}

@ this will take the current string buffer and print it to the oled  and out pins
.thumb_func
HANDLE_OUT:
push {r4, lr}

    bl MEMORY_READ
    mov r4, r0

    bl OUTPUT_BUS_ENABLE

    mov r0, r4
    bl OUTPUT_BUS_WRITE

pop {r4, pc}

@ ------------------------------------------------------------------------ so i can find the top of the most important func easily
@ agony!!!
.thumb_func
RUNTIME_STEP:
push {r4, lr}

    @ if we dont save the previous pins the entire runtime hangs
    ldr r0, =CONTROL_PINS_STABLE
    ldrb r1, [r0]
    ldr r2, =PREV_CONTROL_PINS_STABLE
    strb r1, [r2]

    
    ldr r0, =PIO_PINS_STABLE
    ldrb r1, [r0]
    ldr r2, =PREV_PIO_PINS_STABLE
    strb r1, [r2]

    
    @ commented out sometimes for testing purposes 
    @bl VERIFY_CONTROL_PINS_STABLE
    @bl VERIFY_PIO_PINS_STABLE
    @bl READ_IN_BUS_VALUE

    CHECK_PRELOAD:
        ldr r0, =PREV_PIO_PINS_STABLE
        ldrb r1, [r0]

        ldr r0, =PIO_PINS_STABLE
        ldrb r3, [r0]

        movs r2, #0x1

        ands r1, r2
        ands r3, r2

        cmp r1, #0
    bne CHECK_HANDLE_HALT

        cmp r3, #0
    bne DO_HANDLE_PRELOAD

    @ handle_in case logic
    CHECK_HANDLE_IN:
        ldr r0, =PREV_PIO_PINS_STABLE
        ldrb r1, [r0]

        ldr r0, =PIO_PINS_STABLE
        ldrb r3, [r0]

        movs r2, #0x2

        ands r1, r2            @ old IN bit
        ands r3, r2            @ new IN bit

        cmp r1, #0
    bne CHECK_HANDLE_OUT

        cmp r3, #0
    bne DO_HANDLE_IN

    @ handle_out case logic 
    CHECK_HANDLE_OUT:
        ldr r0, =PREV_PIO_PINS_STABLE
        ldrb r1, [r0]

        ldr r0, =PIO_PINS_STABLE
        ldrb r3, [r0]

        movs r2, #0x4

        ands r1, r2            @ old OUT bit
        ands r3, r2            @ new OUT bit

        cmp r1, #0
    bne CHECK_HANDLE_HALT

        cmp r3, #0
    bne DO_HANDLE_OUT

@ halt case logic
CHECK_HANDLE_HALT:
    ldr r0, =CONTROL_PINS_STABLE
    ldrb r3, [r0]
    movs r1, r3
    movs r2, #0xC
    ands r1, r2
    cmp r1, r2
    beq DO_HANDLE_HALT

@ do next page logic
    ldr r0, =CONTROL_PINS_STABLE
    ldrb r3, [r0]
    movs r1, r3
    movs r2, #0xC
    ands r1, r2
    cmp r1, #0x4
    beq DO_NEXT_PAGE

@ do back page logic
    ldr r0, =CONTROL_PINS_STABLE
    ldrb r3, [r0]
    movs r1, r3
    movs r2, #0xC
    ands r1, r2
    cmp r1, #0x8
    beq DO_BACK_PAGE

    @ mem set location logic 
    ldr r0, =CONTROL_PINS_STABLE
    ldrb r3, [r0]

    movs r1, r3
    movs r2, #0x3      @ READ + WRITE
    ands r1, r2
    cmp r1, r2
beq DO_MEMORY_SET_LOCATION

    @logic for mem write 
    ldr r0, =CONTROL_PINS_STABLE
    ldrb r3, [r0]

    movs r1, r3
    movs r2, #0x3
    ands r1, r2
    cmp r1, #0x2
beq DO_MEMORY_WRITE

    @ output_bus_write logic
    @ this method takes the value at whereever mem pointer is and gives it to the output bus
    ldr r0, =CONTROL_PINS_STABLE
    ldrb r3, [r0]

    movs r1, r3
    movs r2, #0x3
    ands r1, r2
    cmp r1, #0x1
beq DO_OUTPUT_BUS_ENABLE

    @ "default case" do nothing mayb make this report somewhere if happens
    b END_RUNTIME_STEP


    DO_HANDLE_PRELOAD:
    bl HANDLE_PRELOAD
    b END_RUNTIME_STEP

    DO_OUTPUT_BUS_ENABLE:
    bl OUTPUT_BUS_ENABLE
    bl MEMORY_READ
    bl OUTPUT_BUS_WRITE
    @ disabling the bus right after doesnt keep the leds on 
    @bl OUTPUT_BUS_DISABLE
b END_RUNTIME_STEP

    DO_MEMORY_WRITE:
    bl MEMORY_WRITE
b END_RUNTIME_STEP

    @r0 must have bus value for memset location
    DO_MEMORY_SET_LOCATION:
    bl READ_IN_BUS_VALUE
    bl MEMORY_SET_LOCATION
b END_RUNTIME_STEP

    DO_BACK_PAGE:
    bl MEMORY_BACK_PAGE
b END_RUNTIME_STEP

    DO_NEXT_PAGE:
    bl MEMORY_NEXT_PAGE
b END_RUNTIME_STEP

    DO_RESET_PTR:
    bl MEMORY_RESET_POINTER
b END_RUNTIME_STEP

    DO_HANDLE_OUT:
    bl HANDLE_OUT
b END_RUNTIME_STEP

    DO_HANDLE_IN:
    bl HANDLE_IN
b END_RUNTIME_STEP

    DO_HANDLE_HALT:
    bl HANDLE_HALT
b END_RUNTIME_STEP

    END_RUNTIME_STEP:
pop {r4, pc}