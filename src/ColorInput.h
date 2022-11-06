#ifndef EMILYS_NEOPIXEL_COLOR_INPUT_H
#define EMILYS_NEOPIXEL_COLOR_INPUT_H

#include <Arduino.h>

#include "AnalogInput.h"
#include "LockGuard.h"

typedef std::function<void(uint16_t, uint16_t, uint16_t)> ColorInputEventHandler;

class ColorInput {
  public:
    ColorInput(uint8_t redPin, uint8_t greenPin, uint8_t bluePin);
    ~ColorInput();

    void begin();
    void end();
    uint16_t getRedValue();
    uint16_t getGreenValue();
    uint16_t getBlueValue();
    void onEvent(ColorInputEventHandler callback);

  private:
    AnalogInput _red;
    AnalogInput _green;
    AnalogInput _blue;

    ColorInputEventHandler _eventHandler;
    void _onInputEvent(uint16_t value);
    void _raiseOnEvent(uint16_t red, uint16_t green, uint16_t blue);
};

#endif