#include "app.h"
#include "memory.h"
#include "bus.h"
#include "string_builder.h"
#include "preload.h"
#include <stdio.h>

static bool is_halt(uint8_t in)
{
    return ((in & 0xF0) == 0xF0);
}

void app_init(app_t *a)
{
    a->last_input = 0;
    a->output = 0;
    a->output_valid = false;
    sb_clear(a->sb, &a->sb_len);
}

void app_handle_cycle(app_t *a, control_state_t ctrl)
{
    a->input = bus_read_input();

    bool preload_edge = (!a->last_control.preload_pin && ctrl.preload_pin);
    bool halt_edge = (!is_halt(a->last_input) && is_halt(a->input));
    bool in_edge = (!a->last_control.in_pin && ctrl.in_pin);
    bool out_edge = (!a->last_control.out_pin && ctrl.out_pin);

    if (preload_edge)
    {
        preload_memory_image();
        bus_disable_output();
        a->output_valid = false;
        printf("PRELOAD\n");
    }
    else if (halt_edge)
    {
        memory_clear_all();
        sb_clear(a->sb, &a->sb_len);
        bus_disable_output();
        a->output_valid = false;
        printf("HALT\n");
    }
    else
    {
        uint8_t step = memory_step(
            ctrl.write_pin,
            ctrl.read_pin,
            ctrl.next_pin,
            ctrl.back_pin,
            a->input);

        if (!ctrl.write_pin && ctrl.read_pin)
        {
            a->output = step;
            a->output_valid = true;
            bus_enable_output();
            bus_write_output(a->output);
        }
        else
        {
            bus_disable_output();
            a->output_valid = false;
        }

        if (in_edge)
        {
            sb_append(a->sb, &a->sb_len, memory_read());
        }

        if (out_edge)
        {
            printf("OUT: %s\n", a->sb);
        }
    }

    a->last_control = ctrl;
    a->last_input = a->input;
}