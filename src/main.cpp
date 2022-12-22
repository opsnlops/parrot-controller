

#include <climits>

#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include "pico/stdlib.h"

#include "creature/parrot.h"

#include "controller-config.h"
#include "controller/controller.h"

#include "logging/logging.h"
#include "tasks.h"
#include "device/relay.h"
#include "device/display.h"

#include "io/dmx.h"
#include "io/handler.h"
#include "io/uart.h"



extern TaskHandle_t dmx_processing_task_handle;

IOHandler *input;
IOHandler *dmxInput;


int main() {

    // All the SDK to bring up the stdio stuff, so we can write to the serial port
    stdio_init_all();

    logger_init();
    debug("Logging running!");

    // TODO: Just for testing
    input = new UART();
    input->init();

    dmxInput = new DMX();
    ((DMX *) dmxInput)->setInputPin(DMX_GPIO_PIN);
    dmxInput->init();

    auto *controller = new Controller();
    auto *parrot = new Parrot("Beaky");
    auto *display = new Display(controller, dmxInput);

    parrot->setController(controller);
    parrot->init();



    parrot->start();
    info("I see a new parrot! Its name is %s!", parrot->getName());

    display->start();


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


portTASK_FUNCTION(relayDebugTask, pvParameters) {
#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"

    for (EVER) {

        // Test toggling the power on and off to the servos
        vTaskDelay(pdMS_TO_TICKS(10000));
        //creature_power->toggle();

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