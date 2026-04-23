#ifndef OLED_H
#define OLED_H

#include <stdbool.h>
#include <stdint.h>

void oled_init(void);
void oled_clear(void);
void oled_update(void);

void oled_draw_string(int x, int page, const char *text);

void oled_show_output_string(const char *text);
void oled_show_halt(void);

void oled_show_memory_status(uint16_t pointer,
                             uint8_t current_value,
                             uint8_t input_value,
                             uint8_t output_value,
                             bool output_valid,
                             bool read_pin,
                             bool write_pin,
                             bool next_pin,
                             bool back_pin,
                             const char *sb_text);

#endif