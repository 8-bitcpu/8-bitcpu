#include <stdio.h>
#include "pico/stdlib.h"
#include "oled.h"


extern uint8_t MEM_BUFFER[256];
extern uint8_t PTR;
extern uint8_t PAGE_SIZE;

extern uint8_t STRING_BUFFER[256];
extern uint8_t STRING_LEN;

extern uint8_t CONTROL_PINS_STABLE;
extern uint8_t PIO_PINS_STABLE;
extern uint8_t PREV_PIO_PINS_STABLE;

extern uint32_t GPIO_STATE;
extern uint8_t IN_BUS_VALUE;

extern void RUNTIME_STEP(void);
extern void print_mem_buffer(void);
extern void oled_print(void);


// ------------------ INTERNAL HELPERS ------------------

static void set_fake_gpio(uint8_t control_state, uint8_t input_bus)
{
    GPIO_STATE = 0;

    // GP0–7 input bus
    GPIO_STATE |= input_bus;

    // control pins
    if (control_state & (1u << 0)) GPIO_STATE |= (1u << 8);   // GP8
    if (control_state & (1u << 1)) GPIO_STATE |= (1u << 9);   // GP9
    if (control_state & (1u << 2)) GPIO_STATE |= (1u << 10);  // GP10
    if (control_state & (1u << 3)) GPIO_STATE |= (1u << 11);  // GP11
    if (control_state & (1u << 4)) GPIO_STATE |= (1u << 20);  // GP20
    if (control_state & (1u << 5)) GPIO_STATE |= (1u << 21);  // GP21
    if (control_state & (1u << 6)) GPIO_STATE |= (1u << 22);  // GP22

    CONTROL_PINS_STABLE = control_state & 0x0F;
    PIO_PINS_STABLE     = (control_state >> 4) & 0x07;
    IN_BUS_VALUE        = input_bus;
}

static const char *classify_state(uint8_t s)
{
    uint8_t R   = !!(s & (1u << 0));
    uint8_t W   = !!(s & (1u << 1));
    uint8_t N   = !!(s & (1u << 2));
    uint8_t B   = !!(s & (1u << 3));
    uint8_t PRE = !!(s & (1u << 4));
    uint8_t IN  = !!(s & (1u << 5));
    uint8_t OUT = !!(s & (1u << 6));

    if (N && B) return "CONFLICT_NEXT_BACK";
    if (IN && OUT) return "CONFLICT_IN_OUT";
    if (PRE && (R || W || N || B || IN || OUT)) return "PRELOAD_OVERRIDE";

    if (PRE) return "PRELOAD";
    if (IN)  return "IN";
    if (OUT) return "OUT";

    if (N) return "NEXT";
    if (B) return "BACK";

    if (R && W)  return "SET_POINTER";
    if (!R && W) return "READ_MEMORY";
    if (R && !W) return "WRITE_MEMORY";

    return "NO_OP";
}

// ------------------ MAIN STRESS TEST ------------------

void stress_test_runtime(void)
{
    const uint8_t input_tests[] = {0x00, 0x01, 0x7F, 0x80, 0xAA, 0xFF};
    const uint8_t ptr_tests[]   = {0, 1, 15, 16, 240, 255};

    printf("\n=== RUNTIME STEP STRESS TEST START ===\n");

    PAGE_SIZE = 16;

    for (unsigned p = 0; p < sizeof(ptr_tests); p++) {
        for (unsigned b = 0; b < sizeof(input_tests); b++) {
            for (uint8_t s = 0; s < 128; s++) {

                // reset state
                for (int i = 0; i < 256; i++) {
                    MEM_BUFFER[i] = (uint8_t)i;
                    STRING_BUFFER[i] = 0;
                }

                PTR = ptr_tests[p];
                STRING_LEN = 0;

                set_fake_gpio(s, input_tests[b]);

                uint8_t ptr_before = PTR;
                uint8_t mem_before = MEM_BUFFER[ptr_before];
                uint8_t len_before = STRING_LEN;

                printf("\nSTATE=%03u BUS=0x%02X PTR=%03u %-18s\n",
                       s, input_tests[b], PTR, classify_state(s));

                RUNTIME_STEP();
                oled_print();

                printf("PTR: %u -> %u\n", ptr_before, PTR);
                printf("MEM[%u]: 0x%02X -> 0x%02X\n",
                       ptr_before, mem_before, MEM_BUFFER[ptr_before]);
                printf("STRING_LEN: %u -> %u\n", len_before, STRING_LEN);

                if (STRING_LEN < 255 && STRING_BUFFER[STRING_LEN] != 0) {
                    printf("BUG: string not null terminated\n");
                }

                sleep_ms(2);
            }
        }
    }

    printf("=== END ===\n");
}

