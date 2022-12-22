

#include "creature.h"

#include "logging/logging.h"


Creature::Creature(const char* name) {

    this->name = name;
    this->myController = nullptr;

    debug("Creature() called! name: %s", this->name);
}

const char* Creature::getName() {
    return this->name;
}

void Creature::init(Controller *controller) {
    this->myController = controller;

    debug("init done, controller exists");
}