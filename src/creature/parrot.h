
#pragma once

#include "creature.h"

#include "logging/logging.h"

#include "creature/config.h"
#include "device/relay.h"
#include "device/servo.h"

// What our joints and servos are called
#define NECK_LEFT       0
#define NECK_RIGHT      1
#define NECK_ROTATE     2
#define BEAK            3
#define CHEST           4
#define BODY_LEAN       5
#define STAND_ROTATE    6


#define HEAD_OFFSET_MAX  0.35       // The max percent of the total height that the head can be


class Parrot : public Creature {

public:

    explicit Parrot();

    CreatureConfig* getDefaultConfig() override;
    void init(Controller *controller) override;
    void start() override;

private:

    /**
     * Convert a given y coordinate to where the head should be
     * @param y the y coordinate
     * @return head height
     */
    uint16_t convertToHeadHeight(uint16_t y);


    /**
     * Convert the x axis into head tilt
     * @param x the x axis
     * @return head tilt
     */
    uint16_t configToHeadTilt(uint16_t x);

    uint16_t headOffsetMax;

};

/**
 * Used for passing information into our task
 */
typedef struct {
    Controller* controller;
    uint16_t* joints;
    Parrot* parrot;
} ParrotInfo;
