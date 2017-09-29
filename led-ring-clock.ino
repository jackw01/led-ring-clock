//
// Copyright (c) 2015-2017 jackw01
// This code is distrubuted under the MIT License, see LICENSE for details
//

#include <FastLED.h>
#include <Wire.h>
#include <EEPROM.h>
#include "RTClib.h"

//
// Adjust these variables to taste
//

// Pin Assignments
const int pinLeds = 3;
const int pinButton = 4;
const int pinBrightness = 0;

const int ledRingSize = 24; // Number of LEDs in ring

// Default colors
CRGB red = CRGB(255, 0, 0);
CRGB orange = CRGB(255, 78, 0);
CRGB yellow = CRGB(255, 237, 0);
CRGB green = CRGB(0, 255, 23);
CRGB cyan = CRGB(0, 247, 255);
CRGB blue = CRGB(0, 21, 255);
CRGB magenta = CRGB(190, 0, 255);
CRGB white = CRGB(255, 255, 255);
CRGB off = CRGB(0, 0, 0);

// Clock face colors
// red, orange, yellow, green, cyan, blue, magenta, and white are acceptable, along with CRGB(r, g, b)
const int colorSchemeMax = 6;
const CRGB colorSchemes[colorSchemeMax + 1][4] = {{off, // Color when only one is needed (deprecated)
							     				   red, // Color for hour display
							     				   green, // Color for minute display
							     				   blue}, // Color for second display
							     			      {CRGB(0, 0, 0), CRGB(255, 255, 255), CRGB(255, 255, 255), CRGB(0, 130, 255)},
							     			      {CRGB(0, 0, 0), CRGB(255, 255, 255), CRGB(255, 255, 255), CRGB(255, 25, 0)},
						  	     			      {CRGB(0, 0, 0), CRGB(64, 0, 128), CRGB(255, 72, 0), CRGB(255, 164, 0)},
						         			      {CRGB(0, 0, 0), CRGB(255, 25, 0), CRGB(255, 84, 0), CRGB(255, 224, 0)},
						  	     			      {CRGB(0, 0, 0), CRGB(0, 0, 255), CRGB(0, 84, 255), CRGB(0, 255, 255)},
												  {CRGB(0, 0, 0), CRGB(255, 0, 96), CRGB(255, 84, 0), CRGB(0, 255, 164)}};

const int gradientMax = 1;
const CRGB gradients[gradientMax + 1][6] = {{CRGB(72, 0, 96), CRGB(255, 72, 0), CRGB(255, 164, 0), CRGB(255, 224, 0), CRGB(0, 255, 164), CRGB(0, 208, 	255)},
											{CRGB(72, 0, 96), CRGB(255, 72, 0), CRGB(255, 164, 0), CRGB(255, 224, 0), CRGB(0, 255, 164), CRGB(0, 208, 255)}};

// Setup ends
// Code starts here

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

CRGB leds[ledRingSize];
RTC_DS1307 rtc;

int clockMode, colorScheme, gradient;
int clockModeMax = 4;
int buttonState = 0;
const int minBrightness = 4;
int currentBrightness;
uint8_t previousBrightness[16];
int counter = 0;
int timer = 0;

const int debugMessageInterval = 1000;
int lastDebugMessageTime = 0;

CRGB handColor, hourColor, minuteColor, secondColor;

void setup() {

	Serial.begin(57600);

	// Init FastLED
	FastLED.addLeds<NEOPIXEL, pinLeds>(leds, ledRingSize);
	FastLED.setTemperature(Halogen);
    FastLED.show();

    Wire.begin();
    rtc.begin();

    pinMode(pinButton, INPUT);

    colorScheme = EEPROM.read(0);
    clockMode = EEPROM.read(1);
	gradient = 0;

	handColor = colorSchemes[colorScheme][0];
	hourColor = colorSchemes[colorScheme][1];
	minuteColor = colorSchemes[colorScheme][2];
	secondColor = colorSchemes[colorScheme][3];

    // Serial debug
    Serial.println("WS2812B LED Ring Clock by jackw01");

    // Light mode
    if (digitalRead(pinButton) == LOW) {
        for (int i = 0; i < ledRingSize; i++) leds[i] = white;
        FastLED.show();
        delay(60000);
    }

    // Test animation
    for (int i = 0; i < ledRingSize; i++) {
        leds[i] = white;
        FastLED.show();
        delay(40);
    }
    for (int i = 0; i < ledRingSize; i++) {
        leds[i] = off;
        FastLED.show();
        delay(40);
    }
}

