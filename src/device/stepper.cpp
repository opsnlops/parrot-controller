
#include "controller-config.h"

#include "pico/time.h"

#include "logging/logging.h"

#include "stepper.h"


StepperState::StepperState() {

    updatedFrame = 0L;

    requestedSteps = 0;

    currentStep = 0;

    currentDirection = false;
    smoothingValue = 0.0f;

    isHigh = false;
    isAwake = true;

    // Default to full steps
    ms1State = false;
    ms2State = false;

    stepsTaken = 0L;

    startedSleepingAt = 0L;

    wakeupTime = 0L;
    sleepAfter = 0L;
    awakeAt = 0L;

}


Stepper::Stepper(uint8_t slot, const char* name, uint32_t fullSteps, float smoothingValue,
         uint32_t sleepWakeupPauseTimeUs, uint32_t sleepAfterUs, uint8_t  microsteppingConfig, bool inverted) {

    verbose("setting up a new stepper");

    this->state = new StepperState();
    this->slot = slot;
    this->name = name;
    this->fullSteps = fullSteps;
    this->stepsInUse = fullSteps * microsteppingConfig;
    this->smoothingValue = smoothingValue;
    this->sleepWakeupPauseTimeUs = sleepWakeupPauseTimeUs;
    this->sleepAfterUs = sleepAfterUs;
    this->microsteppingConfig = microsteppingConfig;
    this->inverted = inverted;

    // TODO: Calculate the sleep stuff


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
    */

   // TODO: Should this be an enum?
   switch(microsteppingConfig) {

       case STEPPER_USE_EIGHTH_STEPS:
           state->ms1State = true;
           state->ms2State = true;
           break;

       case STEPPER_USE_QUARTER_STEPS:
           state->ms1State = false;
           state->ms2State = true;
           break;

       case STEPPER_USE_HALF_STEPS:
           state->ms1State = true;
           state->ms2State = false;
           break;

       default:
           state->ms1State = false;
           state->ms2State = false;
           break;

   }

    info("set up stepper on slot %u: name: %s, fullSteps: %u, stepsInUse: %u, smoothing: %.4f, inverted: %s",
         slot, name, fullSteps, this->stepsInUse, smoothingValue, inverted ? "yes" : "no");

}

int Stepper::init() {
    return 1;

}

int Stepper::start() {
    return 1;
}


const char* Stepper::getName() const {
    return this->name;
}

uint8_t Stepper::getSlot() const {
    return this->slot;
}

bool Stepper::isInverted() {
    return this->inverted;
}

float Stepper::getSmoothingValue() {
    return this->smoothingValue;
}

uint32_t Stepper::getSleepWakeupPauseTimeUs() {
    return this->sleepWakeupPauseTimeUs;
}

uint32_t Stepper::getSleepAfterUs() {
    return this->sleepAfterUs;
}
