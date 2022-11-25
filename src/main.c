
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"


// Let's use just a normal UART for now while I get my feet under me
// with DMX
#define UART_ID uart1
#define BAUD_RATE 115200
#define UART_TX_PIN 4
#define UART_RX_PIN 5


int main() {

    // Enable USB serial debugging, too
    stdio_init_all();

    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);


#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
    while(true) {

        uart_puts(UART_ID, "Hellorld!\n\r");
        printf("Hellorld!\n");

        sleep_ms(1000);
    }
#pragma clang diagnostic pop
}
