#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "hardware/adc.h"


#define PICO_LED0 2
#define PICO_LED1 3
#define PICO_LED2 4
#define PICO_LED3 5


#define SWITCH_PIN 6
#define POT_PIN 26


#define SPI_PORT spi0
#define SPI_CS   17
#define SPI_SCK  18
#define SPI_MOSI 19

void pico_leds_off() {
    gpio_put(PICO_LED0, 0);
    gpio_put(PICO_LED1, 0);
    gpio_put(PICO_LED2, 0);
    gpio_put(PICO_LED3, 0);
}

void write_pico_leds(uint8_t value) {
    gpio_put(PICO_LED0, (value >> 0) & 1);
    gpio_put(PICO_LED1, (value >> 1) & 1);
    gpio_put(PICO_LED2, (value >> 2) & 1);
    gpio_put(PICO_LED3, (value >> 3) & 1);
}

void send_to_fpga(uint8_t value) {
    uint8_t data = value & 0x0F;

    gpio_put(SPI_CS, 0);
    sleep_us(10);

    spi_write_blocking(SPI_PORT, &data, 1);

    sleep_us(10);
    gpio_put(SPI_CS, 1);
}

void show_pattern(uint8_t pattern) {
    uint8_t pico_bits = pattern & 0x0F;
    uint8_t fpga_bits = (pattern >> 4) & 0x0F;

    write_pico_leds(pico_bits);
    send_to_fpga(fpga_bits);
}

int get_delay_ms() {
    uint16_t raw = adc_read();

    int step = (raw * 9) / 4095;
    int delay_ms = 1000 - (step * 100);

    if (delay_ms < 100) {
        delay_ms = 100;
    }

    if (delay_ms > 1000) {
        delay_ms = 1000;
    }

    return delay_ms;
}

int main() {
    stdio_init_all();


    gpio_init(PICO_LED0);
    gpio_init(PICO_LED1);
    gpio_init(PICO_LED2);
    gpio_init(PICO_LED3);

    gpio_set_dir(PICO_LED0, GPIO_OUT);
    gpio_set_dir(PICO_LED1, GPIO_OUT);
    gpio_set_dir(PICO_LED2, GPIO_OUT);
    gpio_set_dir(PICO_LED3, GPIO_OUT);

    pico_leds_off();

    gpio_init(SWITCH_PIN);
    gpio_set_dir(SWITCH_PIN, GPIO_IN);
    gpio_pull_up(SWITCH_PIN);

    adc_init();
    adc_gpio_init(POT_PIN);
    adc_select_input(0);

    //spi
    spi_init(SPI_PORT, 100 * 1000);

    spi_set_format(
        SPI_PORT,
        8,
        SPI_CPOL_0,
        SPI_CPHA_0,
        SPI_MSB_FIRST
    );

    gpio_set_function(SPI_SCK, GPIO_FUNC_SPI);
    gpio_set_function(SPI_MOSI, GPIO_FUNC_SPI);

    gpio_init(SPI_CS);
    gpio_set_dir(SPI_CS, GPIO_OUT);
    gpio_put(SPI_CS, 1);

    uint8_t pattern1[] = {
        0x01,
        0x02,
        0x04,
        0x08,
        0x10,
        0x20,
        0x40,
        0x80
    };

    uint8_t pattern2[] = {
        0x5A,
        0xA5
    };

    int index1 = 0;
    int index2 = 0;

    int last_mode = -1;

    while (true) {
        int delay = get_delay_ms();

        //active LOW
        int mode = !gpio_get(SWITCH_PIN);

        //if switch changed modes, reset indexes
        if (mode != last_mode) {
            index1 = 0;
            index2 = 0;
            show_pattern(0x00);
            sleep_ms(100);
            last_mode = mode;
        }

        if (mode == 0) {
            // snake line
            show_pattern(pattern1[index1]);

            index1++;
            if (index1 >= 8) {
                index1 = 0;
            }
        } else {
            // christmas lights
            show_pattern(pattern2[index2]);

            index2++;
            if (index2 >= 2) {
                index2 = 0;
            }
        }

        sleep_ms(delay);
    }
}