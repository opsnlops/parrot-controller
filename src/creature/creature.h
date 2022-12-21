
#pragma once

#include <climits>
#include <unistd.h>

#include "controller/controller.h"

#define NUMBER_OF_SERVOS    6
#define NUMBER_OF_JOINTS    6

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
     * Establishes a link back to our controller
     *
     * @param controller a pointer to our controller
     */
    void setController(Controller* controller);

    /**
     * Start running!
     */
    virtual void start() = 0;

protected:

    const char* name;
    Controller* myController;

};

