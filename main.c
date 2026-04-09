#include "pico/stdlib.h"
#include "bus.h"
#include "control.h"
#include "app.h"
#include "memory.h"
#include "oled.h"

int main(void)
{
    stdio_init_all();
    sleep_ms(2000);

    memory_init();
    bus_init_input();
    bus_init_output();
    control_init();
    oled_init();

    app_t app;
    app_init(&app);

    while (true)
    {
        control_state_t raw = control_read_raw();
        control_state_t stable = raw;

        if (!control_same(raw, app.last_control))
            stable = control_read_stable();

        app_handle_cycle(&app, stable);

        sleep_ms(1);
    }
}