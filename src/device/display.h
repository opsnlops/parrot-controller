
#pragma once

#include "hardware/i2c.h"

#include "pico-ssd1306/ssd1306.h"
#include "pico-ssd1306/textRenderer/TextRenderer.h"

#define DISPLAY_I2C_BAUD_RATE 1000000
#define DISPLAY_I2C_CONTROLLER i2c0
#define DISPLAY_I2C_DEVICE_ADDRESS 0x3C

class Display {

public:
    static void set_up_display_i2c();

};


