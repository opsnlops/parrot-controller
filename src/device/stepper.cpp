
#include "controller-config.h"

#include "pico/time.h"

#include "logging/logging.h"

#include "stepper.h"


Stepper::Stepper() {
    debug("new stepper made");

    current_step = 0;
    max_steps = 40000;
    desired_step = 0;
    is_high = false;

}

int Stepper::init() {

    return 1;

}

bool Stepper::getHighAndInvert() {
    is_high = !is_high;
    return !is_high;
}