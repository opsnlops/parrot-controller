
#pragma once

#include <string>


#include "creature.h"

#include "device/servo.h"

namespace Creatures {

    class Joint {

    public:
        Joint(const std::string &name, uint16_t position, Servo *servo);

        [[nodiscard]] uint16_t getPosition() const;

        void setPosition(uint16_t position);

        std::string getName();

        Servo *getServo();

    private:
        std::string name;

        uint16_t position;

        // A pointer to our servo, maybe?
        Servo *servo;

    };
}