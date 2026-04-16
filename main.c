#include <stdio.h>
#include <stdint.h>
#include "pico/stdlib.h"


uint8_t MEM_BUFFER[256];
uint8_t PTR;
uint8_t PAGE_SIZE = 16;

extern void INITIALIZE_PINS_BASIC(void);
extern void MEMORY_CLEAR_ALL(void);
extern void MEMORY_SET_LOCATION(void);
extern void MEMORY_READ(void);
extern void MEMORY_WRITE(void);
extern void MEMORY_RESET_POINTER(void);
extern void MEMORY_NEXT_PAGE(void);
extern void MEMORY_BACK_PAGE(void);
extern void READ_PINS_ASSIGN_VALUES(void);
extern void MEMORY_STEP(void);

void print_state() 
{
    printf("PTR: %u\n", PTR);

    for (int i = 0; i < 16; i++) {
        if (i == PTR)
            printf("[%02X] ", MEM_BUFFER[i]);  // highlight current location
        else
            printf(" %02X  ", MEM_BUFFER[i]);
    }
    printf("\n");
}

int main(void) 
{
    INITIALIZE_PINS_BASIC();
    stdio_usb_init();

    sleep_ms(8000);
    while(1)
    {
        printf("testing");
        sleep_ms(2000);
        print_state();
        MEMORY_STEP();
    }
}