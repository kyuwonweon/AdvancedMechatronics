#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "ssd1306.h"
#include "font.h"

// I2C defines
// This example will use I2C0 on GPIO8 (SDA) and GPIO9 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define I2C_PORT i2c0
#define I2C_SDA 8
#define I2C_SCL 9
#define PICO_LED_PIN 25

void drawChar(uint16_t x, uint16_t y, char letter);
void drawMessage(uint16_t x, uint16_t y, char*m);


int main()
{
    stdio_init_all();

    gpio_init(PICO_LED_PIN);
    gpio_set_dir(PICO_LED_PIN, GPIO_OUT);

    // I2C Initialisation. Using it at 100Khz.
    i2c_init(I2C_PORT, 1700*1000);
    
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    // For more examples of I2C use see https://github.com/raspberrypi/pico-examples/tree/master/i2c

    ssd1306_setup();
    ssd1306_clear();
    ssd1306_update();

    adc_init();
    adc_gpio_init(26);
    adc_select_input(0);

    while (true) {
        absolute_time_t t1, t2;
        t1 = get_absolute_time();
        uint16_t raw = adc_read();
        float voltage = raw*3.3f/4095.0f;

        ssd1306_clear();
        char message[30];
        sprintf(message, "Hello 123456789012345");
        drawMessage(0,0,message);
        sprintf(message, "RoW2 123456789012345");
        drawMessage(0,8,message);
        sprintf(message, "Voltage = %.3f ", voltage);
        drawMessage(0, 16, message);

        static bool blink = false;
        blink = !blink;
        gpio_put(PICO_LED_PIN, blink);
        ssd1306_drawPixel(127, 0, blink);

        t2 = get_absolute_time();
        uint64_t ta;
        ta = to_us_since_boot(t2)-to_us_since_boot(t1);
        sprintf(message, "FPS = %6.3f ", 1.0/(ta/100000.0));
        drawMessage(0, 24, message);

        ssd1306_update();
        sleep_ms(500);
    }
}

void drawChar(uint16_t x, uint16_t y, char letter){
    int char_idx = letter - 0x20;
    for (int col=0; col<5; col++){
        char data = ASCII[char_idx][col];
        for (int row=0; row<8; row++){
            if ((data >> row) & 0x01){
                ssd1306_drawPixel(x+col, y+row, 1);
            }
            else {
                ssd1306_drawPixel(x+col, y+row, 0);
            }
        }
    }
}

void drawMessage(uint16_t x, uint16_t y, char*m){
    int i = 0;
    while (m[i] != '\0'){
        drawChar(x+(i*6), y, m[i]);
        i++;
    }
}