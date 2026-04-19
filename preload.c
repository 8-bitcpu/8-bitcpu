#include "preload.h"
#include "memory.h"
#include <stdint.h>

static const uint8_t preload_image[MEMORY_SIZE] = {
    [0] = 216,
    [1] = 209,
    [2] = 96,
    [3] = 96,
    [4] = 237,
    [5] = 96,
    [6] = 96,
    [7] = 112,
    [8] = 113};

void preload_memory_image(void)
{
    memory_load_image(preload_image, MEMORY_SIZE);
}