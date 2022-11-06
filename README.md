# Emily's NeoPixel

A while back I orded a LiPo battery from Adafruit and they made me add another product in order to ship it. I chose the [NeoPixel FeatherWing](https://www.adafruit.com/product/2945) because it looked cool and I had a ton of ESP32 Feathers lying around but I had no pratical use for it.

Recently my five year old daughter has taken interest to watching me work on a project for my e-bike and I figured it would be a fun project for her and I to build a fancy night light. 


## Technical details


This project leans heavily on the ESP32's dual cores and uses tasks and interrupts to create a non-blocking asynchronous flow

### Hardware:


- [Adafruit ESP32 Feather V2](https://www.adafruit.com/product/5400)
- [Adafruit NeoPixel FeatherWing](https://www.adafruit.com/product/2945)
- 3x potentiometers
- 2x momentary switches


### Software:


Couple points of interest that may be useful as an example for someone else

- `AnalogInput`: Uses a task to "debounce" an analog input since my potentiometers tended to float back and forth when idle

- `DigitalInput`: Uses a task and interrupt to debounce a digital input. Supports multi triggers and long triggers

- `LockGuard`: A FreeRTOS / ESP implementation of the `std::lock_guard` class that uses `SemaphoreHandle_t`

- `NeoPixel`: All the light control is in this class. Most of the patterns were adapted from the offical [`buttoncycler.ino`](https://github.com/adafruit/Adafruit_NeoPixel/blob/master/examples/buttoncycler/buttoncycler.ino) and [`strandtest_wheel.ino`](https://github.com/adafruit/Adafruit_NeoPixel/blob/master/examples/strandtest_wheel/strandtest_wheel.ino) examples