// ------------------ TARGETED TEST ------------------

void test_runtime_handle_in(void)
{
    printf("\n=== HANDLE_IN TEST ===\n");

    STRING_LEN = 0;
    STRING_BUFFER[0] = 0;

    PTR = 0;
    MEM_BUFFER[0] = 'A';

    PREV_PIO_PINS_STABLE = 0x00;
    PIO_PINS_STABLE      = 0x02;  // GP21 rising edge

    RUNTIME_STEP();

    printf("STRING_LEN=%u\n", STRING_LEN);
    printf("STRING_BUFFER[0]=%c (%u)\n", STRING_BUFFER[0], STRING_BUFFER[0]);
    printf("STRING_BUFFER[1]=%u\n", STRING_BUFFER[1]);

    if (STRING_LEN != 1 || STRING_BUFFER[0] != 'A' || STRING_BUFFER[1] != 0) {
        printf("FAIL\n");
        while (1) tight_loop_contents();
    }

    printf("PASS\n");
}

void test_runtime_output_count(void)
{
    // no PIO actions
    PIO_PINS_STABLE = 0;
    PREV_PIO_PINS_STABLE = 0;

    // Read-only state: GP8 high, GP9 low
    CONTROL_PINS_STABLE = 0x1;

    for (uint8_t v = 1; v <= 128; v++) {
        MEM_BUFFER[PTR] = v;

        IN_BUS_VALUE = 0;
        GPIO_STATE = 0;
        GPIO_STATE |= (1u << 8); // GP8 Read high

        RUNTIME_STEP();

        print_mem_buffer();

        sleep_ms(300);
    }
}

void super_debug_report(void)
{
    uint32_t raw = gpio_get_all();

    printf("\n--- SUPER DEBUG ---\n");

    printf("RAW GPIO:        0x%08lX\n", raw);

    printf("INPUT BUS GP0-7: 0x%02lX  dec:%lu\n",
           raw & 0xFF,
           raw & 0xFF);

    printf("CONTROL STABLE:  0x%02X  R:%d W:%d N:%d B:%d\n",
           CONTROL_PINS_STABLE,
           !!(CONTROL_PINS_STABLE & 0x01),
           !!(CONTROL_PINS_STABLE & 0x02),
           !!(CONTROL_PINS_STABLE & 0x04),
           !!(CONTROL_PINS_STABLE & 0x08));

    printf("PIO STABLE:      0x%02X  PRE:%d IN:%d OUT:%d\n",
           PIO_PINS_STABLE,
           !!(PIO_PINS_STABLE & 0x01),
           !!(PIO_PINS_STABLE & 0x02),
           !!(PIO_PINS_STABLE & 0x04));

    printf("DIRECT PINS:     GP8:%d GP9:%d GP10:%d GP11:%d GP20:%d GP21:%d GP22:%d\n",
           gpio_get(8),
           gpio_get(9),
           gpio_get(10),
           gpio_get(11),
           gpio_get(20),
           gpio_get(21),
           gpio_get(22));

    printf("PTR:             %u\n", PTR);
    printf("MEM[PTR]:        %u  hex:0x%02X\n", MEM_BUFFER[PTR], MEM_BUFFER[PTR]);

    printf("IN_BUS_VALUE:    %u  hex:0x%02X\n", IN_BUS_VALUE, IN_BUS_VALUE);

    printf("OUT BUS GP12-19: ");
    for (int i = 12; i <= 19; i++) {
        printf("%d", gpio_get(i));
    }
    printf("\n");

    printf("STRING_LEN:      %u\n", STRING_LEN);
    printf("STRING_BUFFER:   \"");

    for (uint8_t i = 0; i < STRING_LEN; i++) {
        uint8_t c = STRING_BUFFER[i];

        if (c >= 32 && c <= 126) {
            printf("%c", c);
        } else {
            printf("\\x%02X", c);
        }
    }

    printf("\"\n");

    printf("-------------------\n");
}