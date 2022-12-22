
#include "parrot.h"
#include "creature.h"

Parrot::Parrot(const char* name)
        : Creature(name) {

    info("Hello! My name is %s!", name);
}

void Parrot::init(Controller *controller) {
    debug("starting creature init");

    this->myController = controller;

    // Create all of our servos
    myController->initServo(NECK_LEFT, "Neck Left", 250, 2500, false);
    myController->initServo(NECK_RIGHT, "Neck Right", 250, 2500, true);
    myController->initServo(NECK_ROTATE, "Neck Rotate", 250, 2500, true);
    myController->initServo(BEAK, "Beak", 250, 2500, false);
    myController->initServo(CHEST, "Chest", 250, 2500, false);
    myController->initServo(BODY_LEAN, "Body Lean", 250, 2500, false);
    myController->initServo(STAND_ROTATE, "Stand Rotate", 250, 2500, false);
}


void Parrot::start() {

    // Make sure we have a controller
    assert(this->myController != nullptr);


    debug("parrot started!");
}