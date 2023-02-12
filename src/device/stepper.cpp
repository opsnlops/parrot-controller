
#include "controller-config.h"

#include "pico/time.h"

#include "logging/logging.h"

#include "stepper.h"


StepperState::StepperState() {

    updatedFrame = 0L;

    currentStep = 0;
    desiredSteps = 0;
    currentDirection = false;

    isHigh = false;
    isAwake = true;

    // Default to half steps
    ms1State = true;
    ms2State = false;

    startedSleepingAt = 0L;

}


Stepper::Stepper(uint8_t slot, const char* name, uint32_t maxSteps, float smoothingValue, bool inverted) {

    verbose("setting up a new stepper");

    this->state = new StepperState();
    this->slot = slot;
    this->name = name;
    this->maxSteps = maxSteps;
    this->smoothingValue = smoothingValue;
    this->inverted = inverted;

    info("set up servo on slot %d: name: %s, max_steps: %d, smoothing: %.4f, inverted: %s",
         slot, name, maxSteps, smoothingValue, inverted ? "yes" : "no");

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

float Stepper::getSmoothingValue() const {
    return this->smoothingValue;
}

uint8_t Stepper::getSlot() const {
    return this->slot;
}
