
#include <cstdio>
#include <cstdarg>
#include <cstring>


#include "controller-config.h"


#include "pico/time.h"

#include "logging/logging.h"
#include "tasks.h"

#include "io/pio_uart.h"


extern TaskHandle_t stepper_uart_task_handle;

int PioUART::init(PIO tx_pio, uint8_t tx_pin, uint32_t tx_baud_rate) {

    debug("initing a PioUART");

    pio = tx_pio;
    sm = 0;
    offset = pio_add_program(pio, &uart_tx_program);
    uart_tx_program_init(pio, sm, offset, tx_pin, tx_baud_rate);

    debug("PIO uart created! sm: %d, offset: %d", sm, offset);

    return 1;

}

int PioUART::start() {


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

    auto* ourInstance = (PioUART*)pvParameters;

    uint8_t max_length = 100;
    uint8_t actual_length;

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"

    auto buffer = (char*)pvPortMalloc(max_length);
    memset(buffer, '\0', max_length);

    TickType_t xLastWakeTime;
    for (EVER) {

        // Just because I'm testing timing for PIO things
        xLastWakeTime = xTaskGetTickCount();

        snprintf(buffer, max_length-1, "Hellorld!!! %lu\n\r", to_ms_since_boot(get_absolute_time()));
        actual_length = strlen(buffer);

        /*
         * There is an "uart_tx_program_puts()" but it's non-blocking, and we
         * pass a buffer into it. Let's do it character by character, which is
         * blocking, and seems to behave a lot better.
         */

        // For now just toss a message down the pipe
        for(uint8_t i = 0; i < actual_length; i++)
            uart_tx_program_putc(ourInstance->pio, ourInstance->sm, buffer[i]);

        memset(buffer, '\0', max_length);
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(50));

    }
#pragma clang diagnostic pop
}