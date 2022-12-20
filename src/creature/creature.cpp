
#include <string>
#include <list>

#include "creature.h"

namespace Creatures {

    Creature::Creature(const std::string &name, std::list<Creatures::Joint> *joints) {
        this->name = name;
        this->joints = joints;
    }

    std::list<Creatures::Joint> *Creature::getJoints() {
        return this->joints;
    }

    std::string Creature::getName() {
        return this->name;
    }

}