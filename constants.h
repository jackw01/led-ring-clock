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
CRGB red = CRGB(255, 0, 0);
CRGB orange = CRGB(255, 78, 0);
CRGB yellow = CRGB(255, 237, 0);
CRGB green = CRGB(0, 255, 23);
CRGB cyan = CRGB(0, 247, 255);
CRGB blue = CRGB(0, 21, 255);
CRGB magenta = CRGB(190, 0, 255);
CRGB white = CRGB(255, 255, 255);
CRGB off = CRGB(0, 0, 0);

// Default clock face colors
// red, orange, yellow, green, cyan, blue, magenta, and white are acceptable, along with CRGB(r, g, b)
const int colorSchemeCount = 7;
const CRGB colorSchemes[colorSchemeCount][4] = {
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
const int buttonClickRepeatDelayMs = 1500;
const int buttonLongPressDelayMs = 300;
const bool showSecondHand = true;
const bool twelveHour = true;

// Serial
const long serialPortBaudRate = 115200;
const int debugMessageIntervalMs = 2000;

// Clock modes
typedef enum {
    ClockModeRingClock,
    ClockModeDotClock,
    ClockModeDotClockColorChange,
    ClockModeDotClockTimeColor,
    ClockModeGlowClock,
    ClockModeCount
} ClockMode;

// Brightness
const uint8_t minBrightness = 4;

// Run loop
const int runLoopIntervalMs = 30;

// EEPROM addresses
const uint16_t eepromAddrColorScheme = 0;
const uint16_t eepromAddrClockMode = 1;

// Gamma correction values
const uint8_t PROGMEM gamma[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
    2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
    5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
   10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
   17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
   25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
   37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
   51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
   69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
   90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
  115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
  144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
  177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
  215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255 };

// LED blend modes
typedef enum {
    BlendModeOver,
    BlendModeAlpha,
    BlendModeAdd
} BlendMode;

#endif
