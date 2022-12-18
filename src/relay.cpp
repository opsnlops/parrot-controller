
#include <cstdlib>

#include "pico/stdlib.h"

#include "relay.h"

#include "logging/logging.h"


Relay* init_relay(uint8_t gpio_pin, bool on) {

    debug("creating relay on gpio %d", gpio_pin);


    auto r = (Relay*)malloc(sizeof(Relay));

    r->gpio_pin = gpio_pin;
    r->on = on;

    gpio_init(gpio_pin);
    gpio_set_dir(gpio_pin, true);   // This is an output pin
    gpio_set_outover(gpio_pin, GPIO_OVERRIDE_INVERT); // The relays are active low
    gpio_put(gpio_pin, on);

    return r;
}

int relay_on(Relay *r) {

    debug("setting relay on GPIO %d to on", r->gpio_pin);

    gpio_put(r->gpio_pin, true);
    r->on = true;

    return 0;
}

int relay_off(Relay *r) {

    debug("setting relay on GPIO %d to off", r->gpio_pin);

    gpio_put(r->gpio_pin, false);
    r->on = false;

    return 0;
}


int relay_toggle(Relay *r) {

    debug("toggling relay on GPIO %d", r->gpio_pin);

    if(r->on) {
        relay_off(r);
    } else {
        relay_on(r);
    }

    return 0;
}
