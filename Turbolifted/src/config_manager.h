#pragma once

#include <stdint.h>
#include <Arduino.h>
#include "config.h"

#ifndef UNIT_TEST
#include <LittleFS.h>
#endif

/**
 * @brief Configuration manager for runtime parameters
 *
 * This class provides a simple way to store and retrieve configuration
 * parameters that can be modified at runtime.
 */
class ConfigManager
{
public:
  /**
   * @brief Initialize the configuration manager
   */
  static void begin()
  {
    // Initialize default values for legacy gradient effects
    rotationSpeed = 2;   // Default gradient move speed
    maxBrightness = 255; // Default max brightness
    hueMin = 160;        // Default minimum hue (blue)
    hueMax = 200;        // Default maximum hue (purple)
    satMin = 128;        // Default minimum saturation
    satMax = 255;        // Default maximum saturation
    turboliftMode = 0;   // Default to classic mode
    effectNeedsRegeneration = false;

    // Initialize new lift animation parameters with defaults from config.h
    liftSpeed = TurboliftConfig::Effects::DEFAULT_SPEED;
    liftWidth = TurboliftConfig::Effects::DEFAULT_WIDTH;
    liftSpacing = TurboliftConfig::Effects::DEFAULT_SPACING;
    liftHue = TurboliftConfig::Effects::DEFAULT_HUE;
    liftSaturation = TurboliftConfig::Effects::DEFAULT_SATURATION;
    liftBrightness = TurboliftConfig::Effects::DEFAULT_BRIGHTNESS;
    liftSubmode = TurboliftConfig::Effects::DEFAULT_SUBMODE;
    liftSkipStart = TurboliftConfig::Effects::DEFAULT_SKIP_START;
    liftSkipMiddle = TurboliftConfig::Effects::DEFAULT_SKIP_MIDDLE;
    liftSkipEnd = TurboliftConfig::Effects::DEFAULT_SKIP_END;
    liftPulseMin = TurboliftConfig::Effects::DEFAULT_PULSE_MIN;
    liftPulseMax = TurboliftConfig::Effects::DEFAULT_PULSE_MAX;
    liftPulseSpeed = TurboliftConfig::Effects::DEFAULT_PULSE_SPEED;
    effectMode = static_cast<uint8_t>(TurboliftConfig::Effects::EffectMode::LIFT_ANIMATION);
    dirty = false;
    lastChangeMs = 0;

    // Override defaults with persisted values (if available)
    loadConfig();
  }

  // =====================================================
  // LEGACY GRADIENT EFFECT PARAMETERS
  // =====================================================

  /**
   * @brief Get the current rotation speed (gradient move value)
   * @return Rotation speed (0-10)
   */
  static int getRotationSpeed()
  {
    return rotationSpeed;
  }

  /**
   * @brief Set the rotation speed (gradient move value)
   * @param speed Rotation speed (0-10)
   */
  static void setRotationSpeed(int speed)
  {
    rotationSpeed = constrain(speed, 0, 10);
  }

  /**
   * @brief Get the current max brightness
   * @return Max brightness (0-255)
   */
  static uint8_t getMaxBrightness()
  {
    return maxBrightness;
  }

  /**
   * @brief Set the max brightness
   * @param brightness Max brightness (0-255)
   */
  static void setMaxBrightness(uint8_t brightness)
  {
    maxBrightness = constrain(brightness, 0, 255);
  }

  /**
   * @brief Get the minimum hue value
   * @return Minimum hue (0-255)
   */
  static uint8_t getHueMin()
  {
    return hueMin;
  }

  /**
   * @brief Set the minimum hue value
   * @param minHue Minimum hue (0-255)
   */
  static void setHueMin(uint8_t minHue)
  {
    hueMin = constrain(minHue, 0, 255);
    effectNeedsRegeneration = true;
  }

  /**
   * @brief Get the maximum hue value
   * @return Maximum hue (0-255)
   */
  static uint8_t getHueMax()
  {
    return hueMax;
  }

  /**
   * @brief Set the maximum hue value
   * @param maxHue Maximum hue (0-255)
   */
  static void setHueMax(uint8_t maxHue)
  {
    hueMax = constrain(maxHue, 0, 255);
    effectNeedsRegeneration = true;
  }

  /**
   * @brief Get the minimum saturation value
   * @return Minimum saturation (0-255)
   */
  static uint8_t getSatMin()
  {
    return satMin;
  }

  /**
   * @brief Set the minimum saturation value
   * @param minSat Minimum saturation (0-255)
   */
  static void setSatMin(uint8_t minSat)
  {
    satMin = constrain(minSat, 0, 255);
    effectNeedsRegeneration = true;
  }

