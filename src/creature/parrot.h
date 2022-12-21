
#pragma once

#include "creature.h"

#include "logging/logging.h"

#include "device/relay.h"
#include "device/servo.h"


class Parrot : public Creature {

public:

    Parrot(const char* name);

    void start();

    enum joint {
        neck_left,
        neck_right,
        neck_rotate,
        beak,
        chest,
        body_lean,
        stand_rotate
    };

};

