
#pragma once

#include <climits>
#include <unistd.h>

#include <tasks.h>

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

    /**
     * Returns a task to be notified when there is a new frame to process
     *
     * @return a `TaskHandle_t` pointing to the task
     */
    TaskHandle_t getWorkerTaskHandle();


protected:

    const char* name;
    Controller* controller;
    TaskHandle_t workerTaskHandle;

    /**
     * Converts a value that DMX speaks (0-255) to one the servo controller
     * uses (MIN_SERVO_POSITION to MAX_SERVO_POSITION).
     *
     * @param dmxValue a `uint8_t` to convert to the servo mappings
     * @return a value between MIN_SERVO_POSITION and MAX_SERVO_POSITION
     */
    uint16_t convertDmxValueToServoValue(uint8_t dmxValue);

};

