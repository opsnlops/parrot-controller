

#pragma once

#include <cstdio>
#include "pico/stdlib.h"


#include <DmxInput.h>






// DMX Configuration
#define DMX_GPIO_PIN 18
#define DMX_START_CHANNEL 0
#define DMX_NUM_CHANNELS 9



int dmx_init(int dmx_input_pin);
