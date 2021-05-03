#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"

#define D0 0;
#define D1 1;
#define D2 2;
#define D3 3;
#define D4 4;
#define D5 5;
#define D6 6;
#define D7 7;

#define CS 28;
#define DC 27;
#define RST 26;
#define EN 22;
#define RW 21;

int main() {
    stdio_init_all();

    uint32_t outputMask = 0x0000;
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

    gpio_set_dir_out_masked(outputMask);
    
    while (true) {
        sleep_ms(10000);
    }
    return 0;
}