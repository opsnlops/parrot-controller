

#include <cstdio>
#include <cstring>

#include "hardware/gpio.h"

#include "shell.h"

#include "logging/logging.h"

#include "tasks.h"


extern TaskHandle_t debug_console_task_handle;

DebugShell::DebugShell(Creature *creature, Controller *controller, IOHandler *io) {

    debug("created the debug shell");

    this->creature = creature;
    this->controller = controller;
    this->io = io;
}


uint8_t DebugShell::init() {

    debug("init()-ing the debug shell");

    uart_init(uart1, DEBUG_SHELL_BAUD_RATE);
    gpio_set_function(DEBUG_SHELL_TX, GPIO_FUNC_UART);
    gpio_set_function(DEBUG_SHELL_RX, GPIO_FUNC_UART);
    uart_set_format(uart1, 8, 1, UART_PARITY_NONE);
    uart_set_translate_crlf(uart1, true);


    return 0;
}


uint8_t DebugShell::start() {

    debug("starting the debug shell");

    xTaskCreate(debug_console_task,
                "debug_console_task",
                1024,
                (void*)this,         // Pass in a reference to ourselves
                0,                      // Low priority
                &debug_console_task_handle);

    return 0;
}

Creature* DebugShell::getCreature() {
    return creature;
}

Controller* DebugShell::getController() {
    return controller;
}

IOHandler* DebugShell::getIOHandler() {
    return io;
}

portTASK_FUNCTION(debug_console_task, pvParameters) {

    debug("hello from the debug shell task!");

    auto shell = (DebugShell*)pvParameters;

    auto creature = shell->getCreature();
    auto controller = shell->getController();
    auto io = shell->getIOHandler();


    char tx_buffer[256] = {};
    memset(tx_buffer, '\0', 256);

    uint8_t rx_buffer[1];

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
    for(EVER) {

        // Block until there's something to read
        if(uart_is_readable(uart1)) {
            uart_read_blocking(uart1, rx_buffer, 1);

            switch(rx_buffer[0]) {

                case('d'):
                    snprintf(tx_buffer, 256, "\nWraps: %lu, Mem: %d, Frames: %lu",
                             controller->getNumberOfPWMWraps(),
                             xPortGetFreeHeapSize(),
                             io->getNumberOfFramesReceived());
                    uart_puts(uart1, tx_buffer);
                    break;

                case('p'):
                    controller->powerToggle();

                    snprintf(tx_buffer, 256, "\nPower is now %s", controller->isPoweredOn() ? "on" : "off");
                    uart_puts(uart1, tx_buffer);
                    break;

                default:
                    const char *helpMenu = "\n\n%s Debug Shell\n\n  d = show debug data\n  p = toggle power\n";

                    snprintf(tx_buffer, 256, helpMenu, creature->getName());
                    uart_puts(uart1, tx_buffer);
            }

            // Show the prompt
            uart_puts(uart1, "\n> ");

            memset(tx_buffer, '\0', 256);
        }
    }
#pragma clang diagnostic pop

}