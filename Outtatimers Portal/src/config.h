#pragma once

#include <stdint.h>

/**
 * @file config.h
 * @brief Configuration constants for the Portal LED Controller
 *
 * This file centralizes all magic numbers and configuration parameters
 * to improve maintainability and make the system easier to tune.
 */

namespace PortalConfig
{
  // Hardware Configuration
  namespace Hardware
  {
    constexpr int LED_PIN = 4;                    // GPIO4 (D2 on Lolin D1)
    constexpr int NUM_LEDS = 800;                 // Total LED count in strip
    constexpr uint8_t DEFAULT_BRIGHTNESS = 255;   // Maximum brightness
    constexpr uint8_t DIAGNOSTIC_BRIGHTNESS = 25; // ~10% for startup diagnostics

    // Button pin assignments
    constexpr int BUTTON1_PIN = 14; // GPIO14 (D5) - Portal toggle
    constexpr int BUTTON2_PIN = 12; // GPIO12 (D6) - Malfunction trigger
    constexpr int BUTTON3_PIN = 13; // GPIO13 (D7) - Fade out
  }

  // Timing Configuration
  namespace Timing
  {
    constexpr unsigned long UPDATE_INTERVAL_MS = 10;   // ~100 FPS update rate
    constexpr unsigned long DEBOUNCE_INTERVAL_MS = 50; // Button debounce time

    // Startup sequence timing
    constexpr unsigned long STARTUP_INITIAL_DELAY_MS = 100;
    constexpr unsigned long STARTUP_COLOR_DURATION_MS = 500;

    // Effect timing
    constexpr unsigned long FADE_IN_DURATION_MS = 3000; // 3 second fade in
    constexpr unsigned long FADE_OUT_DURATION_MS = 200; // 200ms fade out

    // Malfunction effect timing
    constexpr unsigned long MALFUNCTION_MIN_JUMP_MS = 40;
    constexpr unsigned long MALFUNCTION_MAX_JUMP_MS = 200;
  }

  // Effect Configuration
  namespace Effects
  {
    constexpr int GRADIENT_STEP_DEFAULT = 10; // Generate color every Nth LED
    constexpr int GRADIENT_MOVE_DEFAULT = 2;  // LEDs to move per update (2x speed)

    // Portal effect parameters
    constexpr int MIN_DRIVER_DISTANCE = 5;  // Minimum distance between color drivers
    constexpr int MAX_DRIVER_DISTANCE = 15; // Maximum distance between color drivers

    // Color generation parameters
    constexpr uint8_t PORTAL_HUE_BASE = 160;       // Base hue for portal colors (blue-purple range)
    constexpr uint8_t PORTAL_HUE_RANGE = 41;       // Hue variation range
    constexpr uint8_t PORTAL_SAT_BASE = 180;       // Base saturation
    constexpr uint8_t PORTAL_SAT_RANGE = 76;       // Saturation variation range
    constexpr uint8_t PORTAL_SAT_LOW_BASE = 30;    // Low saturation base (rare occurrence)
    constexpr uint8_t PORTAL_SAT_LOW_RANGE = 40;   // Low saturation range
    constexpr uint8_t PORTAL_VAL_BASE = 51;        // Base value (brightness)
    constexpr uint8_t PORTAL_VAL_RANGE = 205;      // Value variation range
    constexpr int PORTAL_LOW_SAT_PROBABILITY = 10; // 1 in 10 chance for low saturation

    // Malfunction effect parameters
    constexpr float MALFUNCTION_BRIGHTNESS_MIN = 0.2f;
    constexpr float MALFUNCTION_BRIGHTNESS_RANGE = 1.3f;
    constexpr float MALFUNCTION_BRIGHTNESS_SMOOTHING_MIN = 0.3f;
    constexpr float MALFUNCTION_BRIGHTNESS_SMOOTHING_RANGE = 0.5f;
    constexpr int MALFUNCTION_NOISE_RANGE = 61; // -30 to +30
    constexpr int MALFUNCTION_NOISE_OFFSET = 30;
    constexpr float MALFUNCTION_BRIGHTNESS_CLAMP_MIN = 0.05f;
    constexpr float MALFUNCTION_BRIGHTNESS_CLAMP_MAX = 1.5f;
    constexpr uint8_t MALFUNCTION_BASE_BRIGHTNESS = 170;
    constexpr uint8_t MALFUNCTION_BRIGHTNESS_OFFSET = 85;
  }

  // Mathematical Constants
  namespace Math
  {
    constexpr float PI_F = 3.14159265358979323846f;
  }

  // Pin States (type-safe alternatives to HIGH/LOW)
  enum class PinState : int
  {
    Low = 0,
    High = 1
  };
}