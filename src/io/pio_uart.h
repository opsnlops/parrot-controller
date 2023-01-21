
#pragma once


#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "uart_rx.pio.h"
#include "uart_tx.pio.h"

#include "logging/logging.h"
#include "tasks.h"


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