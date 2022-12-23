
#include <climits>

#include "parrot.h"
#include "creature.h"
#include "tasks.h"

Parrot::Parrot(const char* name)
        : Creature(name) {

    info("Hello! My name is %s!", name);
}

void Parrot::init(Controller *controller) {
    debug("starting creature init");

    this->controller = controller;

    // Create all of our servos
    controller->initServo(NECK_LEFT, "Neck Left", 250, 2500, false);
    controller->initServo(NECK_RIGHT, "Neck Right", 250, 2500, true);
    controller->initServo(NECK_ROTATE, "Neck Rotate", 250, 2500, true);
    controller->initServo(BEAK, "Beak", 250, 2500, false);
    controller->initServo(CHEST, "Chest", 250, 2500, false);
    controller->initServo(BODY_LEAN, "Body Lean", 250, 2500, false);
    controller->initServo(STAND_ROTATE, "Stand Rotate", 250, 2500, false);
}


void Parrot::start() {

    // Make sure we have a controller
    assert(this->controller != nullptr);

    // Declare this on the heap so it lasts once start() goes out of scope
    auto info = (ParrotInfo*)pvPortMalloc(sizeof(ParrotInfo));
    info->controller = this->controller;
    info->parrot= this;

    // Fire off our worker task
    xTaskCreate(creature_worker_task,
                "creature_worker_task",
                2048,
                (void*)&info,
                1,
                &workerTaskHandle);

    debug("parrot started!");
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

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"

    for (EVER) {

        // Wait for the controller to signal to us that a new frame has been
        // received off the wire
        xTaskNotifyWait(0x00, ULONG_MAX, &ulNotifiedValue, portMAX_DELAY);

        // Fetch the current frame
        currentFrame = controller->getCurrentFrame();

        // For now, let's just set the servo to the requested position with no processing
        for(int i = 0; i < NUMBER_OF_SERVOS; i++) {
            controller->requestServoPosition(i,
                                             parrot->convertDmxValueToServoValue(currentFrame[i]));
        }
    }
#pragma clang diagnostic pop
}