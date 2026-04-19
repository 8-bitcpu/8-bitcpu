#include "app.h"
#include "memory.h"
#include "bus.h"
#include "string_builder.h"
#include "preload.h"
#include "oled.h"
#include "pico/stdlib.h"
#include <stdio.h>

#define OUTPUT_SCREEN_TIME_MS 5000
#define HALT_SCREEN_TIME_MS 5000

void app_init(app_t *a)
{
    a->last_control.read_pin = false;
    a->last_control.write_pin = false;
    a->last_control.next_pin = false;
    a->last_control.back_pin = false;
    a->last_control.in_pin = false;
    a->last_control.out_pin = false;
    a->last_control.preload_pin = false;

    a->last_input = 0;
    a->input = 0;
    a->output = 0;
    a->output_valid = false;

    sb_clear(a->sb, &a->sb_len);

    a->showing_output_screen = false;
    a->output_screen_until = get_absolute_time();

    a->showing_halt_screen = false;
    a->halt_screen_until = get_absolute_time();

    /*
     * Preload immediately when the Pico/OLED starts up.
     */
    preload_memory_image();

    oled_show_memory_status(
        memory_get_pointer(),
        memory_read(),
        0,
        0,
        false,
        false,
        false,
        false,
        false,
        a->sb);
}

void app_handle_cycle(app_t *a, control_state_t ctrl)
{
    a->input = bus_read_input();

    bool preload_edge = (!a->last_control.preload_pin && ctrl.preload_pin);
    bool halt_condition = (ctrl.next_pin && ctrl.back_pin);
    bool in_edge = (!a->last_control.in_pin && ctrl.in_pin);
    bool out_edge = (!a->last_control.out_pin && ctrl.out_pin);

    if (preload_edge)
    {
        preload_memory_image();
        bus_disable_output();
        a->output_valid = false;
        a->showing_output_screen = false;
        a->showing_halt_screen = false;
        // printf("PRELOAD\n");
    }
    else if (halt_condition)
    {
        /*
         * HALT now means:
         * - clear all memory
         * - reset pointer
         * - clear string builder
         * - show HALT screen for 5 seconds
         *
         * memory_clear_all() already clears memory and resets pointer to 0.
         */
        memory_clear_all();
        sb_clear(a->sb, &a->sb_len);
        bus_disable_output();
        a->output_valid = false;

        a->showing_output_screen = false;
        a->showing_halt_screen = true;
        a->halt_screen_until = make_timeout_time_ms(HALT_SCREEN_TIME_MS);

        oled_show_halt();
        // printf("HALT\n");
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
            a->showing_halt_screen = false;
            a->showing_output_screen = true;
            a->output_screen_until = make_timeout_time_ms(OUTPUT_SCREEN_TIME_MS);
            oled_show_output_string(a->sb);
            // printf("OUT: %s\n", a->sb);
        }
    }

    if (a->showing_halt_screen)
    {
        if (absolute_time_diff_us(get_absolute_time(), a->halt_screen_until) <= 0)
        {
            a->showing_halt_screen = false;
        }
        else
        {
            oled_show_halt();
        }
    }
    else if (a->showing_output_screen)
    {
        if (absolute_time_diff_us(get_absolute_time(), a->output_screen_until) <= 0)
        {
            a->showing_output_screen = false;
        }
        else
        {
            oled_show_output_string(a->sb);
        }
    }

    if (!a->showing_halt_screen && !a->showing_output_screen)
    {
        oled_show_memory_status(
            memory_get_pointer(),
            memory_read(),
            a->input,
            a->output,
            a->output_valid,
            ctrl.read_pin,
            ctrl.write_pin,
            ctrl.next_pin,
            ctrl.back_pin,
            a->sb);
    }

    a->last_control = ctrl;
    a->last_input = a->input;
}