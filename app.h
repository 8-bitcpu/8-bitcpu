#ifndef APP_H
#define APP_H

#include "control.h"
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
} app_t;

void app_init(app_t *a);

void app_handle_cycle(app_t *a, control_state_t ctrl);

#endif