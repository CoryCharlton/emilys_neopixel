#ifndef EMILYS_NEOPIXEL_DIGITAL_INPUT_H
#define EMILYS_NEOPIXEL_DIGITAL_INPUT_H

#include <Arduino.h>
#include "LockGuard.h"

#define DIGITAL_INPUT_DEFAULT_DEBOUNCE_WINDOW 20
#define DIGITAL_INPUT_DEFAULT_LONG_TRIGGER_WINDOW 150
#define DIGITAL_INPUT_DEFAULT_MULTI_TRIGGER_WINDOW 400

#define DIGITAL_INPUT_NOTIFICATION_WINDOW 5000

#define DIGITAL_INPUT_TASK_CORE tskNO_AFFINITY
#define DIGITAL_INPUT_TASK_PRIORITY (configMAX_PRIORITIES-1)
#define DIGITAL_INPUT_TASK_STACK_SIZE 2048

#define DIGITAL_INPUT_RELEASED LOW
#define DIGITAL_INPUT_TRIGGERED HIGH

enum class DigitalInputEvent: uint8_t {
    Release = 0,
    Trigger = 1,
    LongTrigger = 2,
    MultiTrigger = 3
};

typedef std::function<void(DigitalInputEvent)> DigitalInputEventHandler;

class DigitalInput {
  public:
    DigitalInput(uint8_t pin, uint8_t trigger = LOW);
    ~DigitalInput();

    void begin();
    void end();
    bool isTriggered();
    void onEvent(DigitalInputEventHandler callback);
    void setDebounce(uint16_t debounceWindow = DIGITAL_INPUT_DEFAULT_DEBOUNCE_WINDOW);
    void setLongTrigger(uint16_t longTriggerWindow = DIGITAL_INPUT_DEFAULT_LONG_TRIGGER_WINDOW);
    void setMultiTrigger(uint16_t multiTriggerWindow = DIGITAL_INPUT_DEFAULT_MULTI_TRIGGER_WINDOW);

  protected:
    bool _inverted;
    uint8_t _pin;
    uint8_t _trigger;

    uint16_t _debounceWindow = 0;
    uint16_t _longTriggerWindow = 0; 
    uint8_t _multiTriggerTarget = 2;
    uint16_t _multiTriggerWindow = 0;

    uint32_t _lastChangeTime = 0;
    uint8_t _multiTriggerCount = 0;
    uint32_t _multiTriggerStartTime = 0;
    uint32_t _triggerStartTime = 0;

    bool _currentRawState;
    bool _currentState;
    bool _lastRawState;
    bool _lastState;

    DigitalInputEventHandler _eventHandler;
    TaskHandle_t _inputTask;
    SemaphoreHandle_t _lock;

    void _createInputTask();
    bool _debounceInput();
    void _deleteInputTask();
    bool _getRawState();
    void _handleInput();
    static void _inputTaskCode(void *args);
    static void IRAM_ATTR _onInputChange(void *args);
    void _raiseOnEvent(DigitalInputEvent event);
};
#endif