#include <FastLED.h>
#include "effects.h"
#include "portal_effect.h"
#include "led_driver.h"
#include "debounce.h"
#include "config.h"
#include "startup_sequence.h"

// FastLED timing options for ESP8266 stability with long strips
#define FASTLED_INTERRUPT_RETRY_COUNT 0
#define FASTLED_ALLOW_INTERRUPTS 0

// LED Strip Configuration - using config constants
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB

// Static driver and portal effect using template-based class
static FastLEDDriver<PortalConfig::Hardware::NUM_LEDS> fastDriver(PortalConfig::Hardware::LED_PIN);
static PortalEffectTemplate<PortalConfig::Hardware::NUM_LEDS, PortalConfig::Effects::GRADIENT_STEP_DEFAULT, PortalConfig::Effects::GRADIENT_MOVE_DEFAULT> portal(&fastDriver);
bool lastButton1State = static_cast<int>(PortalConfig::PinState::High);
bool lastButton2State = static_cast<int>(PortalConfig::PinState::High);
bool portalRunning = false;
Debounce btn1Debounce(PortalConfig::Timing::DEBOUNCE_INTERVAL_MS);
Debounce btn2Debounce(PortalConfig::Timing::DEBOUNCE_INTERVAL_MS);

// Startup sequence manager
StartupSequence startupSequence;

// PortalEffect encapsulates malfunction and gradient logic now.

void setup()
{
  Serial.begin(115200);
  Serial.println("WS2812 Traveling Light Test Starting...");

  // Initialize portal effect (which initializes LEDs)
  portal.begin();

  // Initialize startup sequence
  startupSequence.begin(&fastDriver);

  // Initialize buttons with internal pull-up resistors
  pinMode(PortalConfig::Hardware::BUTTON1_PIN, INPUT_PULLUP);
  pinMode(PortalConfig::Hardware::BUTTON2_PIN, INPUT_PULLUP);
  pinMode(PortalConfig::Hardware::BUTTON3_PIN, INPUT_PULLUP);

  Serial.println("Setup started; running non-blocking startup diagnostics...");
  Serial.println("Press button 1 to start portal effect.");
  Serial.print("Total LEDs: ");
  Serial.println(PortalConfig::Hardware::NUM_LEDS);
  Serial.print("Circle radius: ");
  Serial.print(PortalConfig::Hardware::NUM_LEDS / (2.0 * PortalConfig::Math::PI_F));
  Serial.println(" LEDs");
}

void loop()
{
  unsigned long now = millis();

  // Handle non-blocking startup diagnostics
  if (!startupSequence.isComplete())
  {
    if (startupSequence.update(now))
    {
      // State changed - log it
      Serial.print("Startup: ");
      Serial.println(startupSequence.getStateString());

      if (startupSequence.isComplete())
      {
        Serial.println("Setup complete.");
      }
    }
    return; // Don't process buttons during startup
  }

  // Check button presses (with non-blocking debounce)
  bool currentButton1StateRaw = digitalRead(PortalConfig::Hardware::BUTTON1_PIN);
  bool currentButton2StateRaw = digitalRead(PortalConfig::Hardware::BUTTON2_PIN);
  bool currentButton3StateRaw = digitalRead(PortalConfig::Hardware::BUTTON3_PIN);
  bool changed1 = btn1Debounce.sample(currentButton1StateRaw, now);
  bool currentButton1State = btn1Debounce.getState();
  bool changed2 = btn2Debounce.sample(currentButton2StateRaw, now);
  bool currentButton2State = btn2Debounce.getState();
  bool currentButton3State = currentButton3StateRaw;

  // Button 1: toggle portal animation
  // Use Debounce transition detection for button1 (falling edge)
  if (changed1 && lastButton1State == static_cast<int>(PortalConfig::PinState::High) && currentButton1State == static_cast<int>(PortalConfig::PinState::Low))
  {
    portalRunning = !portalRunning;
    if (portalRunning)
    {
      portal.start();
      Serial.println("Animation STARTED - Portal effect active (fade in)");
    }
    else
    {
      portal.stop();
      Serial.println("Animation STOPPED");
    }
  }
  lastButton1State = currentButton1State;

  // Button 3: fade out animation (works from any mode)
  if (currentButton3State == static_cast<int>(PortalConfig::PinState::Low))
  {
    Serial.println("Fade out triggered by button 3");
    portal.triggerFadeOut();
  }

  // Button 2: portal malfunction effect
  // Use Debounce transition detection for button2 (falling edge)
  if (changed2 && lastButton2State == static_cast<int>(PortalConfig::PinState::High) && currentButton2State == static_cast<int>(PortalConfig::PinState::Low))
  {
    Serial.println("Portal MALFUNCTION triggered!");
    portal.triggerMalfunction();
  }
  lastButton2State = currentButton2State;

  // Run effects
  portal.update(now);
}
