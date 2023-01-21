
#include <cstdio>
#include <cstring>

#include "pico/stdlib.h"
#include "pico/unique_id.h"

#include "shell.h"

#include "logging/logging.h"

#include "tasks.h"

extern uint32_t number_of_moves;
extern TaskHandle_t debug_console_task_handle;

DebugShell::DebugShell(Creature *creature, Controller *controller, IOHandler *io) {

    debug("created the debug shell");

    this->creature = creature;
    this->controller = controller;
    this->io = io;
    this->config = controller->getConfig();
    this->uart = new PioUART();
}


uint8_t DebugShell::init() {

    debug("init()-ing the debug shell");

    // Set up our UART
    this->uart->init(DEBUG_SHELL_UART_PIO, DEBUG_SHELL_RX,
                     DEBUG_SHELL_TX, DEBUG_SHELL_BAUD_RATE);

    return 1;
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

Creature *DebugShell::getCreature() {
    return creature;
}

Controller *DebugShell::getController() {
    return controller;
}

IOHandler *DebugShell::getIOHandler() {
    return io;
}

CreatureConfig *DebugShell::getConfig() {
    return config;
}

PioUART *DebugShell::getUart() {
    return uart;
}

void ds_reset_buffers(char *tx_buffer, uint8_t *rx_buffer) {
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
    auto uart = shell->getUart();

    // Grab our unique_id
    pico_unique_board_id_t board_id;
    pico_get_unique_board_id(&board_id);
    auto pico_board_id = (char *) pvPortMalloc(sizeof(char) * (2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1));
    memset(pico_board_id, '\0', 2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1);
    pico_get_unique_board_id_string(pico_board_id, 2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1);


    // Set up our buffers on the heap
    auto tx_buffer = (char*)pvPortMalloc(sizeof(char) * DS_TX_BUFFER_SIZE);
    auto rx_buffer = (uint8_t*)pvPortMalloc(sizeof(uint8_t) * DS_RX_BUFFER_SIZE);   // The pico SDK wants uint8_t

    ds_reset_buffers(tx_buffer, rx_buffer);


#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
    for (EVER) {

        // Block until there's something to read
        rx_buffer[0] = uart->getc();

        // Echo back the input from the user
        uart->puts((char*)rx_buffer);

        switch (rx_buffer[0]) {

            case ('c'):

                snprintf(tx_buffer, DS_TX_BUFFER_SIZE,
                         "\n\r\n\r Creature Controller Config\n\r --------------------------\n\r\n\r");
                uart->puts(tx_buffer);
                ds_reset_buffers(tx_buffer, rx_buffer);

                snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "              Name: %s\n\r", config->getName());
                uart->puts(tx_buffer);
                ds_reset_buffers(tx_buffer, rx_buffer);

                snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "  Number of Servos: %d\n\r", config->getNumberOfServos());
                uart->puts(tx_buffer);
                ds_reset_buffers(tx_buffer, rx_buffer);

                snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "      Base Channel: %d\n\r", config->getDmxBaseChannel());
                uart->puts(tx_buffer);
                ds_reset_buffers(tx_buffer, rx_buffer);

                snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "   Servo Frequency: %luHz\n\r", config->getServoFrequencyHz());
                uart->puts(tx_buffer);
                ds_reset_buffers(tx_buffer, rx_buffer);

                snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "          Board ID: %s\n\r", pico_board_id);
                uart->puts(tx_buffer);
                ds_reset_buffers(tx_buffer, rx_buffer);

                snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "\n\r   Servo Config:\n\r");
                uart->puts(tx_buffer);
                ds_reset_buffers(tx_buffer, rx_buffer);

                snprintf(tx_buffer, DS_TX_BUFFER_SIZE,
                         "      num |        name           |   min   |   max   | smooth | inverted\n\r");
                uart->puts(tx_buffer);
                ds_reset_buffers(tx_buffer, rx_buffer);
                snprintf(tx_buffer, DS_TX_BUFFER_SIZE,
                         "      ---------------------------------------------------------------------\n\r");
                uart->puts(tx_buffer);
                ds_reset_buffers(tx_buffer, rx_buffer);

                for (int i = 0; i < config->getNumberOfServos(); i++) {
                    snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "       %-2d | %-21s |  %6d |  %6d | %.4f |   %3s\n\r",
                             i,
                             config->getServoConfig(i)->name,
                             config->getServoConfig(i)->minPulseUs,
                             config->getServoConfig(i)->maxPulseUs,
                             config->getServoConfig(i)->smoothingValue,
                             config->getServoConfig(i)->inverted ? "yes" : "no");
                    uart->puts(tx_buffer);
                }

                break;

            case ('d'):

                snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "\n\r\n\r Info:\n\r");
                uart->puts(tx_buffer);
                ds_reset_buffers(tx_buffer, rx_buffer);

                snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "     power: %s\n\r", controller->isPoweredOn() ? "on" : "off");
                uart->puts(tx_buffer);
                ds_reset_buffers(tx_buffer, rx_buffer);

                snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "     wraps: %lu\n\r", controller->getNumberOfPWMWraps());
                uart->puts(tx_buffer);
                ds_reset_buffers(tx_buffer, rx_buffer);

                snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "    frames: %lu\n\r", io->getNumberOfFramesReceived());
                uart->puts(tx_buffer);
                ds_reset_buffers(tx_buffer, rx_buffer);

                snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "     moves: %lu\n\r", number_of_moves);
                uart->puts(tx_buffer);
                ds_reset_buffers(tx_buffer, rx_buffer);

                snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "  free mem: %d\n\r", xPortGetFreeHeapSize());
                uart->puts(tx_buffer);
                ds_reset_buffers(tx_buffer, rx_buffer);

                snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "   min mem: %d\n\r", xPortGetMinimumEverFreeHeapSize());
                uart->puts(tx_buffer);
                ds_reset_buffers(tx_buffer, rx_buffer);

                snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "    uptime: %lums\n\r", to_ms_since_boot(get_absolute_time()));
                uart->puts(tx_buffer);
                ds_reset_buffers(tx_buffer, rx_buffer);


                snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "\n\r Servos:\n\r");
                uart->puts(tx_buffer);
                ds_reset_buffers(tx_buffer, rx_buffer);

                snprintf(tx_buffer, DS_TX_BUFFER_SIZE,
                         "      num | gpio | sl | ch |         name           |  pos  |  ctick  |  dtick \n\r");
                uart->puts(tx_buffer);
                ds_reset_buffers(tx_buffer, rx_buffer);
                snprintf(tx_buffer, DS_TX_BUFFER_SIZE,
                         "      --------------------------------------------------------------------------\n\r");
                uart->puts(tx_buffer);
                ds_reset_buffers(tx_buffer, rx_buffer);

                for (int i = 0; i < config->getNumberOfServos(); i++) {

                    Servo *s = Controller::getServo(i);
                    snprintf(tx_buffer, DS_TX_BUFFER_SIZE,
                             "      %3d |  %2d  | %2d |  %s |  %-21s |  %4d |  %5lu  |  %5lu\n\r",
                             i,
                             controller->getPinMapping(i),
                             s->getSlice(),
                             s->getChannel() == 0 ? "A" : "B",
                             s->getName(),
                             s->getPosition(),
                             s->getCurrentTick(),
                             s->getDesiredTick());
                    uart->puts(tx_buffer);
                }


                break;

            case ('p'):
                controller->powerToggle();

                snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "\n\rPower is now %s", controller->isPoweredOn() ? "on" : "off");
                uart->puts(tx_buffer);
                break;

            default:
                const char *helpMenu = "\n\r\n\r%s Debug Shell\n\r\n\r  c = show running config\n\r  d = show debug data\n\r  p = toggle power\n\r";

                snprintf(tx_buffer, DS_TX_BUFFER_SIZE, helpMenu, controller->getConfig()->getName());
                uart->puts(tx_buffer);
        }

        // Show the prompt
        uart->puts("\n\r> ");

        // Wipe out the buffers for next time
        ds_reset_buffers(tx_buffer, rx_buffer);
    }
#pragma clang diagnostic pop

}