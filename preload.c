#include "preload.h"
#include "memory.h"
#include <stdint.h>

static const uint8_t preload_image[MEMORY_SIZE] = {
    [0] = 10,
    [1] = 27,
    [2] = 240,
    [10] = 42,
    [11] = 0};

void preload_memory_image(void)
{
    memory_load_image(preload_image, MEMORY_SIZE);
}