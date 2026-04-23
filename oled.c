#include <stdint.h>
#include <stdbool.h>

#include "oled.h"

extern uint8_t MEM_BUFFER[256];
extern uint8_t PTR;

extern uint8_t STRING_BUFFER[256];
extern uint8_t STRING_LEN;

extern uint8_t CONTROL_PINS_STABLE;
extern uint8_t PIO_PINS_STABLE;

extern uint8_t IN_BUS_VALUE;


static uint8_t last_output_value = 0;
static bool last_output_valid = false;

void oled_report_exact_repo_style(void)
{
    bool read_pin  = (CONTROL_PINS_STABLE & 0x1) != 0;
    bool write_pin = (CONTROL_PINS_STABLE & 0x2) != 0;
    bool next_pin  = (CONTROL_PINS_STABLE & 0x4) != 0;
    bool back_pin  = (CONTROL_PINS_STABLE & 0x8) != 0;

    bool in_pin    = (PIO_PINS_STABLE & 0x1) != 0;
    bool out_pin   = (PIO_PINS_STABLE & 0x2) != 0;
    bool preload   = (PIO_PINS_STABLE & 0x4) != 0;

    uint8_t current_value = MEM_BUFFER[PTR];

    // Keep the last byte shown on OUT, so the OLED has something stable to display.
    if (out_pin || (read_pin && !write_pin)) {
        last_output_value = current_value;
        last_output_valid = true;
    }

    if (preload) {
        oled_clear();
        oled_draw_string(0, 3, "PRELOAD");
        oled_update();
        return;
    }

    if (next_pin && back_pin) {
        oled_show_halt();
        return;
    }

    if (in_pin && STRING_LEN > 0) {
        oled_show_output_string((const char *)STRING_BUFFER);
        return;
    }

    oled_show_memory_status(
        PTR,                 // pointer
        current_value,       // current_value
        IN_BUS_VALUE,        // input_value
        last_output_value,   // output_value
        last_output_valid,   // output_valid
        read_pin,
        write_pin,
        next_pin,
        back_pin,
        (const char *)STRING_BUFFER
    );
}