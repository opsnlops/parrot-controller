
#include "controller-config.h"

#include "pico/time.h"

#include "logging/logging.h"

#include "stepper.h"


Stepper::Stepper() {
    debug("new stepper made");

    current_step = 0;
    max_steps = 40000;
    desired_step = 0;

}

int Stepper::init() {

    return 1;

}