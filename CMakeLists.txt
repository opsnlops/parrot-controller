cmake_minimum_required(VERSION 3.13)

# Pull in SDK (must be before project)
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

include($ENV{FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)

project(app C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the SDK
pico_sdk_init()

add_executable(controller
        src/main.c
        )

target_include_directories(controller PRIVATE ${CMAKE_CURRENT_LIST_DIR})

# pull in common dependencies
target_link_libraries(controller
        FreeRTOS-Kernel
        FreeRTOS-Kernel-Heap4
        hardware_gpio
        hardware_pwm
        pico_stdlib)

# enable usb output, disable uart output
pico_enable_stdio_usb(controller 1)
pico_enable_stdio_uart(controller 0)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(controller)
