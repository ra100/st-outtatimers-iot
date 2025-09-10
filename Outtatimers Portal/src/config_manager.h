#pragma once

#include <stdint.h>
#include <Arduino.h>

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
    // Initialize default values
    rotationSpeed = 2;   // Default gradient move speed
    maxBrightness = 255; // Default max brightness
    hueMin = 160;        // Default minimum hue (blue)
    hueMax = 200;        // Default maximum hue (purple)
  }

  /**
   * @brief Get the current rotation speed (gradient move value)
   * @return Rotation speed (1-10)
   */
  static int getRotationSpeed()
  {
    return rotationSpeed;
  }

  /**
   * @brief Set the rotation speed (gradient move value)
   * @param speed Rotation speed (1-10)
   */
  static void setRotationSpeed(int speed)
  {
    rotationSpeed = constrain(speed, 1, 10);
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
  }

private:
  static int rotationSpeed;
  static uint8_t maxBrightness;
  static uint8_t hueMin;
  static uint8_t hueMax;
};