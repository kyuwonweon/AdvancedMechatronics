#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"

#define UART_ID   uart0
#define BAUD_RATE 115200
#define TX_PIN    0
#define RX_PIN    1

int main() {
    stdio_init_all();  // USB serial to computer

    // Init UART immediately so echo works regardless of USB monitor
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(RX_PIN, GPIO_FUNC_UART);

    sleep_ms(2000);  // give USB serial monitor time to connect
    printf("Pico UART echo ready\n");

    uint32_t last_ping = 0;

    while (true) {
        uint32_t now = to_ms_since_boot(get_absolute_time());

        // Send a ping to STM32 every 2 seconds so we can verify TX direction
        if (now - last_ping >= 2000) {
            last_ping = now;
            uart_puts(UART_ID, "ping\r\n");
            printf("[Pico] sent ping, uart_readable=%d\n", uart_is_readable(UART_ID));
        }

        // UART0 (from STM32) -> echo back to STM32 + print to computer
        if (uart_is_readable(UART_ID)) {
            char ch = uart_getc(UART_ID);
            uart_putc(UART_ID, ch);
            printf("[echo] %c\n", ch);
        }

        // USB serial (from computer) -> send to STM32 via UART0
        int ch = getchar_timeout_us(0);
        if (ch != PICO_ERROR_TIMEOUT) {
            uart_putc(UART_ID, (char)ch);
        }
    }
}
