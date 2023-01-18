
#include "controller-config.h"

#include <FreeRTOS.h>
#include <task.h>

#include "pico/stdlib.h"

#include "creature/parrot.h"
#include "controller/controller.h"
#include "logging/logging.h"
#include "device/display.h"
#include "io/dmx.h"
#include "io/handler.h"
#include "io/stepper_uart.h"
#include "shell/shell.h"


#define INPUT_DMX 1

int main() {

    // All the SDK to bring up the stdio stuff, so we can write to the serial port
    stdio_init_all();

    logger_init();
    debug("Logging running!");

    auto *parrot = new Parrot();

    // Bring up the controller
    auto *controller = new Controller();

    // Load the config! For now, since we don't have a way to store the config somewhere, let's
    // just use the default
    controller->init(parrot->getDefaultConfig());

    // Bring up the I/O
    IOHandler* io = nullptr;
#ifdef INPUT_DMX
    io = new DMX(controller);
    ((DMX *) io)->setInputPin(DMX_GPIO_PIN);
#elif
    io = new UART(controller);
#endif
    io->init();

    parrot->init(controller);

    auto *display = new Display(controller, io);
    display->init();

    auto *shell = new DebugShell(parrot, controller, io);
    shell->init();

    auto *stepper_uart = new StepperUART();
    stepper_uart->init();

    // Start the things running!
    controller->start();
    display->start();
    parrot->start();
    io->start();
    stepper_uart->start();
    shell->start();

    // Let the controller know how to find the creature worker task
    controller->setCreatureWorkerTaskHandle(parrot->getWorkerTaskHandle());

    info("I see a new parrot! Its name is %s!", controller->getConfig()->getName());

    // Turn the power on to the servos
    controller->powerOn();

    // And fire up the tasks!
    vTaskStartScheduler();
}
