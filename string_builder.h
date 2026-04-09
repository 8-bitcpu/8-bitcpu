#ifndef STRING_BUILDER_H
#define STRING_BUILDER_H

#include <stdint.h>

#define STRING_BUILDER_MAX 18

void sb_clear(char *buf, int *len);
void sb_append(char *buf, int *len, uint8_t value);

#endif