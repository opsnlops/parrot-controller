
#pragma once

#include <string>
#include <list>

#include "joint.h"

#define NUMBER_OF_SERVOS    6

namespace Creatures {

    class Creature {

    public:
        Creature(const std::string &name, std::list<Creatures::Joint> *joints);

        std::string getName();

        std::list<Creatures::Joint> *getJoints();

    private:

        std::string name;
        std::list<Creatures::Joint> *joints;
    };

}