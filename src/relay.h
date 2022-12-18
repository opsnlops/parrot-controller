
#pragma once

#include <cstdio>
#include "pico/stdlib.h"


/**
 * This is one relay in the system
 */
typedef struct {
    uint8_t gpio_pin;
    bool on;
} Relay;

/**
 * Creates a relay
 * @param gpio_pin The GPIO pin the relay is on
 * @param on Should it be on by default?
 * @return
 */
Relay* init_relay(uint8_t gpio_pin, bool on);

int relay_on(Relay *r);
int relay_off(Relay *r);
int relay_toggle(Relay *r);
