

#include <climits>

#include <FreeRTOS.h>


#include "device/servo.h"
#include "logging/logging.h"

#include "controller.h"


extern TaskHandle_t controllerHousekeeperTaskHandle;
extern TaskHandle_t stepper_step_task_handle;
BaseType_t isrPriorityTaskWoken = pdFALSE;

// Initialize the static members
Servo *Controller::servos[MAX_NUMBER_OF_SERVOS] = {};
uint8_t Controller::numberOfServosInUse = 0;
uint32_t Controller::numberOfPWMWraps = 0;

// TODO: Nope
bool step(struct repeating_timer *t);

Controller::Controller() {

    debug("setting up the controller");

    creatureWorkerTaskHandle = nullptr;
    poweredOn = false;
    powerRelay = new Relay(E_STOP_PIN, poweredOn);

}

void Controller::init(CreatureConfig *incomingConfig) {

    this->config = incomingConfig;
    this->numberOfChannels = this->config->getNumberOfServos() + 1; // The number of servos + the e-stop

    // Initialize all the slots in the controller
    for (auto &servo: servos) {
        servo = nullptr;
    }

    for (auto &stepper: steppers) {
        stepper = nullptr;
    }

    // Set up the servos
    for (int i = 0; i < this->config->getNumberOfServos(); i++) {
        ServoConfig *servo = this->config->getServoConfig(i);
        initServo(i, servo->name, servo->minPulseUs, servo->maxPulseUs, servo->smoothingValue, servo->inverted);
    }

    // Declare some space on the heap for our current frame buffer
    currentFrame = (uint8_t *) pvPortMalloc(sizeof(uint8_t) * this->numberOfChannels);

    // Set the currentFrame buffer to the middle value as a safe-ish default
    for (int i = 0; i < this->numberOfChannels; i++) {
        currentFrame[i] = UCHAR_MAX / 2;
    }

    // TODO: This is all temp
    steppers[0] = new Stepper();
    steppers[0]->directionPin = STEPPER_DIR_PIN;
    steppers[0]->stepsPin = STEPPER_STEPS_PIN;

    gpio_init(steppers[0]->directionPin);
    gpio_set_dir(steppers[0]->directionPin, true);   // This is an output pin
    gpio_put(steppers[0]->directionPin, true);

    gpio_init(steppers[0]->stepsPin);
    gpio_set_dir(steppers[0]->stepsPin, true);   // This is an output pin
    gpio_put(steppers[0]->stepsPin, true);

}

void Controller::setCreatureWorkerTaskHandle(TaskHandle_t taskHandle) {
    this->creatureWorkerTaskHandle = taskHandle;
}

void Controller::start() {
    info("starting controller!");

    // Install the IRQ handler for the servos
    pwm_set_irq_enabled(servos[0]->getSlice(), true);
    irq_set_exclusive_handler(PWM_IRQ_WRAP, Controller::on_pwm_wrap_handler);
    irq_set_enabled(PWM_IRQ_WRAP, true);

    // Fire off the housekeeper
    xTaskCreate(controller_housekeeper_task,
                "controller_housekeeper_task",
                256,
                (void *) this,
                1,
                &controllerHousekeeperTaskHandle);

    // Fire off the stepper task
    xTaskCreate(stepper_step_task,
                "stepper_step_task",
                256,
                (void *) this,
                1,
                &stepper_step_task_handle);

}

/**
 * Accepts input from an IOHandler
 *
 * @param input a buffer of size DMX_NUMBER_OF_CHANNELS containing the incoming data
 * @return true if it worked
 */
bool Controller::acceptInput(uint8_t *input) {

    // Copy the incoming buffer into our buffer
    memcpy(currentFrame, input, this->numberOfChannels);

    /**
     * If there's no worker task, stop here.
     */
    if (creatureWorkerTaskHandle == nullptr) {
        return false;
    }

    // Send the processor a message
    xTaskNotify(creatureWorkerTaskHandle,
                0,
                eNoAction);

    return true;
}

