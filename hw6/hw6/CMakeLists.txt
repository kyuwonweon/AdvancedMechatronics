cmake_minimum_required(VERSION 3.13)

# == VS CODE SDK LOCATOR (Required to find ~/.pico-sdk) ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# =========================================================

include(pico_sdk_import.cmake)

project(hw6 C CXX ASM)
pico_sdk_init()

add_executable(hw6 hw6.c usb_descriptors.c)

target_include_directories(hw6 PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(hw6
        pico_stdlib
        hardware_i2c
        tinyusb_device
        tinyusb_board
        )

# Enable USB serial and disable UART
pico_enable_stdio_usb(hw6 0)
pico_enable_stdio_uart(hw6 1)

# Create the .uf2 file
pico_add_extra_outputs(hw6)