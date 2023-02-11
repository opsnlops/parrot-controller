
#pragma once

#include "controller-config.h"

#include <cstdio>
#include "pico/stdlib.h"


class Stepper {

public:
    Stepper(uint8_t slot, const char* name, uint32_t maxSteps, float smoothingValue, bool inverted);
    int init();
    int start();

    /**
     * Which slot it is on the mux
     */
    uint8_t slot;

    const char* name;
    bool inverted;

    uint32_t currentStep;
    uint32_t desiredSteps;
    uint32_t maxSteps;
    float smoothingValue;       // The constant to use when smoothing the input

    bool isHigh;

    uint8_t getSlot();
    uint32_t getDesiredStep() ;   // Where we want it to go
    uint32_t getCurrentStep();   // Where the servo currently is
    void setDesiredStep(uint32_t desiredStep);

    uint32_t positionToSteps(uint16_t position);

    float getSmoothingValue();

    const char* getName();
    bool getHighAndInvert();
};

