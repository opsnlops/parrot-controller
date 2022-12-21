
#include "parrot.h"
#include "creature.h"

Parrot::Parrot(const char* name)
        : Creature(name) {

    info("Hello! My name is %s!", name);
}

void Parrot::start() {

    // Make sure we have a controller
    assert(this->myController != nullptr);


    debug("started!");
}