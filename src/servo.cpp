
#include "servo.h"

#include <cstdio>
#include "pico/stdlib.h"

#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include "hardware/gpio.h"
#include "hardware/pwm.h"

uint PIN_OUT = 22;
uint slice = 0;
uint channel = 0;

void set_up_servo() {
    double divider = 125000000 / (4096 * 50) / 16; // 50Hz

    gpio_set_function(PIN_OUT, GPIO_FUNC_PWM);
    slice = pwm_gpio_to_slice_num(PIN_OUT);
    channel = pwm_gpio_to_channel(PIN_OUT);

    pwm_set_clkdiv(slice, (float) divider);      // Set the divider to what we came up with
    pwm_set_wrap(slice, 65465);          // Set the wrap time to 20 ms (50Hz)
    pwm_set_enabled(slice, true);

    // One millisecond
    int ms = 65465 / 20;

    // Don't yell about this being endless loop
#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
    while (true) {

        // Sweep from 250us to 2500us
        for (int i = ms / 4; i < (ms * 2.5); i += 10) {

            pwm_set_chan_level(slice, channel, i);
            vTaskDelay(20);
        }

    }
#pragma clang diagnostic pop
}