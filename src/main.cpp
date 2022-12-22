

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

#define INPUT_DMX 1

int main() {

    // All the SDK to bring up the stdio stuff, so we can write to the serial port
    stdio_init_all();

    logger_init();
    debug("Logging running!");

    // TODO: Just for testing, maybe use a switch?
    IOHandler* io = nullptr;
#ifdef INPUT_DMX
    io = new DMX();
    ((DMX *) io)->setInputPin(DMX_GPIO_PIN);
#elif
    io = new UATRT();
#endif
    io->init();

    auto *controller = new Controller();
    auto *parrot = new Parrot("Beaky92");
    auto *display = new Display(controller, io);

    controller->init();
    parrot->init(controller);


    controller->start();

    debug("calling display->start()");
    display->start();


    debug("calling start()");
    parrot->start();
    info("I see a new parrot! Its name is %s!", parrot->getName());




    xTaskCreate(dmx_processing_task,
                "dmx_processing_task",
                2048,
                nullptr,
                1,
                &dmx_processing_task_handle);

    controller->powerOn();

    vTaskStartScheduler();
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