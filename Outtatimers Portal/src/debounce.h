#pragma once

#ifdef UNIT_TEST
using unsigned_long_t = unsigned long;
#define HIGH 1
#define LOW 0
#else
#include <Arduino.h>
using unsigned_long_t = unsigned long;
#endif

class Debounce
{
public:
  Debounce(unsigned_long_t intervalMs = 50) : _interval(intervalMs), _lastChange(0), _stableState(HIGH), _lastRead(HIGH) {}

  // Call periodically with the raw pin state (HIGH/LOW). Returns true when a stable state change is detected.
  bool sample(int rawState, unsigned_long_t now)
  {
    if (rawState != _lastRead)
    {
      _lastChange = now;
      _lastRead = rawState;
    }
    else if (now - _lastChange >= _interval)
    {
      if (_stableState != _lastRead)
      {
        _stableState = _lastRead;
        return true;
      }
    }
    return false;
  }

  int getState() const { return _stableState; }

private:
  unsigned_long_t _interval;
  unsigned_long_t _lastChange;
  int _stableState;
  int _lastRead;
};
