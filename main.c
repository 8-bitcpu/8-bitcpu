#include <stdio.h>
#include "pico/stdlib.h"

// ASM functions
extern void MEMORY_CLEAR_ALL(void);
extern void MEMORY_RESET_POINTER(void);
extern void MEMORY_STEP(void);

// Globals from ASM
uint8_t MEM_BUFFER[256];
uint8_t PTR;

void print_state() {
    printf("PTR: %u\n", PTR);

    for (int i = 0; i < 16; i++) {
        if (i == PTR)
            printf("[%02X] ", MEM_BUFFER[i]);  // highlight current location
        else
            printf(" %02X  ", MEM_BUFFER[i]);
    }
    printf("\n");
}

int main() {
    stdio_init_all();  // needed for printf over USB

    // small delay so USB connects (otherwise printf disappears)
    sleep_ms(8000);

    printf("Starting memory test..\n");

    MEMORY_CLEAR_ALL();
    MEMORY_RESET_POINTER();

    while (1) {
        PTR++;
        sleep_ms(2000);
        MEMORY_STEP();     // this should read pins + act

        print_state();     // optional, remove if too spammy
        sleep_ms(200);     // slow it down so you can see behavior
    }

    return 0;
}