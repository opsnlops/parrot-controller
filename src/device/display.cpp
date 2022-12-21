
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
#include "io/dmx.h"


// Use the namespace for convenience
using namespace pico_ssd1306;

extern uint32_t bytes_received;
extern uint32_t number_of_moves;
extern uint32_t dmx_packets_read;
extern volatile uint8_t dmx_buffer[DMXINPUT_BUFFER_SIZE(DMX_BASE_CHANNEL, DMX_NUMBER_OF_CHANNELS)];

// Located in tasks.cpp
extern TaskHandle_t displayUpdateTaskHandle;

// Static members
SSD1306 Display::display = SSD1306(DISPLAY_I2C_CONTROLLER, DISPLAY_I2C_DEVICE_ADDRESS, Size::W128xH32);;
Controller* Display::controller;
IOHandler* Display::io;

Display::Display(Controller *controller, IOHandler *io) {

    debug("starting up the display");

    Display::controller = controller;
    Display::io = io;
}



void Display::init() {

    debug("setting up the display's i2c");

    // Display
    i2c_init(DISPLAY_I2C_CONTROLLER, DISPLAY_I2C_BAUD_RATE);

    // Set up pins 12 and 13
    gpio_set_function(12, GPIO_FUNC_I2C);
    gpio_set_function(13, GPIO_FUNC_I2C);
    gpio_pull_up(12);
    gpio_pull_up(13);

    // Give the display a moment to get started, as per the docs
    vTaskDelay(pdMS_TO_TICKS(250));
}

void Display::start()
{
    xTaskCreate(displayUpdateTask,
                "displayUpdateTask",
                1024,
                nullptr,
                0,          // Low priority
                &displayUpdateTaskHandle);
}


// Read from the queue and print it to the screen for now
portTASK_FUNCTION(displayUpdateTask, pvParameters) {


    Display::display.setOrientation(false);  // False means horizontally

    // Allocate one buffer_line_one for the display
    char buffer[DISPLAY_NUMBER_OF_LINES][DISPLAY_BUFFER_SIZE + 1];

    for(int i = 0; i < DISPLAY_NUMBER_OF_LINES; i++)
        memset(buffer[i], '\0', DISPLAY_BUFFER_SIZE + 1);


    uint32_t time;

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
    for (EVER) {

        // Clear the display
        Display::display.clear();

        // Null out the buffers
        for(int i = 0; i < DISPLAY_NUMBER_OF_LINES; i++)
            memset(buffer[i], '\0', DISPLAY_BUFFER_SIZE + 1);

        sprintf(buffer[0], "Wraps: %-5lu  P: %-3d %d", Controller::numberOfPWMWraps, Controller::getServoPosition(0), Controller::getServoPosition(1));
        sprintf(buffer[1], "Moves: %-5lu  Pwr: %s", number_of_moves, Display::controller->isPoweredOn() ? "On" : "Off");
        sprintf(buffer[2], "  DMX: %-5lu  Mem: %d", Display::io->getNumberOfFramesReceived(), xPortGetFreeHeapSize());
        sprintf(buffer[3], "%3d %3d %3d %3d %3d %3d", (int)dmx_buffer[1], (int)dmx_buffer[2], (int)dmx_buffer[3],
                (int)dmx_buffer[4], (int)dmx_buffer[5],(int)dmx_buffer[6]);


        drawText(&Display::display, font_5x8, buffer[0], 0, 0);
        drawText(&Display::display, font_5x8, buffer[1], 0, 7);
        drawText(&Display::display, font_5x8, buffer[2], 0, 14);
        drawText(&Display::display, font_5x8, buffer[3], 0, 21);
        //drawText(&display, font_5x8, buffer[4], 0, 28);

        Display::display.sendBuffer();

        vTaskDelay(pdMS_TO_TICKS(DISPLAY_UPDATE_TIME_MS));
    }
#pragma clang diagnostic pop
}
