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


    for(int i=0; i < 10; i++)
    {
        set_bus(i);
        set_ptr();
        set_bus(i);
        write_value();
    }


    printf("PTR=%u\n", PTR);

    for (int i = 0; i <= 256; i++) 
    {
        printf("MEM_BUFFER[%d] = %u\n", i, MEM_BUFFER[i]);
    }

    while (1) 
    {
        tight_loop_contents();
    }
}