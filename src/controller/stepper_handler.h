
#pragma once

#include <cstdint>

#include "device/stepper.h"


bool stepper_timer_handler(struct repeating_timer *t);
uint32_t set_ms1_ms2_and_get_steps(StepperState* state);
