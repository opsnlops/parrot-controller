
#include <cstdio>
#include <cstdlib>

#include "device/status_lights.h"
#include "ws2812.pio.h"

#include "tasks.h"
#include "logging/logging.h"


/*
 *
 * Status Light Order:
 *
 * 0 = DMX Status
 * 1 = Running
 * 2 = Servo 0
 * 3..n = Each servo after that
 */



// Located in tasks.cpp
extern TaskHandle_t status_lights_task_handle;

StatusLights::StatusLights(Controller *controller, IOHandler *io) {

    this->controller = controller;
    this->io = io;

    this->last_input_frame = 0;

    debug("new status lights created");

}


inline void StatusLights::put_pixel(uint32_t pixel_grb) {
    pio_sm_put_blocking(STATUS_LIGHTS_PIO, 0, pixel_grb << 8u);
}

inline uint32_t StatusLights::urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return
            ((uint32_t) (r) << 8) |
            ((uint32_t) (g) << 16) |
            (uint32_t) (b);
}


Controller *StatusLights::getController() {
    return this->controller;
}

IOHandler *StatusLights::getIOHandler() {
    return this->io;
}


void StatusLights::init() {

    this->pio = STATUS_LIGHTS_PIO;
    this->state_machine = 0;
    uint offset = pio_add_program(pio, &ws2812_program);

    ws2812_program_init(pio, this->state_machine, offset, STATUS_LIGHTS_PIN, 800000, STATUS_LIGHTS_IS_RGBW);

}

void StatusLights::start() {

    debug("starting the status lights");
    xTaskCreate(status_lights_task,
                "status_lights_task",
                1024,
                (void *) this,         // Pass in a reference to ourselves
                1,                      // Low priority
                &status_lights_task_handle);
}





// Read from the queue and print it to the screen for now
portTASK_FUNCTION(status_lights_task, pvParameters) {

    auto statusLights = (StatusLights *) pvParameters;
    auto io = statusLights->getIOHandler();

    TickType_t lastDrawTime;

    // What frame are we on?
    uint64_t frame = 0;


    uint32_t currentIOFrameNumber = 0;
    uint32_t lastIOFrameNumber = 0;
    uint64_t lastIOFrame = 0;

    // Are we currently getting data?
    bool ioActive = false;


    uint32_t runningLightColor = rand();


    uint64_t lastServoFrame[MAX_NUMBER_OF_SERVOS + MAX_NUMBER_OF_STEPPERS] = {0};
    for (unsigned long long &i: lastServoFrame)
        i = 0;

    uint16_t currentLightState[MAX_NUMBER_OF_SERVOS + MAX_NUMBER_OF_STEPPERS] = {0};
    for (unsigned short &i: currentLightState)
        i = 0;


#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
    for (EVER) {

        // Make note of now
        lastDrawTime = xTaskGetTickCount();

        frame++;


        /*
         * The first light on the chain is a status light on if we're getting data from the
         * I/O handler
         */

        currentIOFrameNumber = io->getNumberOfFramesReceived();

        // If we've heard from the IO Handler recently-ish, set the light to green
        if (currentIOFrameNumber > lastIOFrameNumber || lastIOFrame + STATUS_LIGHTS_IO_RESPONSIVENESS > frame) {
            // If we entered here because the frame changed, update our info
            if (currentIOFrameNumber > lastIOFrameNumber) {
                lastIOFrameNumber = currentIOFrameNumber;
                lastIOFrame = frame;
            }

            statusLights->put_pixel(StatusLights::urgb_u32(0, 255, 0));

            if (!ioActive) {
                info("Now receiving data from the IO handler");
                ioActive = true;
            }
        } else {
            // We haven't heard from the IO handler, so set it red
            statusLights->put_pixel(StatusLights::urgb_u32(255, 0, 0));

            if (ioActive) {
                warning("Not getting data from the IO handler!");
                ioActive = false;
            }
        }



        /*
         * The second light is an "is running" light, so let's just change color
         */
        if (frame % 33 == 0) {
            runningLightColor = rand();
        }
        statusLights->put_pixel(runningLightColor);


        /*
         * The rest of the lights are the status of the motors
         */

        for (uint8_t currentServo = 0; currentServo < Controller::getNumberOfServosInUse(); currentServo++) {

            uint16_t currentPosition = Controller::getServo(currentServo)->getPosition();

            // Has this one changed?
            if (currentLightState[currentServo] != currentPosition) {

                currentLightState[currentServo] = currentPosition;
                lastServoFrame[currentServo] = frame;
            }

            // If we should be on, what color?
            if (lastServoFrame[currentServo] + STATUS_LIGHTS_MOTOR_OFF_FRAMES > frame) {

                // Current Position is 10 bit, let's convert it to 8
                uint16_t color = (currentPosition >> 2);
                statusLights->put_pixel(StatusLights::urgb_u32(color, 255 - color, 0));

            } else {

                // Just turn off
                statusLights->put_pixel(0);
            }
        }


        vTaskDelayUntil(&lastDrawTime, pdMS_TO_TICKS(STATUS_LIGHTS_TIME_MS));
    }
#pragma clang diagnostic pop
}
