
#include <cstdio>
#include <cstdarg>
#include <cstring>


#include "controller-config.h"


#include "pico/time.h"

#include "logging/logging.h"
#include "tasks.h"

#include "io/stepper_uart.h"


extern TaskHandle_t stepper_uart_task_handle;

int StepperUART::init() {

    debug("initing the StepperUART");


    pio = STEPPER_UART_PIO;
    sm = 0;
    offset = pio_add_program(pio, &uart_tx_program);
    uart_tx_program_init(pio, sm, offset, STEPPER_UART_GPIO_PIN, STEPPER_UART_BAUD_RATE);

    debug("PIO uart created!");

    return 1;

}

int StepperUART::start() {


    xTaskCreate(stepper_uart_task,
                "stepper_uart_task",
                512,
                (void*)(this),
                1,
                &stepper_uart_task_handle);

    info("started the PIO-based UART for the steppers");

    return 1;
}


portTASK_FUNCTION(stepper_uart_task, pvParameters) {

    auto* ourInstance = (StepperUART*)pvParameters;



#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"

    auto buffer = (char*)pvPortMalloc(100);
    memset(buffer, '\0', 100);


    TickType_t xLastWakeTime;

    for (EVER) {

        // Just because I'm testing timing for PIO things
        xLastWakeTime = xTaskGetTickCount();

        snprintf(buffer, 99, "Hellorld!!! %lu\n\r", to_ms_since_boot(get_absolute_time()));

        // For now just toss a message down the pipe
        uart_tx_program_puts(ourInstance->pio, ourInstance->sm, buffer);

        memset(buffer, '\0', 100);
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(50));

    }
#pragma clang diagnostic pop
}