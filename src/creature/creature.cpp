

#include <cmath>

#include "creature.h"

#include "logging/logging.h"


Creature::Creature() {

    this->controller = nullptr;
    this->workerTaskHandle = nullptr;
    this->numberOfJoints = 0;
    this->numberOfServos = 0;
    this->numberOfSteppers = 0;

    debug("Creature() called!");
}

TaskHandle_t Creature::getWorkerTaskHandle() {
    return workerTaskHandle;
}

void Creature::init(Controller *c) {
    this->controller = c;

    debug("init done, controller exists");
}

uint16_t Creature::convertInputValueToServoValue(uint8_t inputValue) {

    // TODO: Play with the results if we do bit shifts instead (8 -> 10)

    uint16_t servoRange = MAX_POSITION - MIN_POSITION;

    double movementPercentage = (double)inputValue / (double)UCHAR_MAX;
    auto servoValue = (uint16_t)(round((double)servoRange * movementPercentage) + MIN_POSITION);

    verbose("mapped %d -> %d", inputValue, servoValue);

    return servoValue;
}

int32_t Creature::convertRange(int32_t input, int32_t oldMin, int32_t oldMax, int32_t newMin, int32_t newMax) {

    if( input > oldMax ) {
        int32_t newInput = oldMax;
        warning("input (%d) is out of range %d to %d. capping at %d", input, oldMin, oldMax, newInput);
        input = newInput;
    }

    if( input < oldMin ) {
        int32_t newInput = oldMin;
        warning("input (%d) is out of range %d to %d. capping at %d", input, oldMin, oldMax, newInput);
        input = newInput;
    }

    int32_t oldRange = oldMax - oldMin;
    int32_t newRange = newMax - newMin;
    int32_t newValue = (((input - oldMin) * newRange) / oldRange) + newMin;

    verbose("mapped %d -> %d", input, newValue);
    return newValue;
}


uint8_t Creature::getNumberOfJoints() const {
    return numberOfJoints;
}

uint8_t Creature::getNumberOfServos() const {
    return numberOfServos;
}

uint8_t Creature::getNumberOfSteppers() const {
    return numberOfSteppers;
}