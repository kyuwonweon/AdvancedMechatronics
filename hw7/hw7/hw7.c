#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "math.h"

#define SPI_PORT spi_default
#define PIN_MISO 16
#define PIN_CS   15 
#define PIN_SCK  18
#define PIN_MOSI 19

void setDac(int channel, float v);
static inline void cs_select(uint cs_pin);
static inline void cs_deselect(uint cs_pin);

int main()
{
    stdio_init_all();
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    
    spi_init(SPI_PORT, 1000 * 1000); 
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    gpio_init(PIN_CS);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);

    float t = 0;
    bool led_state = false;

    while (true) {
        led_state = !led_state;
        gpio_put(PICO_DEFAULT_LED_PIN, led_state);

        t = t + 0.01;
        float voltageA = (sin(2 * M_PI * 2 * t) + 1) / 2 * 3.3;
        float ramp = fmod(t, 1.0);
        float voltageB = (ramp < 0.5) ? (ramp * 2 * 3.3) : ((1.0 - ramp) * 2 * 3.3);
        
        setDac(0, voltageA);
        setDac(1, voltageB);
        
        sleep_ms(10);
    }
}

void setDac(int channel, float v){
    uint8_t data[2];
    data[0] =  0b01110000;
    data[0] = data[0] | ((channel & 0b1) << 7);
    uint16_t theV = v / 3.3 * 1023;
    
    data[0] = data[0] | (theV >> 6);
    data[1] = (theV << 2) & 0xFF;
    
    cs_select(PIN_CS);
    spi_write_blocking(SPI_PORT, data, 2);
    cs_deselect(PIN_CS);
}

static inline void cs_select(uint cs_pin) {
    asm volatile("nop \n nop \n nop"); 
    gpio_put(cs_pin, 0);
    asm volatile("nop \n nop \n nop"); 
}

static inline void cs_deselect(uint cs_pin) {
    asm volatile("nop \n nop \n nop"); 
    gpio_put(cs_pin, 1);
    asm volatile("nop \n nop \n nop"); 
}