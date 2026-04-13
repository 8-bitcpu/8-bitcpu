#include "pico/stdlib.h"
#include "bus.h"
#include "control.h"
#include "app.h"
#include "memory.h"

uint8_t MEM_BUFFER[256];
uint8_t PTR;
uint8_t PAGE_SIZE = 16;//perhaps not needed
volatile uint32_t IN_PINS_STATE;


int main(void)
{
    stdio_init_all();
    sleep_ms(2000);

    memory_clear_all();
    bus_init_input();
    bus_init_output();
    control_init();

    app_t app;
    app_init(&app);

    while (1)
    {
        control_state_t raw = control_read_raw();
        control_state_t stable = raw;

        printf("alive\n");

        if (!control_same(raw, app.last_control))
            stable = control_read_stable();

        app_handle_cycle(&app, stable);

        sleep_ms(1);
    }
}