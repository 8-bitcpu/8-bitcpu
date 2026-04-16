#ifndef APP_H
#define APP_H

#include "control.h"
#include "pico/time.h"
#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    control_state_t last_control;
    uint8_t last_input;

    uint8_t input;
    uint8_t output;
    bool output_valid;

    char sb[18];
    int sb_len;

    bool showing_output_screen;
    absolute_time_t output_screen_until;

    bool showing_halt_screen;
    absolute_time_t halt_screen_until;
} app_t;

void app_init(app_t *a);
void app_handle_cycle(app_t *a, control_state_t ctrl);

#endif