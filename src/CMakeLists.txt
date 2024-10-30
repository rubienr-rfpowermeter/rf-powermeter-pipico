cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 23)

set(CMAKE_EXE_LINKER_FLAGS "-Wl,--print-memory-usage")

# pico-sdk fallback to $USER/pico/pico-sdk if environment PICO_SDK_PATH is not set
if (DEFINED ENV{PICO_SDK_PATH})
    set(PICO_SDK_PATH "$ENV{PICO_SDK_PATH}")
else ()
    set(PICO_SDK_PATH "$ENV{USER}/pico/pico-sdk")
endif ()

set(PICO_BOARD pico2 CACHE STRING "Pico Board Type")
set(PICO_PLATFORM rp2350-arm-s CACHE STRING "Pico Platform")

# pull in Raspberry Pi Pico SDK
include(pico_sdk_import.cmake)


if (PICO_SDK_VERSION_STRING VERSION_LESS "1.4.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.4.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif ()

project(rf_probe C CXX ASM)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# LVGL
include_directories(./)
add_subdirectory(./gitmodules/lvgl)
include_directories(./gitmodules/lvgl)
add_definitions(-DLV_LVGL_H_INCLUDE_SIMPLE=1)

# add executable
add_executable(rf_probe
        lib/ad7887/AD7887.cpp
        lib/ad7887/registers.cpp
        lib/ad8318/AD8318.cpp
        lib/ad8318/KValues3rdOrder.hpp
        lib/si_units/si_unit_type.cpp
        lib/average/AverageT.hpp
        modules/globals/globals.cpp
        modules/lvgl/lv_display.cpp
        modules/lvgl/lv_input.cpp
        modules/periphery/input/buttons.cpp
        modules/periphery/input/input.cpp
        modules/periphery/input/joystick.cpp
        modules/periphery/display/display.cpp
        modules/periphery/led/user_led.cpp
        modules/main_sampler/main.cpp
        modules/main_ui/image_data.c
        modules/main_ui/main.cpp
        modules/main_ui/ui.cpp
        main.cpp
)

# include_directories(./modules/globals)

#pico_generate_pio_header(rf_probe
#        ${CMAKE_CURRENT_LIST_DIR}/modules/periphery/ws2812/ws2812.pio
#        OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/generated)

pico_set_program_name(rf_probe "rf_probe")
pico_set_program_version(rf_probe "0.1")

pico_enable_stdio_uart(rf_probe 1)
pico_enable_stdio_usb(rf_probe 0)

# standard libraries
target_link_libraries(rf_probe
        hardware_adc
        hardware_dma
        hardware_i2c
        hardware_pio
        hardware_pwm
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
        # PICO_STACK_SIZE=0x1000
        PICO_STDIO_STACK_BUFFER_SIZE=64 # use a small printf on stack buffer
)

target_compile_options(rf_probe PUBLIC -Werror -Wall -Wnull-dereference)

pico_set_program_url(${PROJECT_NAME} "github.com/rubienr-rfpowermeter/rf-powermeter-pipico")
pico_set_program_description(${PROJECT_NAME} "Power Meter UI (sv1afn.com)")
pico_add_extra_outputs(rf_probe)
