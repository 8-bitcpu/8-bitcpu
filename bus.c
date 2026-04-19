#include "bus.h"
#include "pico/stdlib.h"

#define BUS_IN_START 0
#define BUS_OUT_START 12

void bus_init_input(void)
{
    for (int i = 0; i < 8; i++)
    {
        gpio_init(BUS_IN_START + i);
        gpio_set_dir(BUS_IN_START + i, GPIO_IN);
        gpio_pull_down(BUS_IN_START + i);
    }
}

void bus_init_output(void)
{
    for (int i = 0; i < 8; i++)
    {
        gpio_init(BUS_OUT_START + i);
        gpio_set_dir(BUS_OUT_START + i, GPIO_IN);
        gpio_disable_pulls(BUS_OUT_START + i);
    }
}

uint8_t bus_read_input(void)
{
    uint8_t value = 0;

    for (int i = 0; i < 8; i++)
    {
        value |= (gpio_get(BUS_IN_START + i) ? 1 : 0) << i;
    }

    return value;
}

void bus_write_output(uint8_t value)
{
    for (int i = 0; i < 8; i++)
    {
        gpio_put(BUS_OUT_START + i, (value >> i) & 1);
    }
}

void bus_enable_output(void)
{
    for (int i = 0; i < 8; i++)
        gpio_set_dir(BUS_OUT_START + i, GPIO_OUT);
}

void bus_disable_output(void)
{
    for (int i = 0; i < 8; i++)
        gpio_set_dir(BUS_OUT_START + i, GPIO_IN);
}