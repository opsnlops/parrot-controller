
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

void PioUART::putc(char c) {
    uart_tx_program_putc(this->pio, this->tx_state_machine, c);
}

void PioUART::puts(const char *s) const {
    uart_tx_program_puts(this->pio, this->tx_state_machine, s);
}

char PioUART::getc() {
    return uart_rx_program_getc(this->pio, this->rx_state_machine);
}
