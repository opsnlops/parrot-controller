
#pragma once

#include <FreeRTOS.h>
#include <tasks.h>

#include "controller-config.h"
#include "device/relay.h"
#include "device/servo.h"


class Controller {

public:
    Controller();

    void initServo(uint8_t indexNumber, const char* name, uint16_t minPulseUs,
                   uint16_t maxPulseUs, bool inverted);

    uint32_t getNumberOfPWMWraps();
    uint16_t getServoPosition(uint8_t indexNumber);

    void requestServoPosition(uint8_t servoIndexNumber, uint16_t requestedPosition);

    void init();
    void start();

    void setCreatureWorkerTaskHandle(TaskHandle_t creatureWorkerTaskHandle);

    void powerOn();
    void powerOff();
    void powerToggle();
    bool isPoweredOn() const;

    uint8_t* getCurrentFrame();

    bool acceptInput(uint8_t* input);

    // ISR, called when the PWM wraps
    static void __isr on_pwm_wrap_handler();

private:
    bool poweredOn;
    Relay* powerRelay;

    /**
     * An array of all of the servos we have. Set to the max number possible,
     * and then we wait for whatever creature is this to init the ones it
     * intends to use.
     */
    static Servo* servos[MAX_NUMBER_OF_SERVOS];

    // The current state of the input from the controller
    uint8_t* currentFrame;

    /**
     * A handle to our creature's working task. Used to signal that a new
     * frame has been received off the wire.
     */
    TaskHandle_t creatureWorkerTaskHandle;

    // The ISR needs access to these values
    static uint8_t numberOfServosInUse;
    static uint32_t numberOfPWMWraps;

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