uint8_t *Controller::getCurrentFrame() {
    return currentFrame;
}

void Controller::initServo(uint8_t indexNumber, const char *name, uint16_t minPulseUs,
                           uint16_t maxPulseUs, float smoothingValue, bool inverted) {

    uint8_t gpioPin = getPinMapping(indexNumber);

    servos[indexNumber] = new Servo(gpioPin, name, minPulseUs,
                                    maxPulseUs, smoothingValue, inverted,
                                    this->config->getServoFrequencyHz());
    numberOfServosInUse++;

    info("servo init: index: %d, pin: %d, name: %s", indexNumber, gpioPin, name);

}

CreatureConfig *Controller::getConfig() {
    return config;
}

uint8_t Controller::getPinMapping(uint8_t servoNumber) {
    return pinMappings[servoNumber];
}

uint16_t Controller::getServoPosition(uint8_t indexNumber) {
    return servos[indexNumber]->getPosition();
}

void Controller::requestServoPosition(uint8_t servoIndexNumber, uint16_t requestedPosition) {

    if (servos[servoIndexNumber]->getPosition() != requestedPosition) {
        debug("requested to move servo %d from %d to position %d", servoIndexNumber,
              servos[servoIndexNumber]->getPosition(), requestedPosition);
        servos[servoIndexNumber]->move(requestedPosition);
    }

}

void __isr Controller::on_pwm_wrap_handler() {

    for (int i = 0; i < numberOfServosInUse; i++)

        pwm_set_chan_level(servos[i]->getSlice(),
                           servos[i]->getChannel(),
                           servos[i]->getCurrentTick());

    pwm_clear_irq(servos[0]->getSlice());

    Controller::numberOfPWMWraps++;

 /**
 * If there's no worker task, stop here.
 */
    if (controllerHousekeeperTaskHandle != nullptr) {

        // Tell the housekeeper to go when it can
        xTaskNotifyFromISR(controllerHousekeeperTaskHandle,
                    0,
                    eNoAction,
                    &isrPriorityTaskWoken);
    }
}

uint32_t Controller::getNumberOfPWMWraps() {
    return numberOfPWMWraps;
}

void Controller::powerOn() {
    powerRelay->turnOn();
    poweredOn = true;
    info("turned on the power");
}

void Controller::powerOff() {
    powerRelay->turnOff();
    poweredOn = false;
    info("turned off the power");
}

void Controller::powerToggle() {
    poweredOn ? powerOff() : powerOn();
}

bool Controller::isPoweredOn() const {
    return poweredOn;
}

uint8_t Controller::getNumberOfServosInUse() {
    return numberOfServosInUse;
}

Servo *Controller::getServo(uint8_t index) {
    return servos[index];
}

Stepper *Controller::getStepper(uint8_t index) {
    return steppers[index];
}

portTASK_FUNCTION(controller_housekeeper_task, pvParameters) {

    auto controller = (Controller *) pvParameters;

    uint32_t ulNotifiedValue;

    debug("controller housekeeper running");

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"

    for (EVER) {

        // Wait for the ISR to signal us to go
        xTaskNotifyWait(0x00, ULONG_MAX, &ulNotifiedValue, portMAX_DELAY);

        for (int i = 0; i < controller->getNumberOfServosInUse(); i++) {

            // Do housekeeping on each servo
            controller->getServo(i)->calculateNextTick();

        }

    }
#pragma clang diagnostic pop
}

portTASK_FUNCTION(stepper_step_task, pvParameters) {

    auto controller = (Controller *) pvParameters;

    debug("stepper step task running");

    struct repeating_timer timer;

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"

    add_repeating_timer_us(750, step, controller->getStepper(0), &timer);


    for (EVER) {


        vTaskDelay(pdMS_TO_TICKS(1000));


    }
#pragma clang diagnostic pop
}

bool high = true;

// A free function to be an interrupt handler
bool step(struct repeating_timer *t) {

    auto* s = (Stepper*)(t->user_data);

    gpio_put(s->stepsPin, high);
    high = !high;

    return true;
}