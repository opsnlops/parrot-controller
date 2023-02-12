

#include <climits>

#include <FreeRTOS.h>
#include <cstdlib>


#include "device/servo.h"
#include "logging/logging.h"

#include "controller.h"


extern TaskHandle_t controllerHousekeeperTaskHandle;
extern TaskHandle_t stepper_step_task_handle;
BaseType_t isrPriorityTaskWoken = pdFALSE;

// Initialize the static members
Servo *Controller::servos[MAX_NUMBER_OF_SERVOS] = {};
Stepper *Controller::steppers[MAX_NUMBER_OF_STEPPERS] = {};
uint8_t Controller::numberOfServosInUse = 0;
uint8_t Controller::numberOfSteppersInUse = 0;
uint32_t Controller::numberOfPWMWraps = 0;


/**
 * Simple array for setting the address lines of the stepper latches
 */
static bool stepperAddressMapping[MAX_NUMBER_OF_STEPPERS][STEPPER_MUX_BITS] = {

        {false,     false,      false},     // 0
        {false,     false,      true},      // 1
        {false,     true,       false},     // 2
        {false,     true,       true},      // 3
        {true,      false,      false},     // 4
        {true,      false,      true},      // 5
        {true,      true,       false},     // 6
        {true,      true,       true}       // 7
};

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
    this->numberOfChannels = this->config->getNumberOfServos() + this->config->getNumberOfSteppers() + 1; // The number of motors + the e-stop

    // Initialize all the slots in the controller
    for (auto &servo: servos) {
        servo = nullptr;
    }

    for (auto &stepper: steppers) {
        stepper = nullptr;
    }

    // Set up the servos
    debug("building servo objects");
    for (int i = 0; i < this->config->getNumberOfServos(); i++) {
        ServoConfig *servo = this->config->getServoConfig(i);
        initServo(i, servo->name, servo->minPulseUs, servo->maxPulseUs, servo->smoothingValue, servo->inverted);
    }

    // Set up the steppers
    debug("building stepper objects");
    for (int i = 0; i < this->config->getNumberOfSteppers(); i++) {
        StepperConfig *stepper = this->config->getStepperConfig(i);
        initStepper(i, stepper->name, stepper->maxSteps, stepper->smoothingValue, stepper->inverted);
    }

    // Declare some space on the heap for our current frame buffer
    currentFrame = (uint8_t *) pvPortMalloc(sizeof(uint8_t) * this->numberOfChannels);

    // Set the currentFrame buffer to the middle value as a safe-ish default
    for (int i = 0; i < this->numberOfChannels; i++) {
        currentFrame[i] = UCHAR_MAX / 2;
    }

    debug("setting up the stepper pins");

    // Output Pins
    configureGPIO(STEPPER_STEP_PIN, true, false);
    configureGPIO(STEPPER_DIR_PIN, true, false);
    configureGPIO(STEPPER_MS1_PIN, true, false);
    configureGPIO(STEPPER_MS2_PIN, true, false);
    configureGPIO(STEPPER_A0_PIN, true, false);
    configureGPIO(STEPPER_A1_PIN, true, false);
    configureGPIO(STEPPER_A2_PIN, true, false);
    configureGPIO(STEPPER_LATCH_PIN, true, false);
    configureGPIO(STEPPER_SLEEP_PIN, true, false);

    // Input pins
    configureGPIO(STEPPER_FAULT_PIN, false, false);
    configureGPIO(STEPPER_END_S_LOW_PIN, false, false);
    configureGPIO(STEPPER_END_S_HIGH_PIN, false, false);

   debug("done setting up the stepper pins");

}

void Controller::configureGPIO(uint8_t pin, bool out, bool initialValue) {

    verbose("setting up stepper pin %d: direction: %s, initialValue: %s",
            pin,
            out ? "out" : "in",
            initialValue ? "on" : "off");
    gpio_init(pin);
    gpio_set_dir(pin, out);
    gpio_put(pin, initialValue);

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

void Controller::initStepper(uint8_t slot, const char *name, uint32_t maxSteps,
                             float smoothingValue, bool inverted) {

    steppers[slot] = new Stepper(slot, name, maxSteps, smoothingValue, inverted);
    numberOfSteppersInUse++;

    info("stepper init: slot: %d, name: %s, max_steps: %d", slot, name, maxSteps);

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

uint8_t Controller::getNumberOfSteppersInUse() {
    return numberOfSteppersInUse;
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

    // Add the worker
    add_repeating_timer_us(750, step, nullptr, &timer);


    for (EVER) {

        for(int i = 0; i < Controller::getNumberOfSteppersInUse(); i++)
        {
            Stepper* stepper = controller->getStepper(i);

            uint32_t newStep = rand() % 300;
            stepper->setDesiredStep(newStep);

            debug("set stepper %d to %d", stepper->slot, newStep);

        }

        vTaskDelay(pdMS_TO_TICKS(500));


    }
#pragma clang diagnostic pop
}


// A free function to be an interrupt handler
bool step(struct repeating_timer *t) {


    // Look at each stepper we have and adjust if needed
    for(int i = 0; i < Controller::getNumberOfSteppersInUse(); i++) {

        Stepper *s = Controller::getStepper(i);

        uint8_t slot = s->getSlot();

        // Configure the address lines
        gpio_put(STEPPER_A0_PIN, stepperAddressMapping[slot][2]);
        gpio_put(STEPPER_A1_PIN, stepperAddressMapping[slot][1]);
        gpio_put(STEPPER_A2_PIN, stepperAddressMapping[slot][0]);



        // For now let's use half steps
        gpio_put(STEPPER_MS1_PIN, true);
        gpio_put(STEPPER_MS2_PIN, false);

        // Now that we've selected it, let's toggle the bits

        if (s->isHigh) {
            s->isHigh = false;
            gpio_put(STEPPER_STEP_PIN, false);
            // Leave the direction pin alone
        } else {
            // If we need to move, let's move!
            if (s->currentStep != s->desiredSteps) {

                // If we have to move, let's move
                if (s->currentStep != s->desiredSteps) {

                    if (s->currentStep < s->desiredSteps) {

                        gpio_put(STEPPER_DIR_PIN, false);
                        s->currentStep++;
                    } else {

                        gpio_put(STEPPER_DIR_PIN, true);
                        s->currentStep--;
                    }

                    gpio_put(STEPPER_STEP_PIN, true);
                    s->isHigh = true;

                }
            }
                // They're equal, no steps needed
            else {
                gpio_put(STEPPER_STEP_PIN, false);
            }
        }

        // Enable the latch
        gpio_put(STEPPER_LATCH_PIN, false);     // It's active low

        // Stall long enough to let the latch go
        for(int j = 0; j < 100; j++) {}

        // Now that we've toggled everything, turn the latch back off
        gpio_put(STEPPER_LATCH_PIN, true);     // It's active low

    }

    return true;
}