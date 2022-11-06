#ifndef EMILYS_NEOPIXEL_ANALOG_INPUT_H
#define EMILYS_NEOPIXEL_ANALOG_INPUT_H

#include <Arduino.h>
#include "LockGuard.h"

#define ANALOG_INPUT_DEFAULT_DEBOUNCE_WINDOW 10

#define ANALOG_INPUT_TASK_CORE tskNO_AFFINITY
#define ANALOG_INPUT_TASK_PRIORITY (configMAX_PRIORITIES-1)
#define ANALOG_INPUT_TASK_STACK_SIZE 2048

typedef std::function<void(uint16_t)> AnalogInputEventHandler;

class AnalogInput {
  public:
    AnalogInput(uint8_t pin);
    ~AnalogInput();

    void begin();
    void end();
    uint16_t getValue();
    void onEvent(AnalogInputEventHandler callback);
    void setDebounce(uint16_t debounceWindow = ANALOG_INPUT_DEFAULT_DEBOUNCE_WINDOW);

  protected:
    uint8_t _pin;

    uint16_t _debounceWindow = 0;

    uint16_t _currentRawValue;
    uint16_t _currentValue;
    uint32_t _lastChangeTime = 0;
    uint16_t _lastRawValue;
    uint16_t _lastValue;

    AnalogInputEventHandler _eventHandler;
    TaskHandle_t _inputTask;
    SemaphoreHandle_t _lock;

    void _createInputTask();
    bool _debounceInput();
    void _deleteInputTask();
    uint16_t _getRawValue();
    void _handleInput();
    static void _inputTaskCode(void *args);
    void _raiseOnEvent(uint16_t value);
};
#endif