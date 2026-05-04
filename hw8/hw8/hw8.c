#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"

#define SPI_PORT spi0
#define PIN_MISO    16
#define PIN_SCK     18
#define PIN_MOSI    19
#define PIN_CS_DAC  17
#define PIN_CS_RAM  14

static inline void cs_select(uint cs_pin){
    gpio_put(cs_pin, 0);
    sleep_us(1); // 1 entire microsecond delay (1000ns)
}

static inline void cs_deselect(uint cs_pin){
    sleep_us(1); // Ensure data is finished before pulling high
    gpio_put(cs_pin, 1);
    sleep_us(1);
}
void update_dac(uint8_t channel, float voltage);
void update_dac_from_ram(int i);
void ram_write_sine();
void spi_ram_init();
void spi_ram_write(uint16_t addr, uint8_t *data, int len);
void spi_ram_read(uint16_t addr, uint8_t* data, int len);

int main()
{
    stdio_init_all();
    spi_init(SPI_PORT, 1000*1000*2);
    
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS_DAC, GPIO_FUNC_SIO);
    gpio_set_function(PIN_CS_RAM, GPIO_FUNC_SIO);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    gpio_set_dir(PIN_CS_DAC, GPIO_OUT);
    gpio_put(PIN_CS_DAC, 1);

    gpio_set_dir(PIN_CS_RAM, GPIO_OUT);
    gpio_put(PIN_CS_RAM, 1);

    spi_ram_init();
    ram_write_sine();
    
    int i = 0;
    while (true){
        // for (i=0;i<1024;i++){
        //     // update_dac_from_ram(i*2);
        //     // sleep_ms(1);

        //     float test_voltage = (sin(2 * 3.1415926535 * i / 1024.0) + 1.0) / 2.0 * 3.3;
        //     update_dac(0, test_voltage); 
            
        //     sleep_ms(1);
        // }

        update_dac(0, 0.5); // Command DAC to output 0.5V
        sleep_ms(500);      // Wait half a second
        
        update_dac(0, 3.0); // Command DAC to output 3.0V
        sleep_ms(500);
    }
}

void update_dac(uint8_t channel, float voltage){
    uint8_t data[2];
    uint16_t data_short = 0;
    data_short = (channel&0b1)<<15;
    data_short = data_short | (0b111<<12);

    uint16_t v = voltage/3.3*1023;

    data_short = data_short | (v<<2);
    data[0] = data_short >> 8;
    data[1] = data_short & 0xFF;

    cs_select(PIN_CS_DAC);
    spi_write_blocking(SPI_PORT, data, 2);
    cs_deselect(PIN_CS_DAC);
}

void update_dac_from_ram(int i){
    uint8_t data[2];
    spi_ram_read(i, data, 2);
    cs_select(PIN_CS_DAC);
    spi_write_blocking(SPI_PORT, data, 2);
    cs_deselect(PIN_CS_DAC);
}

void ram_write_sine(){
    int i = 0;
    uint8_t data[2];
    uint16_t data_short = 0;
    uint8_t channel = 0b0;
    float voltage = 0;
    uint16_t addr = 0;

    for (i=0; i<1024; i++){
        voltage = (sin(2 * 3.1415926535 * i / 1024.0) + 1.0) / 2.0 * 3.3;
        data_short = (channel&0b1) << 15;
        data_short = data_short | (0b111<<12);
        
        uint16_t v = voltage/3.3*1023;

        data_short = data_short | (v<<2);
        data[0] = data_short >> 8;
        data[1] = data_short & 0xFF;

        spi_ram_write(addr, data, 2);
        addr = addr + 2;
    }
}

void spi_ram_init(){
    uint8_t data[2];
    int len=2;
    data[0] = 0b00000001;
    data[1] = 0b01000000;
    cs_select(PIN_CS_RAM);
    spi_write_blocking(SPI_PORT, data, len);
    cs_deselect(PIN_CS_RAM);
}

void spi_ram_write(uint16_t addr, uint8_t *data, int len){
    uint8_t packet[5];
    packet[0] = 0b00000010;
    packet[1] = addr>>8;
    packet[2] = addr&0xFF;
    packet[3] = data[0];
    packet[4] = data[1];

    cs_select(PIN_CS_RAM);
    spi_write_blocking(SPI_PORT, packet, 5);
    cs_deselect(PIN_CS_RAM);
}

void spi_ram_read(uint16_t addr, uint8_t* data, int len){
    uint8_t packet[5];
    packet[0] = 0b00000011;
    packet[1] = addr>>8;
    packet[2] = addr&0xFF;
    packet[3] = 0;
    packet[4] = 0;
    uint8_t dst[5];
    cs_select(PIN_CS_RAM);
    spi_write_read_blocking(SPI_PORT, packet, dst, 5);
    cs_deselect(PIN_CS_RAM);
    data[0] = dst[3];
    data[1] = dst[4];
}