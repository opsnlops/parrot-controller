
#include <cstdio>
#include <cstring>


#include <FreeRTOS.h>
#include <queue.h>

#include "pico/stdlib.h"
#include "pico/unique_id.h"

#include "shell.h"

#include "logging/logging.h"
#include "usb/usb.h"

#include "tasks.h"

extern uint32_t number_of_moves;
extern TaskHandle_t debug_console_task_handle;

QueueHandle_t debug_shell_incoming_keys;

DebugShell::DebugShell(Creature *creature, Controller *controller, IOHandler *io) {

    debug("created the debug shell");

    this->creature = creature;
    this->controller = controller;
    this->io = io;
    this->config = controller->getRunningConfig();
}


uint8_t DebugShell::init() {

    debug("init()-ing the debug shell");

    // Create a small queue
    debug_shell_incoming_keys = xQueueCreate(1, sizeof(uint8_t));
    vQueueAddToRegistry(debug_shell_incoming_keys, "debug_shell_incoming_keys");

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    return 1;
}


uint8_t DebugShell::start() {

    debug("starting the debug shell");

    xTaskCreate(debug_console_task,
                "debug_console_task",
                512,
                (void *) this,         // Pass in a reference to ourselves
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

void ds_reset_buffers(char *tx_buffer, uint8_t *rx_buffer) {
    memset(rx_buffer, '\0', DS_RX_BUFFER_SIZE);
    memset(tx_buffer, '\0', DS_TX_BUFFER_SIZE);
}

/**
 * Invoked from TUSB when there's data to be read
 *
 * This prevents us from having to poll, which is nice!
 *
 * @param itf the CDC interface number
 */
void tud_cdc_rx_cb(uint8_t itf) {
    debug("callback from tusb that there's data there");
    uint8_t ch = tud_cdc_read_char();
    xQueueSendToBack(debug_shell_incoming_keys, &ch, (TickType_t) 10);
}

/**
 * Write a line to the CDC port
 *
 * TODO: This isn't the best way to do this, but it works. Figure something else out that's faster.
 *
 * @param line the line to write
 */
void write_to_cdc(char* line) {

    uint32_t count = strlen(line);
    for(int i = 0; i < count; i++) {

        // Use the onboard LED as a "TX" light
        gpio_put(LED_PIN, true);

        tud_cdc_n_write_char(0, line[i]);
        tud_cdc_n_write_flush(0);

        // If we go to fast it overwhelms the buffers
        vTaskDelay(1);

        gpio_put(LED_PIN, false);
    }

}

portTASK_FUNCTION(debug_console_task, pvParameters) {

    debug("hello from the debug shell task!");

    auto shell = (DebugShell *) pvParameters;

    auto creature = shell->getCreature();
    auto controller = shell->getController();
    auto io = shell->getIOHandler();
    auto config = shell->getConfig();

    // Our prompt
    char prompt[5] = "\n\r> ";

    // Grab our unique_id
    pico_unique_board_id_t board_id;
    pico_get_unique_board_id(&board_id);
    auto pico_board_id = (char *) pvPortMalloc(sizeof(char) * (2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1));
    memset(pico_board_id, '\0', 2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1);
    pico_get_unique_board_id_string(pico_board_id, 2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1);


    // Set up our buffers on the heap
    auto tx_buffer = (char *) pvPortMalloc(sizeof(char) * DS_TX_BUFFER_SIZE);
    auto rx_buffer = (uint8_t *) pvPortMalloc(sizeof(uint8_t) * DS_RX_BUFFER_SIZE);   // The pico SDK wants uint8_t

    ds_reset_buffers(tx_buffer, rx_buffer);


#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
    for (EVER) {

        uint8_t ch;
        if (xQueueReceive(debug_shell_incoming_keys, &ch, (TickType_t) portMAX_DELAY) == pdPASS) {

            // Look at just the first keypress
            rx_buffer[0] = ch;

            // Echo back the input from the user
            tud_cdc_n_write_char(0, rx_buffer[0]);

            switch (rx_buffer[0]) {

                case ('c'):

                    snprintf(tx_buffer, DS_TX_BUFFER_SIZE,
                             "\n\r\n\r Creature Controller Config\n\r --------------------------\n\r\n\r");
                    write_to_cdc(tx_buffer);
                    ds_reset_buffers(tx_buffer, rx_buffer);

                    snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "                    Name: %s\n\r", config->getName());
                    write_to_cdc(tx_buffer);
                    ds_reset_buffers(tx_buffer, rx_buffer);

                    snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "      Number of Steppers: %d\n\r", config->getNumberOfSteppers());
                    write_to_cdc(tx_buffer);
                    ds_reset_buffers(tx_buffer, rx_buffer);

                    snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "  Number of DMX Channels: %d\n\r", controller->getNumberOfDMXChannels());
                    write_to_cdc(tx_buffer);
                    ds_reset_buffers(tx_buffer, rx_buffer);

                    snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "        Number of Servos: %d\n\r", config->getNumberOfServos());
                    write_to_cdc(tx_buffer);
                    ds_reset_buffers(tx_buffer, rx_buffer);

                    snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "            Base Channel: %d\n\r", config->getDmxBaseChannel());
                    write_to_cdc(tx_buffer);
                    ds_reset_buffers(tx_buffer, rx_buffer);

                    snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "         Servo Frequency: %luHz\n\r",
                             config->getServoFrequencyHz());
                    write_to_cdc(tx_buffer);
                    ds_reset_buffers(tx_buffer, rx_buffer);

                    snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "                Board ID: %s\n\r", pico_board_id);
                    write_to_cdc(tx_buffer);
                    ds_reset_buffers(tx_buffer, rx_buffer);

                    snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "\n\r   Servo Config:\n\r");
                    write_to_cdc(tx_buffer);
                    ds_reset_buffers(tx_buffer, rx_buffer);

                    snprintf(tx_buffer, DS_TX_BUFFER_SIZE,
                             "      num |        name           |   min   |   max   | smooth | inverted\n\r");
                    write_to_cdc(tx_buffer);
                    ds_reset_buffers(tx_buffer, rx_buffer);
                    snprintf(tx_buffer, DS_TX_BUFFER_SIZE,
                             "      ---------------------------------------------------------------------\n\r");
                    write_to_cdc(tx_buffer);
                    ds_reset_buffers(tx_buffer, rx_buffer);

                    for (int i = 0; i < config->getNumberOfServos(); i++) {
                        snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "       %-2d | %-21s |  %6d |  %6d | %.4f |   %3s\n\r",
                                 i,
                                 config->getServoConfig(i)->name,
                                 config->getServoConfig(i)->minPulseUs,
                                 config->getServoConfig(i)->maxPulseUs,
                                 config->getServoConfig(i)->smoothingValue,
                                 config->getServoConfig(i)->inverted ? "yes" : "no");
                        write_to_cdc(tx_buffer);
                    }

                    snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "\n\r   Stepper Config:\n\r");
                    write_to_cdc(tx_buffer);
                    ds_reset_buffers(tx_buffer, rx_buffer);

                    snprintf(tx_buffer, DS_TX_BUFFER_SIZE,
                             "      num | slot |         name          |   max   | smooth | inverted\n\r");
                    write_to_cdc(tx_buffer);
                    ds_reset_buffers(tx_buffer, rx_buffer);
                    snprintf(tx_buffer, DS_TX_BUFFER_SIZE,
                             "      ---------------------------------------------------------------\n\r");
                    write_to_cdc(tx_buffer);
                    ds_reset_buffers(tx_buffer, rx_buffer);

                    for (int i = 0; i < config->getNumberOfSteppers(); i++) {
                        snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "       %-2d |   %-2d | %-21s |  %6lu | %.4f |   %3s\n\r",
                                 i,
                                 config->getStepperConfig(i)->slot,
                                 config->getStepperConfig(i)->name,
                                 config->getStepperConfig(i)->maxSteps,
                                 config->getStepperConfig(i)->smoothingValue,
                                 config->getStepperConfig(i)->inverted ? "yes" : "no");
                        write_to_cdc(tx_buffer);
                    }

                    break;

                case ('d'):

                    snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "\n\r\n\r Info:\n\r");
                    write_to_cdc(tx_buffer);
                    ds_reset_buffers(tx_buffer, rx_buffer);

                    snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "     power: %s\n\r",
                             controller->isPoweredOn() ? "on" : "off");
                    write_to_cdc(tx_buffer);
                    ds_reset_buffers(tx_buffer, rx_buffer);

                    snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "     wraps: %lu\n\r", controller->getNumberOfPWMWraps());
                    write_to_cdc(tx_buffer);
                    ds_reset_buffers(tx_buffer, rx_buffer);

                    snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "    frames: %lu\n\r", io->getNumberOfFramesReceived());
                    write_to_cdc(tx_buffer);
                    ds_reset_buffers(tx_buffer, rx_buffer);

                    snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "     moves: %lu\n\r", number_of_moves);
                    write_to_cdc(tx_buffer);
                    ds_reset_buffers(tx_buffer, rx_buffer);

                    snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "  free mem: %d\n\r", xPortGetFreeHeapSize());
                    write_to_cdc(tx_buffer);
                    ds_reset_buffers(tx_buffer, rx_buffer);

                    snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "   min mem: %d\n\r", xPortGetMinimumEverFreeHeapSize());
                    write_to_cdc(tx_buffer);
                    ds_reset_buffers(tx_buffer, rx_buffer);

                    snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "    uptime: %lums\n\r",
                             to_ms_since_boot(get_absolute_time()));
                    write_to_cdc(tx_buffer);
                    ds_reset_buffers(tx_buffer, rx_buffer);


                    snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "\n\r Servos:\n\r");
                    write_to_cdc(tx_buffer);
                    ds_reset_buffers(tx_buffer, rx_buffer);

                    snprintf(tx_buffer, DS_TX_BUFFER_SIZE,
                             "      num | gpio | sl | ch |         name           |  pos  |  ctick  |  dtick \n\r");
                    write_to_cdc(tx_buffer);
                    ds_reset_buffers(tx_buffer, rx_buffer);
                    snprintf(tx_buffer, DS_TX_BUFFER_SIZE,
                             "      --------------------------------------------------------------------------\n\r");
                    write_to_cdc(tx_buffer);
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
                        write_to_cdc(tx_buffer);
                    }

                    snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "\n\r Steppers:\n\r");
                    write_to_cdc(tx_buffer);
                    ds_reset_buffers(tx_buffer, rx_buffer);

                    snprintf(tx_buffer, DS_TX_BUFFER_SIZE,
                             "      num | slot |         name          |  cstep  |  dstep \n\r");
                    write_to_cdc(tx_buffer);
                    ds_reset_buffers(tx_buffer, rx_buffer);
                    snprintf(tx_buffer, DS_TX_BUFFER_SIZE,
                             "      --------------------------------------------------------\n\r");
                    write_to_cdc(tx_buffer);
                    ds_reset_buffers(tx_buffer, rx_buffer);

                    for (int i = 0; i < config->getNumberOfSteppers(); i++) {

                        Stepper *s = Controller::getStepper(i);
                        snprintf(tx_buffer, DS_TX_BUFFER_SIZE,
                                 "      %3d |  %2d  | %-21s |  %5lu |  %5lu\n\r",
                                 i,
                                 s->getSlot(),
                                 s->getName(),
                                 s->getCurrentStep(),
                                 s->getDesiredStep());
                        write_to_cdc(tx_buffer);
                    }

                    snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "\n\r Current Frame:\n\r");
                    write_to_cdc(tx_buffer);
                    ds_reset_buffers(tx_buffer, rx_buffer);


                    for (int i = 0; i < controller->getNumberOfDMXChannels(); i++) {
                        snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "        [%3d] %d\n\r",
                                 i,
                                 controller->getCurrentFrame()[i]);
                        write_to_cdc(tx_buffer);
                        ds_reset_buffers(tx_buffer, rx_buffer);

                    }

                    break;

                case ('p'):
                    controller->powerToggle();

                    snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "\n\rPower is now %s",
                             controller->isPoweredOn() ? "on" : "off");
                    write_to_cdc(tx_buffer);
                    break;

                default:
                    const char *helpMenu = "\n\r\n\r%s Debug Shell\n\r\n\r  c = show running config\n\r  d = show debug data\n\r  p = toggle power\n\r";

                    snprintf(tx_buffer, DS_TX_BUFFER_SIZE, helpMenu, controller->getRunningConfig()->getName());
                    write_to_cdc(tx_buffer);
            }

            // Show the prompt
            write_to_cdc(prompt);

            // Wipe out the buffers for next time
            ds_reset_buffers(tx_buffer, rx_buffer);
        }
    }
#pragma clang diagnostic pop

}