//
// Copyright (c) 2016-2018 jackw01
// This code is distrubuted under the MIT License, see LICENSE for details
//

#include <math.h>
#include <FastLED.h>
#include <Wire.h>
#include <EEPROM.h>
#include <RTClib.h>

#include "config.h"

CRGB leds[ledRingSize];
RTC_DS1307 rtc;

// Globals to keep track of state
int clockMode, colorScheme;
int lastLoopTime = 0;
int lastButtonClickTime = 0;
int lastDebugMessageTime = 0;
uint8_t currentBrightness;
uint8_t previousBrightness[16];
DateTime now;

void setup() {
	// Begin serial port
	Serial.begin(serialPortBaudRate);

	// Init FastLED
	FastLED.addLeds<NEOPIXEL, pinLeds>(leds, ledRingSize);
	FastLED.setTemperature(Halogen);
    FastLED.show();

	// Connect to the RTC
    Wire.begin();
    rtc.begin();

	// Set button pin
    pinMode(pinButton, INPUT);

	// Read saved config from EEPROM
    colorScheme = EEPROM.read(eepromAddrColorScheme);
    clockMode = EEPROM.read(eepromAddrClockMode);

    // If button is pressed at startup, light all LEDs
    if (digitalRead(pinButton) == LOW) {
        for (int i = 0; i < ledRingSize; i++) leds[i] = white;
        FastLED.show();
        delay(60000);
    }
}

void loop() {
	int currentTime = millis();
	if (currentTime - lastLoopTime > runLoopIntervalMs) {
		lastLoopTime = millis();
		// Handle button
	    if (digitalRead(pinButton) == LOW && currentTime - lastButtonClickTime > buttonClickRepeatDelayMs) {
			delay(buttonLongPressDelayMs);
			if (digitalRead(pinButton) == LOW) {
				lastButtonClickTime = currentTime;
				colorScheme ++;
				if (colorScheme >= colorSchemeCount) colorScheme = 0;
				EEPROM.write(0, colorScheme);
			} else {
				clockMode ++;
				if (clockMode >= ClockModeCount) clockMode = 0;
				EEPROM.write(1, clockMode);
			}
	    }

		// Print debug message
	    if (currentTime > lastDebugMessageTime + debugMessageIntervalMs) {
	        lastDebugMessageTime = currentTime;
	        printDebugMessage();
	    }

		// Update brightness - do a moving average to smooth out noisy potentiometers
	    int sum = 0;
	    for (uint8_t i = 15; i > 0; i--) {
	        previousBrightness[i] = previousBrightness[i - 1];
	        sum += previousBrightness[i];
	    }
	    previousBrightness[0] = map(analogRead(pinBrightness), 0, 1023, minBrightness, 255);
	    sum += previousBrightness[0];
	    currentBrightness = sum / 16;
		FastLED.setBrightness(currentBrightness);

		// Show clock
		now = rtc.now();
		clearLeds();
	    showClock();
	}
}

// Display the current clock
void showClock() {
	switch (clockMode) {
		case ClockModeRingClock:
			ringClock();
			break;
		case ClockModeDotClock:
			dotClock();
			break;
		case ClockModeDotClockColorChange:
			rainbowDotClock();
			break;
		case ClockModeDotClockTimeColor:
			timeColorClock();
			break;
		case ClockModeGlowClock:
			glowClock();
			break;
	}
}

// Print debugging info over serial
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
	Serial.println("");
}

// Show a ring clock
void ringClock() {
    int h = hourPosition();
    int m = minutePosition();
    int s = secondPosition();

    if (m > h) {
        for (int i = 0; i < m; i++) leds[i] = minuteColor();
        for (int i = 0; i < h; i++) leds[i] = hourColor();
    } else {
        for (int i = 0; i < h; i++) leds[i] = hourColor();
        for (int i = 0; i < m; i++) leds[i] = minuteColor();
    }

    if (showSecondHand) leds[s] = secondColor();

    FastLED.show();
}

// Show a more traditional dot clock
void dotClock() {
	int h = hourPosition();
    int m = minutePosition();
    int s = secondPosition();

	for (int i = h - 1; i < h + 2; i++) leds[wrap(i)] = hourColor();
    leds[m] = minuteColor();
    if (showSecondHand) [s] = secondColor();

    FastLED.show();
}

