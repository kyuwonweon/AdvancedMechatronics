#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#define I2C_PORT i2c0
#define I2C_SDA 8
#define I2C_SCL 9
#define ADDR 0x20 
#define PICO_LED_PIN 25
void set_mcp_register(uint8_t reg, uint8_t value);
uint8_t read_mcp_register(uint8_t reg);


int main()
{
    stdio_init_all();
    gpio_init(PICO_LED_PIN);
    gpio_set_dir(PICO_LED_PIN, GPIO_OUT);

    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400*1000);
    
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    // For more examples of I2C use see https://github.com/raspberrypi/pico-examples/tree/master/i2c
    sleep_ms(100);
    set_mcp_register(0x00, 0x7F); //

    while (true) {
        static bool blink = false;
        gpio_put(PICO_LED_PIN, blink);
        blink = !blink;
        uint8_t status = read_mcp_register(0x09); 

        if (!(status & 0x01)) { // If bit 0 is LOW
            set_mcp_register(0x0A, 0x80); // LED ON
        } else {
            set_mcp_register(0x0A, 0x00); // LED OFF
        }
        sleep_ms(250);
    }
}


void set_mcp_register(uint8_t reg, uint8_t value) {
    uint8_t buf[2] = {reg, value};
    i2c_write_blocking(I2C_PORT, ADDR, buf, 2, false);
}

uint8_t read_mcp_register(uint8_t reg) {
    uint8_t buf;
    i2c_write_blocking(I2C_PORT, ADDR, &reg, 1, true);
    i2c_read_blocking(I2C_PORT, ADDR, &buf, 1, false);
    return buf;
}