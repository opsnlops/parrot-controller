
#pragma once

#include <cstdio>
#include "pico/stdlib.h"

#include "hardware/gpio.h"
#include "hardware/pwm.h"

/**
 * A representation of a servo that we are controlling with the controller
 *
 * At init the slice, channel, and resolution are calculated from the Pico
 * SDK.
 *
 * The min and max pulse define the length of the travel for this servo within
 * the creature. It is highly specific to the individual creature.
 *
 * BIG NOTE: Some of the GPIO pins share a slice. Be aware that the frequency must
 *           be the same for both halves of the slice.
 *
 * @brief A representation of a servo motor
 */
class Servo {

public:
    Servo(uint gpio, const char* name, uint16_t min_pulse_us, uint16_t max_pulse_us,
          float smoothingValue, bool inverted, uint32_t frequency);
    void turnOn();
    void turnOff();
    uint16_t getPosition() const;
    uint getSlice() const;
    uint getChannel() const;

    // These are PWM values
    uint32_t getDesiredTicks() const;   // Where we want it to go
    uint32_t getCurrentTicks() const;   // Where the servo currently is

    float getSmoothingValue() const;
    const char* getName() const;
    void move(uint16_t position);

private:
    uint gpio;                  // GPIO pin the servo is connected to
    uint16_t min_pulse_us;      // Lower bound on the servo's pulse size in microseconds
    uint16_t max_pulse_us;      // Upper bound on the servo's pulse size in microseconds
    uint slice;                 // PWM slice for this servo
    uint channel;               // PWM channel for this servo
    uint32_t resolution;        // The resolution for this servo
    uint32_t frame_length_us;   // How many microseconds are in each frame
    uint16_t current_position;  // Where we think the servo currently is in our position
    bool on;                    // Is the servo active?
    bool inverted;              // Should the movements be inverted?
    uint32_t desired_ticks;     // The number of ticks we should be set to on the next cycle
    uint32_t current_ticks;     // Which tick is the servo at
    const char* name;           // This servo's name
    float smoothingValue;       // The constant to use when smoothing the input

    static uint32_t pwm_set_freq_duty(uint slice_num, uint chan, uint32_t frequency, int d);
};
