#pragma once

#include "globals.h"

extern MY_DRAM_ATTR uint8_t dpPlace;

extern MY_DRAM_ATTR uint64_t counter;
extern MY_DRAM_ATTR uint8_t counterDigits[10];

// the scanner writes these segments each scan
extern MY_DRAM_ATTR uint8_t segmentValues[10];

extern /* const */ MY_DRAM_ATTR uint8_t digitToSegments[];

void set_decimal_point(uint8_t digit);

void counter_to_segments(void);

void setup_pins(void);

/* IRAM_ATTR */ void writeSegments(uint8_t segments);

/* IRAM_ATTR */ void activateDecoderChannel(uint8_t n);

void setDP(bool on);

void counter_to_segments(void);
