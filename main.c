#include "pico/stdlib.h"
#include "app.h"
#include "bus.h"
#include "control.h"
#include "oled.h"

int main(void)
{
    // stdio_init_all();   // TinyUSB debug disabled for now

    bus_init_input();
    bus_init_output();
    control_init();
    oled_init();

    app_t app;
    app_init(&app);

    while (1)
    {
        control_state_t ctrl = control_read_stable();
        app_handle_cycle(&app, ctrl);
        sleep_ms(10);
    }

    return 0;
}