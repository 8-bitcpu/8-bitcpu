cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(memory_controller C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(memory_controller
    main.c
    memory.S 
    RUN_TIME.S
    STRING_BUILDER.S
    preloads.c
    oled.c
    oled.h
)

target_link_libraries(memory_controller
    pico_stdlib
    hardware_i2c
)

pico_enable_stdio_usb(memory_controller 1)
pico_enable_stdio_uart(memory_controller 0)

pico_add_extra_outputs(memory_controller)