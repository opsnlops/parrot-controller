
#include "controller-config.h"


#include "logging/logging.h"

#include "stepper.h"


int Stepper::init() {

    uart_init(STEPPER_UART, STEPPER_UART_BAUD_RATE);
    gpio_set_function(STEPPER_UART_TX, GPIO_FUNC_UART);
    gpio_set_function(STEPPER_UART_RX, GPIO_FUNC_UART);

    uint actual = uart_set_baudrate(STEPPER_UART, STEPPER_UART_BAUD_RATE);
    debug("Actual baud: %d", actual);

    // Set UART flow control CTS/RTS, we don't want these, so turn them off
    uart_set_hw_flow(STEPPER_UART, false, false);

    // Set our data format
    uart_set_format(STEPPER_UART, STEPPER_DATA_BITS, STEPPER_STOP_BITS, STEPPER_PARITY);

    // Turn off FIFO's - we want to do this character by character
    uart_set_fifo_enabled(STEPPER_UART, false);

    TMC_HAL::TMC_UART(STEPPER_UART);

    this->stepper = new TMC2209Stepper(STEPPER_UART, 0.11);

    return 1;

}