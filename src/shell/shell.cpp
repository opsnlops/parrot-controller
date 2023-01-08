

#include <cstdio>
#include <cstring>
#include <cstdlib>

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
    this->config = controller->getConfig();
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
                512,
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

CreatureConfig* DebugShell::getConfig() {
    return config;
}

void ds_reset_buffers(char* tx_buffer, uint8_t* rx_buffer)
{
    memset(rx_buffer, '\0', DS_RX_BUFFER_SIZE);
    memset(tx_buffer, '\0', DS_TX_BUFFER_SIZE);
}

portTASK_FUNCTION(debug_console_task, pvParameters) {

    debug("hello from the debug shell task!");

    auto shell = (DebugShell*)pvParameters;

    auto creature = shell->getCreature();
    auto controller = shell->getController();
    auto io = shell->getIOHandler();
    auto config = shell->getConfig();

    // Set up our buffers on the heap
    auto tx_buffer = (char*)malloc(sizeof(char) * DS_TX_BUFFER_SIZE);
    auto rx_buffer = (uint8_t*)malloc(sizeof(uint8_t) * DS_RX_BUFFER_SIZE);     // The pico SDK wants uint8_t

    ds_reset_buffers(tx_buffer, rx_buffer);


#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
    for(EVER) {

        // Block until there's something to read
        if(uart_is_readable(uart1)) {
            uart_read_blocking(uart1, rx_buffer, DS_RX_BUFFER_SIZE);

            // Echo back the input from the user
            uart_puts(uart1, (char*)rx_buffer);

            switch(rx_buffer[0]) {

                case('c'):

                    snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "\n\n Creature Controller Config\n --------------------------\n\n");
                    uart_puts(uart1, tx_buffer);
                    ds_reset_buffers(tx_buffer, rx_buffer);

                    snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "              Name: %s\n",config->getName());
                    uart_puts(uart1, tx_buffer);
                    ds_reset_buffers(tx_buffer, rx_buffer);

                    snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "  Number of Servos: %d\n", config->getNumberOfServos());
                    uart_puts(uart1, tx_buffer);
                    ds_reset_buffers(tx_buffer, rx_buffer);

                    snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "      Base Channel: %d\n", config->getDmxBaseChannel());
                    uart_puts(uart1, tx_buffer);
                    ds_reset_buffers(tx_buffer, rx_buffer);

                    snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "   Servo Frequency: %luHz\n", config->getServoFrequencyHz());
                    uart_puts(uart1, tx_buffer);
                    ds_reset_buffers(tx_buffer, rx_buffer);

                    snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "\n   Servo Config:\n");
                    uart_puts(uart1, tx_buffer);
                    ds_reset_buffers(tx_buffer, rx_buffer);

                    snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "      num |        name           |   min   |   max   | smooth | inverted\n");
                    uart_puts(uart1, tx_buffer);
                    ds_reset_buffers(tx_buffer, rx_buffer);
                    snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "      ---------------------------------------------------------------------\n");
                    uart_puts(uart1, tx_buffer);
                    ds_reset_buffers(tx_buffer, rx_buffer);

                    for(int i = 0; i < config->getNumberOfServos(); i++) {
                        snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "       %-2d | %-21s |  %6d |  %6d | %.4f |   %3s\n",
                                 i,
                                 config->getServoConfig(i)->name,
                                 config->getServoConfig(i)->minPulseUs,
                                 config->getServoConfig(i)->maxPulseUs,
                                 config->getServoConfig(i)->smoothingValue,
                                 config->getServoConfig(i)->inverted ? "yes" : "no");
                        uart_puts(uart1, tx_buffer);
                        ds_reset_buffers(tx_buffer, rx_buffer);
                    }

                    break;

                case('d'):
                    snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "\nWraps: %lu, Mem: %d, Frames: %lu",
                             controller->getNumberOfPWMWraps(),
                             xPortGetFreeHeapSize(),
                             io->getNumberOfFramesReceived());
                    uart_puts(uart1, tx_buffer);
                    break;

                case('p'):
                    controller->powerToggle();

                    snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "\nPower is now %s", controller->isPoweredOn() ? "on" : "off");
                    uart_puts(uart1, tx_buffer);
                    break;

                default:
                    const char *helpMenu = "\n\n%s Debug Shell\n\n  c = show running config\n  d = show debug data\n  p = toggle power\n";

                    snprintf(tx_buffer, DS_TX_BUFFER_SIZE, helpMenu, controller->getConfig()->getName());
                    uart_puts(uart1, tx_buffer);
            }

            // Show the prompt
            uart_puts(uart1, "\n> ");

            // Wipe out the buffers for next time
            ds_reset_buffers(tx_buffer, rx_buffer);
        }
    }
#pragma clang diagnostic pop

}