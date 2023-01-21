cmake_minimum_required(VERSION 3.13)

# Pull in the Pico SDK
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

# Pull in FreeRTOS
include($ENV{FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)

project(app C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the SDK
pico_sdk_init()

# Pull in the DMX library
include(Pico-DMX/interfaceLibForPicoSDK.cmake)

# Set up our includes
include_directories(
        src/
        )

file(GLOB_RECURSE TMCStepper_source TMCStepper/src/*.cpp)

# Our source files
file(GLOB sourceFiles
        src/*
        src/controller/*
        src/creature/*
        src/device/*
        src/io/*
        src/logging/*
        src/shell/*
        )

add_executable(controller
        ${sourceFiles}
        ${TMCStepper_source}
        FreeRTOSConfig.h
        )

target_include_directories(controller PRIVATE
        .
        ${CMAKE_CURRENT_LIST_DIR}
        TMCStepper/src/
        )

add_subdirectory(pico-ssd1306)
add_library(TMCStepper ${TMCStepper_source})
target_link_libraries(
        TMCStepper
        pico_stdlib
        hardware_spi
)

# PIO-based UART from the Pico examples
pico_generate_pio_header(controller
        ${CMAKE_CURRENT_LIST_DIR}/src/io/uart_rx.pio
        ${CMAKE_CURRENT_LIST_DIR}/src/io/uart_tx.pio
        )

# pull in common dependencies
target_link_libraries(controller
        FreeRTOS-Kernel
        FreeRTOS-Kernel-Heap4
        hardware_dma
        hardware_gpio
        hardware_pio
        hardware_pwm
        hardware_uart
        pico_stdlib
        pico_ssd1306
        pico_unique_id
        picodmx
        TMCStepper
        )

# Enable the shell over UART (which is the picoprobe!)
pico_enable_stdio_usb(controller 0)
pico_enable_stdio_uart(controller 1)

# Create all of the files
pico_add_extra_outputs(controller)
