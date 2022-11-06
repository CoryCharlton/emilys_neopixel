#ifndef EMILYS_NEOPIXEL_LOCK_GUARD_H
#define EMILYS_NEOPIXEL_LOCK_GUARD_H

#include <Arduino.h>

class LockGuard final {
  public:
    explicit LockGuard(SemaphoreHandle_t& __m) : _mutex(__m) {
      xSemaphoreTake(_mutex, portMAX_DELAY);
    }
    
    ~LockGuard() {
      xSemaphoreGive(_mutex);
    }

      LockGuard(const LockGuard&) = delete;
      LockGuard& operator=(const LockGuard&) = delete;
      
  private:
    SemaphoreHandle_t& _mutex;
};
#endif