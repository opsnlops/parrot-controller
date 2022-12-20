

#include <list>
#include <string>


#include "parrot.h"
#include "creature.h"

namespace Creatures {

    Parrot::Parrot(const std::string &name, std::list<Creatures::Joint> *joints)
            : Creature(name, joints) {
        info("My name is %s", name.c_str());
    }

}