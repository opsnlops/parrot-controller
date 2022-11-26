
#include "controller.h"

#include <cstdio>

#include "hardware/i2c.h"
#include "pico/stdlib.h"

#include "display.h"
#include "logging/logging.h"

// Use the namespace for convenience
using namespace pico_ssd1306;

extern uint32_t chars_rxed;

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

    // Allocate one buffer_line_one for the display
    char buffer_line_one[DISPLAY_BUFFER_SIZE + 1];
    memset(buffer_line_one, '\0', DISPLAY_BUFFER_SIZE + 1);

    char buffer_line_two[DISPLAY_BUFFER_SIZE + 1];
    memset(buffer_line_two, '\0', DISPLAY_BUFFER_SIZE + 1);

    uint32_t time;

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
    for (EVER) {

        // Clear the display
        display.clear();

        time = to_ms_since_boot(get_absolute_time());

        // Null out the buffers
        memset(buffer_line_one, '\0', DISPLAY_BUFFER_SIZE + 1);
        memset(buffer_line_two, '\0', DISPLAY_BUFFER_SIZE + 1);

        sprintf(buffer_line_one, "Time: %lu", time);
        sprintf(buffer_line_two, "  Rx: %lu", chars_rxed);

        drawText(&display, font_8x8, buffer_line_one, 0, 0);
        drawText(&display, font_8x8, buffer_line_two, 0, 10);

        display.sendBuffer();

        vTaskDelay(pdMS_TO_TICKS(DISPLAY_UPDATE_TIME_MS));
    }
#pragma clang diagnostic pop
}
