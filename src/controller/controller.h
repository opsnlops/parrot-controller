
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

    CreatureConfig* getRunningConfig();

    uint32_t getNumberOfPWMWraps();
    uint16_t getServoPosition(uint8_t indexNumber);
    uint32_t getStepperPosition(uint8_t indexNumber);

    void requestServoPosition(uint8_t servoIndexNumber, uint16_t requestedPosition);
    static void requestStepperPosition(uint8_t stepperIndexNumber, uint32_t requestedPosition);

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

    static uint8_t getNumberOfServosInUse();
    static uint8_t getNumberOfSteppersInUse();

    uint16_t getNumberOfDMXChannels();

    // Get the servo, used for debugging
    static Servo* getServo(uint8_t index);

    static Stepper* getStepper(uint8_t index);

    // ISR, called when the PWM wraps
    static void __isr on_pwm_wrap_handler();

private:
    bool poweredOn;
    Relay* powerRelay;

    // The configuration to use
    CreatureConfig* config{};

    /**
     * An array of all of the servos we have. Set to the max number possible,
     * and then we wait for whatever creature is this to init the ones it
     * intends to use.
     */
    static Servo* servos[MAX_NUMBER_OF_SERVOS];

    static Stepper* steppers[MAX_NUMBER_OF_STEPPERS];

    // The current state of the input from the controller
    uint8_t* currentFrame{};

    // How many channels we're expecting from the I/O handler
    uint16_t numberOfChannels{};

    /**
     * A handle to our creature's working task. Used to signal that a new
     * frame has been received off the wire.
     */
    TaskHandle_t creatureWorkerTaskHandle;

    // The ISR needs access to these values
    static uint8_t numberOfServosInUse;
    static uint32_t numberOfPWMWraps;

    static uint8_t numberOfSteppersInUse;

    static void configureGPIO(uint8_t pin, bool out, bool initialValue);

    void initServo(uint8_t indexNumber, const char* name, uint16_t minPulseUs,
                   uint16_t maxPulseUs, float smoothingValue, bool inverted);

    void initStepper(uint8_t indexNumber, const char* name, uint32_t maxSteps, float smoothingValue,
                     uint32_t sleepWakeupPauseTimeUs, uint32_t sleepAfterUs, uint8_t microsteppingConfig, bool inverted);

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
            SERVO_7_GPIO_PIN};

};
