cmake_minimum_required(VERSION 3.13)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(dongle_emu)

pico_sdk_init()

add_executable(dongle_emu dongle_emu.c)

target_link_libraries(dongle_emu
    pico_stdlib
    tinyusb_device
)

pico_add_extra_outputs(dongle_emu)
