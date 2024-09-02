cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 23)

# initialise pico_sdk from installed location (note this can come from environment, CMake cache etc)
#set(PICO_SDK_PATH "{$USER}/pico/pico-sdk")

set(PICO_BOARD pico CACHE STRING "Pico Board Type")
set(PICO_PLATFORM rp2040 CACHE STRING "Pico Platform")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)


if (PICO_SDK_VERSION_STRING VERSION_LESS "1.4.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.4.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif ()

project(rf_probe C CXX ASM)

# initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# LVGL
include_directories(./)
add_subdirectory(./gitmodules/lvgl)
include_directories(./gitmodules/lvgl)
add_definitions(-DLV_LVGL_H_INCLUDE_SIMPLE=1)
# file(GLOB_RECURSE DEMO_SOURCES ./gitmodules/lvgl/demos/*.c)

# add executable
add_executable(rf_probe
        # ${DEMO_SOURCES}
        # gitmodules/lvgl/demos/keypad_encoder/lv_demo_keypad_encoder.c
        img.c
        lv_port_disp.c
        lv_port_indev.c
        modules/ws2812/rgbw.c
        main.c
)

pico_generate_pio_header(rf_probe
        ${CMAKE_CURRENT_LIST_DIR}/modules/ws2812/ws2812.pio
        OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/generated)

pico_set_program_name(rf_probe "rf_probe")
pico_set_program_version(rf_probe "0.1")

pico_enable_stdio_uart(rf_probe 1)
pico_enable_stdio_usb(rf_probe 0)

# standard libraries
target_link_libraries(rf_probe
        hardware_adc
        # hardware_dma
        hardware_i2c
        hardware_pio
        hardware_spi
        lvgl
        lvgl_demos
        pico_multicore
        pico_stdlib
)

# add the standard include files to the build
target_include_directories(rf_probe PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        # ${CMAKE_CURRENT_LIST_DIR}/..
)

target_compile_definitions(rf_probe PRIVATE
        PICO_STACK_SIZE=0x1000
        PICO_STDIO_STACK_BUFFER_SIZE=64 # use a small printf on stack buffer
)

pico_add_extra_outputs(rf_probe)
