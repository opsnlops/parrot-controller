
#pragma once

#include "controller-config.h"
#include "device/relay.h"
#include "device/servo.h"


class Controller {

private:
    uint8_t numberOfServos;
    uint8_t numberOfJoints;
    bool poweredOn;

    Servo* servos[MAX_NUMBER_OF_SERVOS];
    Relay* powerRelay;


};