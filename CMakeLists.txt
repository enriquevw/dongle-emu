cmake_minimum_required(VERSION 3.13)

if(NOT DEFINED PICO_SDK_PATH)
    message(FATAL_ERROR "PICO_SDK_PATH is not set")
endif()

include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(dongle_emu)

pico_sdk_init()

add_executable(dongle_emu dongle_emu.c)

target_include_directories(dongle_emu PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_definitions(dongle_emu PRIVATE
    PICO_RP2040_USB_DEVICE_ENUMERATION_FIX=1
)

target_link_libraries(dongle_emu
    pico_stdlib
    tinyusb_device
)

pico_add_extra_outputs(dongle_emu)
