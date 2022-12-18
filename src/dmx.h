

#pragma once

#include <cstdio>
#include "pico/stdlib.h"


#include <DmxInput.h>


int dmx_init(int dmx_input_pin);
void __isr dmxDataGotten(DmxInput* instance);