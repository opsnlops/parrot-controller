
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
    bool currentDirection;

    uint32_t desiredSteps;
    uint32_t maxSteps;
    float smoothingValue;       // The constant to use when smoothing the input

    bool isHigh;

    [[nodiscard]] uint8_t getSlot() const;
    [[nodiscard]] bool getCurrentDirection() const;
    [[nodiscard]] uint32_t getDesiredStep() const ;   // Where we want it to go
    [[nodiscard]] uint32_t getCurrentStep() const;   // Where the servo currently is
    void setDesiredStep(uint32_t desiredStep);
    void setCurrentDirection(bool direction);

    [[nodiscard]] float getSmoothingValue() const;

    [[nodiscard]] const char* getName() const;
    bool getHighAndInvert();
};

