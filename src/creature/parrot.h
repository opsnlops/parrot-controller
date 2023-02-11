
#pragma once

#include "creature.h"

#include "logging/logging.h"

#include "creature/config.h"
#include "device/relay.h"
#include "device/servo.h"

// Servo mappings
#define NECK_LEFT       0
#define NECK_RIGHT      1
#define BEAK            2
#define CHEST           3

// Stepper mappings
#define NECK_ROTATE     0
#define BODY_LEAN       1
#define STAND_ROTATE    2


// Input mapping. Defines which axis is each
#define INPUT_HEAD_HEIGHT   3
#define INPUT_HEAD_TILT     2
#define INPUT_BODY_LEAN     4
#define INPUT_BEAK          5
#define INPUT_NECK_ROTATE   0
#define INPUT_CHEST         1
#define INPUT_STAND_ROTATE  6



#define HEAD_OFFSET_MAX  0.4       // The max percent of the total height that the head can be


typedef struct {
    uint16_t left;
    uint16_t right;
} head_position_t;

class Parrot : public Creature {

public:

    explicit Parrot();

    CreatureConfig* getDefaultConfig() override;
    void init(Controller *controller) override;
    void start() override;

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
    int32_t configToHeadTilt(uint16_t x);

    head_position_t calculateHeadPosition(uint16_t height, int32_t offset);

private:

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
