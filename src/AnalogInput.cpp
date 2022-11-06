#include "AnalogInput.h"

AnalogInput::AnalogInput(uint8_t pin): _pin(pin) {
  _currentRawValue = _getRawValue();
  _currentValue = _currentRawValue;
  _lastRawValue = _currentRawValue;
  _lastValue = _currentRawValue;

  if(_lock == NULL) {
    _lock = xSemaphoreCreateMutex();
    if(_lock == NULL) {
      // TODO: I should throw here...
      log_e("xSemaphoreCreateMutex failed");
      return;
    }
  }
}

AnalogInput::~AnalogInput() {
  end();

  if (_lock != NULL) {
    vSemaphoreDelete(_lock);
  }
}

void AnalogInput::_createInputTask() {
    xTaskCreateUniversal(_inputTaskCode, "analog_input_task", ANALOG_INPUT_TASK_STACK_SIZE, this, ANALOG_INPUT_TASK_PRIORITY, &_inputTask, ANALOG_INPUT_TASK_CORE);
    if (_inputTask == NULL) {
        log_e(" -- Error creating input task");
    }
}

bool AnalogInput::_debounceInput() {
  LockGuard lock (_lock);

  _currentRawValue = _getRawValue();

  if (_currentRawValue == _lastRawValue && (abs(_currentRawValue - _currentValue) > 1)) {
    if (millis() - _lastChangeTime >= _debounceWindow) {
      _currentValue = _currentRawValue;
      return true;
    }
  } else {
    _lastChangeTime = millis();
    _lastRawValue = _currentRawValue;
  }

  return false;
}

void AnalogInput::_deleteInputTask() {
  if (_inputTask != NULL) {
    vTaskDelete(_inputTask);
    _inputTask = NULL;
  }
}

uint16_t AnalogInput::_getRawValue() {
  uint16_t currentRawValue = analogRead(_pin);
  return currentRawValue;
}

void AnalogInput::_handleInput() {
  if (!_debounceInput()) {
    return;
  }

  if (_currentValue != _lastValue) {
    _raiseOnEvent(_currentValue);
  }

  _lastValue = _currentValue;
}

void AnalogInput::_inputTaskCode(void *args) {
  AnalogInput *analogInput = (AnalogInput *)args;
  uint32_t notificationTime = 0;
  uint32_t notificationValue;
  uint32_t ticksToWait = pdMS_TO_TICKS(10);// portMAX_DELAY;

  for(;;) {
    //ticksToWait = pdMS_TO_TICKS(analogInput->_debounceWindow / 5);
    xTaskNotifyWait(0, ULONG_MAX, &notificationValue, ticksToWait);
    analogInput->_handleInput();
  }

  vTaskDelete(NULL);
}

void AnalogInput::_raiseOnEvent(uint16_t value) {
  AnalogInputEventHandler eventHandler = _eventHandler;

  if (eventHandler != NULL) {
    eventHandler(value);
  }
}

void AnalogInput::begin() {
  pinMode(_pin, INPUT);

  if (_inputTask == NULL) {
    _createInputTask();
  }
}

void AnalogInput::end() {
  _deleteInputTask();
}

uint16_t AnalogInput::getValue() {
  return _currentValue;
}

void AnalogInput::onEvent(AnalogInputEventHandler callback) {
  LockGuard lock (_lock);
  _eventHandler = callback;
}

void AnalogInput::setDebounce(uint16_t debounceWindow) {
  _debounceWindow = debounceWindow;
}