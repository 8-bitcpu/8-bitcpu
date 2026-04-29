#ifndef OLED_H
#define OLED_H

void oled_init(void);
void oled_print(void);
void oled_print_string_buffer(void);
void oled_edge_flash_animation(void);
void oled_draw_string_big(int x, int page, const char *text);
void oled_draw_char_big(int x, int page, char c);

#endif