
#include "controller.h"

#include <cstdio>

#include "hardware/i2c.h"
#include "pico/stdlib.h"

#include "display.h"
#include "logging/logging.h"
#include "servo.h"

// Use the namespace for convenience
using namespace pico_ssd1306;

extern uint32_t bytes_received;
extern uint32_t pwm_wraps;
extern uint32_t number_of_moves;
extern Servo servos[NUMBER_OF_SERVOS];

void set_up_display_i2c() {

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

// Read from the queue and print it to the screen for now
portTASK_FUNCTION(displayUpdateTask, pvParameters) {

    set_up_display_i2c();

    SSD1306 display = SSD1306(DISPLAY_I2C_CONTROLLER, DISPLAY_I2C_DEVICE_ADDRESS, Size::W128xH32);
    display.setOrientation(false);  // False means horizontally

    uint8_t number_lines = 4;

    // Allocate one buffer_line_one for the display
    char buffer[number_lines][DISPLAY_BUFFER_SIZE + 1];

    for(int i = 0; i < number_lines; i++)
        memset(buffer[i], '\0', DISPLAY_BUFFER_SIZE + 1);


    uint32_t time;

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
    for (EVER) {

        // Clear the display
        display.clear();

        // Null out the buffers
        for(int i = 0; i < number_lines; i++)
            memset(buffer[i], '\0', DISPLAY_BUFFER_SIZE + 1);

        sprintf(buffer[0], "Wraps: %lu", pwm_wraps);
        sprintf(buffer[1], "   Rx: %lu", bytes_received);
        sprintf(buffer[2], "  Pos: %d, %d", servos[0].current_position, servos[1].current_position);
        sprintf(buffer[3], "Moves: %lu", number_of_moves);
        //sprintf(buffer[4], "  Mem: %d (%d)", xPortGetFreeHeapSize(), xPortGetMinimumEverFreeHeapSize());


        drawText(&display, font_5x8, buffer[0], 0, 0);
        drawText(&display, font_5x8, buffer[1], 0, 7);
        drawText(&display, font_5x8, buffer[2], 0, 14);
        drawText(&display, font_5x8, buffer[3], 0, 21);
        //drawText(&display, font_5x8, buffer[4], 0, 28);

        display.sendBuffer();

        vTaskDelay(pdMS_TO_TICKS(DISPLAY_UPDATE_TIME_MS));
    }
#pragma clang diagnostic pop
}
