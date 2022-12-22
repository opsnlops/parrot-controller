

#include "device/servo.h"
#include "logging/logging.h"

#include "controller.h"


// Initialize the static members
Servo* Controller::servos[MAX_NUMBER_OF_SERVOS] = {};
uint8_t Controller::numberOfServosInUse = 0;
uint32_t Controller::numberOfPWMWraps = 0;

Controller::Controller() {

    debug("setting up the controller");

    // Initialize all the slots in the controller
    for(auto & servo : servos) {
        servo = nullptr;
    }

    poweredOn = false;
    powerRelay = new Relay(E_STOP_PIN, poweredOn);

}

void Controller::init() {
    // NOOP
}

void Controller::start() {
    info("starting controller!");

    // Install the IRQ handler for the servos
    pwm_set_irq_enabled(servos[0]->getSlice(), true);
    irq_set_exclusive_handler(PWM_IRQ_WRAP, Controller::on_pwm_wrap_handler);
    irq_set_enabled(PWM_IRQ_WRAP, true);

}


void Controller::initServo(uint8_t indexNumber, const char* name, uint16_t minPulseUs,
                           uint16_t maxPulseUs, bool inverted) {

    uint8_t gpioPin = pinMappings[indexNumber];

    servos[indexNumber] = new Servo(gpioPin, name, minPulseUs,
                                    maxPulseUs, inverted);
    numberOfServosInUse++;

    info("servo init: index: %d, pin: %d, name: %s", indexNumber, gpioPin, name);

}

uint16_t Controller::getServoPosition(uint8_t indexNumber) {
   return servos[indexNumber]->getPosition();
}

void __isr Controller::on_pwm_wrap_handler() {

    for(int i = 0; i < numberOfServosInUse; i++)
        pwm_set_chan_level(servos[i]->getSlice(),
                           servos[i]->getChannel(),
                           servos[i]->getDesiredTicks());

    pwm_clear_irq(servos[0]->getSlice());

    Controller::numberOfPWMWraps++;
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

bool Controller::isPoweredOn() {
    return poweredOn;
}