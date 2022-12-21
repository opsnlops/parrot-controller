
#pragma once

#include <climits>
#include <cstdio>
#include <unistd.h>

#include "hardware/uart.h"

#include "controller/controller.h"
#include "handler.h"

extern TaskHandle_t messageQueueReaderTaskHandle;

#define UART_ID uart1
#define UART_TX_PIN 4
#define UART_RX_PIN 5

#define BAUD_RATE 57600
#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY    UART_PARITY_NONE

#define INCOMING_CHARACTER_QUEUE_SIZE 10

/**
 * The size of the header (in bytes)
 */
#define HEADER_SIZE 3


class UART : public IOHandler {

public:

    UART();
    int init() override;
    uint32_t getNumberOfFramesReceived() override;

    static __isr void on_uart_rx();

private:
    static uint32_t messagesProcessed;
    static u_int8_t header[HEADER_SIZE];
    static QueueHandle_t incomingQueue;

    static uint16_t convert_dmx_position_to_servo_position(u_int8_t incoming_value);
    void process_uart_frame(u_int8_t *buffer);

};
