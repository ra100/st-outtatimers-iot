#pragma once

#include "effects.h"
#include "led_driver.h"
#include "config.h"
#ifndef UNIT_TEST
#include <Arduino.h>
#else
// When running unit tests on host, provide a declaration for millis() with C linkage
extern "C" unsigned long millis();
#endif

#ifdef UNIT_TEST
// Provide small helpers to emulate Arduino behavior used in portal_effect
#include <cstdlib>
static inline int rnd(int max) { return rand() % max; }
static inline int rndRange(int a, int b) { return a + (rand() % (b - a)); }
static inline float rndf(int max) { return (float)(rand() % max); }
static inline float constrainf(float v, float a, float b) { return v < a ? a : (v > b ? b : v); }

#ifdef UNIT_TEST
// Simple CHSV -> CRGB, using hue only as index into a small palette approximation
static inline CRGB CHSV(uint8_t h, uint8_t s, uint8_t v)
{
  // naive mapping for tests: treat hue 0 -> red, 85 -> green, 170 -> blue
  if (h < 85)
    return CRGB(v, (uint8_t)((h * s) / 85), 0);
  if (h < 170)
    return CRGB((uint8_t)(((170 - h) * s) / 85), v, 0);
  return CRGB(0, (uint8_t)(((h - 170) * s) / 85), v);
}

#ifdef UNIT_TEST
// Provide Arduino-like random() overloads for host tests but avoid defining a
// function named `random` that conflicts with libc; use arduino_random and map
// the macro name `random` to it.
static inline long arduino_random(long max)
{
  if (max <= 0)
    return 0;
  return rand() % max;
}
static inline long arduino_random(long min, long max)
{
  if (max <= min)
    return min;
  return min + (rand() % (max - min));
}
#define random(...) arduino_random(__VA_ARGS__)

// constrain macro compatibility
#define constrain(x, a, b) (constrainf((x), (a), (b)))
#endif
#endif

#endif

// Template PortalEffect uses a driver and static buffers sized at compile time
template <int N, int GRADIENT_STEP, int GRADIENT_MOVE>
class PortalEffectTemplate
{
public:
  PortalEffectTemplate(ILEDDriver *driver) : _driver(driver)
  {
    NUM_LEDS = N;
    gradientPosition = 0;
    animationActive = false;
    fadeInActive = false;
    fadeInStart = 0;
    fadeOutActive = false;
    fadeOutStart = 0;
    malfunctionActive = false;
    lastUpdate = 0;
    numGradientPoints = 0;
  }

  void begin()
  {
    _driver->begin();
    _leds = _driver->getBuffer();
    // effectLeds and gradientColors are static arrays
  }

  void setBrightness(uint8_t b) { _driver->setBrightness(b); }
  void fillSolid(const CRGB &c)
  {
    _driver->fillSolid(c);
    _driver->show();
  }
  void clear()
  {
    _driver->clear();
    _driver->show();
  }

  void start()
  {
    if (!animationActive)
    {
      animationActive = true;
      fadeInActive = true;
      fadeInStart = millis();
      gradientPosition = 0;
      generatePortalEffect();
    }
  }

  void stop()
  {
    animationActive = false;
    _driver->clear();
    _driver->show();
  }

  void triggerFadeOut()
  {
    if (!fadeOutActive && (animationActive || malfunctionActive))
    {
      fadeOutActive = true;
      fadeOutStart = millis();
      fadeInActive = false;
      animationActive = false;
      malfunctionActive = false;
    }
  }

  void triggerMalfunction()
  {
    if (!malfunctionActive)
    {
      malfunctionActive = true;
      animationActive = false;
    }
  }

  void update(unsigned long now)
  {
    if (fadeOutActive || malfunctionActive || animationActive)
    {
      if (now - lastUpdate >= 10)
      {
        gradientPosition = (gradientPosition + GRADIENT_MOVE) % NUM_LEDS;
        if (fadeOutActive || animationActive)
          portalEffect();
        else if (malfunctionActive)
          portalMalfunctionEffect();
        lastUpdate = now;
      }
    }
  }

private:
  ILEDDriver *_driver;
  CRGB *_leds;
  CRGB effectLeds[N];                         // Changed from static to instance storage
  CRGB gradientColors[N / GRADIENT_STEP + 2]; // Changed from static to instance storage
  int numGradientPoints;

  int NUM_LEDS;
  int gradientPosition;
  bool animationActive;
  bool fadeInActive;
  unsigned long fadeInStart;
  bool fadeOutActive;
  unsigned long fadeOutStart;
  bool malfunctionActive;
  unsigned long lastUpdate;

  CRGB getRandomDriverColorInternal()
  {
    uint8_t hue = PortalConfig::Effects::PORTAL_HUE_BASE + random(PortalConfig::Effects::PORTAL_HUE_RANGE);
    uint8_t sat = PortalConfig::Effects::PORTAL_SAT_BASE + random(PortalConfig::Effects::PORTAL_SAT_RANGE);
    if (random(PortalConfig::Effects::PORTAL_LOW_SAT_PROBABILITY) == 0)
      sat = PortalConfig::Effects::PORTAL_SAT_LOW_BASE + random(PortalConfig::Effects::PORTAL_SAT_LOW_RANGE);
    uint8_t val = PortalConfig::Effects::PORTAL_VAL_BASE + random(PortalConfig::Effects::PORTAL_VAL_RANGE);
    return CHSV(hue, sat, val);
  }

