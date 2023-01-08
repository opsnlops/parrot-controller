
#include "creature/config.h"

#include "logging/logging.h"


CreatureConfig::CreatureConfig(const char* name, uint32_t servoFrequencyHz, uint8_t numberOfServos, uint16_t dmxBaseChannel) {

    debug("creating a new creatureConfig");

    // Initialize the name
    memset(this->name, '\0', CREATURE_CONFIG_NAME_MAX_SIZE + 1);
    strncpy(this->name, name, CREATURE_CONFIG_NAME_MAX_SIZE);

    this->servoFrequencyHz = servoFrequencyHz;
    this->numberOfServos = numberOfServos;
    this->dmxBaseChannel = dmxBaseChannel;

    // Initialize the servoConfig array, too
    memset(this->servoConfigs, '\0', sizeof(ServoConfig*) * MAX_NUMBER_OF_SERVOS);
}

void CreatureConfig::setServoConfig(uint8_t index, ServoConfig* config) {
    this->servoConfigs[index] = config;
}

ServoConfig* CreatureConfig::getServoConfig(uint8_t servoNumber) {
    return this->servoConfigs[servoNumber];
}

uint32_t CreatureConfig::getServoFrequencyHz() {
    return this->servoFrequencyHz;
}

uint16_t CreatureConfig::getDmxBaseChannel() {
    return this->dmxBaseChannel;
}

uint8_t CreatureConfig::getNumberOfServos() {
    return this->numberOfServos;
}

char* CreatureConfig::getName() {
    return this->name;
}


ServoConfig::ServoConfig() {

    // Initialize the name
    memset(this->name, '\0', CREATURE_CONFIG_NAME_MAX_SIZE + 1);
    strncpy(this->name, "???", CREATURE_CONFIG_NAME_MAX_SIZE);

    this->minPulseUs = 0;
    this->maxPulseUs = 0;
    this->smoothingValue = 0.0f;
    this->inverted = false;
}


ServoConfig::ServoConfig(const char* name, uint16_t minPulseUs, uint16_t maxPulseUs,
                         float smoothingValue, bool inverted) {

    // Initialize the name
    memset(this->name, '\0', CREATURE_CONFIG_NAME_MAX_SIZE + 1);
    strncpy(this->name, name, CREATURE_CONFIG_NAME_MAX_SIZE);

    this->minPulseUs = minPulseUs;
    this->maxPulseUs = maxPulseUs;
    this->smoothingValue = smoothingValue;
    this->inverted = inverted;

}