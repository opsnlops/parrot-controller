
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

#if USE_STEPPERS
extern volatile uint64_t stepper_frame_count;
extern volatile uint64_t time_spent_in_stepper_handler;
#endif

QueueHandle_t debug_shell_incoming_keys;

bool volatile log_to_shell = false;
char* volatile pico_board_id;

DebugShell::DebugShell(Creature* creature, Controller* controller, IOHandler *io) {

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

    gpio_init(CDC_ACTIVE_PIN);
    gpio_set_dir(CDC_ACTIVE_PIN, GPIO_OUT);

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

Creature* DebugShell::getCreature() {
    return creature;
}

Controller* DebugShell::getController() {
    return controller;
}

IOHandler *DebugShell::getIOHandler() {
    return io;
}

CreatureConfig* DebugShell::getConfig() {
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
    verbose("callback from tusb that there's data there");
    uint8_t ch = tud_cdc_read_char();

    // Make sure it's not a control character that accidentally ended up in there
    if(ch >= 30  && ch <= 126)
        xQueueSendToBackFromISR(debug_shell_incoming_keys, &ch, nullptr);
    else {
        info("discarding character from shell: 0x%x", ch);
        tud_cdc_n_read_flush(itf);
    }
}

/**
 * Write a line to the CDC port
 *
 * @param line the line to write
 */
void write_to_cdc(char* line) {

    // Use the onboard LED as a "TX" light
    gpio_put(CDC_ACTIVE_PIN, true);

    cdc_send(line);

    gpio_put(CDC_ACTIVE_PIN, false);

}

void print_log_to_shell(char* logLine) {
    write_to_cdc(logLine);
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
    pico_board_id = (char *) pvPortMalloc(sizeof(char) * (2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1));
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
                    shell->showConfig(tx_buffer, rx_buffer);
                    break;

                case ('d'):
                    shell->showDebug(tx_buffer, rx_buffer);
                    break;

                case ('p'):
                    controller->powerToggle();

                    snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "\n\rPower is now %s",
                             controller->isPoweredOn() ? "on" : "off");
                    write_to_cdc(tx_buffer);
                    break;

                case ('o'):
                    controller->setOnline(!controller->isOnline());

                    snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "\n\rController is now %s",
                             controller->isOnline() ? "online" : "offline");
                    write_to_cdc(tx_buffer);
                    break;

                case('l'):

                    if(!log_to_shell) {
                        snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "\n\r\n\rEnabling logging to shell! Press 'l' again to stop. Good luck!!\n\r");
                        write_to_cdc(tx_buffer);
                        log_to_shell = true;
                        info("enabled logging to shell");
                    }
                    else {
                        snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "\n\rDisabled logging to shell! :)\n\r");
                        write_to_cdc(tx_buffer);
                        log_to_shell = false;
                        info("disabled logging to shell");
                    }
                    break;


                default:
                    const char *helpMenu = "\n\r\n\r%s Debug Shell\n\r\n\r  c = show running config\n\r  d = show debug data\n\r  l = toggle logging\n\r  p = toggle power\n\r  o = toggle online state\n\r";

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

/**
 * Show the configuration of the system on the CDC line
 *
 * @param tx_buffer the `tx_buffer` to write to
 * @param rx_buffer the `rx_buffer` to read from
 */
void DebugShell::showConfig(char* tx_buffer, uint8_t* rx_buffer) {

    snprintf(tx_buffer, DS_TX_BUFFER_SIZE,
             "\n\r\n\r Creature Controller Config\n\r --------------------------\n\r\n\r");
    write_to_cdc(tx_buffer);
    ds_reset_buffers(tx_buffer, rx_buffer);

    snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "                    Name: %s\n\r", config->getName());
    write_to_cdc(tx_buffer);
    ds_reset_buffers(tx_buffer, rx_buffer);

    snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "  Number of DMX Channels: %u\n\r", controller->getNumberOfDMXChannels());
    write_to_cdc(tx_buffer);
    ds_reset_buffers(tx_buffer, rx_buffer);

    snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "            Base Channel: %u\n\r", config->getDmxBaseChannel());
    write_to_cdc(tx_buffer);
    ds_reset_buffers(tx_buffer, rx_buffer);

