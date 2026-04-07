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

#define PRELOAD_PIN 20 // GP20
#define IN_PIN 21      // GP21
#define OUT_PIN 22     // GP22

#define OLED_UPDATE_US 120000
#define STRING_BUILDER_SIZE 64

typedef struct
{
    bool read_pin;
    bool write_pin;
    bool next_pin;
    bool back_pin;
} control_state_t;

static char output_builder[STRING_BUILDER_SIZE];
static int output_builder_len = 0;

static void builder_clear(void)
{
    output_builder_len = 0;
    output_builder[0] = '\0';
}

static void builder_append_ascii(uint8_t value)
{
    if (value < 32 || value > 126)
    {
        printf("APPEND BLOCKED: value=%u not printable ASCII\n", value);
        return;
    }

    if (output_builder_len >= STRING_BUILDER_SIZE - 1)
    {
        printf("APPEND BLOCKED: builder full\n");
        return;
    }

    output_builder[output_builder_len++] = (char)value;
    output_builder[output_builder_len] = '\0';

    printf("APPENDED: %u '%c' -> \"%s\"\n", value, (char)value, output_builder);
}

static bool is_halt_instruction(uint8_t value)
{
    return ((value & 0xF0) == 0xF0);
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
    int pins[] = {READ_PIN, WRITE_PIN, NEXT_PIN, BACK_PIN};

    for (int i = 0; i < 4; i++)
    {
        gpio_init(pins[i]);
        gpio_set_dir(pins[i], GPIO_IN);
        gpio_pull_down(pins[i]);
    }
}

static void init_extra_pins(void)
{
    int pins[] = {PRELOAD_PIN, IN_PIN, OUT_PIN};

    for (int i = 0; i < 3; i++)
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
    return s;
}

static bool same_control_state(control_state_t a, control_state_t b)
{
    return (a.read_pin == b.read_pin) &&
           (a.write_pin == b.write_pin) &&
           (a.next_pin == b.next_pin) &&
           (a.back_pin == b.back_pin);
}

static bool is_idle_state(control_state_t s)
{
    return !s.read_pin && !s.write_pin && !s.next_pin && !s.back_pin;
}

static void do_preload(void)
{
    memory_preload_cpu_workflow();
    builder_clear();

    printf("PRELOAD TRIGGERED\n");
    printf("memory[0]  = %u\n", memory_peek(0));
    printf("memory[4]  = %u\n", memory_peek(4));
    printf("memory[16] = %u '%c'\n", memory_peek(16), memory_peek(16));
    printf("memory[17] = %u '%c'\n", memory_peek(17), memory_peek(17));
    printf("memory[18] = %u '%c'\n", memory_peek(18), memory_peek(18));
}

