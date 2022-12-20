
#include <cassert>
#include <string>

#include "logging/logging.h"

#include "joint.h"


namespace Creatures {

    Joint::Joint(const std::string &name, uint16_t position, Servo *servo) {

        assert(!name.empty());

        this->name = name;
        this->position = position;
        this->servo = servo;

        info("Created joint %s", name.c_str());
    }

    uint16_t Joint::getPosition() const {
        return position;
    }

    void Joint::setPosition(uint16_t newPosition) {
        assert(newPosition <= MAX_SERVO_POSITION);

        this->position = newPosition;

        verbose("set position to %d", newPosition);
    }

    std::string Joint::getName() {
        return name;
    }

    Servo *Joint::getServo() {
        return servo;
    }
}