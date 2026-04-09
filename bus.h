#ifndef BUS_H
#define BUS_H

#include <stdint.h>

void bus_init_input(void);
void bus_init_output(void);

uint8_t bus_read_input(void);

void bus_write_output(uint8_t value);
void bus_enable_output(void);
void bus_disable_output(void);

#endif