  /**
   * @brief Get the maximum saturation value
   * @return Maximum saturation (0-255)
   */
  static uint8_t getSatMax()
  {
    return satMax;
  }

  /**
   * @brief Set the maximum saturation value
   * @param maxSat Maximum saturation (0-255)
   */
  static void setSatMax(uint8_t maxSat)
  {
    satMax = constrain(maxSat, 0, 255);
    effectNeedsRegeneration = true;
  }

  /**
   * @brief Check if effect regeneration is needed
   * @return true if regeneration is required
   */
  static bool needsEffectRegeneration()
  {
    return effectNeedsRegeneration;
  }

  /**
   * @brief Clear the effect regeneration flag
   */
  static void clearEffectRegenerationFlag()
  {
    effectNeedsRegeneration = false;
  }

  /**
   * @brief Get the current turbolift mode
   * @return Turbolift mode (0: classic, 1: virtual gradients)
   */
  static int getTurboliftMode()
  {
    return turboliftMode;
  }

  /**
   * @brief Set the turbolift mode
   * @param mode Turbolift mode (0: classic, 1: virtual gradients)
   */
  static void setTurboliftMode(int mode)
  {
    turboliftMode = constrain(mode, 0, 1);
    effectNeedsRegeneration = true;
  }

  // =====================================================
  // NEW LIFT ANIMATION PARAMETERS
  // =====================================================

  /**
   * @brief Get the lift animation speed
   * @return Speed (0-10 scale)
   */
  static uint8_t getLiftSpeed()
  {
    return liftSpeed;
  }

  /**
   * @brief Set the lift animation speed
   * @param speed Speed (0-10 scale, maps to delay per LED)
   */
  static void setLiftSpeed(uint8_t speed)
  {
    liftSpeed = constrain(speed, 0, 100);
    markDirty();
  }

  /**
   * @brief Get the light beam width
   * @return Width in LEDs (1-20)
   */
  static uint8_t getLiftWidth()
  {
    return liftWidth;
  }

  /**
   * @brief Set the light beam width
   * @param width Width in LEDs (1-50)
   */
  static void setLiftWidth(int width)
  {
    liftWidth = (uint8_t)constrain(width, 1, 100);
    markDirty();
  }

  /**
   * @brief Get the spacing between beam packets
   * @return Spacing in LEDs (0-50)
   */
  static uint8_t getLiftSpacing()
  {
    return liftSpacing;
  }

  /**
   * @brief Set the spacing between beam packets
   * @param spacing Spacing in LEDs (0-100)
   */
  static void setLiftSpacing(int spacing)
  {
    liftSpacing = (uint8_t)constrain(spacing, 0, 200);
    markDirty();
  }

  /**
   * @brief Get the lift animation hue
   * @return Hue value (0-255)
   */
  static uint8_t getLiftHue()
  {
    return liftHue;
  }

  /**
   * @brief Set the lift animation hue
   * @param hue Hue value (0-255)
   */
  static void setLiftHue(uint8_t hue)
  {
    liftHue = hue;
    markDirty();
  }

  /**
   * @brief Get the lift animation saturation
   * @return Saturation value (0-255)
   */
  static uint8_t getLiftSaturation()
  {
    return liftSaturation;
  }

  /**
   * @brief Set the lift animation saturation
   * @param saturation Saturation value (0-255)
   */
  static void setLiftSaturation(uint8_t saturation)
  {
    liftSaturation = saturation;
    markDirty();
  }

  /**
   * @brief Get the lift animation brightness
   * @return Brightness value (0-255)
   */
  static uint8_t getLiftBrightness()
  {
    return liftBrightness;
  }

  /**
   * @brief Set the lift animation brightness
   * @param brightness Brightness value (0-255)
   */
  static void setLiftBrightness(uint8_t brightness)
  {
    liftBrightness = constrain(brightness, 0, 255);
    markDirty();
  }

  // =====================================================
  // LIFT ANIMATION SUB-MODE AND SKIP REGION PARAMETERS
  // =====================================================

  static uint8_t getLiftSubmode() { return liftSubmode; }
  static void setLiftSubmode(int v)    { liftSubmode    = (uint8_t)constrain(v, 0, 3);   markDirty(); }

  static uint16_t getLiftSkipStart() { return liftSkipStart; }
  static void setLiftSkipStart(int v) {
    liftSkipStart = (uint16_t)constrain(v, 0, TurboliftConfig::Hardware::NUM_LEDS / 2);
    markDirty();
  }

