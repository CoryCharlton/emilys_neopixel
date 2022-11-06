#include "DigitalInput.h"

DigitalInput::DigitalInput(uint8_t pin, uint8_t trigger): _pin(pin), _trigger(trigger) {
  _inverted = (trigger == LOW);

  _currentRawState = _getRawState();
  _currentState = _currentRawState;
  _lastRawState = _currentRawState;
  _lastState = _currentRawState;

  if(_lock == NULL) {
    _lock = xSemaphoreCreateMutex();
    if(_lock == NULL) {
      log_e("xSemaphoreCreateMutex failed");
      return;
    }
  }
}

DigitalInput::~DigitalInput() {
  end();

  if (_lock != NULL) {
    vSemaphoreDelete(_lock);
  }
}

void DigitalInput::_createInputTask() {
    xTaskCreateUniversal(_inputTaskCode, "digital_input_task", DIGITAL_INPUT_TASK_STACK_SIZE, this, DIGITAL_INPUT_TASK_PRIORITY, &_inputTask, DIGITAL_INPUT_TASK_CORE);
    if (_inputTask == NULL) {
        log_e(" -- Error creating input task");
    }
}

bool DigitalInput::_debounceInput() {
  LockGuard lock (_lock);

  _currentRawState = _getRawState();

  if (_currentRawState == _lastRawState) {
    if (millis() - _lastChangeTime >= _debounceWindow) {
      _currentState = _currentRawState;
      return true;
    }
  } else {
    _lastChangeTime = millis();
    _lastRawState = _currentRawState;
  }

  return false;
}

void DigitalInput::_deleteInputTask() {
  if (_inputTask != NULL) {
    vTaskDelete(_inputTask);
    _inputTask = NULL;
  }
}

bool DigitalInput::_getRawState() {
  bool currentRawState = digitalRead(_pin);

  if (_inverted) {
    currentRawState = !currentRawState;
  }

  return currentRawState;
}

void DigitalInput::_handleInput() {
  if (!_debounceInput()) {
    return;
  }

  if (_currentState == DIGITAL_INPUT_TRIGGERED && _lastState == DIGITAL_INPUT_TRIGGERED) {
    if (_longTriggerWindow > 0 && _triggerStartTime != 0 && millis() - _triggerStartTime >= _longTriggerWindow) {
      _raiseOnEvent(DigitalInputEvent::LongTrigger);
      _triggerStartTime = 0;
    }
  } else if (_currentState == DIGITAL_INPUT_TRIGGERED && _lastState == DIGITAL_INPUT_RELEASED) {
      _raiseOnEvent(DigitalInputEvent::Trigger);
      _triggerStartTime = millis();
  } else if (_currentState == DIGITAL_INPUT_RELEASED && _lastState == DIGITAL_INPUT_TRIGGERED) {
      _raiseOnEvent(DigitalInputEvent::Release);

      _multiTriggerCount++;

      if (_multiTriggerCount == 1) {
        _multiTriggerStartTime = millis();
      } else {
        if (_multiTriggerWindow > 0 && millis() - _multiTriggerStartTime <= _multiTriggerWindow) {
          if (_multiTriggerCount >= _multiTriggerTarget) {
            _raiseOnEvent(DigitalInputEvent::MultiTrigger);
            _multiTriggerCount = 0;
          }
        } else {
          _multiTriggerCount = 1;
          _multiTriggerStartTime = millis();
        }
      }
  }

  _lastState = _currentState;
}

void DigitalInput::_inputTaskCode(void *args) {
  DigitalInput *digitalInput = (DigitalInput *)args;
  uint32_t notificationTime = 0;
  uint32_t notificationValue;
  uint32_t ticksToWait = portMAX_DELAY;

  for(;;) {
    ticksToWait = (millis() - notificationTime) >= DIGITAL_INPUT_NOTIFICATION_WINDOW ? portMAX_DELAY : 1;
  
    if (xTaskNotifyWait(0, ULONG_MAX, &notificationValue, ticksToWait) == pdTRUE) {
      notificationTime = millis();
    }

    digitalInput->_handleInput();
  }

  vTaskDelete(NULL);
}

void IRAM_ATTR DigitalInput::_onInputChange(void *args) {
  DigitalInput *digitalInput = (DigitalInput *)args;
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  xTaskNotifyFromISR(digitalInput->_inputTask, (uint32_t)true, eSetValueWithOverwrite, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void DigitalInput::_raiseOnEvent(DigitalInputEvent event) {
  DigitalInputEventHandler eventHandler = _eventHandler;

  if (eventHandler != NULL) {
    eventHandler(event);
  }
}

void DigitalInput::begin() {
  if (_inputTask == NULL) {
    _createInputTask();
  }

  attachInterruptArg(_pin, _onInputChange, this, CHANGE);
}

void DigitalInput::end() {
  _deleteInputTask();
}

bool DigitalInput::isTriggered() {
  return _currentState;
}

void DigitalInput::onEvent(DigitalInputEventHandler callback) {
  LockGuard lock (_lock);
  _eventHandler = callback;
}

void DigitalInput::setDebounce(uint16_t debounceWindow) {
  _debounceWindow = debounceWindow;
}

void DigitalInput::setLongTrigger(uint16_t longTriggerWindow) {
  _longTriggerWindow = longTriggerWindow;
}

void DigitalInput::setMultiTrigger(uint16_t multiTriggerWindow) {
  _multiTriggerWindow = multiTriggerWindow;
}