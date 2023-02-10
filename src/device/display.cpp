
#include "controller-config.h"

#include <cstdio>

#include <FreeRTOS.h>
#include <task.h>

#include "hardware/i2c.h"
#include "pico/stdlib.h"

#include "pico-ssd1306/ssd1306.h"
#include "pico-ssd1306/textRenderer/TextRenderer.h"

#include "tasks.h"
#include "controller/controller.h"
#include "display.h"
#include "logging/logging.h"


// Use the namespace for convenience
using namespace pico_ssd1306;

extern uint32_t number_of_moves;


// Located in tasks.cpp
extern TaskHandle_t displayUpdateTaskHandle;

Display::Display(Controller *controller, IOHandler *io) {

    debug("starting up the display");

    this->controller = controller;
    this->io = io;
    this->oled = nullptr;

    debug("setting up the display's i2c");

    // Display
    i2c_init(DISPLAY_I2C_CONTROLLER, DISPLAY_I2C_BAUD_RATE);

    // Set up pins 12 and 13
    gpio_set_function(DISPLAY_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(DISPLAY_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(DISPLAY_SDA_PIN);
    gpio_pull_up(DISPLAY_SCL_PIN);

    debug("leaving Display()");

}

Controller* Display::getController() {
    return controller;
}

IOHandler* Display::getIOHandler() {
    return io;
}

SSD1306* Display::getOLED() {
    return oled;
}

void Display::init() {
    // NOOP
}

void Display::start()
{
    debug("starting display");
    xTaskCreate(displayUpdateTask,
                "displayUpdateTask",
                1024,
                (void*)this,         // Pass in a reference to ourselves
                1,                      // Low priority
                &displayUpdateTaskHandle);
}


void Display::createOLEDDisplay() {
    oled = new SSD1306(DISPLAY_I2C_CONTROLLER, DISPLAY_I2C_DEVICE_ADDRESS, Size::W128xH32);
}

// Read from the queue and print it to the screen for now
portTASK_FUNCTION(displayUpdateTask, pvParameters) {

    auto display = (Display*)pvParameters;

    auto controller = display->getController();
    auto io = display->getIOHandler();

    /**
     * So this is a bit weird. The display needs some time to settle after the I2C bus
     * is set up. If the main task (before the scheduler is started) is delayed, FreeRTOS
     * throws an assert and halts.
     *
     * Since it does that, let's start it here, once we're in a task. It's safe to bake in
     * a delay at this point.
     */
    vTaskDelay(pdMS_TO_TICKS(250));
    display->createOLEDDisplay();
    auto oled = display->getOLED();

    oled->setOrientation(false);  // False means horizontally

    // Allocate one buffer_line_one for the display
    char buffer[DISPLAY_NUMBER_OF_LINES][DISPLAY_BUFFER_SIZE + 1];

    for(auto & i : buffer)
        memset(i, '\0', DISPLAY_BUFFER_SIZE + 1);

    TickType_t lastDrawTime;

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
    for (EVER) {

        // Make note of now
        lastDrawTime = xTaskGetTickCount();

        // Clear the display
        oled->clear();

        // Null out the buffers
        for(auto & i : buffer)
            memset(i, '\0', DISPLAY_BUFFER_SIZE + 1);

        // Go grab the current frame
        uint8_t* currentFrame = controller->getCurrentFrame();

        sprintf(buffer[0], "Wraps: %-5lu  P: %-3d %d",
                controller->getNumberOfPWMWraps(),
                controller->getServoPosition(0),
                controller->getServoPosition(1));
        sprintf(buffer[1], "Moves: %-5lu  Pwr: %s", number_of_moves, controller->isPoweredOn() ? "On" : "Off");
        sprintf(buffer[2], "Frame: %-5lu  Mem: %d", io->getNumberOfFramesReceived(), xPortGetFreeHeapSize());
        sprintf(buffer[3], "%3d %3d %3d %3d %3d %3d", currentFrame[0], currentFrame[1], currentFrame[2],
                currentFrame[3], currentFrame[4], currentFrame[5]);


        drawText(oled, font_5x8, buffer[0], 0, 0);
        drawText(oled, font_5x8, buffer[1], 0, 7);
        drawText(oled, font_5x8, buffer[2], 0, 14);
        drawText(oled, font_5x8, buffer[3], 0, 21);

        oled->sendBuffer();

        vTaskDelayUntil(&lastDrawTime, pdMS_TO_TICKS(DISPLAY_UPDATE_TIME_MS));
    }
#pragma clang diagnostic pop
}
