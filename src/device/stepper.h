
#pragma once

#include "controller-config.h"

#include <cstdio>
#include "pico/stdlib.h"


class Stepper {

public:
    Stepper();
    int init();
    int start();

    uint8_t stepsPin = STEPPER_STEP_PIN;
    uint8_t directionPin = STEPPER_DIR_PIN;

    uint32_t current_step;
    uint32_t desired_step;
    uint32_t max_steps;

    bool is_high;

    bool getHighAndInvert();
};