void loop() {

    if (digitalRead(pinButton) == LOW && counter >= 14) {

		counter = 0;

		delay(280);

		if (digitalRead(pinButton) == LOW) {

			counter = 0;

			colorScheme ++;
			if (colorScheme > colorSchemeMax) colorScheme = 0;
			EEPROM.write(0, colorScheme);

			handColor = colorSchemes[colorScheme][0];
			hourColor = colorSchemes[colorScheme][1];
			minuteColor = colorSchemes[colorScheme][2];
			secondColor = colorSchemes[colorScheme][3];

		} else {

			clockMode ++;
			if (clockMode > clockModeMax) clockMode = 0;
			EEPROM.write(1, clockMode);
		}
    }

    if (millis() > lastDebugMessageTime + debugMessageInterval) {
        lastDebugMessageTime = millis();
        printDebugMessage();
    }

    showClock();
    delay(20);

    counter ++;
    if (counter > 40) counter = 40;

	timer ++;
    if (timer > 255) timer = 0;
}

void showClock() {

    // Moving average of brightness to smooth out noisy potentiometers
    int sum = 0;

    for (int i = 15; i > 0; i--) {
        previousBrightness[i] = previousBrightness[i - 1];
        sum += previousBrightness[i];
    }

    previousBrightness[0] = map(analogRead(pinBrightness), 0, 1023, minBrightness, 255);
    sum += previousBrightness[0];

    currentBrightness = sum / 16;

	FastLED.setBrightness(currentBrightness);

    if (clockMode == 0) ringClock();
	else if (clockMode == 1) dotClock();
	else if (clockMode == 2) rainbowDotClock();
	else if (clockMode == 3) timeColorClock();
	else if (clockMode == 4) glowClock();
	else if (clockMode == 5) gradientHandsClock();
}

void printDebugMessage() {

    Serial.print("Current date/time: ");
    DateTime now = rtc.now();
    Serial.print(now.year(), DEC);
    Serial.print("/");
    Serial.print(now.month(), DEC);
    Serial.print("/");
    Serial.print(now.day(), DEC);
    Serial.print(" ");
    Serial.print(now.hour(), DEC);
    Serial.print(":");
    Serial.print(now.minute(), DEC);
    Serial.print(":");
    Serial.print(now.second(), DEC);
    Serial.println();
    Serial.print("Display mode: ");
    Serial.println(clockMode);
    Serial.print("Color scheme: ");
    Serial.println(colorScheme);
    Serial.print("Brightness: ");
    Serial.println(currentBrightness);
}

// Show a ring clock
void ringClock() {

    clearLeds();

    DateTime now = rtc.now();

    int newHour;
    int newMinute;
    int newSecond;

    int hour;

	if (now.hour() > 12) hour = (now.hour() - 12) * (ledRingSize / 12);
    else hour = now.hour() * (ledRingSize / 12);

    newHour = hour + int(map(now.minute(), 0, 59, 0, (ledRingSize / 12) - 1));
    newMinute = int(map(now.minute(), 0, 59, 0, ledRingSize - 1));
    newSecond = int(map(now.second(), 0, 59, 0, ledRingSize - 1));

    if (newMinute > newHour) {
        for (int i = 0; i < newMinute; i++) leds[i] = minuteColor;
        for (int i = 0; i < newHour; i++) leds[i] = hourColor;
    } else {
        for (int i = 0; i < newHour; i++) leds[i] = hourColor;
        for (int i = 0; i < newMinute; i++) leds[i] = minuteColor;
    }

    leds[newSecond] = secondColor;

    FastLED.show();
}

// Show a more traditional dot clock
void dotClock() {

    clearLeds();

    DateTime now = rtc.now();

    int newHour;
    int newMinute;
    int newSecond;

    int hour;

	if (now.hour() > 12) hour = (now.hour() - 12) * (ledRingSize / 12);
    else hour = now.hour() * (ledRingSize / 12);

    newHour = hour + int(map(now.minute(), 0, 59, 0, (ledRingSize / 12) - 1));
    newMinute = int(map(now.minute(), 0, 59, 0, ledRingSize - 1));
    newSecond = int(map(now.second(), 0, 59, 0, ledRingSize - 1));

	for (int i = newHour - 1; i < newHour + 2; i++) leds[wrap(i)] = hourColor;

    leds[newMinute] = minuteColor;

    leds[newSecond] = secondColor;

    FastLED.show();
}

// Show a dot clock with hands that change color based on their position
void rainbowDotClock() {

    clearLeds();

    DateTime now = rtc.now();

    int newHour;
    int newMinute;
    int newSecond;

    int hour;

	if (now.hour() > 12) hour = (now.hour() - 12) * (ledRingSize / 12);
    else hour = now.hour() * (ledRingSize / 12);

    newHour = hour + int(map(now.minute(), 0, 59, 0, (ledRingSize / 12) - 1));
    newMinute = int(map(now.minute(), 0, 59, 0, ledRingSize - 1));
    newSecond = int(map(now.second(), 0, 59, 0, ledRingSize - 1));

	CRGB newHourColor = Wheel(int(map(newHour, 0, 12, 0, 255)));
	CRGB newMinuteColor = Wheel(int(map(now.minute(), 0, 59, 0, 255)));
	CRGB newSecondColor = Wheel(int(map(now.second(), 0, 59, 0, 255)));

    for (int i = newHour - 1; i < newHour + 2; i++) leds[wrap(i)] = newHourColor;

    leds[newMinute] = newMinuteColor;

    leds[newSecond] = newSecondColor;

    FastLED.show();
}

