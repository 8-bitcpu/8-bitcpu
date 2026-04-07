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

/*
 * Preload the CPU-print workflow:
 *
 * Page 0:
 *  0  LOAD 12
 *  1  EQ 13
 *  2  SKIF
 *  3  JMP 5
 *  4  HALT
 *  5  LOAD 12
 *  6  PRINT
 *  7  LOAD 12
 *  8  ADD 14
 *  9  STORE 12
 * 10  JMP 0
 * 11  0
 * 12  i = 0
 * 13  limit = 3
 * 14  one = 1
 * 15  0
 *
 * Page 1:
 * 16 'C'
 * 17 'P'
 * 18 'U'
 * 19 0
 */
void memory_preload_cpu_workflow(void)
{
    memory_clear_all();

    memory[0] = 0x0C;  /* LOAD 12  */
    memory[1] = 0x5D;  /* EQ 13    */
    memory[2] = 0x80;  /* SKIF     */
    memory[3] = 0x75;  /* JMP 5    */
    memory[4] = 0xF0;  /* HALT     */
    memory[5] = 0x0C;  /* LOAD 12  */
    memory[6] = 0xC0;  /* PRINT    */
    memory[7] = 0x0C;  /* LOAD 12  */
    memory[8] = 0x2E;  /* ADD 14   */
    memory[9] = 0x1C;  /* STORE 12 */
    memory[10] = 0x70; /* JMP 0    */
    memory[11] = 0x00;
    memory[12] = 0x00; /* i = 0    */
    memory[13] = 0x03; /* limit=3  */
    memory[14] = 0x01; /* one = 1  */
    memory[15] = 0x00;

    memory[16] = 'C';
    memory[17] = 'P';
    memory[18] = 'U';
    memory[19] = 0x00;

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