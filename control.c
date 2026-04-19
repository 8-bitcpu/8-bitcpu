#include "control.h"
#include "pico/stdlib.h"

#define READ_PIN 8
#define WRITE_PIN 9
#define NEXT_PIN 10
#define BACK_PIN 11
#define IN_PIN 20
#define OUT_PIN 21
#define PRELOAD_PIN 22

#define CONTROL_SETTLE_US 1000

void control_init(void)
{
    int pins[] = {READ_PIN, WRITE_PIN, NEXT_PIN, BACK_PIN, IN_PIN, OUT_PIN, PRELOAD_PIN};

    for (int i = 0; i < 7; i++)
    {
        gpio_init(pins[i]);
        gpio_set_dir(pins[i], GPIO_IN);
        gpio_pull_down(pins[i]);
    }
}

control_state_t control_read_raw(void)
{
    control_state_t s;

    s.read_pin = gpio_get(READ_PIN);
    s.write_pin = gpio_get(WRITE_PIN);
    s.next_pin = gpio_get(NEXT_PIN);
    s.back_pin = gpio_get(BACK_PIN);
    s.in_pin = gpio_get(IN_PIN);
    s.out_pin = gpio_get(OUT_PIN);
    s.preload_pin = gpio_get(PRELOAD_PIN);

    return s;
}

bool control_same(control_state_t a, control_state_t b)
{
    return (a.read_pin == b.read_pin) &&
           (a.write_pin == b.write_pin) &&
           (a.next_pin == b.next_pin) &&
           (a.back_pin == b.back_pin) &&
           (a.in_pin == b.in_pin) &&
           (a.out_pin == b.out_pin) &&
           (a.preload_pin == b.preload_pin);
}

control_state_t control_read_stable(void)
{
    control_state_t first = control_read_raw();
    sleep_us(CONTROL_SETTLE_US);
    control_state_t second = control_read_raw();

    while (!control_same(first, second))
    {
        first = second;
        sleep_us(CONTROL_SETTLE_US);
        second = control_read_raw();
    }

    return second;
}

bool control_is_idle(control_state_t s)
{
    return !s.read_pin &&
           !s.write_pin &&
           !s.next_pin &&
           !s.back_pin &&
           !s.in_pin &&
           !s.out_pin &&
           !s.preload_pin;
}