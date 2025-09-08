/**
 * @file status_led.h
 * @brief Status LED manager for WiFi connection indication
 *
 * This module provides a simple status LED manager that blinks the on-board LED
 * to indicate WiFi connection status. The LED blinks slowly when connected and
 * fast when attempting to connect.
 */

#ifndef STATUS_LED_H
#define STATUS_LED_H

#include <Arduino.h>
#include "config.h"

namespace PortalConfig
{
  namespace Hardware
  {
    enum class WiFiStatus
    {
      DISCONNECTED,
      CONNECTING,
      CONNECTED
    };
  }
}

/**
 * @class StatusLED
 * @brief Manages the on-board status LED for WiFi connection indication
 *
 * The status LED provides visual feedback about WiFi connection state:
 * - Off: WiFi disabled or disconnected
 * - Fast blink: Attempting to connect to WiFi
 * - Slow blink: Successfully connected to WiFi
 */
class StatusLED
{
public:
  /**
   * @brief Initialize the status LED
   *
   * Sets up the LED pin as output and initializes to off state.
   */
  static void begin();

  /**
   * @brief Update the LED state based on WiFi status
   *
   * @param status Current WiFi connection status
   * @param currentTime Current time in milliseconds (from millis())
   */
  static void update(PortalConfig::Hardware::WiFiStatus status, unsigned long currentTime);

  /**
   * @brief Turn off the status LED
   */
  static void off();

private:
  static bool initialized_;
  static unsigned long lastToggleTime_;
  static bool ledState_;
  static PortalConfig::Hardware::WiFiStatus currentStatus_;

  /**
   * @brief Set the LED state (handles active-low logic)
   * @param state true for on, false for off
   */
  static void setLED(bool state);

  /**
   * @brief Get the blink interval based on current status
   * @return Blink interval in milliseconds
   */
  static unsigned long getBlinkInterval();
};

#endif // STATUS_LED_H