#if USE_STEPPERS
    snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "      Number of Steppers: %u\n\r", config->getNumberOfSteppers());
                    write_to_cdc(tx_buffer);
                    ds_reset_buffers(tx_buffer, rx_buffer);
#endif

    snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "        Number of Servos: %u\n\r", config->getNumberOfServos());
    write_to_cdc(tx_buffer);
    ds_reset_buffers(tx_buffer, rx_buffer);

    snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "         Servo Frequency: %luHz\n\r",
             config->getServoFrequencyHz());
    write_to_cdc(tx_buffer);
    ds_reset_buffers(tx_buffer, rx_buffer);
    snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "            Stepper Loop: %uμs\n\r",
             STEPPER_LOOP_PERIOD_IN_US);
    write_to_cdc(tx_buffer);
    ds_reset_buffers(tx_buffer, rx_buffer);

    snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "                Board ID: %s\n\r", pico_board_id);
    write_to_cdc(tx_buffer);
    ds_reset_buffers(tx_buffer, rx_buffer);

    snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "        FreeRTOS Version: %s\n\r", tskKERNEL_VERSION_NUMBER);
    write_to_cdc(tx_buffer);
    ds_reset_buffers(tx_buffer, rx_buffer);

    snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "         TinyUSB Version: %u.%u.%u\n\r",
             TUSB_VERSION_MAJOR, TUSB_VERSION_MINOR, TUSB_VERSION_REVISION);
    write_to_cdc(tx_buffer);
    ds_reset_buffers(tx_buffer, rx_buffer);

    snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "        Pico SDK Version: %s\n\r", PICO_SDK_VERSION_STRING);
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
             "     ---------------------------------------------------------------------\n\r");
    write_to_cdc(tx_buffer);
    ds_reset_buffers(tx_buffer, rx_buffer);

    for (int i = 0; i < config->getNumberOfServos(); i++) {
        snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "       %-2d | %-21s |  %6d |  %6d | %.4f |      %3s\n\r",
                 i,
                 config->getServoConfig(i)->name,
                 config->getServoConfig(i)->minPulseUs,
                 config->getServoConfig(i)->maxPulseUs,
                 config->getServoConfig(i)->smoothingValue,
                 config->getServoConfig(i)->inverted ? "yes" : "no");
        write_to_cdc(tx_buffer);
    }

#if USE_STEPPERS
    snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "\n\r   Stepper Config:\n\r");
                    write_to_cdc(tx_buffer);
                    ds_reset_buffers(tx_buffer, rx_buffer);

                    snprintf(tx_buffer, DS_TX_BUFFER_SIZE,
                             "      num | slot |         name          |   steps  |  microsteps  |   dec  | inverted\n\r");
                    write_to_cdc(tx_buffer);
                    ds_reset_buffers(tx_buffer, rx_buffer);
                    snprintf(tx_buffer, DS_TX_BUFFER_SIZE,
                             "     ----------------------------------------------------------------------------------\n\r");
                    write_to_cdc(tx_buffer);
                    ds_reset_buffers(tx_buffer, rx_buffer);

                    for (int i = 0; i < config->getNumberOfSteppers(); i++) {
                        snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "       %-2d |   %-2d | %-21s |  %6lu  |      %6lu  | %5u  |      %3s\n\r",
                                 i,
                                 config->getStepperConfig(i)->slot,
                                 config->getStepperConfig(i)->name,
                                 config->getStepperConfig(i)->maxSteps,
                                 config->getStepperConfig(i)->maxMicrosteps,
                                 config->getStepperConfig(i)->decelerationAggressiveness,
                                 config->getStepperConfig(i)->inverted ? "yes" : "no");
                        write_to_cdc(tx_buffer);
                    }

#endif
}

