#include "effects.h"

void getLEDPosition(int ledIndex, int numLeds, float radius, float &x, float &y)
{
  const float PI_F = 3.14159265358979323846f;
  float angle = (2.0f * PI_F * ledIndex) / numLeds;
  x = radius * cosf(angle);
  y = radius * sinf(angle);
}

float getCircleDistance(float x, float y)
{
  return sqrtf(x * x + y * y);
}

CRGB interpolateColor(const CRGB &c1, const CRGB &c2, float ratio)
{
  if (ratio < 0.0f)
    ratio = 0.0f;
  if (ratio > 1.0f)
    ratio = 1.0f;
  uint8_t r = (uint8_t)(c1.r + (c2.r - c1.r) * ratio);
  uint8_t g = (uint8_t)(c1.g + (c2.g - c1.g) * ratio);
  uint8_t b = (uint8_t)(c1.b + (c2.b - c1.b) * ratio);
  return CRGB(r, g, b);
}
