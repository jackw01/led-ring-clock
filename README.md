# led-ring-clock
An Arduino-based analog clock using an Adafruit NeoPixel ring as a display.

This sketch requires the FastLED library, the Wire library (built-in), the EEPROM library (built-in), and the Adafruit RTClib.

https://github.com/FastLED/FastLED
https://github.com/adafruit/RTClib

The NeoPixel ring is connected to digital pin 3. Momentary N/O button is connected to digital pin 4. 20K potentiometer is connected to power and ground on both sides and the wiper is connected to Analog 0. DS1307 RTC breakout is connected to default I2C pins. This code has only been tested with a ring of 24 NeoPixels, but should (hopefully) work with any size. The code should work on any ATmega328 based board
