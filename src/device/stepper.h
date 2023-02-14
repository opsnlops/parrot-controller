
#pragma once

#include "controller-config.h"

#include <cstdio>
#include "pico/stdlib.h"


class StepperState {

public:

    StepperState();

    /**
     * Used to determine how aggressive when we should switch to microsteps.
     *
     * Zero means disable.
     */
    uint16_t decelerationAggressiveness;

    uint64_t updatedFrame;

    // The controller requests things in whole steps
    uint32_t requestedSteps;

    uint32_t currentMicrostep;
    uint32_t desiredMicrostep;

    bool currentDirection;

    bool moveRequested;

    bool isHigh;
    bool isAwake;

    bool ms1State;
    bool ms2State;

    /**
     * How many frames have we moved?
     *
     * This is just for metrics, it's not used for anything.
     */
    uint64_t actualSteps;


    /**
     * Which frame did we fall asleep at?
     */
    uint64_t startedSleepingAt;


    /**
     * How many frames do we have to wait after sending wakeup to start
     * moving again?
     *
     * This allows time for the stepper driver to re-power after sleep.
     */
    uint64_t wakeupTime;

    /**
     * How many frames of idle time do we have to wait before going to sleep?
     */
    uint64_t sleepAfter;

    /**
     * At which frame can we resume motion after wakeup>
     */
     uint64_t awakeAt;


};

class Stepper {

public:
    Stepper(uint8_t slot, const char* name, uint32_t maxSteps, uint16_t decelerationAggressiveness,
            uint32_t sleepWakeupPauseTimeUs, uint32_t sleepAfterUs, bool inverted);
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

    uint16_t decelerationAggressiveness;
    uint32_t sleepWakeupPauseTimeUs;
    uint32_t sleepAfterUs;

    [[nodiscard]] uint8_t getSlot() const;

    bool isInverted();

    [[nodiscard]] const char* getName() const;

    uint16_t getDecelerationAggressiveness();
    uint32_t getSleepWakeupPauseTimeUs();
    uint32_t getSleepAfterUs();

};
