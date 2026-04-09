#include "memory.h"

static uint8_t memory[MEMORY_SIZE];
static uint16_t pointer = 0;

void memory_clear_all(void)
{
    for (int i = 0; i < MEMORY_SIZE; i++)
    {
        memory[i] = 0;
    }

    pointer = 0;
}

void memory_init(void)
{
    memory_clear_all();
}

void memory_set_location(uint8_t value)
{
    pointer = value % MEMORY_SIZE;
}

uint8_t memory_read(void)
{
    return memory[pointer];
}

void memory_write(uint8_t value)
{
    memory[pointer] = value;
}

void memory_next_page(void)
{
    pointer = (pointer + PAGE_SIZE) % MEMORY_SIZE;
}

void memory_back_page(void)
{
    pointer = (pointer + MEMORY_SIZE - PAGE_SIZE) % MEMORY_SIZE;
}

void memory_reset_pointer(void)
{
    pointer = 0;
}

uint16_t memory_get_pointer(void)
{
    return pointer;
}

uint8_t memory_peek(uint8_t address)
{
    return memory[address % MEMORY_SIZE];
}

void memory_load_image(const uint8_t *image, uint16_t count)
{
    memory_clear_all();

    if (image == 0)
    {
        return;
    }

    if (count > MEMORY_SIZE)
    {
        count = MEMORY_SIZE;
    }

    for (uint16_t i = 0; i < count; i++)
    {
        memory[i] = image[i];
    }

    pointer = 0;
}

uint8_t memory_step(int write_pin, int read_pin, int next_pin, int back_pin, uint8_t input_value)
{
    if (next_pin && back_pin)
    {
        memory_reset_pointer();
    }
    else if (next_pin)
    {
        memory_next_page();
    }
    else if (back_pin)
    {
        memory_back_page();
    }
    else if (write_pin && read_pin)
    {
        memory_set_location(input_value);
    }
    else if (write_pin && !read_pin)
    {
        memory_write(input_value);
    }
    else if (!write_pin && read_pin)
    {
        return memory_read();
    }

    return 0;
}