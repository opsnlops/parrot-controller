
#include "controller-config.h"
#include "controller/controller.h"

#include <climits>
#include <unistd.h>

#include <FreeRTOS.h>
#include <queue.h>

#include "creature/creature.h"

#include "pico/stdlib.h"
#include "hardware/irq.h"
#include "hardware/uart.h"

#include "uart.h"
#include "logging/logging.h"

// Statics
uint32_t UART::messagesProcessed;
u_int8_t UART::header[HEADER_SIZE];
QueueHandle_t UART::incomingQueue;


UART::UART() {

    messagesProcessed = 0;

    // HOP
    header[0] = 0x48;
    header[1] = 0x4F;
    header[2] = 0x50;

    incomingQueue = nullptr;
}

uint32_t UART::getNumberOfFramesReceived() {
    return messagesProcessed;
}

int UART::init() {

    debug("starting up the UART I/O processor");

    // Create the queue for incoming characters and register it with FreeRTOS's
    // registry for debugging purposes
    incomingQueue = xQueueCreate(INCOMING_CHARACTER_QUEUE_SIZE, sizeof(uint8_t));
    vQueueAddToRegistry(incomingQueue, "incomingQueue");

    uart_init(UART_ID, 2400);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    uint actual = uart_set_baudrate(UART_ID, BAUD_RATE);
    debug("Actual baud: %d\n", actual);

    // Set UART flow control CTS/RTS, we don't want these, so turn them off
    uart_set_hw_flow(UART_ID, false, false);

    // Set our data format
    uart_set_format(UART_ID, DATA_BITS, STOP_BITS, PARITY);

    // Turn off FIFO's - we want to do this character by character
    uart_set_fifo_enabled(UART_ID, false);

    // Set up a RX interrupt
    int UART_IRQ = UART1_IRQ;

    // And set up and enable the interrupt handlers
    irq_set_exclusive_handler(UART_IRQ, on_uart_rx);
    irq_set_enabled(UART_IRQ, true);

    // Now enable the UART to send interrupts - RX only
    uart_set_irq_enables(UART_ID, true, false);

    return 1;
}

/**
 * ISR that handles incoming characters from the external port
 */
void __isr UART::on_uart_rx() {

    while (uart_is_readable(UART_ID)) {

        uint8_t value = uart_getc(UART_ID);

        // Drop it into the queue
        xQueueSendToBackFromISR(incomingQueue, &value, NULL);
        messagesProcessed++;
    }
}

uint16_t UART::convert_dmx_position_to_servo_position(u_int8_t incoming_value) {

    float percentage_requested = (float) incoming_value / (float) UCHAR_MAX;

    uint16_t local_requested = MAX_SERVO_POSITION * percentage_requested;

    verbose("converted %d -> %d", incoming_value, local_requested);

    return local_requested;

}

void UART::process_uart_frame(u_int8_t *buffer) {

    debug("processing frame!");

    // We've already verified the last byte, so let's check the header
    if(memcmp(reinterpret_cast<char*>(buffer), reinterpret_cast<char*>(header), HEADER_SIZE) == 0)
    {
        debug("valid frame");

        for(int i = HEADER_SIZE; i < HEADER_SIZE + NUMBER_OF_SERVOS; i++)
        {
            // TODO: This should fill out the desired state to hand to the controller
            //servo_move(&servos[i - HEADER_SIZE], convert_dmx_position_to_servo_position(buffer[i]));
        }

    }
    else
    {
        warning("header not found in frame; dropping");
    }



}



#ifdef USE_UART_CONTROL


// Read from the queue and print it to the screen for now
portTASK_FUNCTION(messageQueueReaderTask, pvParameters) {

    uint8_t incoming;

    // Keep a buffer of what we've gotten
    uint8_t buffer[HEADER_SIZE + NUMBER_OF_SERVOS + 1];
    memset(&buffer, '\0', HEADER_SIZE + NUMBER_OF_SERVOS + 1);


    uint8_t current_position = 0;

    bool discard_until_null = false;

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
    for (EVER) {
        if (xQueueReceive(incomingQueue, &incoming, (TickType_t) portMAX_DELAY) == pdPASS) {
            //printf("incoming: 0x%x, current_position: %d\n", incoming, current_position);

            if (discard_until_null) {
                // is this a null?
                if (incoming != 0x00)
                    continue;
                else
                    discard_until_null = false;
            } else {

                // If we have a null, and we're not at the end of the frame, something went wrong
                if (current_position == HEADER_SIZE + NUMBER_OF_SERVOS && incoming != 0x00) {

                    warning("at the end of the frame and got something other than 0x00. something's wrong. discarding until we find a 0x00.");
                    current_position = 0;
                    memset(&buffer, '\0', HEADER_SIZE + NUMBER_OF_SERVOS + 1);

                    discard_until_null = true;

                } else {

                    // Do we still have room in the buffer?
                    if (current_position <= HEADER_SIZE + NUMBER_OF_SERVOS) {

                        // Toss this in the buffer
                        buffer[current_position++] = incoming;
                    }

                    if (current_position == HEADER_SIZE + NUMBER_OF_SERVOS + 1) {

                        debug("looks like we got a full frame, processing!");
                        process_uart_frame(buffer);

                        // Reset the buffer
                        current_position = 0;
                        memset(&buffer, '\0', HEADER_SIZE + NUMBER_OF_SERVOS + 1);
                    }
                }
            }
        }
    }
#pragma clang diagnostic pop

}

#endif