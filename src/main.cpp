

#include <climits>

#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include "pico/stdlib.h"

#include "creature/parrot.h"

#include "controller-config.h"
#include "controller/controller.h"

#include "logging/logging.h"
#include "device/display.h"

#include "io/dmx.h"
#include "io/handler.h"
#include "io/uart.h"


#define INPUT_DMX 1

int main() {

    // All the SDK to bring up the stdio stuff, so we can write to the serial port
    stdio_init_all();

    logger_init();
    debug("Logging running!");

    // Bring up the controller
    auto *controller = new Controller();
    controller->init();

    // Bring up the I/O
    IOHandler* io = nullptr;
#ifdef INPUT_DMX
    io = new DMX(controller);
    ((DMX *) io)->setInputPin(DMX_GPIO_PIN);
#elif
    io = new UART(controller);
#endif
    io->init();

    auto *parrot = new Parrot("Beaky");
    parrot->init(controller);

    auto *display = new Display(controller, io);
    display->init();


    // Start the things running!
    controller->start();
    display->start();
    parrot->start();
    io->start();

    info("I see a new parrot! Its name is %s!", parrot->getName());


    // Turn the power on to the servos
    controller->powerOn();

    // And fire up the tasks!
    vTaskStartScheduler();
}