void DebugShell::showDebug(char *tx_buffer, uint8_t *rx_buffer) {
    snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "\n\r\n\r Info:\n\r");
    write_to_cdc(tx_buffer);
    ds_reset_buffers(tx_buffer, rx_buffer);

    snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "                   power: %s\n\r",
             controller->isPoweredOn() ? "on" : "off");
    write_to_cdc(tx_buffer);
    ds_reset_buffers(tx_buffer, rx_buffer);

    snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "       controller online: %s\n\r",
             controller->isOnline() ? "yes" : "no");
    write_to_cdc(tx_buffer);
    ds_reset_buffers(tx_buffer, rx_buffer);

    snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "                   wraps: %lu\n\r", controller->getNumberOfPWMWraps());
    write_to_cdc(tx_buffer);
    ds_reset_buffers(tx_buffer, rx_buffer);

    snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "                  frames: %lu\n\r", io->getNumberOfFramesReceived());
    write_to_cdc(tx_buffer);
    ds_reset_buffers(tx_buffer, rx_buffer);

#if USE_STEPPERS
    snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "           stepper frame: %llu\n\r", stepper_frame_count);
    write_to_cdc(tx_buffer);
    ds_reset_buffers(tx_buffer, rx_buffer);

    snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "         time in stepper: %lluμs\n\r", time_spent_in_stepper_handler);
    write_to_cdc(tx_buffer);
    ds_reset_buffers(tx_buffer, rx_buffer);

    snprintf(tx_buffer, DS_TX_BUFFER_SIZE,  "    average stepper time: %lluμs\n\r", time_spent_in_stepper_handler / stepper_frame_count);
    write_to_cdc(tx_buffer);
    ds_reset_buffers(tx_buffer, rx_buffer);
#endif

    snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "                   moves: %lu\n\r", number_of_moves);
    write_to_cdc(tx_buffer);
    ds_reset_buffers(tx_buffer, rx_buffer);

    snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "                free mem: %u\n\r", xPortGetFreeHeapSize());
    write_to_cdc(tx_buffer);
    ds_reset_buffers(tx_buffer, rx_buffer);

    snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "                 min mem: %u\n\r", xPortGetMinimumEverFreeHeapSize());
    write_to_cdc(tx_buffer);
    ds_reset_buffers(tx_buffer, rx_buffer);

    snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "                  uptime: %lums\n\r",
             to_ms_since_boot(get_absolute_time()));
    write_to_cdc(tx_buffer);
    ds_reset_buffers(tx_buffer, rx_buffer);


    snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "\n\r Servos:\n\r");
    write_to_cdc(tx_buffer);
    ds_reset_buffers(tx_buffer, rx_buffer);

    snprintf(tx_buffer, DS_TX_BUFFER_SIZE,
             "      num | gpio | sl | ch |         name           |  pos  |  ctick  |  dtick\n\r");
    write_to_cdc(tx_buffer);
    ds_reset_buffers(tx_buffer, rx_buffer);
    snprintf(tx_buffer, DS_TX_BUFFER_SIZE,
             "     --------------------------------------------------------------------------\n\r");
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

#if USE_STEPPERS
    snprintf(tx_buffer, DS_TX_BUFFER_SIZE, "\n\r Steppers:\n\r");
    write_to_cdc(tx_buffer);
    ds_reset_buffers(tx_buffer, rx_buffer);

    snprintf(tx_buffer, DS_TX_BUFFER_SIZE,
             "      num | slot | slp |         name          |  cstep  |  dstep  |  d  |  last updated  |  steps taken\n\r");
    write_to_cdc(tx_buffer);
    ds_reset_buffers(tx_buffer, rx_buffer);
    snprintf(tx_buffer, DS_TX_BUFFER_SIZE,
             "     ----------------------------------------------------------------------------------------------------\n\r");
    write_to_cdc(tx_buffer);
    ds_reset_buffers(tx_buffer, rx_buffer);

    for (int i = 0; i < config->getNumberOfSteppers(); i++) {

        Stepper *s = Controller::getStepper(i);
        snprintf(tx_buffer, DS_TX_BUFFER_SIZE,
                 "      %3d |  %2d  |  %s  | %-21s |  %5lu  |  %5lu  |  %s  |  %12llu  | %12llu\n\r",
                 i,
                 s->getSlot(),
                 s->state->isAwake ? "n" : "y",
                 s->getName(),
                 s->state->currentMicrostep,
                 s->state->desiredMicrostep,
                 s->state->currentDirection ? "f" : "r",
                 s->state->updatedFrame,
                 s->state->actualSteps);
        write_to_cdc(tx_buffer);
    }
#endif

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


}