  static uint16_t getLiftSkipMiddle() { return liftSkipMiddle; }
  static void setLiftSkipMiddle(int v) {
    liftSkipMiddle = (uint16_t)constrain(v, 0, TurboliftConfig::Hardware::NUM_LEDS / 2);
    markDirty();
  }

  static uint16_t getLiftSkipEnd() { return liftSkipEnd; }
  static void setLiftSkipEnd(int v) {
    liftSkipEnd = (uint16_t)constrain(v, 0, TurboliftConfig::Hardware::NUM_LEDS / 2);
    markDirty();
  }

  static uint8_t getLiftPulseMin() { return liftPulseMin; }
  static void setLiftPulseMin(int v)   { liftPulseMin   = (uint8_t)constrain(v, 0, 255); markDirty(); }

  static uint8_t getLiftPulseMax() { return liftPulseMax; }
  static void setLiftPulseMax(int v)   { liftPulseMax   = (uint8_t)constrain(v, 0, 255); markDirty(); }

  static uint8_t getLiftPulseSpeed() { return liftPulseSpeed; }
  static void setLiftPulseSpeed(int v) { liftPulseSpeed = (uint8_t)constrain(v, 0, 100);  markDirty(); }

  /**
   * @brief Get the current effect mode
   * @return Effect mode (0: single color, 1: lift animation, 2: classic, 3: virtual gradient)
   */
  static uint8_t getEffectMode()
  {
    return effectMode;
  }

  /**
   * @brief Set the current effect mode
   * @param mode Effect mode (0: single color, 1: lift animation)
   */
  static void setEffectMode(uint8_t mode)
  {
    effectMode = constrain(mode, 0, 1);
    effectNeedsRegeneration = true;
  }

  /**
   * @brief Convert speed (0-100) to delay in milliseconds between offset advances
   * @param speed Speed value (0-100). 0 = stopped, 100 = fastest.
   * @return Delay in milliseconds (clamped to [SPEED_MAX_DELAY_MS, SPEED_MIN_DELAY_MS])
   */
  static unsigned long speedToDelay(uint8_t speed)
  {
    // Linear interpolation: speed 0 = 100ms, speed 100 = 5ms
    // Use unsigned long arithmetic throughout to avoid underflow
    unsigned long reduction = (unsigned long)speed *
      (TurboliftConfig::Effects::SPEED_MIN_DELAY_MS - TurboliftConfig::Effects::SPEED_MAX_DELAY_MS) / 100;
    if (reduction >= TurboliftConfig::Effects::SPEED_MIN_DELAY_MS)
      return TurboliftConfig::Effects::SPEED_MAX_DELAY_MS;
    return TurboliftConfig::Effects::SPEED_MIN_DELAY_MS - reduction;
  }

  // =====================================================
  // CONFIG PERSISTENCE (LittleFS, debounced)
  // =====================================================

  /**
   * @brief Mark config dirty — triggers a deferred write
   */
  static void markDirty()
  {
    dirty = true;
    lastChangeMs = millis();
  }

  /**
   * @brief Call from main loop: flushes config to flash if dirty and 5s have elapsed
   */
  static void flushIfNeeded()
  {
    if (dirty && (millis() - lastChangeMs >= 5000UL))
    {
      saveConfig();
      dirty = false;
    }
  }

  /**
   * @brief Save all lift parameters to /config.json on LittleFS
   */
  static void saveConfig()
  {
#ifndef UNIT_TEST
    File f = LittleFS.open("/config.json", "w");
    if (!f) return;
    char buf[384];
    snprintf(buf, sizeof(buf),
      "{"
      "\"effectMode\":%u,"
      "\"liftSubmode\":%u,"
      "\"liftSpeed\":%u,"
      "\"liftWidth\":%u,"
      "\"liftSpacing\":%u,"
      "\"liftHue\":%u,"
      "\"liftSaturation\":%u,"
      "\"liftBrightness\":%u,"
      "\"liftSkipStart\":%u,"
      "\"liftSkipMiddle\":%u,"
      "\"liftSkipEnd\":%u,"
      "\"liftPulseMin\":%u,"
      "\"liftPulseMax\":%u,"
      "\"liftPulseSpeed\":%u"
      "}",
      (unsigned)effectMode,
      (unsigned)liftSubmode,
      (unsigned)liftSpeed,
      (unsigned)liftWidth,
      (unsigned)liftSpacing,
      (unsigned)liftHue,
      (unsigned)liftSaturation,
      (unsigned)liftBrightness,
      (unsigned)liftSkipStart,
      (unsigned)liftSkipMiddle,
      (unsigned)liftSkipEnd,
      (unsigned)liftPulseMin,
      (unsigned)liftPulseMax,
      (unsigned)liftPulseSpeed);
    f.print(buf);
    f.close();
#endif
  }

