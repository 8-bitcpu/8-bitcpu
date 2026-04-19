#include "memory.h"

uint8_t memory[MEMORY_SIZE];
uint8_t pointer = 0;
// need a pos int bias var

void memory_clear_all(void)
{
    for (int i = 0; i < MEMORY_SIZE; i++)
    {
        memory[i] = 0;
    }

    pointer = 0;
}

void memory_set_location(uint8_t value)
{
    pointer = value % MEMORY_SIZE; // change to pointer = value + (bias*16)
}

uint8_t memory_read(void)
{
    return memory[pointer];
}

void memory_write(uint8_t value)
{
    memory[pointer] = value;
}

void memory_next_page(void) // change to inc bias
{
    pointer = (pointer + PAGE_SIZE) % MEMORY_SIZE; // change to bias (or page if u wanna call it that) ++; (bias++;)
}

void memory_back_page(void)                                      // change to dec bias (if bias>0)
{                                                                // put if here to test if bias is > 0
    pointer = (pointer + MEMORY_SIZE - PAGE_SIZE) % MEMORY_SIZE; // bias --;
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

uint8_t memory_step(int write_pin, int read_pin, int next_pin, int back_pin, uint8_t input_value) // does not have in out pins defined
{
    // if input pin = 1 add string to memory
    // if out pin is = 1 it prints "the string" to the display

    if (next_pin && back_pin) // order of this is non arbitrary
    {
        memory_reset_pointer(); // CASE ONE
    }
    else if (next_pin)
    {
        memory_next_page(); // CASE TWO
    }
    else if (back_pin)
    {
        memory_back_page(); // CASE THREE
    }
    else if (write_pin && read_pin)
    {
        memory_set_location(input_value); // CASE FOUR
    }
    else if (write_pin && !read_pin)
    {
        memory_write(input_value); // CASE FIVE
    }
    else if (!write_pin && read_pin)
    {
        return memory_read(); // CASE SIX
    }

    return 0;
}