#include <stdio.h>
#include <math.h>
#include <vector>
#include "pico/stdlib.h"
#include "pico/binary_info.h"

#define D0 2
#define D1 3
#define D2 4
#define D3 5
#define D4 6
#define D5 7
#define D6 8
#define D7 9

const uint32_t CS12 = 1 << 16;
const uint32_t CS11 = 1 << 17;
const uint32_t CS10 = 1 << 18;
const uint32_t CS09 = 1 << 21;
const uint32_t CS08 = 1 << 22;
const uint32_t CS07 = 1 << 28;
const uint32_t CS06 = 1 << 10;
const uint32_t CS05 = 1 << 11;
const uint32_t CS04 = 1 << 12;
const uint32_t CS03 = 1 << 13;
const uint32_t CS02 = 1 << 14;
const uint32_t CS01 = 1 << 15;

#define DC 20
#define RST 19
#define EN 27
#define RW 26

#define LED 25

#define numberOfScreens 10
uint32_t pinMaskData = 0;
uint32_t CS_All = CS01 | CS02 | CS03 | CS04 | CS05 | CS06 | CS07 | CS08 | CS09 | CS10 | CS11 | CS12;
uint32_t CSToggle = 0;
uint32_t CSbyIndex[numberOfScreens] = {CS01, CS02, CS03, CS04, CS05, CS06, CS07, CS08, CS09, CS10};

static inline void cs_select() {
    asm volatile("nop \n nop \n nop");
    gpio_put_masked(CS_All, ~CSToggle);
    // gpio_put(csArray[index], 0);  // Active low
    asm volatile("nop \n nop \n nop");
}

static inline void cs_deselect() {
    asm volatile("nop \n nop \n nop");
    gpio_put_masked(CS_All, CS_All);
    // gpio_put(csArray[index], 1);
    asm volatile("nop \n nop \n nop");
}

static inline void deselectAll(){
    CSToggle = 0;
    cs_deselect();
}

static inline void selectAll(){
    CSToggle = CS_All;
    cs_select();
}

static inline void selectScreen(uint8_t screen)
{
    CSToggle = CSbyIndex[screen];
    cs_select();
}

static void write_byte(uint8_t byte) {
    cs_select();
    
    uint32_t val = 0x0000 | (byte << D0);
    gpio_put_masked(pinMaskData, val);
    
    cs_deselect();
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
    deselectAll();

    gpio_put(RST, false);
    sleep_ms(1);
    gpio_put(RST, true);
    sleep_ms(120);

    selectAll();    

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

    deselectAll();
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
    pinMaskData = 1 << D0 | 1 << D1 | 1 << D2 | 1 << D3 | 1 << D4 | 1 << D5 | 1 << D6 | 1 << D7;
    uint32_t pinMaskAll =  1 << LED | 1 << DC | 1 << RST | 1 << EN | 1 << RW | pinMaskData | CS_All;
    
    gpio_init_mask(pinMaskAll);
    gpio_set_dir_out_masked(pinMaskAll);

    Blink(2,250);
    OLED_init();
    Blink(3,150);
    
    uint8_t screenCount = 12;
    
    while (true) {
        
        screenCount++;
        if (screenCount >= numberOfScreens)
        {
            screenCount = 0;
        }
        selectScreen(screenCount);

          for (int i = 0; i < 128; i++)
            {
                for (int y = 0; y < 8; y++)
                {
                    SetStartPage(y);
                    SetStartColumn(i);
                    writeData(0xff);
                }
                sleep_ms(10);
            }
            Fill_RAM(0x00);

            for (int y = 7; y >= 0; y--)
            {
                SetStartPage(y);
                uint8_t pixelVal = 0;
                for (int p = 7; p >= 0; p--)
                {
                    pixelVal |= 1 << p;
                    for (int i = 0; i < 128; i++)
                    {
                        SetStartColumn(i);
                        writeData(pixelVal);
                    }
                    sleep_ms(10);
                }
            }
            Fill_RAM(0x00);
        
    }
    return 0;
}