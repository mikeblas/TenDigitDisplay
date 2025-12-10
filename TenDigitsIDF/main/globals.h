#pragma once

#define _countof(array) (sizeof(array) / sizeof(array[0]))

#define LOW 0
#define HIGH 1

#define MY_DRAM_ATTR DRAM_ATTR
// #define MY_DRAM_ATTR 

extern const char* LOG_TAG;

extern const char *pass;
extern const char *ssid;

// pinouts:
//      https://lastminuteengineers.com/esp32-pinout-reference/

#define LED_BUILTIN 2
// extern const uint8_t LED_BUILTIN = 2;
