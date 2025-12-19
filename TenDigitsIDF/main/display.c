
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_flash.h"

#include "driver/gpio.h"
#include "hal/gpio_ll.h"


#include "globals.h"



MY_DRAM_ATTR uint8_t dpPlace = 0;

MY_DRAM_ATTR uint64_t counter = 0;
MY_DRAM_ATTR uint8_t counterDigits[10];

// the scanner writes these segments each scan
MY_DRAM_ATTR uint8_t segmentValues[10];


// segments are active low
static const uint8_t SEGMENT_A = 23; // was 32
static const uint8_t SEGMENT_B = 33;
static const uint8_t SEGMENT_C = 27;
static const uint8_t SEGMENT_D = 13;
static const uint8_t SEGMENT_E = 4;
static const uint8_t SEGMENT_F = 25;
static const uint8_t SEGMENT_G = 26;
static const uint8_t SEGMENT_DP = 14;

// these go to CD4028 decoder
static const uint8_t DECODE_A = 18;
static const uint8_t DECODE_B = 22;
static const uint8_t DECODE_C = 21;
static const uint8_t DECODE_D = 19;

#define MY_DRAM_ATTR DRAM_ATTR
// #define MY_DRAM_ATTR 

/* const */ MY_DRAM_ATTR uint8_t digitToSegments[] =
{
    0b00111111,   // digit 0
    0b00000110,   // digit 1
    0b01011011,   // digit 2
    0b01001111,   // digit 3
    0b01100110,   // digit 4
    0b01101101,   // digit 5
    0b01111101,   // digit 6
    0b00000111,   // digit 7
    0b01111111,   // digit 8
    0b01100111,   // digit 9
    0b01110111,   // digit A
    0b01111100,   // digit b
    0b00111001,   // digit C
    0b01011110,   // digit d
    0b01111001,   // digit E
    0b01110001,   // digit F
};

static /* const */ MY_DRAM_ATTR uint8_t segmentMap[] =
{
    SEGMENT_A,
    SEGMENT_B,
    SEGMENT_C,
    SEGMENT_D,
    SEGMENT_E,
    SEGMENT_F,
    SEGMENT_G,
    SEGMENT_DP
};

static /* const */ MY_DRAM_ATTR uint8_t decodeMap[] =
{
    DECODE_A,
    DECODE_B,
    DECODE_C,
    DECODE_D
};

/*
    activate the given common anode on the given digit
    n is [0..9], and 0 is the left-most digit
*/
/* IRAM_ATTR */ void activateDecoderChannel(uint8_t n)
{
#if 0
    gpio_set_level(DECODE_A, (n & 1) == 0 ? LOW : HIGH);
    gpio_set_level(DECODE_B, (n & 2) == 0 ? LOW : HIGH);
    gpio_set_level(DECODE_C, (n & 4) == 0 ? LOW : HIGH);
    gpio_set_level(DECODE_D, (n & 8) == 0 ? LOW : HIGH);
#else

    uint32_t sets = 0;
    uint32_t clears = 0;

    uint8_t mask = 1;
    for (uint8_t i = 0; i < 4; i++, mask <<= 1)
    {
        if ((n & mask) == 0)
            clears |= (uint32_t)1 << decodeMap[i];
        else
            sets |= (uint32_t)1 << decodeMap[i];
        // ESP_LOGI(LOG_TAG, "n = %d, mask = %d, sets = %ld and clears = %ld", i, mask, sets, clears);
    }

    // ESP_LOGI(LOG_TAG, "got %d, sets = %ld and clears = %ld", (int) n, sets, clears);

    GPIO.out_w1ts = sets;
    GPIO.out_w1tc = clears;

#endif
}


/*
    Write the given segment bitmask to the segment output pins, 
    observing active low logic for the common anode part.

    The segments parameter has a bit on for a segment to be lit. 
    Segment A is the LSB, and the DP is the MSB. Bits in the parameter
    are set to turn the corresponding segment on.
*/
/* IRAM_ATTR */ void writeSegments(uint8_t segments)
{
#if 0
    uint8_t mask = 1;
    for (int x = 0; x < 8; x++)
    {
        gpio_set_level(segmentMap[x], ((segments & mask) == 0) ? HIGH : LOW);
        mask <<= 1;
    }
#else
    uint32_t sets = 0;
    uint32_t clears = 0;

    uint32_t sets32 = 0;
    uint32_t clears32 = 0;

    uint8_t mask = 1;
    for (uint8_t i = 0; i < 8; i++, mask <<= 1)
    {
        // note this implements active low: a 1 in the bitmask will clear the GPIO line
        if ((segments & mask) != 0)
        {
            if (segmentMap[i] < 32)
                clears |= (uint32_t)1U << segmentMap[i];
            else
                clears32 |= (uint32_t)1U << (segmentMap[i] - 32);
        }
        else
        {
            if (segmentMap[i] < 32)
                sets |= (uint32_t)1U << segmentMap[i];
            else
                sets32 |= (uint32_t)1U << (segmentMap[i] - 32);
        }
    }

    GPIO.out_w1ts = sets;
    GPIO.out_w1tc = clears;

    GPIO.out1_w1ts.val = sets32;
    GPIO.out1_w1tc.val = clears32;
#endif
}


/*
    turn on the DP in the given digit
*/
void set_decimal_point(uint8_t digit)
{
    segmentValues[digit] |= 0x80;
}


/*
    turn off the DP in the given digit
*/
void clear_decimal_point(uint8_t digit)
{
    segmentValues[digit] &= 0x80;
}


/*
    clear the given GPIO pin, then set it up as an output
*/
static void set_output(gpio_num_t pin)
{
    gpio_reset_pin(pin);
    gpio_set_direction(pin, GPIO_MODE_OUTPUT);
}


/*
    sets up our GPIO pins during initialization
*/
void setup_pins(void)
{
    // all the segments are outputs
    for (int n = 0; n < _countof(segmentMap); n++)
        set_output(segmentMap[n]);

    // built-in is an output
    set_output(LED_BUILTIN);

    // drives are outputs
    set_output(DECODE_A);
    set_output(DECODE_B);
    set_output(DECODE_C);
    set_output(DECODE_D);
}


/*
void setDP(bool on)
{
    gpio_set_level(SEGMENT_DP, on ? LOW : HIGH);
}
*/


void counter_to_segments(void)
{
    for (int n = 0; n < 10; n++)
    {
        segmentValues[n] = digitToSegments[counterDigits[n]];
    }
}
