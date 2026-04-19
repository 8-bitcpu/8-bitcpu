#include "string_builder.h"

static char ascii_from_value(uint8_t v)
{
    if (v >= 32 && v <= 126)
        return (char)v;
    return '?';
}

void sb_clear(char *buf, int *len)
{
    *len = 0;
    buf[0] = '\0';
}

void sb_append(char *buf, int *len, uint8_t value)
{
    char c = ascii_from_value(value);

    if (*len < STRING_BUILDER_MAX - 1)
    {
        buf[*len] = c;
        (*len)++;
        buf[*len] = '\0';
    }
    else
    {
        for (int i = 1; i < STRING_BUILDER_MAX - 1; i++)
            buf[i - 1] = buf[i];

        buf[STRING_BUILDER_MAX - 2] = c;
        buf[STRING_BUILDER_MAX - 1] = '\0';
        *len = STRING_BUILDER_MAX - 1;
    }
}