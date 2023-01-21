
#pragma once

#include <cstdio>
#include "pico/stdlib.h"

#include "TMCStepper.h"
#define STEPPER_DATA_BITS 8
#define STEPPER_STOP_BITS 1
#define STEPPER_PARITY    UART_PARITY_NONE
class Stepper {

public:
    int init();
    int start();


    TMC2209Stepper* stepper;


};

