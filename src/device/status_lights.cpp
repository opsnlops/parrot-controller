
#include <cstdio>
#include <cstdlib>

#include "device/status_lights.h"
#include "ws2812.pio.h"

#include "tasks.h"
#include "logging/logging.h"
#include "util/fast_hsv2rgb.h"
#include "util/ranges.h"


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



/**
 * @brief Returns the requested step in a fade between two hues
 *
 * This was taken from my old seconds ring clock like what's in the family room :)
 *
 * @param oldColor Starting hue
 * @param newColor Finishing hue
 * @param totalSteps How many steps are we fading
 * @param currentStep Which one to get
 * @return uint16_t The hue requested
 */
uint16_t StatusLights::interpolateHue(uint16_t oldHue, uint16_t newHue, uint8_t totalSteps, uint8_t currentStep)
{
    // How much is each step?
    uint16_t differentialStep = (newHue - oldHue) / totalSteps;

    uint16_t stepHue = oldHue + (differentialStep * currentStep);
    verbose("old: %u, new: %u, differential: %u, current: %u", oldHue, newHue, differentialStep, stepHue);

    return stepHue;
}


uint16_t StatusLights::getNextColor(uint16_t oldColor) {

    // https://martin.ankerl.com/2009/12/09/how-to-create-random-colors-programmatically/
    uint32_t tempColor = oldColor + GOLDEN_RATIO_CONJUGATE;
    tempColor = tempColor %= HSV_HUE_MAX;

    verbose("new hue is: %u", tempColor);
    return tempColor;
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


    uint32_t dmxLightColor = 0;
    uint32_t runningLightColor = 0;


    uint16_t runningLightHue = rand() * USHRT_MAX;
    uint16_t oldHue = runningLightHue;
    uint16_t runningLightFadeStep = 0;


    uint32_t motorLightColor[MAX_NUMBER_OF_SERVOS + MAX_NUMBER_OF_STEPPERS] = {0};
    for(unsigned long & i : motorLightColor)
        i = 0;


    uint64_t lastServoFrame[MAX_NUMBER_OF_SERVOS + MAX_NUMBER_OF_STEPPERS] = {0};
    for (unsigned long long &i: lastServoFrame)
        i = 0;

    uint16_t currentLightState[MAX_NUMBER_OF_SERVOS + MAX_NUMBER_OF_STEPPERS] = {0};
    for (unsigned short &i: currentLightState)
        i = 0;


    uint8_t r, g, b;


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

            dmxLightColor = StatusLights::urgb_u32(0, 255, 0);

            if (!ioActive) {
                info("Now receiving data from the IO handler");
                ioActive = true;
            }
        } else {
            // We haven't heard from the IO handler, so set it red
            dmxLightColor = StatusLights::urgb_u32(255, 0, 0);

            if (ioActive) {
                warning("Not getting data from the IO handler!");
                ioActive = false;
            }
        }



        /*
         * The second light is an "is running" light, so let's just change color
         */
        fast_hsv2rgb_32bit(StatusLights::interpolateHue(oldHue, runningLightHue, STATUS_LIGHTS_RUNNING_FRAME_CHANGE,
                                                        runningLightFadeStep++), 255, STATUS_LIGHTS_RUNNING_BRIGHTNESS, &r, &g, &b);
        runningLightColor = StatusLights::urgb_u32(r, g, b);

        if(runningLightFadeStep > STATUS_LIGHTS_RUNNING_FRAME_CHANGE)
        {
            oldHue = runningLightHue;
            runningLightHue = StatusLights::getNextColor(runningLightHue);
            runningLightFadeStep = 0;
        }



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

                // Convert the position to a hue
                uint16_t hue = convertRange(currentPosition,
                                            MIN_POSITION,
                                            MAX_POSITION,
                                            HSV_HUE_MIN,
                                            HSV_HUE_MAX);

                // Dim slowly until we've hit the limit for when we'd turn off
                uint8_t brightness = convertRange(frame - lastServoFrame[currentServo],
                                                  0,
                                                  STATUS_LIGHTS_MOTOR_OFF_FRAMES,
                                                  0,
                                                  255);
                brightness = 255 - brightness;

                fast_hsv2rgb_32bit(hue, 255, brightness, &r, &g, &b);
                motorLightColor[currentServo] = StatusLights::urgb_u32(r, g, b);

            } else {

                // Just turn off
                motorLightColor[currentServo] = 0;
            }
        }

        // Now write out the colors of the lights in one big chunk
        statusLights->put_pixel(dmxLightColor);
        statusLights->put_pixel(runningLightColor);
        for(uint8_t i = 0; i < Controller::getNumberOfServosInUse(); i++) {
            statusLights->put_pixel(motorLightColor[i]);
        }


        vTaskDelayUntil(&lastDrawTime, pdMS_TO_TICKS(STATUS_LIGHTS_TIME_MS));
    }
#pragma clang diagnostic pop
}
