#pragma once

#include "effects.h"
#include "led_driver.h"
#include "config.h"
#include "config_manager.h"
#ifndef UNIT_TEST
#include <Arduino.h>
#include <math.h>
#else
// When running unit tests on host, provide a declaration for millis() with C linkage
extern "C" unsigned long millis();
#endif

#ifdef UNIT_TEST
// Provide small helpers to emulate Arduino behavior used in turbolift_effect
#include <cstdlib>
static inline int rnd(int max) { return rand() % max; }
static inline int rndRange(int a, int b) { return a + (rand() % (b - a)); }
static inline float rndf(int max) { return (float)(rand() % max); }
static inline float constrainf(float v, float a, float b) { return v < a ? a : (v > b ? b : v); }

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

// Template TurboliftEffect uses a driver and static buffers sized at compile time
template <int N, int GRADIENT_STEP, int GRADIENT_MOVE>
class TurboliftEffectTemplate
{
public:
  TurboliftEffectTemplate(ILEDDriver *driver) : _driver(driver)
  {
    NUM_LEDS = N;
    animationActive = false;
    fadeInActive = false;
    fadeInStart = 0;
    fadeOutActive = false;
    fadeOutStart = 0;
    malfunctionActive = false;
    lastUpdate = 0;
    // Lift animation state
    liftOffset = 0;
    liftLastMove = 0;
    liftPrevSubmode = 0;
    previousColor = CRGB(0, 0, 0);
    targetColor = CRGB(0, 0, 0);
    colorBlendFactor = 0.0f;
  }

  void begin()
  {
    _driver->begin();
    _leds = _driver->getBuffer();
    // effectLeds are static arrays
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
      if (now - lastUpdate >= TurboliftConfig::Timing::UPDATE_INTERVAL_MS)
      {
        uint8_t mode = ConfigManager::getEffectMode();

        // Dispatch to appropriate effect based on mode
        switch (mode)
        {
        case (uint8_t)TurboliftConfig::Effects::EffectMode::SINGLE_COLOR:
          singleColorEffect(now);
          break;
        case (uint8_t)TurboliftConfig::Effects::EffectMode::LIFT_ANIMATION:
          liftAnimationEffect(now);
          break;
        }
          
        lastUpdate = now;
      }
    }
  }

private:
  ILEDDriver *_driver;
  CRGB *_leds;
