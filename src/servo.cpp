
#include <cassert>

// Global
#include "controller.h"

// This module
#include "servo.h"

// Our modules
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "logging/logging.h"


/**
 * @brief Initializes a servo and gets it ready for use
 *
 * The Servo that will be set up is the one that's passed in. Calling this function
 * will set up the PWM hardware to match the frequency that's provided. (Typically
 * 50Hz on a standard servo.)
 *
 * We work off pulse widths to servos, not percentages. The min and max pulse need
 * to be defined in microseconds. This will be mapped to the MIN_SERVO_POSITION and
 * MAX_SERVO_POSITION as defined in controller.h. (0-999 is typical.) The size of the
 * pulses need to be set to the particular servo and what it does inside of the
 * creature itself. (ie, don't bend a joint further than it should go!)
 *
 * The servo's PWM controller will be off by default to make sure it's not turned
 * on before we're ready to go.
 *
 * @param s Servo to build
 * @param gpio The GPIO pin this servo is connected to
 * @param frequency PWM frequency
 * @param min_pulse_us Min pulse length in microseconds
 * @param max_pulse_us Max pulse length in microseconds
 * @param inverted are this servo's movements inverted?
 */
void servo_init(Servo *s, uint gpio, uint32_t frequency, uint16_t min_pulse_us, uint16_t max_pulse_us, bool inverted) {

    gpio_set_function(gpio, GPIO_FUNC_PWM);
    s->gpio = gpio;
    s->frequency = frequency;
    s->min_pulse_us = min_pulse_us;
    s->max_pulse_us = max_pulse_us;
    s->frame_length_us = 1000000 / frequency;   // Number of microseconds in each frame (frequency)
    s->slice = pwm_gpio_to_slice_num(gpio);
    s->channel = pwm_gpio_to_channel(gpio);
    s->resolution = pwm_set_freq_duty(s->slice, s->channel, s->frequency, 0);
    s->inverted = inverted;
    s->current_position = MIN_SERVO_POSITION;

    // Turn the servo off by default
    pwm_set_enabled(s->slice, false);
    s->on = false;

    // TODO: What's a good default to set the servo to on power on?

    info("set up servo on pin %d", gpio);
}

/**
 * @brief Turns on the PWM pulse for a servo
 *
 * Note that this turns on the PWM pulse _for the entire slice_ (both A and B
 * outputs).
 *
 * @param s The servo to enable
 */
void servo_on(Servo* s) {
    pwm_set_enabled(s->slice, true);
    s->on = true;

    info("Enabled servo on pin %d (slice %d)", s->gpio, s->slice);
}


/**
 * @brief Turns off the PWM pulse for a servo
 *
 * Note that this turns off the PWM pulse _for the entire slice_ (both A and B
 * outputs).
 *
 * @param s The servo to disable
 */
void servo_off(Servo* s) {
    pwm_set_enabled(s->slice, false);
    s->on = false;

    info("Disabled servo on pin %d (slice %d)", s->gpio, s->slice);
}

/**
 * @brief Requests that a servo be moved to a given position
 *
 * The value must be between `MIN_SERVO_POSITION` and `MAX_SERVO_POSITION`. An
 * assert will be fired in not to prevent damage to the creature or a motor.
 *
 * `servo_move()` does not actually move the servo. Instead, it marks it's requested
 * position in `desired_ticks` and waits for the IRQ handler to fire off to actually
 * move to that position. Working this way decouples changing the duty cycle from
 * the request to the PWM circuit, which is useful when moving servos following
 * some sort of external control. (DMX in our case!)
 *
 * The Pi Pico only adjusts the duty cycle when the counter rolls over. Doing it
 * more often wastes CPU cycles. (Calling `pwm_set_chan_level()` more than once a
 * frame is a waste of effort.)
 *
 * The IRQ handler follows the frequency of the first servo in the array declared in
 * main(). I would like to be able to support a bunch of different frequencies at once,
 * but as of right now, there's really not a need, and I'd rather not make the ISR
 * any more complex than it needs to be.
 *
 * @param s The servo to move
 * @param position The requested position
 */
void servo_move(Servo* s, uint16_t position) {

    //
    // Remember: Float point can be slow on the Pico! 🐢
    //

    // Error checking. This could result in damage to a motor or
    // creature if not met, so this is a hard stop if it's wrong. 😱
    assert(position >= MIN_SERVO_POSITION && position <= MAX_SERVO_POSITION);

    // TODO: This assumes that MIN_SERVO_POSITION is always 0. Is that okay?

    // If this servo is inverted, do it now
    if(s->inverted) position = MAX_SERVO_POSITION - position;

    // What percentage of our travel is expected?
    float travel_percentage = (float)position / MAX_SERVO_POSITION;
    float desired_pulse_length_us = (float)(((s->max_pulse_us - s->min_pulse_us)) * travel_percentage)
            + (float)s->min_pulse_us;

    // Now that we know how many microseconds we're expected to have, map that to
    // a frame and a value that can be passed to the PWM controller
    float frame_active = desired_pulse_length_us / (float)(s->frame_length_us);
    s->desired_ticks = (float)s->resolution * frame_active;
    s->current_position = position;

    verbose("requesting servo %d be set to position %d (%d ticks)",
          s->gpio,
          s->current_position,
          s->desired_ticks);
}

/**
 * @brief Sets the frequency on a PWM channel. Returns the resolution of the slice.
 *
 * The Pi Pico has a 125Mhz clock on the PWM pins. This is far too fast for a servo
 * to work with (most run at 50Hz). This function figures out the correct dividers to
 * use to drop it down to the given frequency.
 *
 * It will return the wrap value for the counter, which can be thought of as the resolution
 * of the channel. (ie, wrap / 2 = 50% duty cycle)
 *
 *
 * This bit of code was taken from the book:
 *    Fairhead, Harry. Programming The Raspberry Pi Pico In C (p. 122). I/O Press. Kindle Edition.
 *
 * @param slice_num The PWM slice number
 * @param chan The PWM channel number
 * @param frequency The frequency of updates (50Hz is normal)
 * @param d speed (currently unused)
 * @return the wrap counter wrap value for this slice an channel (aka the resolution)
 */
uint32_t pwm_set_freq_duty(uint slice_num, uint chan, uint32_t frequency, int d) {
    uint32_t clock = 125000000;
    uint32_t divider16 = clock / frequency / 4096 + (clock % (frequency * 4096) != 0);
    if (divider16 / 16 == 0) divider16 = 16;
    uint32_t wrap = clock * 16 / divider16 / frequency - 1;
    pwm_set_clkdiv_int_frac(slice_num, divider16 / 16, divider16 & 0xF);
    pwm_set_wrap(slice_num, wrap);
    pwm_set_chan_level(slice_num, chan, wrap * d / 100);
    return wrap;
}
