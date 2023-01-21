
#include <cstdio>
#include <cstring>


#include "controller-config.h"

#include "hardware/pio.h"
#include "uart_rx.pio.h"
#include "uart_tx.pio.h"

#include "pico/time.h"

#include "logging/logging.h"
#include "tasks.h"

#include "io/pio_uart.h"


int PioUART::init(PIO uart_pio, uint8_t rx_pin, uint8_t tx_pin, uint32_t baud_rate) {

    debug("initing a PioUART");

    this->pio = uart_pio;

    this->tx_offset = pio_add_program(this->pio, &uart_tx_program);
    uart_tx_program_init(this->pio, this->tx_state_machine, this->tx_offset, tx_pin, baud_rate);

    this->rx_offset = pio_add_program(this->pio, &uart_rx_program);
    uart_rx_program_init(this->pio, this->rx_state_machine, this->rx_offset, rx_pin, baud_rate);

    debug("PIO uart created! tx offset: %d (%d), rx offset: %d (%d)",
          this->tx_offset, this->tx_state_machine,
          this->rx_offset, this->rx_state_machine);

    return 1;

}

int PioUART::start() {

    // NOOP
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
            uart_tx_program_putc(ourInstance->pio, ourInstance->tx_state_machine, buffer[i]);

        memset(buffer, '\0', max_length);
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(50));

    }
#pragma clang diagnostic pop
}