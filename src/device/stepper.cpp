
#include "controller-config.h"

#include "pico/time.h"

#include "logging/logging.h"

#include "stepper.h"


StepperState::StepperState() {

    updatedFrame = 0L;

    requestedSteps = 0;

    currentMicrostep = 0;
    desiredMicrostep = 0;
    currentDirection = false;

    isHigh = false;
    isAwake = true;

    moveRequested = false;

    // Default to full steps
    ms1State = false;
    ms2State = false;

    actualSteps = 0L;

    startedSleepingAt = 0L;

}


Stepper::Stepper(uint8_t slot, const char* name, uint32_t maxSteps, uint16_t decelerationAggressiveness,
         uint32_t sleepWakeupPauseTimeUs, uint32_t sleepAfterUs, bool inverted) {

    verbose("setting up a new stepper");

    this->state = new StepperState();
    this->slot = slot;
    this->name = name;
    this->maxSteps = maxSteps;
    this->maxMicrosteps = maxSteps * STEPPER_MICROSTEP_MAX;
    this->decelerationAggressiveness = decelerationAggressiveness;
    this->sleepWakeupPauseTimeUs = sleepWakeupPauseTimeUs;
    this->sleepAfterUs = sleepAfterUs;
    this->inverted = inverted;

    this->state->decelerationAggressiveness = decelerationAggressiveness;
    // TODO: Calculate the sleep stuff

    info("set up stepper on slot %u: name: %s, max_steps: %u, deceleration: %u, inverted: %s",
         slot, name, maxSteps, decelerationAggressiveness, inverted ? "yes" : "no");

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

uint16_t Stepper::getDecelerationAggressiveness() {
    return this->decelerationAggressiveness;
}

uint32_t Stepper::getSleepWakeupPauseTimeUs() {
    return this->sleepWakeupPauseTimeUs;
}

uint32_t Stepper::getSleepAfterUs() {
    return this->sleepAfterUs;
}
