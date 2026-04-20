#include <stdio.h>
#include <stdint.h>
#include "pico/stdlib.h"


uint8_t MEM_BUFFER[256];
uint8_t PTR;
uint8_t PAGE_SIZE = 16;

uint8_t STRING_BUFFER[256];
uint8_t STRING_LEN; 
uint8_t STING_MAX_SIZE = 255;

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
    stdio_init_all();
    
    int test_var = 1; 
    sleep_ms(8000);
    while(1)
    {
        if(test_var == 4)
        {
            MEMORY_CLEAR_ALL();
        }

        test_var++;
        PTR++;

        printf("testing");
        sleep_ms(2000);
        print_state();
        RUNTIME_STEP();
    }
}