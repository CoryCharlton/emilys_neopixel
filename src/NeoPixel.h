#ifndef EMILYS_NEOPIXEL_NEOPIXEL_H
#define EMILYS_NEOPIXEL_NEOPIXEL_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <Preferences.h>

#include "LockGuard.h"

#define NEOPIXEL_MODE_TASK_CORE tskNO_AFFINITY
#define NEOPIXEL_MODE_TASK_PRIORITY (configMAX_PRIORITIES-1)
#define NEOPIXEL_MODE_TASK_STACK_SIZE 2048

enum class NeoPixelMode: uint8_t {
    Off = 0,
    Solid = 1,
    WipeHorizontal = 2,
    WipeVertical = 3,
    TheaterChase = 4,
    Rainbow = 5,
    RainbowWave = 6,
    TheaterChaseRainbow = 7,
};

#define NEOPIXEL_BRIGHTNESS_STEP 50
#define NEOPIXEL_DEFAULT_MODE 1
#define NEOPIXEL_LED_COLS 8
#define NEOPIXEL_LED_COUNT 32
#define NEOPIXEL_LED_ROWS 4
#define NEOPIXEL_MAX_MODE 7
#define NEOPIXEL_STEP_MILLIS 50

class NeoPixel {
  public:
    NeoPixel(uint8_t pin);
    ~NeoPixel();

    void begin();
    void end();
    void loop();
    void nextBrightness();
    void nextMode();
    void setBrightness(uint8_t brightness);
    void setColor(uint8_t r, uint8_t g, uint8_t b);
    void setMode(NeoPixelMode mode);

  private:
    uint8_t _brightness = NEOPIXEL_BRIGHTNESS_STEP;
    uint8_t _r = 0;
    uint8_t _g = 0;
    uint8_t _b = 0;
    NeoPixelMode _lastMode;
    uint32_t _lastStepTime = 0;
    SemaphoreHandle_t _lock;
    NeoPixelMode _mode = (NeoPixelMode) NEOPIXEL_DEFAULT_MODE;
    TaskHandle_t _modeTask;
    Preferences _preferences;
    Adafruit_NeoPixel _strip;

    static const char* BRIGHTNESS_KEY;
    static const char* MODE_KEY;

    void _createModeTask();
    void _deleteModeTask();
    TickType_t _getTicksToWait();
    uint32_t _getStepMillis();
    uint32_t _getWheelColor(uint8_t position);
    void _handleMode();
    static void _modeTaskCode(void *args);
    void _notifyModeTask();
    void _setBrightness(uint16_t brightness, bool update);
    void _setColor(uint8_t r, uint8_t g, uint8_t b, bool update);
    void _setMode(NeoPixelMode mode, bool update);
};
#endif