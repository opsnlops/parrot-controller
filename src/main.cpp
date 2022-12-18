
#include "controller.h"

#include <cstdio>
#include <climits>

#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>
#include <cstring>

#include "creature.h"

#include "pico/stdlib.h"
#include "hardware/irq.h"
#include "hardware/pwm.h"


#include "logging/logging.h"
#include "servo.h"
#include "tasks.h"
#include "dmx.h"
#include "relay.h"


// Located in tasks.cpp
extern TaskHandle_t displayUpdateTaskHandle;
extern TaskHandle_t hellorldTaskHandle;
extern TaskHandle_t dmx_processing_task_handle;
extern TaskHandle_t servoDebugTaskHandle;
extern TaskHandle_t relayDebugTaskHandle;

// Serial port parameters (if enabled)
#ifdef USE_UART_CONTROL

    #include "hardware/uart.h"

    extern TaskHandle_t messageQueueReaderTaskHandle;

    #define UART_ID uart1
    #define UART_TX_PIN 4
    #define UART_RX_PIN 5

    #define BAUD_RATE 57600
    #define DATA_BITS 8
    #define STOP_BITS 1
    #define PARITY    UART_PARITY_NONE
#endif
// END UART_CONTROL


uint32_t bytes_received = 0;
uint32_t pwm_wraps = 0;


// Setup a queue for incoming messages
//QueueHandle_t incomingQueue = nullptr;

// Create an array of servos
Servo servos[NUMBER_OF_SERVOS];


Relay* creature_power;

#ifdef USE_UART_CONTROL
// RX interrupt handler
void on_uart_rx() {

    while (uart_is_readable(UART_ID)) {

        uint8_t value = uart_getc(UART_ID);

        // Drop it into a queue
        xQueueSendToBackFromISR(incomingQueue, &value, NULL);
        bytes_received++;
    }
}
#endif

/**
 * IRQ handler to update the duty cycle on our servos
 */
void __isr on_pwm_wrap_handler() {

    for(auto & servo : servos)
        pwm_set_chan_level(servo.slice, servo.channel, servo.desired_ticks);

    pwm_clear_irq(servos[0].slice);

    pwm_wraps++;
}


int main() {

    // All the SDK to bring up the stdio stuff, so we can write to the serial port
    stdio_init_all();

    logger_init();
    debug("Logging running!");

    // Turn off the e-stop
    creature_power = new Relay(E_STOP_PIN, true);
    creature_power->turnOn();

    // Fire up DMX
    dmx_init(DMX_GPIO_PIN);


    // Create the servos
    servo_init(&servos[0], 22, SERVO_HZ, 250, 2500, false);
    servo_init(&servos[1], 2, SERVO_HZ, 250, 2500, true);

    // Install the IRQ handler for the servos
    pwm_set_irq_enabled(servos[0].slice, true);
    irq_set_exclusive_handler(PWM_IRQ_WRAP, on_pwm_wrap_handler);
    irq_set_enabled(PWM_IRQ_WRAP, true);

    // And start them up!
    servo_on(&servos[0]);
    servo_on(&servos[1]);

#ifdef USE_UART_CONTROL
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
    xTaskCreate(messageQueueReaderTask,
                "messageQueueReaderTask",
                1024,
                nullptr,
                1,
                &messageQueueReaderTaskHandle);
#endif

    /*
    // Start the task to print Hellorld to the UART as a heartbeat
    xTaskCreate(hellorldTask,
                "hellorldTask",
                256,
                nullptr,
                1,
                &hellorldTaskHandle);
    */

    /*
    xTaskCreate(servoDebugTask,
                "servoDebugTask",
                1024,
                nullptr,
                1,
                &servoDebugTaskHandle);


    xTaskCreate(relayDebugTask,
                "relayDebugTask",
                512,
                nullptr,
                1,
                &relayDebugTaskHandle);
  */

    xTaskCreate(displayUpdateTask,
                "displayUpdateTask",
                1024,
                nullptr,
                0,          // Low priority
                &displayUpdateTaskHandle);


    xTaskCreate(dmx_processing_task,
                "dmx_processing_task",
                2048,
                nullptr,
                1,
                &dmx_processing_task_handle);


    vTaskStartScheduler();
}





// Read from the queue and print it to the screen for now
portTASK_FUNCTION(hellorldTask, pvParameters) {
#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
    for (EVER) {
#ifdef USE_UART_CONTROL
        uart_puts(UART_ID, "Hellorld!\n\r");
#endif
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
#pragma clang diagnostic pop
}


portTASK_FUNCTION(servoDebugTask, pvParameters) {
#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"

    for (EVER) {

        // Declare these here, so we're not making new ones in this tight loop
        int i, j;

        for (i = 0; i < MAX_SERVO_POSITION; i += 5) {
            for (j = 0; j < NUMBER_OF_SERVOS; j++)
                servo_move(&servos[j], i);

            vTaskDelay(pdMS_TO_TICKS(20));
        }
    }
#pragma clang diagnostic pop
}

portTASK_FUNCTION(relayDebugTask, pvParameters) {
#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"

    for (EVER) {

        // Test toggling the power on and off to the servos
        vTaskDelay(pdMS_TO_TICKS(10000));
        creature_power->toggle();

    }
#pragma clang diagnostic pop
}


portTASK_FUNCTION(dmx_processing_task, pvParameters) {
#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"

    uint32_t ulNotifiedValue;
    for (EVER) {

        xTaskNotifyWait(0x00, ULONG_MAX, &ulNotifiedValue, portMAX_DELAY);
        verbose("DMX says hi");


    }
#pragma clang diagnostic pop
}