#ifndef CONTROL_H
#define CONTROL_H

#include <stdbool.h>

typedef struct
{
    bool read_pin;
    bool write_pin;
    bool next_pin;
    bool back_pin;
    bool in_pin;
    bool out_pin;
    bool preload_pin;
} control_state_t;

void control_init(void);
control_state_t control_read_raw(void);
control_state_t control_read_stable(void);

bool control_same(control_state_t a, control_state_t b);
bool control_is_idle(control_state_t s);

#endif