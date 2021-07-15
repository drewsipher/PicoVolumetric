#include <stdio.h>
#include <math.h>
#include <vector>
#include <cstdlib>
#include "pico/stdlib.h"
#include "pico/binary_info.h"

#define screenWidth 128
#define screenHeight 56
#define screenLines screenHeight/8

struct pixel {
    float x;
    float y;
    float z;  
    uint8_t count;  
};

struct screenData 
{
    uint8_t data[screenWidth][screenLines];
};

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



void DrawScreens();
void ClearScreens();
void DrawPixel(pixel &p);




screenData screens[numberOfScreens];



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

void DrawToAll(uint8_t data)
{
    uint8_t i, j;
    for (i = 0; i < 8; i++)
    {
        SetStartPage(i);
        SetStartColumn(0x00);
        for (j = 0; j < 128; j++)
        {
            writeData(data);
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

    DrawToAll(0x00);

    writeCommand(0xAF);

    deselectAll();
}

void ClearScreens()
{
    for (int i = 0; i < numberOfScreens; i++)
    {
        for (int x = 0; x < screenWidth; x++)
        {
            for (int y = 0; y < screenLines; y++)
            {
                screens[i].data[x][y] = 0x00;
            }
        }
    }
}

void DrawScreens()
{
    
    for (int i = 0; i < numberOfScreens; i++)
    {
        selectScreen(i);

        for (int y = 0; y < screenLines; y++)
        {
            SetStartPage(y);
            SetStartColumn(0x00);
            for (int x = 0; x < screenWidth; x++)
            {
                writeData(screens[i].data[x][y]);
            }
        }
    }
}

void DrawPixel(int x, int y, int z)
{
    screens[z].data[x][y/8] |= 1 << (y % 8);
}

void DrawPixel(pixel &p)
{
    DrawPixel(p.x,p.y,p.z);
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

bool IsPixelOutOfDisplay(pixel &p)
{
    return (p.z >= numberOfScreens || p.z < 0 || p.x < 0 || p.x >= screenWidth || p.y < 0 || p.y >= screenHeight);
}

std::vector<pixel> pixels;

void AnimateRandomPoints()
{
        ClearScreens();
        
        pixel p = {float(rand() % screenWidth), float(rand() % screenHeight), float(rand() % numberOfScreens), 0};
        pixels.push_back(p);

        for (int i = 0; i < pixels.size(); i++)
        {
            DrawPixel(pixels[i]);
        }

        DrawScreens();
        
        for (int i = 0; i < pixels.size(); i++)
        {
            pixels[i].count++;
        }
        for (int i = 0; i < pixels.size(); i++)
        {
            if (pixels[i].count > 100)
            {
                pixels.erase(pixels.begin() + i);
                i--;
            }
        }
}

const float centerX = screenWidth/2;
const float centerY = screenHeight/2;

void AnimateWarpDrive(float speed)
{
        ClearScreens();
        int absSpeed = std::abs(speed);
        int direction = speed < 0 ? -1 : 1;

        for (int i = 0; i < pixels.size(); i++)
        {
            pixel p = pixels[i];
            float xOffset = (((float)p.x / centerX) - 1.0f) * 1.5f;
            float yOffset = (((float)p.y / centerY) - 1.0f) * 1.5f;
            //for (int s = 0; s <= absSpeed; s++)
            // {
                // float z = p.z + s*direction;
                // if (z >= numberOfScreens || z < 0)
                //     continue;
            //    DrawPixel(p.x, p.y, z);
            // }
              DrawPixel(p.x, p.y, p.z);
            pixels[i].z += speed;
            pixels[i].x += xOffset * speed;
            pixels[i].y += yOffset * speed;
        }

        DrawScreens();
        
        
        for (int i = 0; i < pixels.size(); i++)
        {
            pixel p = pixels[i];
            if (IsPixelOutOfDisplay(p))
            {
                pixels[i].z = direction < 0 ? numberOfScreens - 1 : 0;
                pixels[i].x = (rand() % screenWidth);
                pixels[i].y = (rand() % screenHeight);
            }
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
    
    ClearScreens();
    
    
    while (true) {

        for (int f = 0; f < 300; f++)
        {
            AnimateRandomPoints();
        }

        for (int f = 30; f < 200; f++)
        {
            float perc = (float(f) / 100.0f);
            float rads = perc * M_PI;
            float cosine = (-std::cos(rads) + 1.0f);
            float sin = std::sin(rads);
            
            AnimateWarpDrive(sin*cosine);
            sleep_ms(30);
        }
        
        
        
    }
    return 0;
}


