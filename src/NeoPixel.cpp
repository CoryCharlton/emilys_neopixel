#include "NeoPixel.h"

const char* NeoPixel::BRIGHTNESS_KEY = "brightness";
const char* NeoPixel::MODE_KEY = "mode";

NeoPixel::NeoPixel(uint8_t pin): 
  _strip(NEOPIXEL_LED_COUNT, pin, NEO_GRB + NEO_KHZ800) {

  if(_lock == NULL) {
    _lock = xSemaphoreCreateMutex();
    if(_lock == NULL) {
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
    _lastStepTime = millis();
  }

  color = _strip.Color(_r, _g, _b);

  if (millis() - _lastStepTime >= _getStepMillis()) {
    _lastStepTime = millis();
    currentStep += stepDirection;
  }

  switch (_mode)
  {
    case NeoPixelMode::Off: {
      esp_light_sleep_start();
      break;
    }
    case NeoPixelMode::Rainbow: {
      if (currentStep >= 256) {
        currentStep = 0;
      }

      uint32_t firstPixelHue = currentStep * 256;

      for (int i = 0; i < NEOPIXEL_LED_COUNT; i++) {
        uint32_t pixelHue = firstPixelHue + (i * 65536L / NEOPIXEL_LED_COUNT);
        _strip.setPixelColor(i, _strip.gamma32(_strip.ColorHSV(pixelHue)));
      }
      
      break;
    }
    case NeoPixelMode::RainbowWave: {
      if (currentStep >= 256) {
        currentStep = 0;
      }

      uint32_t firstPixelHue = currentStep * 256;

      for (int i = 0; i < NEOPIXEL_LED_COLS; i++) {
        uint32_t pixelHue = firstPixelHue + (i * 65536L / NEOPIXEL_LED_COLS);

        _strip.setPixelColor(i, _strip.gamma32(_strip.ColorHSV(pixelHue)));
        _strip.setPixelColor(i + NEOPIXEL_LED_COLS, _strip.gamma32(_strip.ColorHSV(pixelHue)));
        _strip.setPixelColor(i + NEOPIXEL_LED_COLS * 2, _strip.gamma32(_strip.ColorHSV(pixelHue)));
        _strip.setPixelColor(i + NEOPIXEL_LED_COLS * 3, _strip.gamma32(_strip.ColorHSV(pixelHue)));
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

      for (int i = currentStep; i < NEOPIXEL_LED_COUNT; i += 3) {
        _strip.setPixelColor(i, color);
      }
      
      break;
    }
    case NeoPixelMode::TheaterChaseRainbow: {
      if (currentStep >= 256) {
        currentStep = 0;
      }

      _strip.clear();

      uint8_t firstPixel = currentStep % NEOPIXEL_LED_ROWS;
      uint32_t firstPixelHue = currentStep * 256;

      for (int i = firstPixel; i < NEOPIXEL_LED_COUNT; i += 3) {
        uint32_t pixelHue = firstPixelHue + (i * 65536L / NEOPIXEL_LED_COUNT);
        _strip.setPixelColor(i, _strip.gamma32(_strip.ColorHSV(pixelHue)));
      }
      
      break;
    }
    case NeoPixelMode::WipeHorizontal: {
      if (currentStep >= NEOPIXEL_LED_COLS - 1) {
        stepDirection = -1;
      } else if (currentStep <= 0) {
        stepDirection = 1;
      }
      
      _strip.fill(color);
      _strip.setPixelColor(currentStep, 0);
      _strip.setPixelColor(currentStep + NEOPIXEL_LED_COLS, 0);
      _strip.setPixelColor(currentStep + NEOPIXEL_LED_COLS * 2, 0);
      _strip.setPixelColor(currentStep + NEOPIXEL_LED_COLS * 3, 0);
      break;
    }
    case NeoPixelMode::WipeVertical: {
      if (currentStep >= NEOPIXEL_LED_ROWS - 1) {
        stepDirection = -1;
      } else if (currentStep <= 0) {
        stepDirection = 1;
      }
      
      uint16_t firstLed = currentStep * NEOPIXEL_LED_COLS;
      uint16_t lastLed = firstLed + NEOPIXEL_LED_COLS - 1;

      for(int i = 0; i < NEOPIXEL_LED_COUNT; i++) {
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

    log_d("Brightness: %d", brightness);

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
  
    log_d("Color: %d %d %d", r, g, b);

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

    log_d("Mode: %d", mode);

    _mode = mode;

    if (_mode == NeoPixelMode::Off) {
      _preferences.putUChar(MODE_KEY, (uint8_t) NEOPIXEL_DEFAULT_MODE);
    } else {
      _preferences.putUChar(MODE_KEY, (uint8_t) _mode);
    }
  }

  if (update) {
    _notifyModeTask();
  }
}

void NeoPixel::begin() {
  _preferences.begin("emilys_neopixel", false);

  _brightness = _preferences.getUChar(BRIGHTNESS_KEY, NEOPIXEL_BRIGHTNESS_STEP);
  _mode = (NeoPixelMode) _preferences.getUChar(MODE_KEY, 0);

  if (_mode == NeoPixelMode::Off) {
    _mode = (NeoPixelMode) NEOPIXEL_DEFAULT_MODE;
  }

  _strip.begin();
  _strip.setBrightness(_brightness);
  _strip.show();

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
  if (mode > NEOPIXEL_MAX_MODE) {
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