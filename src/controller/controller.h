
#pragma once


#include <list>

#include "controller-config.h"
#include "device/relay.h"
#include "device/servo.h"


class Controller {

private:
    uint8_t numberOfServos;
    uint8_t numberOfJoints;
    bool poweredOn;

    std::list<Servo> servos;
    Relay* powerRelay;


};