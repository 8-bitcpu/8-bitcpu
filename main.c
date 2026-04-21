#include <stdio.h>
#include <stdint.h>
#include "pico/stdlib.h"


uint8_t MEM_BUFFER[256];
uint8_t PTR;
uint8_t PAGE_SIZE = 16;

uint8_t STRING_BUFFER[256];
uint8_t STRING_LEN; 
uint8_t STING_MAX_SIZE = 255;

extern uint8_t CONTROL_PINS_STABLE;
extern uint8_t PIO_PINS_STABLE;
extern uint32_t GPIO_STATE;
extern uint8_t IN_BUS_VALUE;
extern uint8_t STRING_LEN;

extern void INITIALIZE_PINS_BASIC(void);
extern void MEMORY_CLEAR_ALL(void);
extern void MEMORY_SET_LOCATION(void);
extern void MEMORY_READ(void);
extern void MEMORY_WRITE(void);
extern void MEMORY_RESET_POINTER(void);
extern void MEMORY_NEXT_PAGE(void);
extern void MEMORY_BACK_PAGE(void);
extern void READ_PINS_ASSIGN_VALUES(void);
extern void RUNTIME_STEP(void);
extern void HANDLE_IN(void);

static void set_bus(uint8_t value) 
{
    GPIO_STATE = (GPIO_STATE & 0xFFFFFF00u) | value;
}

static void set_ptr() 
{
    CONTROL_PINS_STABLE = 0x3;   // READ + WRITE
    PIO_PINS_STABLE = 0x0;
    RUNTIME_STEP();
}

static void write_value() 
{
    CONTROL_PINS_STABLE = 0x2;   // WRITE only
    PIO_PINS_STABLE = 0x0;
    RUNTIME_STEP();
}

int main() 
{
    stdio_init_all();
    sleep_ms(8000);


for (int i = 0; i < 256; i++)
    {
        MEM_BUFFER[i] = 0;
        STRING_BUFFER[i] = 0;
    }

    PTR = 0;
    STRING_LEN = 0;
    CONTROL_PINS_STABLE = 0;
    PIO_PINS_STABLE = 0;
    GPIO_STATE = 0;
    IN_BUS_VALUE = 0;

    //HELLO KYRAN 
    //CONTROL PINS = GPIO 8-11
    //PIO PINS = 20-22
    // the programs masks for numbers it needs
    // example hex 0x3 = 0011 binary
    // CONTROL_PINS = 0011 = read high & write high 

    GPIO_STATE = (GPIO_STATE & 0xFFFFFF00u) | 0;
    CONTROL_PINS_STABLE = 0x3;
    PIO_PINS_STABLE = 0x0;
    RUNTIME_STEP();

    GPIO_STATE = (GPIO_STATE & 0xFFFFFF00u) | 'h';
    CONTROL_PINS_STABLE = 0x2;
    PIO_PINS_STABLE = 0x0;
    RUNTIME_STEP();

    GPIO_STATE = (GPIO_STATE & 0xFFFFFF00u) | 1;
    CONTROL_PINS_STABLE = 0x3;
    PIO_PINS_STABLE = 0x0;
    RUNTIME_STEP();

    GPIO_STATE = (GPIO_STATE & 0xFFFFFF00u) | 'e';
    CONTROL_PINS_STABLE = 0x2;
    PIO_PINS_STABLE = 0x0;
    RUNTIME_STEP();

    GPIO_STATE = (GPIO_STATE & 0xFFFFFF00u) | 2;
    CONTROL_PINS_STABLE = 0x3;
    PIO_PINS_STABLE = 0x0;
    RUNTIME_STEP();

    GPIO_STATE = (GPIO_STATE & 0xFFFFFF00u) | 'l';
    CONTROL_PINS_STABLE = 0x2;
    PIO_PINS_STABLE = 0x0;
    RUNTIME_STEP();

    GPIO_STATE = (GPIO_STATE & 0xFFFFFF00u) | 3;
    CONTROL_PINS_STABLE = 0x3;
    PIO_PINS_STABLE = 0x0;
    RUNTIME_STEP();

    GPIO_STATE = (GPIO_STATE & 0xFFFFFF00u) | 'l';
    CONTROL_PINS_STABLE = 0x2;
    PIO_PINS_STABLE = 0x0;
    RUNTIME_STEP();

    GPIO_STATE = (GPIO_STATE & 0xFFFFFF00u) | 4;
    CONTROL_PINS_STABLE = 0x3;
    PIO_PINS_STABLE = 0x0;
    RUNTIME_STEP();

    GPIO_STATE = (GPIO_STATE & 0xFFFFFF00u) | 'o';
    CONTROL_PINS_STABLE = 0x2;
    PIO_PINS_STABLE = 0x0;
    RUNTIME_STEP();

    GPIO_STATE = (GPIO_STATE & 0xFFFFFF00u) | 0;
    CONTROL_PINS_STABLE = 0x3;
    PIO_PINS_STABLE = 0x0;
    RUNTIME_STEP();
    CONTROL_PINS_STABLE = 0x0;
    PIO_PINS_STABLE = 0x2;
    RUNTIME_STEP();

    GPIO_STATE = (GPIO_STATE & 0xFFFFFF00u) | 1;
    CONTROL_PINS_STABLE = 0x3;
    PIO_PINS_STABLE = 0x0;
    RUNTIME_STEP();
    CONTROL_PINS_STABLE = 0x0;
    PIO_PINS_STABLE = 0x2;
    RUNTIME_STEP();

    GPIO_STATE = (GPIO_STATE & 0xFFFFFF00u) | 2;
    CONTROL_PINS_STABLE = 0x3;
    PIO_PINS_STABLE = 0x0;
    RUNTIME_STEP();
    CONTROL_PINS_STABLE = 0x0;
    PIO_PINS_STABLE = 0x2;
    RUNTIME_STEP();

    GPIO_STATE = (GPIO_STATE & 0xFFFFFF00u) | 3;
    CONTROL_PINS_STABLE = 0x3;
    PIO_PINS_STABLE = 0x0;
    RUNTIME_STEP();
    CONTROL_PINS_STABLE = 0x0;
    PIO_PINS_STABLE = 0x2;
    RUNTIME_STEP();

    GPIO_STATE = (GPIO_STATE & 0xFFFFFF00u) | 4;
    CONTROL_PINS_STABLE = 0x3;
    PIO_PINS_STABLE = 0x0;
    RUNTIME_STEP();
    CONTROL_PINS_STABLE = 0x0;
    PIO_PINS_STABLE = 0x2;
    RUNTIME_STEP();



    printf("STRING_LEN = %u\n", STRING_LEN);
    printf("STRING_BUFFER = %s\n", STRING_BUFFER);

    for (int i = 0; i < STRING_LEN; i++) {
        printf("STRING_BUFFER[%d] = %u\n", i, STRING_BUFFER[i]);
    }

    printf("PTR=%u\n", PTR);

    for (int i = 0; i <= 16; i++) 
    {
        printf("MEM_BUFFER[%d] = %u\n", i, MEM_BUFFER[i]);
    }

    while (1) 
    {
        tight_loop_contents();
    }
}