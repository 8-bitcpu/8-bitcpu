#include <stdio.h>
#include <stdint.h>
#include "pico/stdlib.h"
#include "oled.h"
#include "tests.h"

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
extern uint8_t OUT_BUS_VALUE;

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
 for (int i = 12; i <= 19; i++) {
    gpio_init(i);
    gpio_set_dir(i, GPIO_OUT);
    gpio_put(i, 0);
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

    for (int i = 0; i < 125; i++) {
        printf("%3d: %3u\n", i, MEM_BUFFER[i]);
    }
}


int main() {
    stdio_init_all();
    sleep_ms(1000);

    oled_init();

    my_bus_init_output();
    my_bus_init_input();
    my_control_init();



    printf("PICO IS LIVE\n");

    //stress_test_runtime();

        MEM_BUFFER[0] = 215; 
        MEM_BUFFER[1] = 209;
        MEM_BUFFER[2] = 96;
        MEM_BUFFER[3] = 96;
        MEM_BUFFER[4] = 236;
        MEM_BUFFER[5] = 96;
        MEM_BUFFER[6] = 96;
        MEM_BUFFER[7] = 112;
        MEM_BUFFER[8] = 113;

    while (1) 
    {


        

        RUNTIME_STEP();
        //super_debug_report();
        oled_print();
        //make the oled print in these methods for bebugging
        //stress_test_runtime();
        //test_runtime_handle_in();
        //test_runtime_output_count();
    }
}


/*
INPUT BUS (8-bit value read together)
GP0  -> bit 0 (LSB)
GP1  -> bit 1
GP2  -> bit 2
GP3  -> bit 3
GP4  -> bit 4
GP5  -> bit 5
GP6  -> bit 6
GP7  -> bit 7 (MSB)

CONTROL PINS (primary logic triggers)
GP8  -> READ
GP9  -> WRITE
GP10 -> NEXT
GP11 -> BACK

OUTPUT BUS (8-bit driven output)
GP12 -> bit 0 (LSB)
GP13 -> bit 1
GP14 -> bit 2
GP15 -> bit 3
GP16 -> bit 4
GP17 -> bit 5
GP18 -> bit 6
GP19 -> bit 7 (MSB)

PIO / MODE PINS (higher-level behavior)
GP20 -> PRELOAD
GP21 -> IN
GP22 -> OUT
*/