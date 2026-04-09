#include "pico/stdlib.h"
#include "memory.h"
#include "oled.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define BUS_IN_START 0   // GP0-GP7
#define READ_PIN 8       // GP8
#define WRITE_PIN 9      // GP9
#define NEXT_PIN 10      // GP10
#define BACK_PIN 11      // GP11
#define BUS_OUT_START 12 // GP12-GP19
#define IN_PIN 20        // GP20 - IN trigger
#define OUT_PIN 21       // GP21 - OUT trigger
#define PRELOAD_PIN 22   // GP22 - preload trigger

#define OLED_UPDATE_US 120000
#define STRING_BUILDER_MAX 18
#define OVERLAY_TIME_MS 5000

/*
 * Control bus settle time:
 * wait this long after a detected control-state change
 * before deciding what the command really is.
 */
#define CONTROL_SETTLE_US 1000

typedef struct
{
    bool read_pin;
    bool write_pin;
    bool next_pin;
    bool back_pin;
    bool in_pin;
    bool out_pin;
    bool preload_pin;
} control_state_t;

typedef enum
{
    DISPLAY_MAIN = 0,
    DISPLAY_HALT,
    DISPLAY_SB
} display_mode_t;

/*
 * Easy-to-edit preload image.
 *
 * Program:
 *   0   LOAD 10
 *   1   STORE 11
 *   2   HALT
 *
 * Data:
 *   10  42
 *   11  0
 *
 * Everything not listed stays 0 automatically.
 */
static const uint8_t preload_image[MEMORY_SIZE] = {
    [0] = 10,  // LOAD 10
    [1] = 27,  // STORE 11  (change this to your real encoded value if needed)
    [2] = 240, // HALT      (change this to your real encoded value if needed)

    [10] = 42,
    [11] = 0};

static const uint16_t preload_image_size = MEMORY_SIZE;

static void preload_memory_image(void)
{
    memory_load_image(preload_image, preload_image_size);
}

static void init_input_bus(void)
{
    for (int i = 0; i < 8; i++)
    {
        gpio_init(BUS_IN_START + i);
        gpio_set_dir(BUS_IN_START + i, GPIO_IN);
        gpio_pull_down(BUS_IN_START + i);
    }
}

static void init_control_pins(void)
{
    int pins[] = {READ_PIN, WRITE_PIN, NEXT_PIN, BACK_PIN, IN_PIN, OUT_PIN, PRELOAD_PIN};

    for (int i = 0; i < 7; i++)
    {
        gpio_init(pins[i]);
        gpio_set_dir(pins[i], GPIO_IN);
        gpio_pull_down(pins[i]);
    }
}

static void init_output_bus(void)
{
    for (int i = 0; i < 8; i++)
    {
        gpio_init(BUS_OUT_START + i);
        gpio_set_dir(BUS_OUT_START + i, GPIO_IN);
        gpio_disable_pulls(BUS_OUT_START + i);
    }
}

static uint8_t read_input_bus(void)
{
    uint8_t value = 0;

    for (int i = 0; i < 8; i++)
    {
        value |= (gpio_get(BUS_IN_START + i) ? 1 : 0) << i;
    }

    return value;
}

static void write_output_bus(uint8_t value)
{
    for (int i = 0; i < 8; i++)
    {
        uint8_t bit = (value >> i) & 1;
        gpio_put(BUS_OUT_START + i, bit);
    }
}

static void enable_output_bus(void)
{
    for (int i = 0; i < 8; i++)
    {
        gpio_set_dir(BUS_OUT_START + i, GPIO_OUT);
    }
}

static void disable_output_bus(void)
{
    for (int i = 0; i < 8; i++)
    {
        gpio_set_dir(BUS_OUT_START + i, GPIO_IN);
    }
}

static control_state_t read_control_state(void)
{
    control_state_t s;
    s.read_pin = gpio_get(READ_PIN);
    s.write_pin = gpio_get(WRITE_PIN);
    s.next_pin = gpio_get(NEXT_PIN);
    s.back_pin = gpio_get(BACK_PIN);
    s.in_pin = gpio_get(IN_PIN);
    s.out_pin = gpio_get(OUT_PIN);
    s.preload_pin = gpio_get(PRELOAD_PIN);
    return s;
}

static bool same_control_state(control_state_t a, control_state_t b)
{
    return (a.read_pin == b.read_pin) &&
           (a.write_pin == b.write_pin) &&
           (a.next_pin == b.next_pin) &&
           (a.back_pin == b.back_pin) &&
           (a.in_pin == b.in_pin) &&
           (a.out_pin == b.out_pin) &&
           (a.preload_pin == b.preload_pin);
}

