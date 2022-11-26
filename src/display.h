
#pragma once

#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include "hardware/i2c.h"

#include "pico-ssd1306/ssd1306.h"
#include "pico-ssd1306/textRenderer/TextRenderer.h"

// Update every 33ms (roughly 30Hz)
#define DISPLAY_UPDATE_TIME_MS 33

#define DISPLAY_BUFFER_SIZE 256

#define DISPLAY_I2C_BAUD_RATE 1000000
#define DISPLAY_I2C_CONTROLLER i2c0
#define DISPLAY_I2C_DEVICE_ADDRESS 0x3C

// Use the namespace for convenience
using namespace pico_ssd1306;

void set_up_display();

portTASK_FUNCTION_PROTO(displayUpdateTask, pvParameters);
