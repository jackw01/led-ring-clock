//
// WS2812 LED Analog Clock Firmware
// Copyright (c) 2016-2018 jackw01
// This code is distrubuted under the MIT License, see LICENSE for details
//

#include <math.h>
#include <FastLED.h>
#include <Wire.h>
#include <EEPROM.h>
#include <RTClib.h>

#include "constants.h"

// LED ring and RTC
CRGB leds[ledRingSize];
RTC_DS1307 rtc;

// Globals to keep track of state
int clockMode, colorScheme;
int lastLoopTime = 0;
int lastButtonClickTime = 0;
int lastDebugMessageTime = 0;
uint8_t currentBrightness;
uint8_t previousBrightness[16];
int lastSecondsValue = 0;
int lastMillisecondsSetTime = 0;
int milliseconds;
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
		lastLoopTime = currentTime;
		// Handle button
	    if (digitalRead(pinButton) == LOW && currentTime - lastButtonClickTime > buttonClickRepeatDelayMs) {
			delay(buttonLongPressDelayMs);
			if (digitalRead(pinButton) == LOW) {
				lastButtonClickTime = currentTime;
				colorScheme ++;
				if (colorScheme >= colorSchemeCount + 2) colorScheme = 0; // 2 special color schemes
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

		// Get time and calculate milliseconds value that is synced with the RTC's second count
		now = rtc.now();
		int currentSeconds = now.second();
		if (currentSeconds != lastSecondsValue) {
			lastSecondsValue = currentSeconds;
			milliseconds = 0;
		}
		currentTime = millis();
		milliseconds = (milliseconds + currentTime - lastMillisecondsSetTime);
		lastMillisecondsSetTime = currentTime;

		// Show clock
		clearLeds();
	    showClock();
	}
}

