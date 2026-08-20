cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 23)

set(CMAKE_EXE_LINKER_FLAGS "-Wl,--print-memory-usage")

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

set(PROJECT_AUXILIARY_FILES
        ../readme.md
        scripts/cmake-make.sh
        scripts/cmake.sh
        scripts/device-reset-openocd.sh
        scripts/auto-format.sh
        scripts/gdb-client.sh
        scripts/gdb-server.sh
        scripts/git-submodule-init.sh
        scripts/install-prerequisites-archlinux.sh
        scripts/install-prerequisites-ubuntu.sh
        scripts/make-upload-openocd.sh
        scripts/make.sh
        scripts/picotool-info.sh
        scripts/readme.md
        scripts/serial-monitor.sh
        scripts/stack-usage-summary.sh
        scripts/upload-openocd.sh
)

# add executable
add_executable(rf_probe
        lib/sample_data/AverageT.hpp
        lib/sample_data/TransactionBuffer.h
        modules/ad7887/ad7887.cpp
        modules/core0_ui/ui.cpp
        modules/core0_ui/core0_main.cpp
        modules/core1_sampler/core1_main.cpp
        modules/lvgl/lv_display.cpp
        modules/lvgl/lv_input.cpp
        modules/periphery/input/buttons.cpp
        modules/periphery/input/input.cpp
        modules/periphery/input/joystick.cpp
        modules/periphery/display/display.cpp
        modules/periphery/led/user_led.cpp
        modules/rf_power_meter/KValues3rdOrder.hpp
        modules/debug.c
        main.cpp
        ${PROJECT_AUXILIARY_FILES}
)

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
        PICO_STACK_SIZE=0x1c00            # core 0: 7 KiB of the combined scratch SRAM
        PICO_CORE1_STACK_SIZE=0x400       # core 1: lower 1 KiB of scratch X
        #PICO_STDIO_STACK_BUFFER_SIZE=64  # use a small printf on stack buffer
        PICO_USE_STACK_GUARDS=1           # guard the lower stack boundary
)

# Repartition the contiguous 8 KiB scratch area without putting either stack
# in main SRAM. Core 1 grows down through the lower 1 KiB; core 0 grows down
# through the upper 7 KiB (all of physical scratch Y plus 3 KiB of X).
pico_set_linker_script_var(rf_probe SCRATCH_X_ORIGIN 0x20080000)
pico_set_linker_script_var(rf_probe SCRATCH_X_LENGTH 0x400)
pico_set_linker_script_var(rf_probe SCRATCH_Y_ORIGIN 0x20080400)
pico_set_linker_script_var(rf_probe SCRATCH_Y_LENGTH 0x1c00)

target_compile_options(rf_probe PUBLIC -Werror -Wall -Wnull-dereference -g3 -Og)
target_compile_options(rf_probe PRIVATE -fstack-usage)

pico_set_program_url(${PROJECT_NAME} "https://github.com/rubienr-rfpowermeter/rf-powermeter-pipico")
pico_set_program_description(${PROJECT_NAME} "Power Meter UI (sv1afn.com)")
pico_add_extra_outputs(rf_probe)

add_custom_target(upload
     COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/scripts/upload-openocd.sh"
     DEPENDS rf_probe
     WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
     USES_TERMINAL
     VERBATIM
 )

add_custom_target(monitor
    COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/scripts/serial-monitor.sh"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    USES_TERMINAL
    VERBATIM
)
