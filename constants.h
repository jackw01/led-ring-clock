//
// WS2812 LED Analog Clock Firmware
// Copyright (c) 2016-2018 jackw01
// This code is distrubuted under the MIT License, see LICENSE for details
//

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <FastLED.h>

// IO Pin Assignments
const uint8_t pinLeds = 3;
const uint8_t pinButton = 4;
const uint8_t pinBrightness = 0;

// Number of LEDs in ring
const int ledRingSize = 24;

// Default colors - tweaked to look right on WS2812Bs
const CRGB red = CRGB(255, 0, 0);
const CRGB orange = CRGB(255, 78, 0);
const CRGB yellow = CRGB(255, 237, 0);
const CRGB green = CRGB(0, 255, 23);
const CRGB cyan = CRGB(0, 247, 255);
const CRGB blue = CRGB(0, 21, 255);
const CRGB magenta = CRGB(190, 0, 255);
const CRGB white = CRGB(255, 255, 255);
const CRGB off = CRGB(0, 0, 0);

// Default clock face colors
// red, orange, yellow, green, cyan, blue, magenta, and white are acceptable, along with CRGB(r, g, b)
const int colorSchemeCount = 7;
const CRGB colorSchemes[colorSchemeCount][3] = {
    {red, // Color for hour display
     green, // Color for minute display
     blue}, // Color for second display
    { CRGB(255, 255, 255), CRGB(255, 255, 255), CRGB(  0, 130, 255) },
	{ CRGB(255, 255, 255), CRGB(255, 255, 255), CRGB(255,  25,   0) },
	{ CRGB( 64,   0, 128), CRGB(255,  72,   0), CRGB(255, 164,   0) },
	{ CRGB(255,  25,   0), CRGB(255,  84,   0), CRGB(255, 224,   0) },
	{ CRGB(  0,   0, 255), CRGB(  0,  84, 255), CRGB(  0, 255, 255) },
	{ CRGB(255,   0,  96), CRGB(255,  84,   0), CRGB(  0, 255, 164) }
};

// Clock settings
const bool useEnhancedRenderer = true;
const bool showSecondHand = true;
const bool twelveHour = true;
const int hourGlowWidth = 4; // Pixels in each direction
const int minuteGlowWidth = 2; // Pixels in each direction
const int secondGlowWidth = 1; // Pixels in each direction
const int buttonClickRepeatDelayMs = 1500;
const int buttonLongPressDelayMs = 300;

// Serial
const long serialPortBaudRate = 115200;
const int debugMessageIntervalMs = 2000;

// Clock modes
typedef enum {
    ClockModeRingClock,
    ClockModeDotClock,
    ClockModeDotClockTrail,
    ClockModeDotClockGlow,
    ClockModeCount
} ClockMode;

// Brightness
const uint8_t minBrightness = 4;

// Run loop
const int runLoopIntervalMs = 30;

// EEPROM addresses
const uint16_t eepromAddrColorScheme = 0;
const uint16_t eepromAddrClockMode = 1;

// LED blend modes
typedef enum {
    BlendModeOver,
    BlendModeAlpha,
    BlendModeAdd
} BlendMode;

#endif
