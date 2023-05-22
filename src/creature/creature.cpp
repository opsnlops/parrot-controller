

#include <cmath>

#include "creature.h"

#include "logging/logging.h"


Creature::Creature() {

    this->controller = nullptr;
    this->workerTaskHandle = nullptr;
    this->numberOfJoints = 0;
    this->numberOfServos = 0;

#if USE_STEPPERS
    this->numberOfSteppers = 0;
#endif

    debug("Creature() called!");
}

TaskHandle_t Creature::getWorkerTaskHandle() {
    return workerTaskHandle;
}

void Creature::init(std::shared_ptr<Controller> c) {
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

CreatureConfig* Creature::getRunningConfig() {
    return runningConfig;
}

uint8_t Creature::getNumberOfJoints() const {
    return numberOfJoints;
}

uint8_t Creature::getNumberOfServos() const {
    return numberOfServos;
}

#if USE_STEPPERS
uint8_t Creature::getNumberOfSteppers() const {
    return numberOfSteppers;
}
#endif

