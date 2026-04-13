#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

#define MEMORY_SIZE 256
#define PAGE_SIZE 16
void memory_init(void);
void memory_clear_all(void);
void memory_set_location(uint8_t value);
uint8_t memory_read(void);
void memory_write(uint8_t value);
void memory_next_page(void);
void memory_back_page(void);
void memory_reset_pointer(void);
uint16_t memory_get_pointer(void);
uint8_t memory_peek(uint8_t address);


#define PAGE_SIZE 16
#define MEMORY_SIZE 256


extern void MEMORY_INIT(void);
extern void MEMORY_SET_LOCATION(uint8_t value);
extern uint8_t MEMORY_READ(void);
extern void MEMORY_WRITE(uint8_t value);
extern void MEMORY_RESET_POINTER(void);

void memory_load_image(const uint8_t *image, uint16_t count);

uint8_t memory_step(int write_pin, int read_pin, int next_pin, int back_pin, uint8_t input_value);

#endif