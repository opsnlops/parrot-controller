
#pragma once

#include <FreeRTOS.h>
#include <tasks.h>

#include "controller-config.h"
#include "creature/config.h"
#include "device/relay.h"
#include "device/servo.h"
#include "device/stepper.h"


class Controller {

public:
    Controller();

    CreatureConfig* getConfig();

    uint32_t getNumberOfPWMWraps();
    uint16_t getServoPosition(uint8_t indexNumber);

    void requestServoPosition(uint8_t servoIndexNumber, uint16_t requestedPosition);

    void init(CreatureConfig* incomingConfig);
    void start();

    void setCreatureWorkerTaskHandle(TaskHandle_t creatureWorkerTaskHandle);

    void powerOn();
    void powerOff();
    void powerToggle();
    [[nodiscard]] bool isPoweredOn() const;

    uint8_t* getCurrentFrame();

    uint8_t getPinMapping(uint8_t servoNumber);

    bool acceptInput(uint8_t* input);

    uint8_t getNumberOfServosInUse();

    // Get the servo, used for debugging
    static Servo* getServo(uint8_t index);

    Stepper* getStepper(uint8_t index);

    // ISR, called when the PWM wraps
    static void __isr on_pwm_wrap_handler();

private:
    bool poweredOn;
    Relay* powerRelay;

    // The configuration to use
    CreatureConfig* config;

    /**
     * An array of all of the servos we have. Set to the max number possible,
     * and then we wait for whatever creature is this to init the ones it
     * intends to use.
     */
    static Servo* servos[MAX_NUMBER_OF_SERVOS];

    Stepper* steppers[1];

    // The current state of the input from the controller
    uint8_t* currentFrame;

    // How many channels we're expecting from the I/O handler
    uint16_t numberOfChannels;

    /**
     * A handle to our creature's working task. Used to signal that a new
     * frame has been received off the wire.
     */
    TaskHandle_t creatureWorkerTaskHandle;

    // The ISR needs access to these values
    static uint8_t numberOfServosInUse;
    static uint32_t numberOfPWMWraps;

    void initServo(uint8_t indexNumber, const char* name, uint16_t minPulseUs,
                   uint16_t maxPulseUs, float smoothingValue, bool inverted);

    /**
     * Map the servo index to the GPO pin to use
     */
    uint8_t pinMappings[MAX_NUMBER_OF_SERVOS] = {
            SERVO_0_GPIO_PIN,
            SERVO_1_GPIO_PIN,
            SERVO_2_GPIO_PIN,
            SERVO_3_GPIO_PIN,
            SERVO_4_GPIO_PIN,
            SERVO_5_GPIO_PIN,
            SERVO_6_GPIO_PIN,
            SERVO_7_GPIO_PIN,
            SERVO_8_GPIO_PIN,
            SERVO_9_GPIO_PIN,
            SERVO_10_GPIO_PIN,
            SERVO_11_GPIO_PIN,
            SERVO_12_GPIO_PIN,
            SERVO_13_GPIO_PIN,
            SERVO_14_GPIO_PIN,
            SERVO_15_GPIO_PIN};
};
