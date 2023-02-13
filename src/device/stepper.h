
#pragma once

#include "controller-config.h"

#include <cstdio>
#include "pico/stdlib.h"


class StepperState {

public:

    StepperState();

    uint64_t updatedFrame;

    uint32_t currentMicrostep;
    uint32_t desiredMicrostep;

    bool currentDirection;

    bool isHigh;
    bool isAwake;

    bool ms1State;
    bool ms2State;

    uint64_t startedSleepingAt;

};

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

    StepperState* state;

    uint32_t maxSteps;
    uint32_t maxMicrosteps;
    float smoothingValue;       // The constant to use when smoothing the input

    [[nodiscard]] uint8_t getSlot() const;

    [[nodiscard]] float getSmoothingValue() const;

    [[nodiscard]] const char* getName() const;
};
