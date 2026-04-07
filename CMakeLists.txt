cmake_minimum_required(VERSION 3.13...3.27)

include(pico_sdk_import.cmake)

project(pico_ram C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(pico_ram
    main.c
    memory.c
    oled.c
)

target_include_directories(pico_ram PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(pico_ram
    pico_stdlib
    hardware_i2c
)

pico_add_extra_outputs(pico_ram)