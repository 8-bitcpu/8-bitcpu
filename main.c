#include <stdio.h>
#include <stdint.h>
#include "pico/stdlib.h"
#include "oled.h"

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

    for (int i = 0; i < 16; i++) {
        printf("%3d: %3u\n", i, MEM_BUFFER[i]);
    }
}

int main() {
    stdio_init_all();
    sleep_ms(6000);

    oled_init();

    my_bus_init_output();
    my_bus_init_input();
    my_control_init();

    printf("PICO IS LIVE\n");

    int loop_counter = 0; 


    while (1) 
    {

        //oled display logic
        bool read_pin  = (CONTROL_PINS_STABLE & 0x1) != 0;
        bool write_pin = (CONTROL_PINS_STABLE & 0x2) != 0;
        bool next_pin  = (CONTROL_PINS_STABLE & 0x4) != 0;
        bool back_pin  = (CONTROL_PINS_STABLE & 0x8) != 0;

        bool in_pin    = (PIO_PINS_STABLE & 0x1) != 0;
        bool out_pin   = (PIO_PINS_STABLE & 0x2) != 0;
        bool preload   = (PIO_PINS_STABLE & 0x4) != 0;

        uint8_t current_value = MEM_BUFFER[PTR];

        if (preload) {
            oled_clear();
            oled_draw_string(0, 3, "PRELOAD");
            oled_update();
        }
        else if (next_pin && back_pin) {
            oled_show_halt();
        }
        else if (in_pin && STRING_LEN > 0) {
            oled_show_output_string((const char *)STRING_BUFFER);
        }
        else {
            oled_show_memory_status(
                PTR,
                current_value,
                IN_BUS_VALUE,
                current_value,
                out_pin || (read_pin && !write_pin),
                read_pin,
                write_pin,
                next_pin,
                back_pin,
                (const char *)STRING_BUFFER
            );
        }
        //end of oled display logic 

        printf("CTRL=0x%X PIO=0x%X\n", CONTROL_PINS_STABLE, PIO_PINS_STABLE);
        
        printf("\npre ref\n");
        sleep_ms(6000);
        RUNTIME_STEP();
        print_mem_buffer();
        printf("post ref loop: %d", loop_counter);
        loop_counter++;
    }
}