// Show a dot clock where the color is based on the time
void timeColorClock() {

    clearLeds();

    DateTime now = rtc.now();

    int newHour;
    int newMinute;
    int newSecond;

    int decimalHour;

    int hour;

	if (now.hour() > 12) hour = (now.hour() - 12) * (ledRingSize / 12);
    else hour = now.hour() * (ledRingSize / 12);

    newHour = hour + int(map(now.minute(), 0, 59, 0, (ledRingSize / 12) - 1));
    newMinute = int(map(now.minute(), 0, 59, 0, ledRingSize - 1));
    newSecond = int(map(now.second(), 0, 59, 0, ledRingSize - 1));

    decimalHour = now.hour() + map(now.minute() + map(now.second(), 0, 59, 0, 1), 0, 59, 0, 1);

    CRGB pixelColor = Wheel(map((20 - decimalHour) % 24, 0, 24, 0, 255));

    for (int i = newHour - 1; i < newHour + 2; i++) leds[wrap(i)] = pixelColor;

    leds[newMinute] = pixelColor;

    leds[newSecond] = pixelColor;

    FastLED.show();
}

// Show a dot clock where the hands overlap with additive blending
void glowClock() {

    clearLeds();

    DateTime now = rtc.now();

    int newHour;
    int newMinute;
    int newSecond;

    int hour;

    if (now.hour() > 12) hour = (now.hour() - 12) * (ledRingSize / 12);
    else hour = now.hour() * (ledRingSize / 12);

    newHour = hour + int(map(now.minute(), 0, 59, 0, (ledRingSize / 12) - 1));
    newMinute = int(map(now.minute(), 0, 59, 0, ledRingSize - 1));
    newSecond = int(map(now.second(), 0, 59, 0, ledRingSize - 1));

    for (int i = -6; i < ledRingSize + 6; i++) {

		int j;

		for (j = 0; j <= 4; j++) {
			if (newHour + j == i || newHour - j == i) blendAdd(wrap(i), CRGB(255, 0, 0), 1 - mapFloat(j, 0, 6, 0.1, 0.99));
		}

		for (j = 0; j <= 2; j++) {
			if (newMinute + j == i || newMinute - j == i) blendAdd(wrap(i), CRGB(0, 255, 0), 1 - mapFloat(j, 0, 3, 0.1, 0.99));
		}

		for (j = 0; j <= 1; j++) {
			if (newSecond + j == i || newSecond - j == i) blendAdd(wrap(i), CRGB(0, 0, 255), 1 - mapFloat(j, 0, 1, 0.1, 0.65));
		}
    }

    FastLED.show();
}

// Show a clock with gradient colored hands
void gradientHandsClock() {

    clearLeds();

    DateTime now = rtc.now();

    int newHour;
    int newMinute;
    int newSecond;

    int hour;

	if (now.hour() > 12) hour = (now.hour() - 12) * (ledRingSize / 12);
    else hour = now.hour() * (ledRingSize / 12);

    newHour = hour + int(map(now.minute(), 0, 59, 0, (ledRingSize / 12) - 1));
    newMinute = int(map(now.minute(), 0, 59, 0, ledRingSize - 1));
    newSecond = int(map(now.second(), 0, 59, 0, ledRingSize - 1));

	for (int i = 0; i < 6; i++) blendAdd(wrap(i + (newHour - 1)), gradients[gradient][i], 0.8);

	for (int i = 0; i < 4; i++) blendAdd(wrap(i + (newMinute - 1)), gradients[gradient][i + 2], 0.9);

	for (int i = 0; i < 2; i++) blendAdd(wrap(i + (newSecond - 1)), gradients[gradient][i + 4], 1);

    FastLED.show();
}

void clearLeds() {

	for (int i = 0; i < ledRingSize; i++) leds[i] = CRGB(0, 0, 0);
}

// Enhanced additive blending
void blendAdd(int position, CRGB color, double brightness) {

	leds[position].r += color.r * brightness;
	leds[position].g += color.g * brightness;
	leds[position].b += color.b * brightness;
}

// Wrap around LED ring
int wrap(int i) {

	if (i >= ledRingSize) return i - ledRingSize;
	else if (i < 0) return ledRingSize + i;
	else return i;
}

// Because Arduino does not
float mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {

	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
CRGB Wheel(byte WheelPos) {

	WheelPos = 255 - WheelPos;

	if (WheelPos < 85) return CRGB(255 - WheelPos * 3, 0, WheelPos * 3);
	else if (WheelPos < 170) {
		WheelPos -= 85;
		return CRGB(0, WheelPos * 3, 255 - WheelPos * 3);
	} else {
		WheelPos -= 170;
		return CRGB(WheelPos * 3, 255 - WheelPos * 3, 0);
	}
}
