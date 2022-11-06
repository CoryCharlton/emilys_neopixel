#include "NeoPixel.h"

const char* NeoPixel::BRIGHTNESS_KEY = "brightness";
const char* NeoPixel::MODE_KEY = "mode";

NeoPixel::NeoPixel(uint8_t pin): 
  _strip(32, pin, NEO_GRB + NEO_KHZ800) {

  if(_lock == NULL) {
    _lock = xSemaphoreCreateMutex();
    if(_lock == NULL) {
      // TODO: I should throw here...
      log_e("xSemaphoreCreateMutex failed");
      return;
    }
  }
}

NeoPixel::~NeoPixel() {
  end();

  if (_lock != NULL) {
    vSemaphoreDelete(_lock);
  }
}

void NeoPixel::_createModeTask() {
    xTaskCreateUniversal(_modeTaskCode, "neopixel_model_task", NEOPIXEL_MODE_TASK_STACK_SIZE, this, NEOPIXEL_MODE_TASK_PRIORITY, &_modeTask, NEOPIXEL_MODE_TASK_CORE);
    if (_modeTask == NULL) {
        log_e(" -- Error creating mode task");
    }
}

void NeoPixel::_deleteModeTask() {
  if (_modeTask != NULL) {
    vTaskDelete(_modeTask);
    _modeTask = NULL;
  }
}

TickType_t NeoPixel::_getTicksToWait() {
  uint32_t stepMillis = _getStepMillis();
  uint32_t sinceLastStep = millis() - _lastStepTime;

  return (sinceLastStep > stepMillis) ? 1 : pdMS_TO_TICKS(stepMillis - sinceLastStep);
}

uint32_t NeoPixel::_getStepMillis() {
  switch (_mode)
  {
    case NeoPixelMode::Off:
    case NeoPixelMode::Solid: {
      return 1000;
    }
    case NeoPixelMode::Rainbow:
    case NeoPixelMode::RainbowWave:
    case NeoPixelMode::RainbowWheel:
      return 10;
    case NeoPixelMode::TheaterChase:
      return 60;
    case NeoPixelMode::TheaterChaseRainbow:
      return 50;
    case NeoPixelMode::WipeHorizontal:
      return 70;
    case NeoPixelMode::WipeVertical:
      return 140;
    default:
      return 50;
  }
}

uint32_t NeoPixel::_getWheelColor(uint8_t position) {
  position = 255 - position;
  if (position < 85) {
    return _strip.Color(255 - position * 3, 0, position * 3);
  }
  if (position < 170) {
    position -= 85;
    return _strip.Color(0, position * 3, 255 - position * 3);
  }
  position -= 170;
  return _strip.Color(position * 3, 255 - position * 3, 0);
}

void NeoPixel::_handleMode() {
  static uint16_t currentStep = 0;
  static uint16_t currentSubStep = 0;
  static uint16_t stepDirection = 1;
  static uint32_t color = 0;

  if (_mode != _lastMode) {
    _strip.clear();
    _strip.show();

    currentStep = 0;
    currentSubStep = 0;
    stepDirection = 1;

    _lastMode = _mode;
    _lastStepTime = 0;
  }

  color = _strip.Color(_r, _g, _b);
  color = _strip.gamma32(color);

  switch (_mode)
  {
    case NeoPixelMode::Rainbow: {
      if (currentStep >= 768) {
        currentStep = 0;
      }

      uint32_t firstPixelHue = currentStep * 256;

      for (int i = 0; i < _strip.numPixels(); i++) {
        uint32_t pixelHue = firstPixelHue + (i * 65536L / _strip.numPixels());
        _strip.setPixelColor(i, _strip.gamma32(_strip.ColorHSV(pixelHue)));
      }
      
      break;
    }
    case NeoPixelMode::RainbowWave: {
      if (currentStep >= 768) {
        currentStep = 0;
      }

      uint32_t firstPixelHue = currentStep * 256;

      for (int i = 0; i < _strip.numPixels() / 4; i++) {
        uint32_t pixelHue = firstPixelHue + (i * 65536L / (_strip.numPixels() / 4));

        _strip.setPixelColor(i, _strip.gamma32(_strip.ColorHSV(pixelHue)));
        _strip.setPixelColor(i + 8, _strip.gamma32(_strip.ColorHSV(pixelHue)));
        _strip.setPixelColor(i + 16, _strip.gamma32(_strip.ColorHSV(pixelHue)));
        _strip.setPixelColor(i + 24, _strip.gamma32(_strip.ColorHSV(pixelHue)));
      }
      
      break;
    }
    case NeoPixelMode::RainbowWheel: {
      if (currentStep >= 256 * 5) {
        currentStep = 0;
      }

      for(int i = 0; i < _strip.numPixels(); i++) {
        _strip.setPixelColor(i, _getWheelColor(((i * 256 / _strip.numPixels()) + currentStep) & 255));
      }
      break;
    }
    case NeoPixelMode::Solid: {
      _strip.fill(color);
      break;
    }
    case NeoPixelMode::TheaterChase: {
      if (currentStep >= 3) {
        currentStep = 0;
      }

      _strip.clear();

      for (int i = currentStep; i < _strip.numPixels(); i += 3) {
        _strip.setPixelColor(i, color);
      }
      
      break;
    }
    case NeoPixelMode::TheaterChaseRainbow: {
      if (currentStep >= 768) {
        currentStep = 0;
      }

      _strip.clear();

      uint8_t firstPixel = abs(currentStep) % 4;
      uint32_t firstPixelHue = abs(currentStep) * 256;

      for (int i = firstPixel; i < _strip.numPixels(); i += 3) {
        uint32_t pixelHue = firstPixelHue + (i * 65536L / _strip.numPixels());
        _strip.setPixelColor(i, _strip.gamma32(_strip.ColorHSV(pixelHue)));
      }
      
      break;
    }
    case NeoPixelMode::WipeHorizontal: {
      if (currentStep >= 7) {
        stepDirection = -1;
      } else if (currentStep <= 0) {
        stepDirection = 1;
      }
      
      for(int i = 0; i < _strip.numPixels(); i++) {
        _strip.setPixelColor(i, i == currentStep || i == currentStep + 8 || i == currentStep + 16 || i == currentStep + 24 ? 0 : color);
      }
      break;
    }
    case NeoPixelMode::WipeVertical: {
      if (currentStep >= 3) {
        stepDirection = -1;
      } else if (currentStep <= 0) {
        stepDirection = 1;
      }
      
      uint16_t firstLed = currentStep * 8;
      uint16_t lastLed = firstLed + 7;

      for(int i = 0; i < _strip.numPixels(); i++) {
        _strip.setPixelColor(i, i >= firstLed && i <= lastLed ? 0 : color);
      }
      break;
    }
    default: {
      break;
    }
  }

  _strip.setBrightness(_brightness);
  _strip.show();

  if (millis() - _lastStepTime >= _getStepMillis()) {
    _lastStepTime = millis();
    currentStep += stepDirection;
  }
}

