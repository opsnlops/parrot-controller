
#pragma once

/*
 * ### Configuration of the Creature ###
 *
 *
 * This is decoupled from the creature itself so that the controller can keep a tweaked copy
 * of the configuration, if needed. This allows the controller to store the configuration somewhere
 * else, like in an EEPROM or flash, so that it's persistent across reboots.
 *
 * The overall goal is to allow a human to be able to change these values at runtime via the debug
 * shell if there's a need.
 *
 * We provide a "default config," which should be fully working, but without any tweaks. From there
 * the controller can do what it likes with it.
 */


#include "controller-config.h"

#include <cctype>
#include <cstdint>

#define CREATURE_CONFIG_NAME_MAX_SIZE   20


class ServoConfig {

public:
    ServoConfig();
    ServoConfig(const char* name, uint16_t minPulseUs, uint16_t maxPulseUs, float smoothingValue, bool inverted);

    // Make name a static array to make the size predictable for storing in an EEPROM or Flash memory
    char name[CREATURE_CONFIG_NAME_MAX_SIZE + 1];
    uint16_t minPulseUs;
    uint16_t maxPulseUs;
    float smoothingValue;
    bool inverted;
};


class CreatureConfig {

public:
    CreatureConfig(const char* name, uint32_t servoFrequencyHz, uint8_t numberOfServos, uint16_t dmxBaseChannel);

    void setServoConfig(uint8_t index, ServoConfig* config);

    char* getName();
    uint32_t getServoFrequencyHz();
    uint16_t getDmxBaseChannel();
    uint8_t getNumberOfServos();
    ServoConfig* getServoConfig(uint8_t servoNumber);

private:
    char name[CREATURE_CONFIG_NAME_MAX_SIZE + 1];

    uint32_t servoFrequencyHz;
    uint16_t dmxBaseChannel;
    uint8_t numberOfServos;
    ServoConfig* servoConfigs[MAX_NUMBER_OF_SERVOS];

};