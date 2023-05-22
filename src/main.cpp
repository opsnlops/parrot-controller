
#include "controller-config.h"

#include <memory>

#include <FreeRTOS.h>
#include <task.h>

// TinyUSB
#include "bsp/board.h"
#include "tusb.h"

#include "pico/stdlib.h"

#include "creature/parrot.h"
#include "controller/controller.h"
#include "logging/logging.h"

#if DISPLAY_ENABLED
#include "device/display.h"
#endif

#include "device/status_lights.h"
#include "io/dmx.h"
#include "io/handler.h"
#include "shell/shell.h"
#include "usb/usb.h"


#define INPUT_DMX 1

int main() {

    // All the SDK to bring up the stdio stuff, so we can write to the serial port
    stdio_init_all();

    logger_init();
    debug("Logging running!");

    //std::shared_ptr<Parrot> parrot = std::make_unique<Parrot>();
    auto parrot = new Parrot();

    // Bring up the controller
    auto controller = new Controller();

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

#if DISPLAY_ENABLED
    auto *display = new Display(controller, io);
    display->init();
#endif

    auto *shell = new DebugShell(parrot, controller, io);
    shell->init();

    auto *statusLights = new StatusLights(controller, io);
    statusLights->init();

    // Start the things running!
    controller->start();

    parrot->start();
    io->start();
    shell->start();
    statusLights->start();

#if DISPLAY_ENABLED
    display->start();
#endif

    // Let the controller know how to find the creature worker task
    controller->setCreatureWorkerTaskHandle(parrot->getWorkerTaskHandle());

    info("I see a new parrot! Its name is %s!", controller->getRunningConfig()->getName());

    // Turn the power on to the servos
    controller->powerOn();

    board_init();
    start_usb_tasks();

    // And fire up the tasks!
    vTaskStartScheduler();
}
