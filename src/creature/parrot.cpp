
#include <climits>
#include <cmath>

#include "creature/config.h"

#include "parrot.h"
#include "creature.h"
#include "tasks.h"

Parrot::Parrot()
        : Creature() {

    // Calculate the head offset max
    this->headOffsetMax = lround((double)(MAX_SERVO_POSITION - MIN_SERVO_POSITION) * (double)HEAD_OFFSET_MAX);
    debug("the head office max is %d", this->headOffsetMax);

    info("Bawk!");
}

CreatureConfig* Parrot::getDefaultConfig() {

    auto config = new CreatureConfig("Beaky", 50, 7,1);

    config->setServoConfig(NECK_LEFT,
                           new ServoConfig("Neck Left", 250, 2500, 0.92, false));
    config->setServoConfig(NECK_RIGHT,
                           new ServoConfig("Neck Right", 250, 2500, 0.92, true));
    config->setServoConfig(NECK_ROTATE,
                           new ServoConfig("Neck Rotate", 250, 2500, 0.95, false));
    config->setServoConfig(BEAK,
                           new ServoConfig("Beak", 250, 2500, 0.4, false));
    config->setServoConfig(CHEST,
                           new ServoConfig("Chest", 250, 2500, 0.99, false));
    config->setServoConfig(BODY_LEAN,
                           new ServoConfig("Body Lean", 250, 1400, 0.8, false));
    config->setServoConfig(STAND_ROTATE,
                           new ServoConfig("Stand Rotate", 250, 2500, 0.99, false));

    return config;

}


void Parrot::init(Controller *controller) {
    debug("starting creature init");

    this->controller = controller;
    this->numberOfJoints = 7;

    // Initialize the array on for the joints on the heap
    this->joints = (uint16_t*) pvPortMalloc(sizeof(uint16_t) * numberOfJoints);
}

void Parrot::start() {

    // Make sure we have a controller
    assert(this->controller != nullptr);

    // Declare this on the heap, so it lasts once start() goes out of scope
    auto info = (ParrotInfo*)pvPortMalloc(sizeof(ParrotInfo));
    info->controller = this->controller;
    info->parrot= this;

    // Fire off our worker task
    xTaskCreate(creature_worker_task,
                "creature_worker_task",
                2048,
                (void*)info,
                1,
                &workerTaskHandle);

    debug("parrot started!");
}

uint16_t Parrot::convertToHeadHeight(uint16_t y) {

    return Parrot::convertRange(y,
                                MIN_SERVO_POSITION,
                                MAX_SERVO_POSITION,
                                MIN_SERVO_POSITION + (this->headOffsetMax / 2),
                                MAX_SERVO_POSITION - (this->headOffsetMax / 2));

}

int32_t Parrot::configToHeadTilt(uint16_t x) {

    return Parrot::convertRange(x,
                               MIN_SERVO_POSITION,
                               MAX_SERVO_POSITION,
                               1 - (this->headOffsetMax / 2),
                               this->headOffsetMax / 2);
}


head_position_t Parrot::calculateHeadPosition(uint16_t height, int32_t offset) {

    uint16_t right = height + offset;
    uint16_t left = height - offset;

    head_position_t headPosition;
    headPosition.left = left;
    headPosition.right = right;

    verbose("calculated head position: height: %d, offset: %d -> %d, %d", height, offset, right, left);

    return headPosition;

}

/**
 * This task is the main worker task for the parrot itself!
 *
 * It's an event loop. It pauses and waits for a signal from the controller
 * that a new frame has been received, and then does whatever it needs to do
 * to make magic.
 *
 * @param pvParameters
 */
portTASK_FUNCTION(creature_worker_task, pvParameters) {

    auto info = (ParrotInfo*)pvParameters;
    Controller* controller = info->controller;
    Parrot* parrot = info->parrot;

    // And give this small amount of memory back! :)
    vPortFree(info);

    uint32_t ulNotifiedValue;
    uint8_t* currentFrame;
    uint8_t numberOfJoints = parrot->getNumberOfJoints();

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"

    for (EVER) {

        // Wait for the controller to signal to us that a new frame has been
        // received off the wire
        xTaskNotifyWait(0x00, ULONG_MAX, &ulNotifiedValue, portMAX_DELAY);

        // Fetch the current frame
        currentFrame = controller->getCurrentFrame();


        uint16_t headHeight = parrot->convertToHeadHeight(Parrot::convertInputValueToServoValue(currentFrame[INPUT_HEAD_HEIGHT]));
        int32_t headTilt = parrot->configToHeadTilt(Parrot::convertInputValueToServoValue(currentFrame[INPUT_HEAD_TILT]));

        verbose("head height: %d, head tilt: %d", headHeight, headTilt);

        head_position_t headPosition = parrot->calculateHeadPosition(headHeight, headTilt);

        parrot->joints[NECK_LEFT] = headPosition.left;
        parrot->joints[NECK_RIGHT] = headPosition.right;


        uint16_t bodyLean = Parrot::convertInputValueToServoValue(currentFrame[INPUT_BODY_LEAN]);
        parrot->joints[BODY_LEAN] = bodyLean;


        // For the others, copy in what's on the wire for now
        parrot->joints[NECK_ROTATE] = Parrot::convertInputValueToServoValue(currentFrame[INPUT_NECK_ROTATE]);
        parrot->joints[BEAK] = Parrot::convertInputValueToServoValue(currentFrame[INPUT_BEAK]);
        parrot->joints[CHEST] = Parrot::convertInputValueToServoValue(currentFrame[INPUT_CHEST]);
        parrot->joints[STAND_ROTATE] = Parrot::convertInputValueToServoValue(currentFrame[INPUT_STAND_ROTATE]);


        // Request these positions from the controller
        for(int i = 0; i < numberOfJoints; i++) {
            controller->requestServoPosition(i,
                                             parrot->joints[i]);
        }


    }
#pragma clang diagnostic pop
}