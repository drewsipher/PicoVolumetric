#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"

#define D0 0
#define D1 1
#define D2 2
#define D3 3
#define D4 4
#define D5 5
#define D6 6
#define D7 7

#define CS 28
#define DC 27
#define RST 26
#define EN 22
#define RW 21

#define LED 25

void writeCommand(uint8_t command)
{
    gpio_put(DC, false); //Command low

    gpio_put(CS, false); //CS LOW to Enable
    
    //write command to D0-D7
    // gpio_put_masked(0x000000FF, 0x00000000 | command);
    gpio_put(D0, (command >> 0) & 0x01);
    gpio_put(D1, (command >> 1) & 0x01);
    gpio_put(D2, (command >> 2) & 0x01);
    gpio_put(D3, (command >> 3) & 0x01);
    gpio_put(D4, (command >> 4) & 0x01);
    gpio_put(D5, (command >> 5) & 0x01);
    gpio_put(D6, (command >> 6) & 0x01);
    gpio_put(D7, (command >> 7) & 0x01);

    gpio_put(RW, false); //clear Read Write
    sleep_us(100);
    gpio_put(RW, true); //set Read Write
    sleep_us(100);

    gpio_put(CS, true); //CS HIGH 
}

void writeData(uint8_t data)
{
    gpio_put(DC, true); //Data high

    gpio_put(CS, false); //CS LOW to Enable

    //write data to D0-D7
    //gpio_put_masked(0x000000FF, 0x00000000 | data);
    gpio_put(D0, (data >> 0) & 0x01);
    gpio_put(D1, (data >> 1) & 0x01);
    gpio_put(D2, (data >> 2) & 0x01);
    gpio_put(D3, (data >> 3) & 0x01);
    gpio_put(D4, (data >> 4) & 0x01);
    gpio_put(D5, (data >> 5) & 0x01);
    gpio_put(D6, (data >> 6) & 0x01);
    gpio_put(D7, (data >> 7) & 0x01);

    gpio_put(RW, false); //clear Read Write
    sleep_us(100);
    gpio_put(RW, true); //set Read Write
    sleep_us(100);
    gpio_put(CS, true); //CS HIGH 
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
    gpio_put(EN, false);
    gpio_put(CS, false); //clear Chip Select

    //gpio_put(RST, true);
    //sleep_ms(1);
    gpio_put(RST, false);
    sleep_ms(1);
    gpio_put(RST, true);
    sleep_ms(120);

    writeCommand(0xFD);
    writeCommand(0x12);
    writeCommand(0xAE);
    writeCommand(0xD5);
    writeCommand(0x30);
    writeCommand(0xA8);
    writeCommand(0x37);
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
    writeCommand(0x25);
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

    uint32_t outputMask = 0x00000000;
    outputMask |= 1 << D0;
    outputMask |= 1 << D1;
    outputMask |= 1 << D2;
    outputMask |= 1 << D3;
    outputMask |= 1 << D4;
    outputMask |= 1 << D5;
    outputMask |= 1 << D6;
    outputMask |= 1 << D7;

    outputMask |= 1 << CS;
    outputMask |= 1 << DC;
    outputMask |= 1 << RST;
    outputMask |= 1 << EN;
    outputMask |= 1 << RW;
    outputMask |= 1 << LED;

    gpio_init_mask(outputMask);
    gpio_set_dir_out_masked(outputMask);
    
    gpio_set_pulls(CS,false,true);
    gpio_set_pulls(DC,false,true);
    gpio_set_pulls(RST,false,true);
    gpio_set_pulls(EN,false,true);
    gpio_set_pulls(RW,false,true);
   
    Blink(2,250);
    OLED_init();
    Blink(3,150);
    while (true) {
        // gpio_put(DC,false);
        // sleep_ms(100);
        // gpio_put(DC,true);
        // sleep_ms(100);
        sleep_ms(1000);
        Fill_RAM(0xff);
        sleep_ms(2000);
        Fill_RAM(0x00);
        //writeCommand(0xE3);
        //sleep_ms(1000);
        //writeData(0x00);
        //sleep_ms(1000);

    }
    return 0;
}