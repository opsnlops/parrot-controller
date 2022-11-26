
#include "controller.h"

#include <cstdio>

#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include "pico/stdlib.h"
#include "hardware/irq.h"
#include "hardware/uart.h"

#include "logging/logging.h"


// Let's use just a normal UART for now while I get my feet under me
// with DMX
#define UART_ID uart1
#define UART_TX_PIN 4
#define UART_RX_PIN 5

// Serial port parameters
#define BAUD_RATE 115200
#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY    UART_PARITY_NONE

uint32_t chars_rxed = 0;


// Setup a queue for incoming messages
QueueHandle_t incomingQueue = nullptr;


// RX interrupt handler
void on_uart_rx() {

    while (uart_is_readable(UART_ID)) {

        uint8_t ch = uart_getc(UART_ID);

        // Drop it into a queue
        xQueueSendToBackFromISR(incomingQueue, &ch, NULL);
        chars_rxed++;
    }
}


int main() {

    // All the SDK to bring up the stdio stuff so we can write to the serial port
    stdio_init_all();

    incomingQueue = xQueueCreate(INCOMING_CHARACTER_QUEUE_SIZE, sizeof(uint8_t));

    //logger_init();
    debug("Logging running!");

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


    // Start the task to read the queue
    TaskHandle_t messageReaderTaskHandle;
    xTaskCreate(messageQueueReaderTask,
                "messageQueueReaderTask",
                10240,
                nullptr,
                1,
                &messageReaderTaskHandle);


    // Start the task to print Hellolrd to the UART as a heartbeat
    TaskHandle_t hellorldTaskHandle;
    xTaskCreate(hellorldTask,
                "hellorldTask",
                10240,
                nullptr,
                1,
                &hellorldTaskHandle);


    TaskHandle_t updateUpdateTaskHandle;
    xTaskCreate(displayUpdateTask,
                "displayUpdateTask",
                10240,
                nullptr,
                1,
                &updateUpdateTaskHandle);


    vTaskStartScheduler();
}


// Read from the queue and print it to the screen for now
portTASK_FUNCTION(messageQueueReaderTask, pvParameters) {

    uint8_t incoming;

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
    for (EVER) {
        if (xQueueReceive(incomingQueue, &incoming, (TickType_t) portMAX_DELAY) == pdPASS) {
            debug("%c", (char) incoming);
        }
    }
#pragma clang diagnostic pop

}

// Read from the queue and print it to the screen for now
portTASK_FUNCTION(hellorldTask, pvParameters) {
#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
    for (EVER) {

        uart_puts(UART_ID, "Hellorld!\n\r");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
#pragma clang diagnostic pop
}
