#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/spi.h"

// #define D0 0
// #define D1 1
// #define D2 2
// #define D3 3
// #define D4 4
// #define D5 5
// #define D6 6
// #define D7 7

// #define CS 28
#define DC 27
#define RST 26
// #define EN 22
// #define RW 21

#define LED 25


static inline void cs_select() {
    asm volatile("nop \n nop \n nop");
    gpio_put(PICO_DEFAULT_SPI_CSN_PIN, 0);  // Active low
    asm volatile("nop \n nop \n nop");
}

static inline void cs_deselect() {
    asm volatile("nop \n nop \n nop");
    gpio_put(PICO_DEFAULT_SPI_CSN_PIN, 1);
    asm volatile("nop \n nop \n nop");
}

static void write_byte(uint8_t byte) {
    uint8_t buf[1];
    // buf[0] = reg & 0x7f;  // remove read bit as this is a write
    // buf[1] = data;
    buf[0] = byte;
    cs_select();
    spi_write_blocking(spi_default, buf, 1);
    cs_deselect();
    // sleep_ms(10);
}

void writeCommand(uint8_t command)
{
    gpio_put(DC, false); //Command low
    write_byte(command);
}

void writeData(uint8_t data)
{
    gpio_put(DC, true); //Data high
    write_byte(data);
}

void SetStartColumn(uint8_t d)
{
    writeCommand(0x00 + d % 16); //lower column start address for Page Addressing Mode
    writeCommand(0x10 + d / 16); //higher column start address for Page Addressing Mode
}

void SetStartPage(uint8_t d)
{
    writeCommand(0xB0 | d);
}

void Fill_RAM(uint8_t data)
{
    uint8_t i, j;
    for (i = 0; i < 8; i++)
    {
        SetStartPage(i);
        // sleep_ms(100);
        SetStartColumn(0x00);
        // sleep_ms(100);

        for (j = 0; j < 128; j++)
        {
            writeData(data);
            // sleep_ms(2);
        }
    }
}

void OLED_init()
{
    // gpio_put(EN, false);
    // gpio_put(CS, false); //clear Chip Select
    cs_deselect();

    gpio_put(RST, false);
    sleep_ms(1);
    gpio_put(RST, true);
    sleep_ms(120);

    writeCommand(0xFD);
    writeCommand(0x12);
    writeCommand(0xAE);
    writeCommand(0xD5);
    writeCommand(0xF0);
    writeCommand(0xA8);
    writeCommand(0x3F);
    writeCommand(0xD3);
    writeCommand(0x08);
    writeCommand(0x40);
    writeCommand(0xA1);
    writeCommand(0xC8);
    writeCommand(0xDA);
    writeCommand(0x12);
    writeCommand(0x81);
    writeCommand(0x4F);
    writeCommand(0xD9);
    writeCommand(0x22);
    writeCommand(0xDB);
    writeCommand(0x34);
    writeCommand(0xA4);
    writeCommand(0xA6);

    Fill_RAM(0x00);

    writeCommand(0xAF);
}




void Blink(int count, uint32_t delay)
{
    for (int i = 0; i < count; i++)
    {
        gpio_put(LED,true);
        sleep_ms(delay);
        gpio_put(LED,false);
        sleep_ms(delay);
    }
}

int main() {
    stdio_init_all();

    spi_init(spi_default, 5000 * 1000);
    gpio_set_function(PICO_DEFAULT_SPI_RX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_TX_PIN, GPIO_FUNC_SPI);

    // Make the SPI pins available to picotool
    bi_decl(bi_3pins_with_func(PICO_DEFAULT_SPI_RX_PIN, PICO_DEFAULT_SPI_TX_PIN, PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI));

    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_init(PICO_DEFAULT_SPI_CSN_PIN);
    gpio_set_dir(PICO_DEFAULT_SPI_CSN_PIN, GPIO_OUT);
    gpio_put(PICO_DEFAULT_SPI_CSN_PIN, 1);

    // Make the CS pin available to picotool
    bi_decl(bi_1pin_with_name(PICO_DEFAULT_SPI_CSN_PIN, "SPI CS"));



    gpio_init(LED);
    gpio_init(DC);
    gpio_init(RST);
    gpio_set_dir(DC, true);
    gpio_set_dir(RST, true);
    gpio_set_dir(LED, true);

    Blink(2,250);
    OLED_init();
    Blink(3,150);
    while (true) {

        Blink(1,50);
        Fill_RAM(0xff);
        sleep_ms(1000);
        Fill_RAM(0x00);
        sleep_ms(1000);
    }
    return 0;
}