#include "ColorInput.h"

ColorInput::ColorInput(uint8_t redPin, uint8_t greenPin, uint8_t bluePin):
  _red(redPin),
  _green(greenPin),
  _blue(bluePin) {

}

ColorInput::~ColorInput() {
  end();
}

void ColorInput::_onInputEvent(uint16_t value) {
  _raiseOnEvent(_red.getValue(), _green.getValue(), _blue.getValue());
}

void ColorInput::_raiseOnEvent(uint16_t red, uint16_t green, uint16_t blue) {
  ColorInputEventHandler eventHandler = _eventHandler;

  if (eventHandler != NULL) {
    eventHandler(red, green, blue);
  }
}

void ColorInput::begin() {
  _red.onEvent([this](uint16_t value) -> void { return this->_onInputEvent(value); });
  _green.onEvent([this](uint16_t value) -> void { return this->_onInputEvent(value); });
  _blue.onEvent([this](uint16_t value) -> void { return this->_onInputEvent(value); });

  _red.begin();
  _green.begin();
  _blue.begin();
}

void ColorInput::end() {
  _red.end();
  _green.end();
  _blue.end();
}

uint16_t ColorInput::getRedValue() {
  return _red.getValue();
}

uint16_t ColorInput::getGreenValue() {
  return _green.getValue();
}

uint16_t ColorInput::getBlueValue() {
  return _blue.getValue();
}

void ColorInput::onEvent(ColorInputEventHandler callback) {
  _eventHandler = callback;
}