
#pragma once


#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "uart_tx.pio.h"

#include "logging/logging.h"
#include "tasks.h"


class PioUART {

public:
    int init(PIO pio, uint8_t tx_pin, uint32_t tx_baud_rate);
    int start();

    PIO pio;
    uint sm;
    uint offset;

};