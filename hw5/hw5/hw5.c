#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "ssd1306.h"

// I2C defines
// This example will use I2C0 on GPIO8 (SDA) and GPIO9 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define I2C_PORT i2c0
#define I2C_SDA 8
#define I2C_SCL 9
#define PICO_LED_PIN 25

#define MPU_ADDR 0x68
#define CONFIG 0x1A
#define GYRO_CONFIG 0x1B
#define ACCEL_CONFIG 0x1C
#define PWR_MGMT_1 0x6B
#define ACCEL_XOUT_H 0x3B
#define WHO_AM_I 0x75

void mpu6050_init();
void mpu6050_read(int16_t *accel, int16_t *gyro, int16_t *temp);

int main()
{
    stdio_init_all();
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }
    printf("USB Serial Connected!\n");

    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400*1000);
    
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    // For more examples of I2C use see https://github.com/raspberrypi/pico-examples/tree/master/i2c

    gpio_init(PICO_LED_PIN);
    gpio_set_dir(PICO_LED_PIN, GPIO_OUT);

    ssd1306_setup(); 
    ssd1306_clear();
    ssd1306_update();
    
    mpu6050_init(); 

    while (true) {
        int16_t accel[3], gyro[3], temp;
        mpu6050_read(accel, gyro, &temp);

        ssd1306_clear();

        ssd1306_drawPixel(64, 16, 1); // Center
        ssd1306_drawPixel(63, 16, 1); // Left
        ssd1306_drawPixel(65, 16, 1); // Right
        ssd1306_drawPixel(64, 15, 1); // Up
        ssd1306_drawPixel(64, 17, 1); // Down

        // Scale the acceleration
        int len_x = accel[0] / 400; 
        int len_y = accel[1] / 1000; 

        // Draw X-line from center
        for(int i = 0; i < abs(len_x); i++) {
            int sign = (len_x > 0) ? 1 : -1;
            ssd1306_drawPixel(64+(i*sign), 16, 1);
        }

        // Draw Y-line from center
        for(int i = 0; i < abs(len_y); i++) {
            int sign = (len_y > 0) ? 1 : -1;
            ssd1306_drawPixel(64, 16+(i*sign), 1);
        }

        ssd1306_update();
        sleep_ms(50);
    }
}

void mpu6050_init() {
    uint8_t buf[2];
    uint8_t reg = WHO_AM_I;
    uint8_t chip_id;

    i2c_write_blocking(I2C_PORT, MPU_ADDR, &reg, 1, true);
    i2c_read_blocking(I2C_PORT, MPU_ADDR, &chip_id, 1, false);

    if (chip_id != 0x68 && chip_id != 0x98) {
        printf("ERROR - MPU6050 not found\n");
        while (1) { 
            gpio_put(PICO_LED_PIN, 1);
            sleep_ms(100);
            gpio_put(PICO_LED_PIN, 0);
            sleep_ms(100);
        }
    }

    // Reset
    buf[0] = PWR_MGMT_1; buf[1] = 0x80;
    i2c_write_blocking(I2C_PORT, MPU_ADDR, buf, 2, false);
    sleep_ms(100);

    // Wake up
    buf[0] = PWR_MGMT_1; buf[1] = 0x00;
    i2c_write_blocking(I2C_PORT, MPU_ADDR, buf, 2, false);
    sleep_ms(100);

    // Accel Config
    buf[0] = ACCEL_CONFIG; buf[1] = 0x00;
    i2c_write_blocking(I2C_PORT, MPU_ADDR, buf, 2, false);

    // Gyro Config
    buf[0] = GYRO_CONFIG; buf[1] = 0x18; 
    i2c_write_blocking(I2C_PORT, MPU_ADDR, buf, 2, false);
}

void mpu6050_read(int16_t *accel, int16_t *gyro, int16_t *temp) {
    uint8_t buffer[14];
    uint8_t reg = ACCEL_XOUT_H;

    i2c_write_blocking(I2C_PORT, MPU_ADDR, &reg, 1, true);
    i2c_read_blocking(I2C_PORT, MPU_ADDR, buffer, 14, false);

    accel[0] = (buffer[0] << 8) | buffer[1];// X
    accel[1] = (buffer[2] << 8) | buffer[3];// Y
    accel[2] = (buffer[4] << 8) | buffer[5];// Z
    *temp    = (buffer[6] << 8) | buffer[7];
    gyro[0]  = (buffer[8] << 8) | buffer[9];// X
    gyro[1]  = (buffer[10] << 8) | buffer[11];// Y
    gyro[2]  = (buffer[12] << 8) | buffer[13];// Z
}
