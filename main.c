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
extern uint8_t PREV_PIO_PINS_STABLE;

extern uint32_t GPIO_STATE;
extern uint8_t IN_BUS_VALUE;

extern void RUNTIME_STEP(void);
extern void run_preload_cli(void);
extern void VERIFY_CONTROL_PINS_STABLE(void);
extern void VERIFY_PIO_PINS_STABLE(void);

static void my_bus_init_input(void) {
    for (int i = 0; i < 8; i++) {
        gpio_init(i);
        gpio_set_dir(i, GPIO_IN);
        gpio_pull_down(i);
    }
}

static void my_bus_init_output(void) {
    for (int i = 0; i < 8; i++) {
        gpio_init(12 + i);
        gpio_set_dir(12 + i, GPIO_IN);
        gpio_disable_pulls(12 + i);
    }
}

static void my_control_init(void) {
    int pins[] = {8, 9, 10, 11, 20, 21, 22};
    for (int i = 0; i < 7; i++) {
        gpio_init(pins[i]);
        gpio_set_dir(pins[i], GPIO_IN);
        gpio_pull_down(pins[i]);
    }
}

void print_mem_buffer(void) {
    printf("MEM_BUFFER:\n");

    for (int i = 0; i < 16; i++) {
        printf("%3d: %3u\n", i, MEM_BUFFER[i]);
    }
}

int main() {
    stdio_init_all();
    sleep_ms(6000);


    
        // --- INPUT BUS GP0–7 ---
    for (int i = 0; i < 8; i++) {
        gpio_init(i);
        gpio_set_dir(i, GPIO_IN);
        gpio_pull_down(i);
    }

    // --- CONTROL PINS GP8–11 ---
    for (int i = 8; i <= 11; i++) {
        gpio_init(i);
        gpio_set_dir(i, GPIO_IN);
        gpio_pull_down(i);
    }

    // --- PIO PINS GP20–22 ---
    for (int i = 20; i <= 22; i++) {
        gpio_init(i);
        gpio_set_dir(i, GPIO_IN);
        gpio_pull_down(i);
    }
    

    printf("PICO IS LIVE\n");


    int loop_counter = 0; 
    while (1) 
    {


        printf("CTRL=0x%X PIO=0x%X\n", CONTROL_PINS_STABLE, PIO_PINS_STABLE);
        
        printf("\npre ref\n");
        sleep_ms(3000);
        RUNTIME_STEP();
        print_mem_buffer();
        printf("post ref loop: %d", loop_counter);
        loop_counter++;
    }
}