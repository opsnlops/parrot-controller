
#include "controller-config.h"

#include "pico/time.h"

#include "logging/logging.h"

#include "stepper.h"


Stepper::Stepper(uint8_t slot, const char* name, uint32_t maxSteps, float smoothingValue, bool inverted) {

    verbose("setting up a new stepper");

    this->slot = slot;
    this->name = name;
    this->maxSteps = maxSteps;
    this->smoothingValue = smoothingValue;
    this->inverted = inverted;

    this->currentStep = 0;
    this->desiredSteps = 0;
    this->isHigh = false;

    info("set up servo on slot %d: name: %s, max_steps: %d, smoothing: %.4f, inverted: %s",
         slot, name, maxSteps, smoothingValue, inverted ? "yes" : "no");

}

/**
 * Since each stepper is unique, have the stepper itself figure out the
 * number of steps that map to a position.
 *
 * @param position between MIN_POSITION and MAX_POSITION
 * @return the number of steps that map to that position
 */
uint32_t Stepper::positionToSteps(uint16_t position) {



}

int Stepper::init() {
    return 1;

}

int Stepper::start() {
    return 1;
}

const char* Stepper::getName() {
    return this->name;
}

bool Stepper::getHighAndInvert() {
    isHigh = !isHigh;
    return !isHigh;
}

float Stepper::getSmoothingValue() {
    return this->smoothingValue;
}

uint32_t Stepper::getCurrentStep() {
    return this->currentStep;
}

uint32_t Stepper::getDesiredStep() {
    return this->desiredSteps;
}

void Stepper::setDesiredStep(uint32_t newDesiredStep) {
    this->desiredSteps = newDesiredStep;
}

uint8_t Stepper::getSlot() {
    return this->slot;
}