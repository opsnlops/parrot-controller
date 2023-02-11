
#pragma once

#include <climits>
#include <unistd.h>

#include <tasks.h>

#include "creature/config.h"
#include "controller/controller.h"

class Creature {

public:

    explicit Creature();

    /**
     * Storage space for the joints!
     *
     * Initialize it to the size of the joints.
     */
    uint16_t* joints;

    /**
     * Set up the controller
     */
    virtual void init(Controller* controller) = 0;

    /**
     * Start running!
     */
    virtual void start() = 0;

    /**
     * Create the default config for this creature
     *
     * @return a non-customized configuration for this creature
     */
    virtual CreatureConfig* getDefaultConfig() = 0;

    /**
     * Returns a task to be notified when there is a new frame to process
     *
     * @return a `TaskHandle_t` pointing to the task
     */
    TaskHandle_t getWorkerTaskHandle();

    /**
     * Converts a value that input handlers speaks (0-255) to one the servo controller
     * uses (MIN_POSITION to MAX_POSITION).
     *
     * @param inputValue a `uint8_t` to convert to the servo mappings
     * @return a value between MIN_POSITION and MAX_POSITION
     */
    static uint16_t convertInputValueToServoValue(uint8_t inputValue);

    /**
     * Gets the number of joints that this creature has
     *
     * @return the number of joints
     */
    [[nodiscard]] uint8_t getNumberOfJoints() const;

    [[nodiscard]] uint8_t getNumberOfServos() const;
    [[nodiscard]] uint8_t getNumberOfSteppers() const;

protected:

    Controller* controller;
    TaskHandle_t workerTaskHandle;

    uint8_t numberOfServos;
    uint8_t numberOfSteppers;
    uint8_t numberOfJoints;

    // This is like the Arduino helper map() function. We do this a lot.
    static int32_t convertRange(int32_t input, int32_t oldMin, int32_t oldMax, int32_t newMin, int32_t newMax);

};

