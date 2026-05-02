#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "bsp/board.h"
#include "tusb.h"
#include "usb_descriptors.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"

// I2C defines
#define I2C_PORT i2c0
#define I2C_SDA 8
#define I2C_SCL 9

#define PICO_LED_PIN 25
#define BUTTON_PIN 15

#define MPU_ADDR 0x68
#define CONFIG 0x1A
#define GYRO_CONFIG 0x1B
#define ACCEL_CONFIG 0x1C
#define PWR_MGMT_1 0x6B
#define ACCEL_XOUT_H 0x3B
#define WHO_AM_I 0x75

// Function prototypes
void mpu6050_init(void);
void mpu6050_read(int16_t *accel, int16_t *gyro, int16_t *temp);
void hid_task(void);

// Global state variables
bool is_circle_mode = false;
absolute_time_t last_button_press;

int main(void) {
    // Initialize USB and Board
    board_init();
    tusb_init();

    sleep_ms(250);

    // I2C Initialization
    i2c_init(I2C_PORT, 400*1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // LED and Button setup
    gpio_init(PICO_LED_PIN);
    gpio_set_dir(PICO_LED_PIN, GPIO_OUT);
    
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);

    // Initialize the IMU
    mpu6050_init();

    last_button_press = get_absolute_time();

    while (1) {
        tud_task();
        hid_task();
    }
    return 0;
}

void hid_task(void) {
    // Poll every 10ms
    const uint32_t interval_ms = 10;
    static uint32_t start_ms = 0;

    if ( board_millis() - start_ms < interval_ms) return;
    start_ms += interval_ms;

    // Handle button press with simple 200ms debounce
    if (gpio_get(BUTTON_PIN) == 0) { 
        if (absolute_time_diff_us(last_button_press, get_absolute_time()) > 200000) {
            is_circle_mode = !is_circle_mode; // Toggle the mode
            gpio_put(PICO_LED_PIN, is_circle_mode); // Toggle LED to show mode
            last_button_press = get_absolute_time();
        }
    }

    if ( tud_suspended() && gpio_get(BUTTON_PIN) == 0 ) tud_remote_wakeup();
    if ( !tud_hid_ready() ) return;

    int8_t speed_x = 0;
    int8_t speed_y = 0;

    if (is_circle_mode) {
        static float angle = 0.0f;
        angle += 0.1f;
        speed_x = (int8_t)(5.0f * cos(angle));
        speed_y = (int8_t)(5.0f * sin(angle));
    } else {
        int16_t accel[3], gyro[3], temp;
        mpu6050_read(accel, gyro, &temp);

        // Map X accel
        if (accel[0] > 12000) speed_x = -5;
        else if (accel[0] > 6000) speed_x = -3;
        else if (accel[0] > 2000) speed_x = -1;
        else if (accel[0] < -12000) speed_x = 5;
        else if (accel[0] < -6000) speed_x = 3;
        else if (accel[0] < -2000) speed_x = 1;
        else speed_x = 0;  // Flat

        // Map y accel
        if (accel[1] > 12000) speed_y = 5;=
        else if (accel[1] > 6000) speed_y = 3;
        else if (accel[1] > 2000) speed_y = 1;
        else if (accel[1] < -12000) speed_y = -5;
        else if (accel[1] < -6000) speed_y = -3;
        else if (accel[1] < -2000) speed_y = -1;
        else speed_y = 0;
    }
    tud_hid_mouse_report(REPORT_ID_MOUSE, 0x00, speed_x, speed_y, 0, 0);
}

uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen) { return 0; }
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize) {}

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