// Show a dot clock with hands that change color based on their position
void rainbowDotClock() {
	int h = hourPosition();
    int m = minutePosition();
    int s = secondPosition();

	CRGB newHourColor = CHSV(map(now.hour(), 0, 24, 0, 255), 255, 255);
	CRGB newMinuteColor = CHSV(map(now.minute(), 0, 59, 0, 255), 255, 255);
	CRGB newSecondColor = CHSV(map(now.second(), 0, 59, 0, 255), 255, 255);

    for (int i = h - 1; i < h + 2; i++) leds[wrap(i)] = newHourColor;
    leds[m] = newMinuteColor;
    if (showSecondHand) leds[s] = newSecondColor;

    FastLED.show();
}

// Show a dot clock where the color is based on the time
void timeColorClock() {
	int h = hourPosition();
    int m = minutePosition();
    int s = secondPosition();
    float decHour = decimalHour();

    CRGB pixelColor = CHSV((uint8_t)mapFloat(fmod(20.0 - decHour, 24.0), 0.0, 24.0, 0.0, 255.0), 255, 255);

    for (int i = h - 1; i < h + 2; i++) leds[wrap(i)] = pixelColor;
    leds[m] = pixelColor;
    if (showSecondHand) leds[s] = pixelColor;

    FastLED.show();
}

// Show a dot clock where the hands overlap with additive blending
void glowClock() {
	int h = hourPosition();
    int m = minutePosition();
    int s = secondPosition();

    for (int i = -6; i < ledRingSize + 6; i++) {
		int j;
		for (j = 0; j <= 4; j++) {
			if (h + j == i || h - j == i) blendAdd(wrap(i), CRGB(255, 0, 0), 1 - mapFloat(j, 0.0, 6.0, 0.1, 0.99));
		}
		for (j = 0; j <= 2; j++) {
			if (m + j == i || m - j == i) blendAdd(wrap(i), CRGB(0, 255, 0), 1 - mapFloat(j, 0.0, 3.0, 0.1, 0.99));
		}
		if (showSecondHand) {
			for (j = 0; j <= 1; j++) {
				if (s + j == i || s - j == i) blendAdd(wrap(i), CRGB(0, 0, 255), 1 - mapFloat(j, 0.0, 1.0, 0.1, 0.65));
			}
		}
    }

    FastLED.show();
}

// Get positions mapped to ring size
int hourPosition() {
	if (twelveHour) {
		int hour;
		if (now.hour() > 12) hour = (now.hour() - 12) * (ledRingSize / 12);
	    else hour = now.hour() * (ledRingSize / 12);
		return hour + int(map(now.minute(), 0, 59, 0, (ledRingSize / 12) - 1));;
	} else {
		int hour = now.hour() * (ledRingSize / 24);
		return hour + int(map(now.minute(), 0, 59, 0, (ledRingSize / 24) - 1));;
	}
}

int minutePosition() {
	return map(now.minute(), 0, 59, 0, ledRingSize - 1);
}

int secondPosition() {
	return map(now.second(), 0, 59, 0, ledRingSize - 1);
}

float decimalHour() {
	return (float)now.hour() + mapFloat(now.minute() + mapFloat(now.second(), 0.0, 59.0, 0.0, 1.0), 0.0, 59.0, 0.0, 1.0);
}

// Get colors
CRGB hourColor() {
	return colorSchemes[colorScheme][0];
}

CRGB minuteColor() {
	return colorSchemes[colorScheme][1];
}

CRGB secondColor() {
	return colorSchemes[colorScheme][2];
}

// Clear the LED ring
void clearLeds() {
	for (int i = 0; i < ledRingSize; i++) leds[i] = CRGB(0, 0, 0);
}

// Enhanced additive blending
void blendAdd(int position, CRGB color, float factor) {
	leds[position].r += color.r * factor;
	leds[position].g += color.g * factor;
	leds[position].b += color.b * factor;
}

// Wrap around LED ring
int wrap(int i) {
	if (i >= ledRingSize) return i - ledRingSize;
	else if (i < 0) return ledRingSize + i;
	else return i;
}

// Because Arduino does not
float mapFloat(float x, float inMin, float inMax, float outMin, float outMax) {
	return (x - inMin) * (outMax - outMin) / (inMax - inMin) + outMin;
}