static bool is_idle_state(control_state_t s)
{
    return !s.read_pin &&
           !s.write_pin &&
           !s.next_pin &&
           !s.back_pin &&
           !s.in_pin &&
           !s.out_pin &&
           !s.preload_pin;
}

static bool is_halt_bus_pattern(uint8_t input_bus)
{
    return ((input_bus & 0xF0) == 0xF0); // GP4,5,6,7 all high
}

static char ascii_from_value(uint8_t value)
{
    if (value >= 32 && value <= 126)
    {
        return (char)value;
    }

    return '?';
}

static void append_char_to_string_builder(char *buffer, int *length, char c)
{
    if (*length < (STRING_BUILDER_MAX - 1))
    {
        buffer[*length] = c;
        (*length)++;
        buffer[*length] = '\0';
    }
    else
    {
        for (int i = 1; i < STRING_BUILDER_MAX - 1; i++)
        {
            buffer[i - 1] = buffer[i];
        }

        buffer[STRING_BUILDER_MAX - 2] = c;
        buffer[STRING_BUILDER_MAX - 1] = '\0';
        *length = STRING_BUILDER_MAX - 1;
    }
}

static void clear_string_builder(char *buffer, int *length)
{
    *length = 0;
    buffer[0] = '\0';
}

/*
 * Read control bus until two consecutive samples match.
 * This prevents reacting to a partial combination like:
 *   R first, then W
 * or
 *   NEXT first, then BACK
 */
static control_state_t read_stable_control_state(void)
{
    control_state_t first = read_control_state();
    sleep_us(CONTROL_SETTLE_US);
    control_state_t second = read_control_state();

    while (!same_control_state(first, second))
    {
        first = second;
        sleep_us(CONTROL_SETTLE_US);
        second = read_control_state();
    }

    return second;
}