#ifdef UNIT_TEST
public:
#endif

  int NUM_LEDS;
  bool animationActive;
  bool fadeInActive;
  unsigned long fadeInStart;
  bool fadeOutActive;
  unsigned long fadeOutStart;
  bool malfunctionActive;
  unsigned long lastUpdate;

  // Lift animation state
  int liftOffset;              // Pixel offset in the repeating pattern
  unsigned long liftLastMove;  // Last time offset was incremented
  uint8_t liftPrevSubmode;     // Previous sub-mode (to detect changes and reset offset)
  CRGB previousColor;          // For smooth color transitions
  CRGB targetColor;            // Target color for smooth transitions
  float colorBlendFactor;      // Current blend factor for color transition


  CRGB getRandomDriverColorInternal()
  {
    // Handle hue range with wrap-around (e.g., min=250, max=10 for crossing 0/255)
    uint8_t hueMin = ConfigManager::getHueMin();
    uint8_t hueMax = ConfigManager::getHueMax();
    uint8_t length;
    if (hueMin <= hueMax)
    {
      length = hueMax - hueMin + 1;
    }
    else
    {
      length = 256 - hueMin + hueMax + 1;
    }
    uint8_t offset = random(length);
    uint8_t hue = (hueMin + offset) % 256;

    uint8_t satMin = ConfigManager::getSatMin();
    uint8_t satMax = ConfigManager::getSatMax();
    uint8_t satRange = satMax - satMin;
    uint8_t sat = satMin + random(satRange + 1);
    if (random(10) == 0)    // 1 in 10 chance for low saturation
      sat = 0 + random(50); // Low saturation range
    uint8_t val = TurboliftConfig::Effects::TURBOLIFT_VAL_BASE + random(TurboliftConfig::Effects::TURBOLIFT_VAL_RANGE);
    return CHSV(hue, sat, val);
  }

  CRGB *generateDriverColors(CRGB *driverColors, int &numDrivers, bool useBlackDrivers = false, uint8_t hue = 0)
  {
    const int minDist = TurboliftConfig::Effects::MIN_DRIVER_DISTANCE;
    const int maxDist = TurboliftConfig::Effects::MAX_DRIVER_DISTANCE;
    static int driverIndices[N];
    numDrivers = 0;
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

    if (useBlackDrivers)
    {
      for (int i = 0; i < numDrivers; i++)
      {
        if (i % 2 != 0)
        {
          driverColors[i] = CRGB(0, 0, 0); // Black for odd drivers
        }
        else
        {
          driverColors[i] = CHSV(hue,
                                 ConfigManager::getSatMin() + random(ConfigManager::getSatMax() - ConfigManager::getSatMin() + 1),
                                 TurboliftConfig::Effects::TURBOLIFT_VAL_BASE + random(TurboliftConfig::Effects::TURBOLIFT_VAL_RANGE));
        }
      }
    }

    return driverColors;
  }

  typedef CRGB (*DriverColorGenerator)(int driverIndex);

  float calculateFade(bool isFadeIn, unsigned long startTime, float duration)
  {
    unsigned long now = millis();
    float t = (now - startTime) / duration;
    float fadeScale = constrain(t, 0.0f, 1.0f);

    if (isFadeIn)
    {
      if (fadeScale >= 1.0f)
      {
        fadeInActive = false;
        return 1.0f;
      }
      return fadeScale;
    }
    else // fade out
    {
      fadeScale = 1.0f - fadeScale;
      if (fadeScale <= 0.0f)
      {
        fadeOutActive = false;
        animationActive = false;
        _driver->clear();
        _driver->show();
        return 0.0f;
      }
      return fadeScale;
    }
  }

  // Apply fade scaling to a color
  void applyFade(CRGB &color, float fadeScale)
  {
    if (fadeScale < 1.0f)
    {
      color.nscale8((uint8_t)(fadeScale * 255));
    }
  }

  void singleColorEffect(unsigned long now)
  {
    (void)now; // Suppress unused parameter warning

    // Handle fade transitions
    float fadeScale = 1.0f;
    if (fadeInActive)
    {
      fadeScale = calculateFade(true, fadeInStart, TurboliftConfig::Timing::FADE_IN_DURATION_MS);
    }
    else if (fadeOutActive)
    {
      fadeScale = calculateFade(false, fadeOutStart, TurboliftConfig::Timing::FADE_OUT_DURATION_MS);
      if (fadeScale == 0.0f)
        return;
    }

    // Get color from ConfigManager (using lift animation color settings)
    uint8_t hue = ConfigManager::getLiftHue();
    uint8_t sat = ConfigManager::getLiftSaturation();
    uint8_t val = ConfigManager::getLiftBrightness();

    // Update target color
    targetColor = CHSV(hue, sat, val);

    // Smooth color transition (blend towards target)
    if (previousColor != targetColor)
    {
      colorBlendFactor += 0.05f; // Gradual blend
      if (colorBlendFactor >= 1.0f)
      {
        previousColor = targetColor;
        colorBlendFactor = 0.0f;
      }
    }

    // Interpolate between previous and target color
    CRGB currentColor = interpolateColor(previousColor, targetColor, colorBlendFactor);

    // Fill all LEDs with the current color
    for (int i = 0; i < NUM_LEDS; i++)
    {
      CRGB ledColor = currentColor;
      if (fadeScale < 1.0f)
      {
        ledColor.nscale8((uint8_t)(fadeScale * 255));
      }
      _driver->setPixel(i, ledColor);
    }

    _driver->setBrightness(ConfigManager::getLiftBrightness());
    _driver->show();
  }

  // =====================================================
  // LIFT ANIMATION EFFECT — TNG turbolift streaming lights
  // =====================================================

  struct LiftSegments
  {
    int leftStart, leftEnd;   // Left active segment [start, end)
    int rightStart, rightEnd; // Right active segment [start, end)
  };

  LiftSegments computeSegments()
  {
    int skipS = (int)ConfigManager::getLiftSkipStart();
    int skipM = (int)ConfigManager::getLiftSkipMiddle();
    int skipE = (int)ConfigManager::getLiftSkipEnd();

    // Guard: clamp total skip so at least 2 active LEDs remain
    int totalSkip = skipS + skipM + skipE;
    if (totalSkip > NUM_LEDS - 2)
    {
      skipM = NUM_LEDS - skipS - skipE - 2;
      if (skipM < 0) skipM = 0;
    }

    int available = NUM_LEDS - skipS - skipM - skipE;
    int halfLen = available / 2;

    LiftSegments seg;
    seg.leftStart  = skipS;
    seg.leftEnd    = skipS + halfLen;
    seg.rightStart = skipS + halfLen + skipM;
    seg.rightEnd   = skipS + halfLen + skipM + halfLen;
    if (seg.rightEnd > NUM_LEDS) seg.rightEnd = NUM_LEDS;

    return seg;
  }

  void liftAnimationEffect(unsigned long now)
  {
    float fadeScale = 1.0f;
    if (fadeInActive)
    {
      fadeScale = calculateFade(true, fadeInStart, TurboliftConfig::Timing::FADE_IN_DURATION_MS);
    }
    else if (fadeOutActive)
    {
      fadeScale = calculateFade(false, fadeOutStart, TurboliftConfig::Timing::FADE_OUT_DURATION_MS);
      if (fadeScale == 0.0f) return;
    }

    // Reset offset on submode change
    uint8_t submode = ConfigManager::getLiftSubmode();
    if (submode != liftPrevSubmode)
    {
      liftOffset = 0;
      liftPrevSubmode = submode;
    }

    LiftSegments seg = computeSegments();

    // Blank entire strip
    for (int i = 0; i < NUM_LEDS; i++)
      _driver->setPixel(i, CRGB(0, 0, 0));

    switch (submode)
    {
      case 0: liftStreamEffect(now, seg, fadeScale, false); break; // STREAM_DOWN
      case 1: liftStreamEffect(now, seg, fadeScale, true);  break; // STREAM_UP
      case 2: liftStaticEffect(seg, fadeScale);              break; // STATIC
      case 3: liftPulseEffect(now, seg, fadeScale);          break; // PULSE
      default: liftStaticEffect(seg, fadeScale);             break;
    }

    // Apply malfunction overlay before show()
    if (malfunctionActive)
      liftMalfunctionOverlay(seg);

    _driver->setBrightness(ConfigManager::getLiftBrightness());
    _driver->show();
  }

  void liftStreamEffect(unsigned long now, LiftSegments seg, float fadeScale, bool upward)
  {
    uint8_t speed   = ConfigManager::getLiftSpeed();
    int width       = (int)ConfigManager::getLiftWidth();
    int spacing     = (int)ConfigManager::getLiftSpacing();
    uint8_t hue     = ConfigManager::getLiftHue();
    uint8_t sat     = ConfigManager::getLiftSaturation();

    int period = width + spacing;
    if (period < 1) period = 1;

    if (speed > 0)
    {
      unsigned long delayMs = ConfigManager::speedToDelay(speed);
      if (now - liftLastMove >= delayMs)
      {
        // Move multiple LEDs per step for high speeds
        int stepSize = (speed > 20) ? (speed / 10) : 1;
        liftOffset = (liftOffset + stepSize) % period;
        liftLastMove = now;
      }
    }

    // Pre-compute Gaussian envelope as uint8 LUT (avoids per-LED float)
    // Using quadratic approximation: intensity = 1 - norm^2, where norm in [-1, 1]
    uint8_t envelope[100]; // max width = 100
    int w = (width > 100) ? 100 : width;
    for (int p = 0; p < w; p++)
    {
      float norm = (w > 1) ? (2.0f * (float)p / (float)(w - 1) - 1.0f) : 0.0f;
      float val  = 1.0f - norm * norm;
      envelope[p] = (val > 0.0f) ? (uint8_t)(val * 255.0f) : 0;
    }

    uint8_t fadeU8 = (uint8_t)(fadeScale * 255.0f);

    // Left segment: upward=false → natural (top to bottom), upward=true → reversed
    renderStreamSegment(seg.leftStart, seg.leftEnd, upward, w, period, hue, sat, envelope, fadeU8);

    // Right segment: upward=false → reversed (also top to bottom), upward=true → natural
    renderStreamSegment(seg.rightStart, seg.rightEnd, !upward, w, period, hue, sat, envelope, fadeU8);
  }

  void renderStreamSegment(int start, int end, bool reverseDir,
                           int width, int period,
                           uint8_t hue, uint8_t sat,
                           const uint8_t *envelope, uint8_t fadeU8)
  {
    int segLen = end - start;
    for (int s = 0; s < segLen; s++)
    {
      int ledIndex = reverseDir ? (end - 1 - s) : (start + s);
      int posInPattern = (s + liftOffset) % period;
      if (posInPattern < width)
      {
        CRGB c = CHSV(hue, sat, envelope[posInPattern]);
        c.nscale8(fadeU8);
        _driver->setPixel(ledIndex, c);
      }
    }
  }

  void liftStaticEffect(LiftSegments seg, float fadeScale)
  {
    uint8_t hue = ConfigManager::getLiftHue();
    uint8_t sat = ConfigManager::getLiftSaturation();
    CRGB color  = CHSV(hue, sat, 255); // brightness handled by setBrightness()
    color.nscale8((uint8_t)(fadeScale * 255.0f));

    for (int i = seg.leftStart;  i < seg.leftEnd;  i++) _driver->setPixel(i, color);
    for (int i = seg.rightStart; i < seg.rightEnd; i++) _driver->setPixel(i, color);
  }

  void liftPulseEffect(unsigned long now, LiftSegments seg, float fadeScale)
  {
    uint8_t hue        = ConfigManager::getLiftHue();
    uint8_t sat        = ConfigManager::getLiftSaturation();
    uint8_t pulseMin   = ConfigManager::getLiftPulseMin();
    uint8_t pulseMax   = ConfigManager::getLiftPulseMax();
    uint8_t pulseSpeed = ConfigManager::getLiftPulseSpeed();

    if (pulseMin > pulseMax) { uint8_t tmp = pulseMin; pulseMin = pulseMax; pulseMax = tmp; }

    // Map pulseSpeed 0-100 → period 10000ms-50ms (very fast at max)
    unsigned long periodMs = (pulseSpeed == 0) ? 10000UL : (10000UL / (unsigned long)pulseSpeed);
    if (periodMs < 50UL) periodMs = 50UL;

    // Integer modulo first to avoid float precision loss after extended uptime
    unsigned long phaseMs = now % periodMs;
    float phase = (float)phaseMs / (float)periodMs;
    float sine  = 0.5f + 0.5f * sinf(phase * 2.0f * TurboliftConfig::Math::PI_F);
    uint8_t val = pulseMin + (uint8_t)((pulseMax - pulseMin) * sine);

    CRGB color = CHSV(hue, sat, val);
    color.nscale8((uint8_t)(fadeScale * 255.0f));

    for (int i = seg.leftStart;  i < seg.leftEnd;  i++) _driver->setPixel(i, color);
    for (int i = seg.rightStart; i < seg.rightEnd; i++) _driver->setPixel(i, color);
  }

  void liftMalfunctionOverlay(LiftSegments seg)
  {
    // Apply random brightness flicker to active lift LEDs (read-modify via buffer)
    float flicker = TurboliftConfig::Effects::MALFUNCTION_BRIGHTNESS_MIN +
                    ((float)rand() / RAND_MAX) * TurboliftConfig::Effects::MALFUNCTION_BRIGHTNESS_RANGE;
    if (flicker < TurboliftConfig::Effects::MALFUNCTION_BRIGHTNESS_CLAMP_MIN)
      flicker = TurboliftConfig::Effects::MALFUNCTION_BRIGHTNESS_CLAMP_MIN;
    if (flicker > TurboliftConfig::Effects::MALFUNCTION_BRIGHTNESS_CLAMP_MAX)
      flicker = TurboliftConfig::Effects::MALFUNCTION_BRIGHTNESS_CLAMP_MAX;

    uint8_t scale = (flicker >= 1.0f) ? 255 : (uint8_t)(flicker * 255.0f);
    CRGB *buf = _driver->getBuffer();

    for (int i = seg.leftStart;  i < seg.leftEnd;  i++) buf[i].nscale8(scale);
    for (int i = seg.rightStart; i < seg.rightEnd; i++) buf[i].nscale8(scale);
  }
};

// Static storage definitions removed - now using instance storage
// This eliminates the critical bug where multiple instances would share the same buffers
