
#include <cstdio>
#include <climits>
#include <string>
#include <list>

#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>
#include <cstring>

#include "pico/stdlib.h"
#include "hardware/irq.h"
#include "hardware/pwm.h"

#include "creature/parrot.h"
#include "creature/creature.h"

#include "controller-config.h"
#include "controller/controller.h"

#include "logging/logging.h"
#include "device/servo.h"
#include "tasks.h"
#include "device/relay.h"

#include "io/dmx.h"
#include "io/handler.h"
#include "io/uart.h"


// Located in tasks.cpp
extern TaskHandle_t displayUpdateTaskHandle;
extern TaskHandle_t dmx_processing_task_handle;

uint32_t pwm_wraps = 0;



// Create an array of servos
Servo* servos[NUMBER_OF_SERVOS];
Relay* creature_power;

IOHandler* input;
IOHandler* dmxInput;

/**
 * IRQ handler to update the duty cycle on our servos
 */
void __isr on_pwm_wrap_handler() {

    for(auto & servo : servos)
        pwm_set_chan_level(servo->getSlice(),
                           servo->getChannel(),
                           servo->getDesiredTicks());

    pwm_clear_irq(servos[0]->getSlice());

    pwm_wraps++;
}


int main() {

    // All the SDK to bring up the stdio stuff, so we can write to the serial port
    stdio_init_all();

    logger_init();
    debug("Logging running!");

    // TODO: Just for testing
    input = new UART();
    input->init();

    dmxInput = new DMX();
    ((DMX*)dmxInput)->setInputPin(DMX_GPIO_PIN);
    dmxInput->init();

    //auto* myJoints = new std::list<Creatures::Joint>();
    //auto it = myJoints->begin();
    //myJoints->insert(it, new Creatures::Joint("etsts", 0, nullptr));
    auto* parrot = new Creatures::Parrot("Pretty Bird", nullptr);

    info("I see a Parrot named %s", parrot->getName().c_str());


    // Turn off the e-stop
    creature_power = new Relay(E_STOP_PIN, true);
    creature_power->turnOn();

    // Fire up DMX
    //dmx_init(DMX_GPIO_PIN);


    // Create the servos
    servos[0] = new Servo(22, "Servo One", SERVO_HZ, 250, 2500, false);
    servos[1] = new Servo(2, "Servo Two", SERVO_HZ, 250, 2500, true);

    // Install the IRQ handler for the servos
    pwm_set_irq_enabled(servos[0]->getSlice(), true);
    irq_set_exclusive_handler(PWM_IRQ_WRAP, on_pwm_wrap_handler);
    irq_set_enabled(PWM_IRQ_WRAP, true);

    // And start them up!
    servos[0]->turnOn();
    servos[1]->turnOn();

#ifdef USE_UART_CONTROL



    // Start the task to read the queue
    xTaskCreate(messageQueueReaderTask,
                "messageQueueReaderTask",
                1024,
                nullptr,
                1,
                &messageQueueReaderTaskHandle);
#endif

    /*
    xTaskCreate(servoDebugTask,
                "servoDebugTask",
                1024,
                nullptr,
                1,
                &servoDebugTaskHandle);


    xTaskCreate(relayDebugTask,
                "relayDebugTask",
                512,
                nullptr,
                1,
                &relayDebugTaskHandle);
  */

    xTaskCreate(displayUpdateTask,
                "displayUpdateTask",
                1024,
                nullptr,
                0,          // Low priority
                &displayUpdateTaskHandle);


    xTaskCreate(dmx_processing_task,
                "dmx_processing_task",
                2048,
                nullptr,
                1,
                &dmx_processing_task_handle);


    vTaskStartScheduler();
}


// Read from the queue and print it to the screen for now
portTASK_FUNCTION(hellorldTask, pvParameters) {
#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
    for (EVER) {
#ifdef USE_UART_CONTROL
        uart_puts(UART_ID, "Hellorld!\n\r");
#endif
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
#pragma clang diagnostic pop
}


portTASK_FUNCTION(servoDebugTask, pvParameters) {
#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"

    for (EVER) {

        // Declare these here, so we're not making new ones in this tight loop
        int i, j;

        for (i = 0; i < MAX_SERVO_POSITION; i += 5) {
            for (j = 0; j < NUMBER_OF_SERVOS; j++)
                servos[j]->move(i);

            vTaskDelay(pdMS_TO_TICKS(20));
        }
    }
#pragma clang diagnostic pop
}

portTASK_FUNCTION(relayDebugTask, pvParameters) {
#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"

    for (EVER) {

        // Test toggling the power on and off to the servos
        vTaskDelay(pdMS_TO_TICKS(10000));
        creature_power->toggle();

    }
#pragma clang diagnostic pop
}


portTASK_FUNCTION(dmx_processing_task, pvParameters) {
#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"

    uint32_t ulNotifiedValue;
    for (EVER) {

        xTaskNotifyWait(0x00, ULONG_MAX, &ulNotifiedValue, portMAX_DELAY);
        verbose("DMX says hi");


    }
#pragma clang diagnostic pop
}