
#pragma once

#include "relay.h"
#include "servo.h"

class Parrot {

public:
    enum joint {
        neck_left,
        neck_right,
        neck_rotate,
        beak,
        chest,
        body_lean,
        stand_rotate
    };

    void setServoPower(bool on);


private:
    Relay* power_relay;

    uint16_t dmx_offset;

};