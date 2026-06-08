cmake_minimum_required(VERSION 3.13)

# Point to the SDK's import file
include(${CMAKE_CURRENT_SOURCE_DIR}/../pico-sdk/external/pico_sdk_import.cmake)

project(dongle_emu)

pico_sdk_init()

add_executable(dongle_emu dongle_emu.c)

target_link_libraries(dongle_emu
    pico_stdlib
    tinyusb_device
)

target_include_directories(dongle_emu PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../pico-sdk/src/common/tinyusb
)

pico_add_extra_outputs(dongle_emu)