// Display the current clock
void showClock() {
	switch (clockMode) {
		case ClockModeRingClock:
			drawRingClock();
			break;
		case ClockModeDotClock:
			drawDotClock();
			break;
		case ClockModeDotClockTrail:
			drawDotClockTrail();
			break;
		case ClockModeDotClockColorChange:
			drawDotClockColorChange();
			break;
		case ClockModeGlowClock:
			drawGlowClock();
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
void drawRingClock() {
	int h = hourPosition();
    int m = minutePosition();
    float s = floatSecondPosition();

    if (m > h) {
        for (int i = 0; i < m; i++) setLed(i, minuteColor(), BlendModeOver, 1.0);
        for (int i = 0; i < h; i++) setLed(i, hourColor(), BlendModeOver, 1.0);
    } else {
        for (int i = 0; i < h; i++) setLed(i, hourColor(), BlendModeOver, 1.0);
        for (int i = 0; i < m; i++) setLed(i, minuteColor(), BlendModeOver, 1.0);
    }

    if (showSecondHand) setLed(s, secondColor(), BlendModeAlpha, 1.0);

    FastLED.show();
}

// Show a more traditional dot clock
void drawDotClock() {
	float h = floatHourPosition();
    float m = floatMinutePosition();
    float s = floatSecondPosition();

	for (float i = h - 1.0; i < h + 2.0; i++) setLed(i, hourColor(), BlendModeAlpha, 1.0);
	setLed(m, minuteColor(), BlendModeAlpha, 1.0);
	if (showSecondHand) setLed(s, secondColor(), BlendModeAlpha, 1.0);

    FastLED.show();
}

// Show a dot clock with longer hands
void drawDotClockTrail() {
	float h = floatHourPosition();
    float m = floatMinutePosition();
    float s = floatSecondPosition();

	for (float i = h - 1.0; i < h + 2.0; i++) setLed(i, hourColor(), BlendModeAdd, 1.0);
	for (float i = -4.0; i < 1.0; i++) setLed(m + i, minuteColor(), BlendModeAdd, 1.0 + (i / 5.0));
	if (showSecondHand) for (float i = -3.0; i < 1.0; i++) setLed(s + i, secondColor(), BlendModeAdd, 1.0 + (i / 4.0));

    FastLED.show();
}

// Show a dot clock with hands that change color based on their position
void drawDotClockColorChange() {
	float h = floatHourPosition();
    float m = floatMinutePosition();
    float s = floatSecondPosition();

	CRGB newHourColor = CHSV(map(now.hour(), 0, 24, 0, 255), 255, 255);
	CRGB newMinuteColor = CHSV(map(now.minute(), 0, 59, 0, 255), 255, 255);
	CRGB newSecondColor = CHSV(map(now.second(), 0, 59, 0, 255), 255, 255);

	for (float i = h - 1.0; i < h + 2.0; i++) setLed(i, newHourColor, BlendModeAdd, 1.0);
	setLed(m, newMinuteColor, BlendModeAdd, 1.0);
	if (showSecondHand) setLed(s, newSecondColor, BlendModeAdd, 1.0);

    FastLED.show();
}

// Show a dot clock where the hands overlap with additive blending
void drawGlowClock() {
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

// Get floating point hour representation
float floatHour() {
	return (float)now.hour() + mapFloat(now.minute() + mapFloat(now.second(), 0.0, 59.0, 0.0, 1.0), 0.0, 59.0, 0.0, 1.0);
}

// Get positions mapped to ring size
int hourPosition() {
	if (twelveHour) {
		int hour;
		if (now.hour() > 12) hour = (now.hour() - 12) * (ledRingSize / 12);
	    else hour = now.hour() * (ledRingSize / 12);
		return hour + map(now.minute(), 0, 59, 0, (ledRingSize / 12) - 1);
	} else {
		int hour = now.hour() * (ledRingSize / 24);
		return hour + map(now.minute(), 0, 59, 0, (ledRingSize / 24) - 1);
	}
}

int minutePosition() {
	return map(now.minute(), 0, 59, 0, ledRingSize - 1);
}

int secondPosition() {
	return map(now.second(), 0, 59, 0, ledRingSize - 1);
}

// Get positions as a float mapped to ring size
float floatHourPosition() {
	if (twelveHour) {
		int hour;
		if (now.hour() > 12) hour = (now.hour() - 12) * (ledRingSize / 12);
	    else hour = now.hour() * (ledRingSize / 12);
		return hour + mapFloat(now.minute(), 0.0, 59.0, 0.0, (ledRingSize / 12.0) - 1.0);
	} else {
		int hour = now.hour() * (ledRingSize / 24);
		return hour + mapFloat(now.minute(), 0, 59, 0, (ledRingSize / 24.0) - 1.0);
	}
}

float floatMinutePosition() {
	return mapFloat((float)now.minute() + ((1.0 / 60.0) * (float)now.second()), 0.0, 59.0, 0.0, (float)ledRingSize);
}

float floatSecondPosition() {
	return mapFloat(now.second() + (0.001 * milliseconds), 0.0, 60.0, 0.0, (float)ledRingSize);
}

// Get colors
CRGB hourColor() {
	if (colorScheme < colorSchemeCount) return colorSchemes[colorScheme][0];
	else if (colorScheme == colorSchemeCount + 0) {
		return white;
	} else if (colorScheme == colorSchemeCount + 1) {
	    return CHSV((uint8_t)mapFloat(fmod(20.0 - floatHour(), 24.0), 0.0, 24.0, 0.0, 255.0), 255, 255);
	}
}

CRGB minuteColor() {
	if (colorScheme < colorSchemeCount) return colorSchemes[colorScheme][1];
	else if (colorScheme == colorSchemeCount + 0) {
		return white;
	} else if (colorScheme == colorSchemeCount + 1) {
		return CHSV((uint8_t)mapFloat(fmod(20.0 - floatHour(), 24.0), 0.0, 24.0, 0.0, 255.0), 255, 255);
	}
}

CRGB secondColor() {
	if (colorScheme < colorSchemeCount) return colorSchemes[colorScheme][2];
	else if (colorScheme == colorSchemeCount + 0) {
		return white;
	} else if (colorScheme == colorSchemeCount + 1) {
		return CHSV((uint8_t)mapFloat(fmod(20.0 - floatHour(), 24.0), 0.0, 24.0, 0.0, 255.0), 255, 255);
	}
}

// Clear the LED ring
void clearLeds() {
	for (int i = 0; i < ledRingSize; i++) {
		leds[i] = CRGB(0, 0, 0);
		//leds[i].r = max(leds[i].r - 1, 0);
		//leds[i].g = max(leds[i].g - 2, 0);
		//leds[i].b = max(leds[i].b - 3, 0);
	}
}

// Set LED(s) at a position with enhanced rendering
void setLed(float position, CRGB color, BlendMode blendMode, float factor) {
	if (useEnhancedRenderer) {
		int low = floor(position);
		int high = ceil(position);
		float lowFactor = ((float)high - position);
		float highFactor = (position - (float)low);
		if (blendMode == BlendModeAdd) {
			blendAdd(wrap(low), color, lowFactor * factor);
			blendAdd(wrap(high), color, highFactor * factor);
		} else if (blendMode == BlendModeAlpha) {
			blendAlpha(wrap(low), color, lowFactor * factor);
			blendAlpha(wrap(high), color, highFactor * factor);
		} else if (blendMode == BlendModeOver) {
			blendOver(wrap(low), color, lowFactor * factor);
			blendOver(wrap(high), color, highFactor * factor);
		}
	} else {
		leds[wrap((int)position)] = color;
	}
}

// Additive blending
void blendAdd(int position, CRGB color, float factor) {
	leds[position].r += min(color.r * factor, 255 - leds[position].r);
	leds[position].g += min(color.g * factor, 255 - leds[position].g);
	leds[position].b += min(color.b * factor, 255 - leds[position].b);
}

// Alpha blending (factor is the alpha value)
void blendAlpha(int position, CRGB color, float factor) {
	leds[position].r = (uint8_t)mapFloat(factor, 0.0, 1.0, leds[position].r, color.r);
	leds[position].g = (uint8_t)mapFloat(factor, 0.0, 1.0, leds[position].g, color.g);
	leds[position].b = (uint8_t)mapFloat(factor, 0.0, 1.0, leds[position].b, color.b);
}

// Overlay/replace blending
void blendOver(int position, CRGB color, float factor) {
	leds[position].r = color.r * factor;
	leds[position].g = color.g * factor;
	leds[position].b = color.b * factor;
	leds[position] = color;
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
