
#pragma once

#include <cstdio>
#include "pico/stdlib.h"


/**
 * This is one relay in the system
 */
class Relay {

public:
    Relay(uint8_t gpio_pin, bool on);
    int turnOn();
    int turnOff();
    int toggle();

    bool isOn() const;

private:
    uint8_t gpio_pin;
    bool on;
};

