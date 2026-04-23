#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "pico/stdlib.h"

extern uint8_t MEM_BUFFER[256];
extern uint8_t PTR;

uint8_t selected_program_index;
bool use_custom_program;
char custom_program_text[512];

typedef struct 
{
    const char *name;
    const uint8_t *bytes;
    uint8_t length;
}preloaded_programs;

static const uint8_t program_flashy_lights[] = {

};

static const preloaded_programs PROGRAMS[] = {
    { "Flashy Lights", program_flashy_lights, sizeof(program_flashy_lights) },
};

static const uint8_t PROGRAM_COUNT = sizeof(PROGRAMS) / sizeof(PROGRAMS[0]);

static void memory_clear_from_c(void) {
    for (int i = 0; i < 256; i++) {
        MEM_BUFFER[i] = 0;
    }
}

bool preload_program_by_index(uint8_t index) {
    if (index >= PROGRAM_COUNT) {
        return false;
    }

    memory_clear_from_c();

    const preloaded_programs *p = &PROGRAMS[index];
    for (uint8_t i = 0; i < p->length && i < 256; i++) {
        MEM_BUFFER[i] = p->bytes[i];
    }

    PTR = 0;
    return true;
}

bool preload_program_by_name(const char *name) {
    for (uint8_t i = 0; i < PROGRAM_COUNT; i++) {
        if (strcmp(name, PROGRAMS[i].name) == 0) {
            return preload_program_by_index(i);
        }
    }
    return false;
}

bool preload_program_from_string(const char *input) {// perhaps not needed
    memory_clear_from_c();

    uint16_t mem_index = 0;
    const char *p = input;

    while (*p != '\0' && mem_index < 256) {
        while (*p != '\0' && isspace((unsigned char)*p)) {
            p++;
        }

        if (*p == '\0') {
            break;
        }

        char *endptr = NULL;
        long value = strtol(p, &endptr, 10);

        if (p == endptr) {
            return false;
        }

        if (value < 0 || value > 255) {
            return false;
        }

        MEM_BUFFER[mem_index++] = (uint8_t)value;
        p = endptr;
    }

    PTR = 0;
    return true;
}

void preload_selected_program(void) {
    if (use_custom_program) {
        preload_program_from_string(custom_program_text);
    } else {
        preload_program_by_index(selected_program_index);
    }
}

void run_preload_cli(void) {
    char line[512];

    printf("we are in CLI\n");

    while (1) {
        printf("\n---PRELOAD MENU---\n");
        printf("1) test\n");
        printf("2) count_up\n");
        printf("3) blank\n");
        printf("4) custom\n");
        printf("5) exit\n> ");

        int choice = getchar();
        printf("%c\n", choice);

        if (choice == '1') {
            preload_program_by_index(0);
            printf("Loaded preset 1\n");
            return;
        }

        if (choice == '2') {
            preload_program_by_index(1);
            printf("Loaded preset 2\n");
            return;
        }

        if (choice == '3') {
            preload_program_by_index(2);
            printf("Loaded preset 3\n");
            return;
        }

        if (choice == '4') {
            printf("Enter bytes 0-255 separated by spaces:\n> ");

            int len = 0;
            while (1) {
                int ch = getchar();

                if (ch == '\r' || ch == '\n') {
                    line[len] = '\0';
                    printf("\n");
                    break;
                }

                if (len < (int)sizeof(line) - 1) {
                    line[len++] = (char)ch;
                    putchar(ch);
                }
            }

            if (preload_program_from_string(line)) {
                printf("Custom program loaded.\n");
                return;
            } else {
                printf("Invalid custom program. Try again.\n");
                continue;
            }
        }

        if (choice == '5') {
            printf("Exiting preload menu.\n");
            return;
        }

        printf("Invalid selection.\n");
    }
}