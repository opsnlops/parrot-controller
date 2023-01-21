
#pragma once


#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "uart_rx.pio.h"
#include "uart_tx.pio.h"

#include "logging/logging.h"
#include "tasks.h"

/*
 * This class provides a clean way to use a PIO-based UART
 *
 * The PIO code is taken from the RPI Pico examples. It's unchanged.
 *
 * This classes uses two state machines on one PIO. (SM 0 and SM 1.) In theory,
 * it could be extended to run two terminals, but there's no reason to do this.
 */
class PioUART {

public:
    int init(PIO pio, uint8_t rx_pin, uint8_t tx_pin, uint32_t baud_rate);
    int start();

    void putc(char c);
    void puts(const char *s) const;

    char getc();

    PIO pio;

    uint rx_state_machine = 0;
    uint rx_offset;

    uint tx_state_machine = 1;
    uint tx_offset;

};