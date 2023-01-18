
#pragma once


#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "uart_tx.pio.h"

#include "logging/logging.h"
#include "tasks.h"


class StepperUART {

public:
    int init();
    int start();

    PIO pio;
    uint sm;
    uint offset;


};