int main(void)
{
    stdio_init_all();
    sleep_ms(2000);

    memory_init();
    init_input_bus();
    init_control_pins();
    init_extra_pins();
    init_output_bus();
    oled_init();
    builder_clear();

    do_preload();
    oled_show_output_string("PRELOADED");
    sleep_ms(1000);

    absolute_time_t last_oled_update = get_absolute_time();

    control_state_t last_control_state = read_control_state();
    bool last_preload_state = gpio_get(PRELOAD_PIN);

    uint8_t last_input_bus = read_input_bus();

    uint8_t input_bus = last_input_bus;
    uint8_t output_value = 0;
    uint8_t current_memory_value = memory_read();
    bool output_valid = false;

    uint16_t last_ptr = 0xFFFF;
    uint8_t last_mem = 0xFF;
    uint8_t last_inbus_for_oled = 0xFF;
    uint8_t last_out = 0xFF;
    bool last_output_valid = false;
    control_state_t last_oled_state = last_control_state;

    /*
     * Append condition latch:
     * append once for a given valid (IN=1, R=1, W=0, PTR, OUTVAL) condition.
     * Re-arm when the condition is broken or when pointer/value changes.
     */
    bool append_done_for_current_condition = false;
    uint16_t append_condition_ptr = 0xFFFF;
    uint8_t append_condition_value = 0xFF;

    while (true)
    {
        bool preload_state = gpio_get(PRELOAD_PIN);
        bool in_state = gpio_get(IN_PIN);
        bool out_state = gpio_get(OUT_PIN);

        control_state_t control_state = read_control_state();
        input_bus = read_input_bus();

        if (preload_state && !last_preload_state)
        {
            do_preload();
            current_memory_value = memory_read();
            output_value = 0;
            output_valid = false;

            append_done_for_current_condition = false;
            append_condition_ptr = 0xFFFF;
            append_condition_value = 0xFF;

            oled_show_output_string("PRELOADED");
            sleep_ms(300);
        }
        last_preload_state = preload_state;

        bool control_changed = !same_control_state(control_state, last_control_state);
        bool input_bus_changed = (input_bus != last_input_bus);

        if (control_changed || input_bus_changed)
        {
            if (control_changed ||
                ((control_state.write_pin && control_state.read_pin) ||
                 (control_state.write_pin && !control_state.read_pin)))
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

                printf("CTRL R=%d W=%d N=%d B=%d | PTR=%u | MEM=%u | INBUS=%u | STEP=%u | INPIN=%d | OUTPIN=%d\n",
                       control_state.read_pin,
                       control_state.write_pin,
                       control_state.next_pin,
                       control_state.back_pin,
                       memory_get_pointer(),
                       current_memory_value,
                       input_bus,
                       step_result,
                       in_state,
                       out_state);
            }
        }

        /*
         * While in READ mode, always keep current output synced.
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
         * Main append rule:
         * if IN=1 and R=1 and W=0 and output_valid=1
         * then append once for this current pointer/value condition.
         */
        bool append_condition_active =
            in_state &&
            control_state.read_pin &&
            !control_state.write_pin &&
            output_valid;

        if (append_condition_active)
        {
            bool same_as_last_append_condition =
                append_done_for_current_condition &&
                (memory_get_pointer() == append_condition_ptr) &&
                (output_value == append_condition_value);

            printf("APPEND CHECK | IN=%d R=%d W=%d PTR=%u OUT=%u DONE=%d SAME=%d\n",
                   in_state,
                   control_state.read_pin,
                   control_state.write_pin,
                   memory_get_pointer(),
                   output_value,
                   append_done_for_current_condition,
                   same_as_last_append_condition);

            if (!same_as_last_append_condition)
            {
                builder_append_ascii(output_value);

                append_done_for_current_condition = true;
                append_condition_ptr = memory_get_pointer();
                append_condition_value = output_value;

                printf("SB NOW -> \"%s\"\n", output_builder);
            }
        }
        else
        {
            /*
             * Condition broke, so re-arm append for next valid condition.
             */
            append_done_for_current_condition = false;
            append_condition_ptr = 0xFFFF;
            append_condition_value = 0xFF;
        }

        if (out_state)
        {
            oled_show_output_string(output_builder);
        }
        else if (!control_state.write_pin &&
                 control_state.read_pin &&
                 output_valid &&
                 is_halt_instruction(output_value))
        {
            oled_show_halt();
        }

        if (absolute_time_diff_us(last_oled_update, get_absolute_time()) > OLED_UPDATE_US)
        {
            bool oled_changed =
                (memory_get_pointer() != last_ptr) ||
                (current_memory_value != last_mem) ||
                (input_bus != last_inbus_for_oled) ||
                (output_value != last_out) ||
                (output_valid != last_output_valid) ||
                !same_control_state(control_state, last_oled_state);

            if (oled_changed || !out_state)
            {
                if (out_state)
                {
                    oled_show_output_string(output_builder);
                }
                else if (!control_state.write_pin &&
                         control_state.read_pin &&
                         output_valid &&
                         is_halt_instruction(output_value))
                {
                    oled_show_halt();
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
                        output_builder);
                }

                last_ptr = memory_get_pointer();
                last_mem = current_memory_value;
                last_inbus_for_oled = input_bus;
                last_out = output_value;
                last_output_valid = output_valid;
                last_oled_state = control_state;
            }

            last_oled_update = get_absolute_time();
        }

        last_control_state = control_state;
        last_input_bus = input_bus;

        sleep_ms(1);
    }

    return 0;
}