#include "hx711.h"
#include "pico/stdlib.h"

#define HX711_SCK_PIN 2
#define HX711_DT_PIN  3

void hx711_init(void) {
    gpio_init(HX711_SCK_PIN);
    gpio_set_dir(HX711_SCK_PIN, GPIO_OUT);
    gpio_put(HX711_SCK_PIN, 0);

    gpio_init(HX711_DT_PIN);
    gpio_set_dir(HX711_DT_PIN, GPIO_IN);
}

int32_t hx711_read_raw(void) {
    // Wait until DT goes low (conversion ready)
    while (gpio_get(HX711_DT_PIN)) {
        tight_loop_contents();
    }

    uint32_t raw = 0;
    // 25 pulses: 24 data bits + 1 to select channel A / gain 128
    for (int i = 0; i < 25; i++) {
        gpio_put(HX711_SCK_PIN, 1);
        busy_wait_us(1);
        if (i < 24) {
            raw = (raw << 1) | gpio_get(HX711_DT_PIN);
        }
        gpio_put(HX711_SCK_PIN, 0);
        busy_wait_us(1);
    }

    // sign-extend 24-bit two's complement to 32-bit signed int
    if (raw & 0x800000) {
        raw |= 0xFF000000;
    }
    return (int32_t)raw;
}
