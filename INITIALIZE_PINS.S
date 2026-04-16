@ IO BANK BASE = 0x40014000

.syntax unified 
.thumb

.global INITIALIZE_PINS_BASIC

.text 

.thumb_func 
INITIALIZE_PINS_BASIC:
@ give gpio to SIO 
    ldr r0, =0x40014004
    movs r1, #5 
    str r1, [r0]

@ gpio pad register 
    ldr r0, =0x4001C000
    movs r1, #64
    str r1, [r0]
bx lr
