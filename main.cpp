#include <stdio.h>
#include <math.h>
#include <vector>
#include <cstdlib>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "Geometry.hpp"


#define screenWidth 128
#define screenHeight 56
#define screenLines screenHeight/8
#define numberOfScreens 10
#define screenVirtualDepth 50

const float ZScale = (float)numberOfScreens / (float)screenVirtualDepth;

class pixel {
public:
    Vector3D position = Vector3D(0,0,0);  
    int count; 

    pixel(){};
    
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


uint32_t pinMaskData = 0;
uint32_t CS_All = CS01 | CS02 | CS03 | CS04 | CS05 | CS06 | CS07 | CS08 | CS09 | CS10 | CS11 | CS12;
uint32_t CSToggle = 0;
uint32_t CSbyIndex[numberOfScreens] = {CS01, CS02, CS03, CS04, CS05, CS06, CS07, CS08, CS09, CS10};


void DrawScreens();
void ClearScreens();
void DrawPixel(Vector3D &p);


Geometry cube(ZScale);

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
    //cs_select();
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

bool IsPixelOutOfDisplay(Vector3D &p)
{
    return (p.z >= numberOfScreens || p.z < 0 || p.x < 0 || p.x >= screenWidth || p.y < 0 || p.y >= screenHeight);
}

void DrawScreens()
{
    
    for (int i = 0; i < numberOfScreens; i++)
    {
        selectScreen(i);

        for (int y = 0; y < screenLines; y++)
        {
            int startPage = y + 1;
            //if (startPage == screenLines)
            //    startPage = 0;
            SetStartPage(startPage);
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
    if (z >= numberOfScreens || z < 0 || x < 0 || x >= screenWidth || y < 0 || y >= screenHeight)
        return;
    screens[z].data[x][y/8] |= 1 << (y % 8);
}

void DrawPixel(Vector3D &p)
{
    DrawPixel(p.x,p.y,p.z);
}

void DrawLine(Vector3D &p1, Vector3D &p2)
{
    float dx = p2.x - p1.x;
    float dy = p2.y - p1.y;
    float dz = p2.z - p1.z;

    if (std::fabs(dx) >= std::fabs(dy) && std::fabs(dx) >= std::fabs(dz))
    {
        if (dx > 0)
        {
            for (float x = p1.x; x <= p2.x; x++)
            {
                float xcalc = (x - p1.x) / dx;
                int y = p1.y + dy * xcalc;
                int z = p1.z + dz * xcalc;
                DrawPixel(x,y,z);
            }
        } else {
            for (float x = p1.x; x >= p2.x; x--)
            {
                float xcalc = (x - p1.x) / dx;
                int y = p1.y + dy * xcalc;
                int z = p1.z + dz * xcalc;
                DrawPixel(x,y,z);
            }
        }
    } else if (std::fabs(dy) >= std::fabs(dz))
    {
        if (dy > 0)
        {
            for (float y = p1.y; y <= p2.y; y++)
            {
                float ycalc = (y - p1.y) / dy;
                int x = p1.x + dx * ycalc;
                int z = p1.z + dz * ycalc;
                DrawPixel(x,y,z);
            }
        } else {
            for (float y = p1.y; y >= p2.y; y--)
            {
                float ycalc = (y - p1.y) / dy;
                int x = p1.x + dx * ycalc;
                int z = p1.z + dz * ycalc;
                DrawPixel(x,y,z);
            }
        }

    } else {
        if (dz > 0)
        {
            for (float z = p1.z; z <= p2.z; z++)
            {
                float zcalc = (z - p1.z) / dz;
                int x = p1.x + dx * zcalc;
                int y = p1.y + dy * zcalc;
                DrawPixel(x,y,z);
            }
        } else {
            for (float z = p1.z; z >= p2.z; z--)
            {
                float zcalc = (z - p1.z) / dz;
                int x = p1.x + dx * zcalc;
                int y = p1.y + dy * zcalc;
                DrawPixel(x,y,z);
            }
        }
    }
}


void DrawGeometry(Geometry &geo)
{
    geo.ComputeModelMatrix();

    size_t numberOfIndices = geo.indices.size();
    assert(numberOfIndices%2 == 0);
    for (int i = 0; i < numberOfIndices; i+=2)
    {
        
        Vector3D p1 = cube.GetScreenSpaceVectorAt(cube.indices[i]);
        Vector3D p2 = cube.GetScreenSpaceVectorAt(cube.indices[i+1]);

        DrawLine(p1, p2);
    }
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



std::vector<pixel> pixels;

void AnimateRandomPoints()
{
        ClearScreens();
        std::size_t pixSize = pixels.size();

        for (int i = 0; i < pixSize; i++)
        {
            DrawPixel(pixels[i].position);
        }

        DrawScreens();
        
        for (int i = 0; i < pixSize; i++)
        {
            pixels[i].count++;
        }
        for (int i = 0; i < pixels.size(); i++)
        {
            if (pixels[i].count > 300 )
            {
                pixels.erase(pixels.begin() + i);
                i--;
            }
        }
        while(pixels.size() < 100)
        {
            pixel p;
            p.position = Vector3D(float(rand() % screenWidth), float(rand() % screenHeight), float(rand() % numberOfScreens));
            p.count = rand() % 300;
            pixels.push_back(p);
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
            float xOffset = (((float)p.position.x / centerX) - 1.0f) * 1.5f;
            float yOffset = (((float)p.position.y / centerY) - 1.0f) * 1.5f;
            //for (int s = 0; s <= absSpeed; s++)
            // {
                // float z = p.z + s*direction;
                // if (z >= numberOfScreens || z < 0)
                //     continue;
            //    DrawPixel(p.x, p.y, z);
            // }
            DrawPixel(p.position);

            pixels[i].position.z += speed;
            pixels[i].position.x += xOffset * speed;
            pixels[i].position.y += yOffset * speed;
        }

        DrawScreens();
        
        
        for (int i = 0; i < pixels.size(); i++)
        {
            pixel p = pixels[i];
            if (IsPixelOutOfDisplay(p.position))
            {
                pixels[i].position.z = direction < 0 ? numberOfScreens - 1 : 0;
                pixels[i].position.x = (rand() % screenWidth);
                pixels[i].position.y = (rand() % screenHeight);
            }
        }
        
}

void AnimateSpinningCube(float speed)
{

    ClearScreens();

    cube = Geometry(ZScale);

    float halfCubeSize = 15.0f;

    cube.points.push_back(Vector3D(-halfCubeSize, -halfCubeSize, -halfCubeSize));
    cube.points.push_back(Vector3D( halfCubeSize, -halfCubeSize, -halfCubeSize));
    cube.points.push_back(Vector3D( halfCubeSize,  halfCubeSize, -halfCubeSize));
    cube.points.push_back(Vector3D(-halfCubeSize,  halfCubeSize, -halfCubeSize));
    cube.points.push_back(Vector3D(-halfCubeSize, -halfCubeSize,  halfCubeSize));
    cube.points.push_back(Vector3D( halfCubeSize, -halfCubeSize,  halfCubeSize));
    cube.points.push_back(Vector3D( halfCubeSize,  halfCubeSize,  halfCubeSize));
    cube.points.push_back(Vector3D(-halfCubeSize,  halfCubeSize,  halfCubeSize));
    
    cube.indices = {0,1,1,2,2,3,3,0,4,5,5,6,6,7,7,4,0,4,1,5,2,6,3,7};
    
    float dx = (screenWidth/2.0f);
    float dy = (screenHeight/2.0f);
    float dz = (screenVirtualDepth/2.0f) ;

    cube.Translate(dx,dy,dz);
    
    float deg = 0.0f;

    while(true)
    {

        float rad = (deg/180.0f) * M_PI;
        
        cube.SetAbsoluteRotation(rad, 0.0f, 1.0f, 0.0f);
        
        ClearScreens();
        DrawGeometry(cube);

        DrawScreens();

        deg += 1.0f;

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
    selectScreen(0);

    while (true) {

        // for (int f = 0; f < 500; f++)
        // {
        //     AnimateRandomPoints();
        //     sleep_ms(30);
        // }

        // for (int f = 0; f < 600; f++)
        // {
        //     float perc = (float(f) / 300.0f);
        //     float rads = perc * M_PI;
        //     float cosine = (-std::cos(rads) + 1.0f);
        //     float sin = std::sin(rads);
            
        //     AnimateWarpDrive(sin*cosine*0.5f);
        //     sleep_ms(30);
        // }

        AnimateSpinningCube(1);
        
        
    }
    return 0;
}


