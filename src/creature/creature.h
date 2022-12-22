
#pragma once

#include <climits>
#include <unistd.h>

#include "controller/controller.h"

#define NUMBER_OF_SERVOS    7
#define NUMBER_OF_JOINTS    7

class Creature {

public:

    explicit Creature(const char* name);

    /**
     * Storage space for the joints!
     *
     * This is static and public since it's accessed constantly, and I want
     * to save the cost of a function call and a bunch of getters and
     * setters.
     */
    static uint16_t joints[NUMBER_OF_JOINTS];

    const char* getName();

    /**
     * Set up the controller
     */
    virtual void init(Controller* controller) = 0;

    /**
     * Start running!
     */
    virtual void start() = 0;

protected:

    const char* name;
    Controller* myController;

};

