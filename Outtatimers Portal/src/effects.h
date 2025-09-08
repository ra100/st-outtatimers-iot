// Lightweight effects helpers made testable
#pragma once

#include <stdint.h>
#include <math.h>

#ifdef UNIT_TEST
// Provide a minimal CRGB struct for unit tests
struct CRGB
{
  uint8_t r, g, b;
  CRGB() : r(0), g(0), b(0) {}
  CRGB(uint8_t rr, uint8_t gg, uint8_t bb) : r(rr), g(gg), b(bb) {}
  void nscale8(uint8_t scale)
  {
    r = (uint8_t)((r * scale) / 255);
    g = (uint8_t)((g * scale) / 255);
    b = (uint8_t)((b * scale) / 255);
  }
  static CRGB Red() { return CRGB(255, 0, 0); }
  static CRGB Green() { return CRGB(0, 255, 0); }
  static CRGB Blue() { return CRGB(0, 0, 255); }
};
#else
#include <FastLED.h>
#endif

// Convert LED index to circular coordinates
void getLEDPosition(int ledIndex, int numLeds, float radius, float &x, float &y);

// Calculate distance from LED to circle center
float getCircleDistance(float x, float y);

// Linear interpolate between two CRGB colors
CRGB interpolateColor(const CRGB &c1, const CRGB &c2, float ratio);
