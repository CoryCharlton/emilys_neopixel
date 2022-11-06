#include <Arduino.h>

#include "ColorInput.h"
#include "DigitalInput.h"
#include "NeoPixel.h"

#define BRIGHTNESS_BUTTON_PIN 25
#define MODE_BUTTON_PIN 26
#define NEOPIXEL_CONTROL_PIN 32

#define RED_PIN 34
#define GREEN_PIN 39
#define BLUE_PIN 36

DigitalInput brightnessButton = DigitalInput(BRIGHTNESS_BUTTON_PIN);
ColorInput colorInput = ColorInput(RED_PIN, GREEN_PIN, BLUE_PIN);
DigitalInput modeButton = DigitalInput(MODE_BUTTON_PIN);
NeoPixel neoPixel = NeoPixel(NEOPIXEL_CONTROL_PIN);

void onBrightnessButtonEvent(DigitalInputEvent event);
void onColorEvent(uint16_t red, uint16_t green, uint16_t blue);
void onModeButtonEvent(DigitalInputEvent event);

// TODO: Use Preferences.h to save mode and brightness
void setup() {
  pinMode(BRIGHTNESS_BUTTON_PIN, INPUT_PULLUP);
  pinMode(MODE_BUTTON_PIN, INPUT_PULLUP);

  analogReadResolution(8);

  brightnessButton.begin();
  brightnessButton.onEvent(onBrightnessButtonEvent);

  colorInput.begin();
  colorInput.onEvent(onColorEvent);

  modeButton.begin();
  modeButton.onEvent(onModeButtonEvent);

  neoPixel.begin();
  neoPixel.setColor(colorInput.getRedValue(), colorInput.getGreenValue(), colorInput.getBlueValue());
}

void loop() {  
  delay(1000); // Short delay to keep the watchdog happy
}

void onBrightnessButtonEvent(DigitalInputEvent event) {
  if (event != DigitalInputEvent::Trigger) {
    return;
  }

  neoPixel.nextBrightness();
}

void onColorEvent(uint16_t red, uint16_t green, uint16_t blue) {
  neoPixel.setColor(red, green, blue);
}

void onModeButtonEvent(DigitalInputEvent event) {
  if (event != DigitalInputEvent::Trigger) {
    return;
  }

  neoPixel.nextMode();
}