int main(void)
{
    stdio_init_all();
    sleep_ms(2000);

    memory_init();
    init_input_bus();
    init_control_pins();
    init_output_bus();
    oled_init();

    absolute_time_t last_oled_update = get_absolute_time();
    absolute_time_t overlay_end_time = get_absolute_time();

    display_mode_t display_mode = DISPLAY_MAIN;

    control_state_t last_control_state = read_stable_control_state();
    uint8_t last_input_bus = read_input_bus();

    uint8_t input_bus = last_input_bus;
    uint8_t output_value = 0;
    uint8_t current_memory_value = memory_read();
    bool output_valid = false;

    char string_builder[STRING_BUILDER_MAX];
    int string_builder_length = 0;
    clear_string_builder(string_builder, &string_builder_length);

    uint16_t last_ptr = 0xFFFF;
    uint8_t last_mem = 0xFF;
    uint8_t last_inbus_for_oled = 0xFF;
    uint8_t last_out = 0xFF;
    bool last_output_valid = false;
    control_state_t last_oled_state = last_control_state;
    bool force_refresh = true;

    while (true)
    {
        control_state_t raw_control_state = read_control_state();
        input_bus = read_input_bus();

        bool raw_control_changed = !same_control_state(raw_control_state, last_control_state);
        bool input_bus_changed = (input_bus != last_input_bus);

        control_state_t control_state = raw_control_state;

        /*
         * If the control bus changed, wait for it to settle before using it.
         */
        if (raw_control_changed)
        {
            control_state = read_stable_control_state();
        }

        bool control_changed = !same_control_state(control_state, last_control_state);

        bool halt_active = is_halt_bus_pattern(input_bus);
        bool last_halt_active = is_halt_bus_pattern(last_input_bus);
        bool halt_rising_edge = (!last_halt_active && halt_active);

        bool out_rising_edge = (!last_control_state.out_pin && control_state.out_pin);
        bool in_rising_edge = (!last_control_state.in_pin && control_state.in_pin);
        bool preload_rising_edge = (!last_control_state.preload_pin && control_state.preload_pin);

        /*
         * PRELOAD has highest priority.
         * It only happens on a settled rising edge of GP22.
         */
        if (preload_rising_edge)
        {
            preload_memory_image();

            current_memory_value = memory_read();
            output_value = 0;
            output_valid = false;
            disable_output_bus();

            display_mode = DISPLAY_MAIN;
            force_refresh = true;

            printf("PRELOAD TRIGGERED ON GP22 | Memory image loaded | Pointer reset to 0\n");
            printf("MEM[0]=%u MEM[1]=%u MEM[2]=%u MEM[10]=%u MEM[11]=%u\n",
                   memory_peek(0),
                   memory_peek(1),
                   memory_peek(2),
                   memory_peek(10),
                   memory_peek(11));
        }
        else if (halt_rising_edge)
        {
            /*
             * HALT clears all memory and resets pointer to 0.
             * Also clear output bus state.
             * Clear SB too if you want HALT to fully reset everything.
             */
            memory_clear_all();
            current_memory_value = memory_read();

            output_value = 0;
            output_valid = false;
            disable_output_bus();

            clear_string_builder(string_builder, &string_builder_length);

            display_mode = DISPLAY_HALT;
            overlay_end_time = make_timeout_time_ms(OVERLAY_TIME_MS);
            force_refresh = true;

            printf("HALT TRIGGERED | Memory cleared | Pointer reset | SB cleared\n");
        }
        else
        {
            /*
             * Execute memory control action once per new stable control state.
             * This avoids:
             *   R alone being interpreted before W also arrives
             *   NEXT alone being interpreted before BACK also arrives
             */
            if (control_changed)
            {
                uint8_t step_result = memory_step(
                    control_state.write_pin,
                    control_state.read_pin,
                    control_state.next_pin,
                    control_state.back_pin,
                    input_bus);

                if (!control_state.write_pin && control_state.read_pin)
                {
                    output_value = step_result;
                    output_valid = true;
                    enable_output_bus();
                    write_output_bus(output_value);
                }
                else
                {
                    output_valid = false;
                    disable_output_bus();
                }

                current_memory_value = memory_read();

                printf("CTRL R=%d W=%d N=%d B=%d IN=%d OUT=%d PRE=%d | PTR=%u | MEM=%u | INBUS=%u | STEP=%u | SB=\"%s\"\n",
                       control_state.read_pin,
                       control_state.write_pin,
                       control_state.next_pin,
                       control_state.back_pin,
                       control_state.in_pin,
                       control_state.out_pin,
                       control_state.preload_pin,
                       memory_get_pointer(),
                       current_memory_value,
                       input_bus,
                       step_result,
                       string_builder);
            }

            /*
             * Keep bus driven while stable READ-only is being held.
             * Do not re-execute memory_step here.
             */
            if (!control_state.write_pin && control_state.read_pin)
            {
                output_value = memory_read();
                output_valid = true;
                enable_output_bus();
                write_output_bus(output_value);
            }
            else if (is_idle_state(control_state))
            {
                output_valid = false;
                disable_output_bus();
            }

            current_memory_value = memory_read();

            /*
             * IN / OUT are also based on the settled control state,
             * so they trigger once when the stable state rises.
             */
            if (in_rising_edge)
            {
                char appended_char = ascii_from_value(current_memory_value);
                append_char_to_string_builder(string_builder, &string_builder_length, appended_char);

                printf("IN TRIGGER | PTR=%u | MEM=%u | ASCII=%c | SB=\"%s\"\n",
                       memory_get_pointer(),
                       current_memory_value,
                       appended_char,
                       string_builder);
            }

            if (out_rising_edge)
            {
                display_mode = DISPLAY_SB;
                overlay_end_time = make_timeout_time_ms(OVERLAY_TIME_MS);
                force_refresh = true;

                printf("OUT TRIGGER | \"%s\"\n", string_builder);
            }
        }

        if (display_mode != DISPLAY_MAIN &&
            absolute_time_diff_us(get_absolute_time(), overlay_end_time) <= 0)
        {
            display_mode = DISPLAY_MAIN;
            force_refresh = true;
        }

        if (absolute_time_diff_us(last_oled_update, get_absolute_time()) > OLED_UPDATE_US || force_refresh)
        {
            bool oled_changed =
                force_refresh ||
                (memory_get_pointer() != last_ptr) ||
                (current_memory_value != last_mem) ||
                (input_bus != last_inbus_for_oled) ||
                (output_value != last_out) ||
                (output_valid != last_output_valid) ||
                !same_control_state(control_state, last_oled_state);

            if (oled_changed)
            {
                if (display_mode == DISPLAY_HALT)
                {
                    oled_show_halt();
                }
                else if (display_mode == DISPLAY_SB)
                {
                    oled_show_output_string(string_builder);
                }
                else
                {
                    oled_show_memory_status(
                        memory_get_pointer(),
                        current_memory_value,
                        input_bus,
                        output_value,
                        output_valid,
                        control_state.read_pin,
                        control_state.write_pin,
                        control_state.next_pin,
                        control_state.back_pin,
                        string_builder);
                }

                last_ptr = memory_get_pointer();
                last_mem = current_memory_value;
                last_inbus_for_oled = input_bus;
                last_out = output_value;
                last_output_valid = output_valid;
                last_oled_state = control_state;
            }

            last_oled_update = get_absolute_time();
            force_refresh = false;
        }

        last_control_state = control_state;
        last_input_bus = input_bus;

        sleep_ms(1);
    }

    return 0;
}