
#pragma once

#include "creature.h"

#include "logging/logging.h"

#include "device/relay.h"
#include "device/servo.h"

// What our joints and servos are called
#define NECK_LEFT       0
#define NECK_RIGHT      1
#define NECK_ROTATE     2
#define BEAK            3
#define CHEST           4
#define BODY_LEAN       5
#define STAND_ROTATE    6

#define SERVO_HZ        50

class Parrot : public Creature {

public:

    explicit Parrot(const char* name);

    void init(Controller *controller) override;
    void start() override;

};