  /**
   * @brief Load parameters from /config.json on LittleFS (called at boot)
   */
  static void loadConfig()
  {
#ifndef UNIT_TEST
    File f = LittleFS.open("/config.json", "r");
    if (!f) return;
    if (f.size() > 1024) { f.close(); return; } // Reject oversized/corrupt files

    char buf[512];
    int len = f.readBytes(buf, sizeof(buf) - 1);
    f.close();
    buf[len] = '\0';

    // Simple key:value integer parser
    auto getVal = [&](const char *key) -> int {
      const char *p = strstr(buf, key);
      if (!p) return -1;
      p += strlen(key);
      while (*p == ':' || *p == ' ') p++;
      return atoi(p);
    };

    int v;
    if ((v = getVal("\"effectMode\""))    >= 0) effectMode    = (uint8_t)constrain(v, 0, 1);
    if ((v = getVal("\"liftSubmode\""))   >= 0) liftSubmode   = (uint8_t)constrain(v, 0, 3);
    if ((v = getVal("\"liftSpeed\""))     >= 0) liftSpeed     = (uint8_t)constrain(v, 0, 100);
    if ((v = getVal("\"liftWidth\""))     >= 0) liftWidth     = (uint8_t)constrain(v, 1, 50);
    if ((v = getVal("\"liftSpacing\""))   >= 0) liftSpacing   = (uint8_t)constrain(v, 0, 100);
    if ((v = getVal("\"liftHue\""))       >= 0) liftHue       = (uint8_t)constrain(v, 0, 255);
    if ((v = getVal("\"liftSaturation\""))>= 0) liftSaturation= (uint8_t)constrain(v, 0, 255);
    if ((v = getVal("\"liftBrightness\""))>= 0) liftBrightness= (uint8_t)constrain(v, 0, 255);
    if ((v = getVal("\"liftSkipStart\"")) >= 0) liftSkipStart = (uint16_t)constrain(v, 0, TurboliftConfig::Hardware::NUM_LEDS / 2);
    if ((v = getVal("\"liftSkipMiddle\""))>= 0) liftSkipMiddle= (uint16_t)constrain(v, 0, TurboliftConfig::Hardware::NUM_LEDS / 2);
    if ((v = getVal("\"liftSkipEnd\""))   >= 0) liftSkipEnd   = (uint16_t)constrain(v, 0, TurboliftConfig::Hardware::NUM_LEDS / 2);
    if ((v = getVal("\"liftPulseMin\""))  >= 0) liftPulseMin  = (uint8_t)constrain(v, 0, 255);
    if ((v = getVal("\"liftPulseMax\""))  >= 0) liftPulseMax  = (uint8_t)constrain(v, 0, 255);
    if ((v = getVal("\"liftPulseSpeed\""))>= 0) liftPulseSpeed= (uint8_t)constrain(v, 0, 100);
#endif
  }

private:
  // Legacy gradient effect parameters
  static int rotationSpeed;
  static uint8_t maxBrightness;
  static uint8_t hueMin;
  static uint8_t hueMax;
  static uint8_t satMin;
  static uint8_t satMax;
  static bool effectNeedsRegeneration;
  static int turboliftMode;

  // New lift animation parameters
  static uint8_t liftSpeed;       // Animation speed (0-10)
  static uint8_t liftWidth;       // Beam width in LEDs (1-50)
  static uint8_t liftSpacing;     // Gap between beams (0-100)
  static uint8_t liftHue;         // Beam color hue (0-255)
  static uint8_t liftSaturation;  // Beam color saturation (0-255)
  static uint8_t liftBrightness;  // Overall brightness (0-255)
  static uint8_t liftSubmode;     // Sub-mode (0=stream_down, 1=stream_up, 2=static, 3=pulse)
  static uint16_t liftSkipStart;  // LEDs to skip at strip start
  static uint16_t liftSkipMiddle; // LEDs to blank in U-bend
  static uint16_t liftSkipEnd;    // LEDs to skip at strip end
  static uint8_t liftPulseMin;    // Pulse minimum brightness (0-255)
  static uint8_t liftPulseMax;    // Pulse maximum brightness (0-255)
  static uint8_t liftPulseSpeed;  // Pulse oscillation speed (0-10)
  static uint8_t effectMode;      // Current effect mode

  // Persistence state
  static bool dirty;                  // True when params changed but not yet flushed
  static unsigned long lastChangeMs;  // Time of last param change
};
