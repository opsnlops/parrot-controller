

#include <cmath>

#include "creature.h"

#include "logging/logging.h"


Creature::Creature() {

    this->controller = nullptr;
    this->workerTaskHandle = nullptr;
    this->numberOfJoints = 0;

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

    uint16_t servoRange = MAX_SERVO_POSITION - MIN_SERVO_POSITION;

    double movementPercentage = (double)inputValue / (double)UCHAR_MAX;
    auto servoValue = (uint16_t)(round((double)servoRange * movementPercentage) + MIN_SERVO_POSITION);

    verbose("mapped %d -> %d", inputValue, servoValue);

    return servoValue;
}

uint16_t Creature::convertRange(uint16_t input, uint16_t oldMin, uint16_t oldMax, uint16_t newMin, uint16_t newMax) {

    if( input > oldMax ) {
        uint16_t newInput = oldMax;
        warning("input (%d) is out of range %d to %d. capping at %d", input, oldMin, oldMax, newInput);
        input = newInput;
    }

    if( input < oldMin ) {
        uint16_t newInput = oldMin;
        warning("input (%d) is out of range %d to %d. capping at %d", input, oldMin, oldMax, newInput);
        input = newInput;
    }

    uint16_t oldRange = oldMax - oldMin;
    uint16_t newRange = newMax - newMin;
    uint16_t newValue = (((input - oldMin) * newRange) / oldRange) + newMin;;

    verbose("mapped %d -> %d", input, newValue);
    return newValue;
}


uint8_t Creature::getNumberOfJoints() const {
    return numberOfJoints;
}