void NeoPixel::_modeTaskCode(void *args) {
  NeoPixel *neoPixel = (NeoPixel *)args;
  uint32_t notificationValue;

  for(;;) {
    xTaskNotifyWait(0, ULONG_MAX, &notificationValue, neoPixel->_getTicksToWait() + 1);
    neoPixel->_handleMode();
  }

  vTaskDelete(NULL);
}

void NeoPixel::_notifyModeTask() {
  TaskHandle_t modeTask = _modeTask;

  if (modeTask == NULL) {
    return;
  }

  xTaskNotify(modeTask, (uint32_t) true, eSetValueWithOverwrite);
}

void NeoPixel::_setBrightness(uint16_t brightness, bool update) {
  {
    LockGuard lock (_lock);

    if (brightness > 255) {
      brightness = NEOPIXEL_BRIGHTNESS_STEP;
    }

    if (_brightness == brightness) {
      return;
    }

    _brightness = (uint8_t) brightness;
    _preferences.putUChar(BRIGHTNESS_KEY, (uint8_t) _brightness);
  }

  if (update) {
    _notifyModeTask();
  }
}

void NeoPixel::_setColor(uint8_t r, uint8_t g, uint8_t b, bool update) {
  {
    LockGuard lock (_lock);

    if (_r == r && _g == g && _b == b) {
      return;
    }
  
    _r = r;
    _g = g;
    _b = b;
  }

  if (update) {
    _notifyModeTask();
  }
}

void NeoPixel::_setMode(NeoPixelMode mode, bool update) {
  {
    LockGuard lock (_lock);

    if (_mode == mode) {
      return;
    }

    _mode = mode;
    _preferences.putUChar(MODE_KEY, (uint8_t) _mode);
  }

  if (update) {
    _notifyModeTask();
  }
}

void NeoPixel::begin() {
  _preferences.begin("emilys_neopixel", false);

  _brightness = _preferences.getUChar(BRIGHTNESS_KEY, NEOPIXEL_BRIGHTNESS_STEP);
  _mode = (NeoPixelMode) _preferences.getUChar(MODE_KEY, 0);

  _strip.begin();
  _strip.setBrightness(_brightness);
  _strip.show();  // Initialize all pixels to 'off'

  if (_modeTask == NULL) {
    _createModeTask();
  }
}

void NeoPixel::end() {
  _deleteModeTask();
  _preferences.end();
}

void NeoPixel::nextBrightness() {
  _setBrightness((uint16_t) _brightness + NEOPIXEL_BRIGHTNESS_STEP, true);
}

void NeoPixel::nextMode() {
  uint8_t mode = (uint8_t) _mode;
  mode++;
  if (mode > NEOPIXEL_MAX_NEOPIXEL_MODE) {
    mode = 0;
  }

  _setMode((NeoPixelMode) mode, true);
}

void NeoPixel::setBrightness(uint8_t brightness) {
  _setBrightness(brightness, true);
}

void NeoPixel::setColor(uint8_t r, uint8_t g, uint8_t b) {
  _setColor(r, g, b, true);
}

void NeoPixel::setMode(NeoPixelMode mode) {
  _setMode(mode, true);
}