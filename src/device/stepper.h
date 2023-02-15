
#pragma once

#include "controller-config.h"

#include <cstdio>
#include "pico/stdlib.h"


class StepperState {

public:

    StepperState();


    uint32_t requestedSteps;
    uint32_t currentStep;

    bool currentDirection;

    bool isHigh;
    bool isAwake;

    // Set by the initStepper()
    bool ms1State;
    bool ms2State;

    float smoothingValue;

    /**
     * How many frames have we moved?
     *
     * This is just for metrics, it's not used for anything.
     */
    uint64_t stepsTaken;


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
     * At which frame can we resume motion after wakeup?
     */
     uint64_t awakeAt;


     /**
      * Last frame when this stepper was touched
      */
    uint64_t updatedFrame;

};




class Stepper {

/*
 * Truth Table for the A3967 Stepper (this is the EasyDriver one!)
 *
 *     +------------------------------+
 *     |  MS1  |  MS2  |  Resolution  |
 *     |-------|-------|--------------|
 *     |   L   |   L   | Full step    |
 *     |   H   |   L   | Half step    |
 *     |   L   |   H   | Quarter step |
 *     |   H   |   H   | Eighth step  |
 *     +------------------------------+
 *
 */

public:
    Stepper(uint8_t slot, const char* name, uint32_t fullSteps, float smoothingValue,
            uint32_t sleepWakeupPauseTimeUs, uint32_t sleepAfterUs, uint8_t microsteppingConfig, bool inverted);
    int init();
    int start();

    /**
     * Which slot it is on the mux
     */
    uint8_t slot;

    const char* name;
    bool inverted;

    StepperState* state;

    uint32_t fullSteps;
    float smoothingValue;

    /**
     * Should we use full steps, half steps, etc.
     *
     * Defined in `creature-config.h`
     */
    uint8_t microsteppingConfig;
    uint32_t stepsInUse;

    uint32_t sleepWakeupPauseTimeUs;
    uint32_t sleepAfterUs;

    [[nodiscard]] uint8_t getSlot() const;

    bool isInverted();

    [[nodiscard]] const char* getName() const;

    float getSmoothingValue();
    uint32_t getSleepWakeupPauseTimeUs();
    uint32_t getSleepAfterUs();

};