  void generatePortalEffect()
  {
    const int minDist = PortalConfig::Effects::MIN_DRIVER_DISTANCE;
    const int maxDist = PortalConfig::Effects::MAX_DRIVER_DISTANCE;
    int driverIndices[N];
    CRGB driverColors[N];
    int numDrivers = 0;
    int idx = 0;
    while (idx < NUM_LEDS - minDist && numDrivers < N - 1)
    {
      driverIndices[numDrivers] = idx;
      driverColors[numDrivers] = getRandomDriverColorInternal();
      numDrivers++;
      int step = minDist + random(maxDist - minDist + 1);
      if (idx + step > NUM_LEDS - minDist)
        break;
      idx += step;
    }
    driverIndices[numDrivers] = NUM_LEDS;
    driverColors[numDrivers] = driverColors[0];
    numDrivers++;

    for (int d = 0; d < numDrivers - 1; d++)
    {
      int start = driverIndices[d];
      int end = driverIndices[d + 1];
      CRGB c1 = driverColors[d];
      CRGB c2 = driverColors[d + 1];
      int segLen = end - start;
      for (int i = 0; i < segLen; i++)
      {
        float ratio = (segLen == 1) ? 0.0f : (float)i / (segLen - 1);
        CRGB col = interpolateColor(c1, c2, ratio);
        int pos = start + i;
        if (pos >= 0 && pos < NUM_LEDS)
          effectLeds[pos] = col;
      }
    }
  }

  void portalEffect()
  {
    float fadeScale = 1.0f;
    if (fadeInActive)
    {
      unsigned long now = millis();
      float t = (now - fadeInStart) / (float)PortalConfig::Timing::FADE_IN_DURATION_MS;
      fadeScale = constrain(t, 0.0f, 1.0f);
      if (fadeScale >= 1.0f)
      {
        fadeInActive = false;
        fadeScale = 1.0f;
      }
    }
    else if (fadeOutActive)
    {
      unsigned long now = millis();
      float t = (now - fadeOutStart) / (float)PortalConfig::Timing::FADE_OUT_DURATION_MS;
      fadeScale = 1.0f - constrain(t, 0.0f, 1.0f);
      if (fadeScale <= 0.0f)
      {
        fadeOutActive = false;
        fadeScale = 0.0f;
        animationActive = false;
        _driver->clear();
        _driver->show();
        return;
      }
    }
    for (int i = 0; i < NUM_LEDS; i++)
    {
      _driver->setPixel(i, effectLeds[(i + gradientPosition) % NUM_LEDS]);
      if (fadeScale < 1.0f)
        _driver->getBuffer()[i].nscale8((uint8_t)(fadeScale * 255));
    }
    _driver->show();
  }

  void portalMalfunctionEffect()
  {
    unsigned long now = millis();
    static unsigned long lastJump = 0;
    static float targetBrightness = 1.0f;
    static float currentBrightness = 1.0f;
    static int jumpInterval = 100;
    gradientPosition = (gradientPosition + GRADIENT_MOVE) % NUM_LEDS;

    if (now - lastJump > jumpInterval)
    {
      targetBrightness = PortalConfig::Effects::MALFUNCTION_BRIGHTNESS_MIN +
                         PortalConfig::Effects::MALFUNCTION_BRIGHTNESS_RANGE * (random(1000) / 1000.0f);
      jumpInterval = PortalConfig::Timing::MALFUNCTION_MIN_JUMP_MS +
                     random(PortalConfig::Timing::MALFUNCTION_MAX_JUMP_MS - PortalConfig::Timing::MALFUNCTION_MIN_JUMP_MS);
      lastJump = now;
    }
    float delta = targetBrightness - currentBrightness;
    currentBrightness += delta * (PortalConfig::Effects::MALFUNCTION_BRIGHTNESS_SMOOTHING_MIN +
                                  PortalConfig::Effects::MALFUNCTION_BRIGHTNESS_SMOOTHING_RANGE * (random(1000) / 1000.0f));
    currentBrightness += (random(-PortalConfig::Effects::MALFUNCTION_NOISE_OFFSET, PortalConfig::Effects::MALFUNCTION_NOISE_OFFSET + 1)) / 255.0f;
    currentBrightness = constrain(currentBrightness,
                                  PortalConfig::Effects::MALFUNCTION_BRIGHTNESS_CLAMP_MIN,
                                  PortalConfig::Effects::MALFUNCTION_BRIGHTNESS_CLAMP_MAX);

    for (int i = 0; i < NUM_LEDS; i++)
    {
      _driver->setPixel(i, effectLeds[(i + gradientPosition) % NUM_LEDS]);
      _driver->getBuffer()[i].nscale8((uint8_t)(currentBrightness * PortalConfig::Effects::MALFUNCTION_BASE_BRIGHTNESS + PortalConfig::Effects::MALFUNCTION_BRIGHTNESS_OFFSET));
    }
    _driver->show();
  }
};

// Static storage definitions removed - now using instance storage
// This eliminates the critical bug where multiple instances would share the same buffers
