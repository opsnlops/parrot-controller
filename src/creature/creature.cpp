

#include <cmath>

#include "creature.h"

#include "logging/logging.h"


Creature::Creature(const char* name) {

    this->name = name;
    this->controller = nullptr;
    this->workerTaskHandle = nullptr;

    debug("Creature() called! name: %s", this->name);
}

TaskHandle_t Creature::getWorkerTaskHandle() {
    return workerTaskHandle;
}

const char* Creature::getName() {
    return this->name;
}

void Creature::init(Controller *c) {
    this->controller = c;

    debug("init done, controller exists");
}

uint16_t Creature::convertDmxValueToServoValue(uint8_t dmxValue) {

    uint16_t servoRange = MAX_SERVO_POSITION - MIN_SERVO_POSITION;

    double movementPercentage = (double)dmxValue / (double)UCHAR_MAX;
    auto servoValue = (uint16_t)(round((double)servoRange * movementPercentage) + MIN_SERVO_POSITION);

    verbose("mapped %d -> %d", dmxValue, servoValue);

    return servoValue;
}