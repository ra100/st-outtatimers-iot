/**
 * @file status_led.cpp
 * @brief Implementation of status LED manager
 */

#include "status_led.h"

bool StatusLED::initialized_ = false;
unsigned long StatusLED::lastToggleTime_ = 0;
bool StatusLED::ledState_ = false;
PortalConfig::Hardware::WiFiStatus StatusLED::currentStatus_ = PortalConfig::Hardware::WiFiStatus::DISCONNECTED;

void StatusLED::begin()
{
  if (!initialized_)
  {
    pinMode(PortalConfig::Hardware::STATUS_LED_PIN, OUTPUT);
    setLED(false); // Start with LED off
    initialized_ = true;
    currentStatus_ = PortalConfig::Hardware::WiFiStatus::DISCONNECTED;
  }
}

void StatusLED::update(PortalConfig::Hardware::WiFiStatus status, unsigned long currentTime)
{
  if (!initialized_)
  {
    begin();
  }

  // If status changed, reset the blink state
  if (status != currentStatus_)
  {
    currentStatus_ = status;
    lastToggleTime_ = currentTime;
    ledState_ = false;
    setLED(false);
  }

  // Handle blinking based on status
  if (status == PortalConfig::Hardware::WiFiStatus::DISCONNECTED)
  {
    // LED off when disconnected
    if (ledState_)
    {
      setLED(false);
      ledState_ = false;
    }
  }
  else
  {
    // Blink for connecting or connected states
    if (currentTime - lastToggleTime_ >= getBlinkInterval())
    {
      ledState_ = !ledState_;
      setLED(ledState_);
      lastToggleTime_ = currentTime;
    }
  }
}

void StatusLED::off()
{
  if (initialized_)
  {
    setLED(false);
    ledState_ = false;
    currentStatus_ = PortalConfig::Hardware::WiFiStatus::DISCONNECTED;
  }
}

void StatusLED::setLED(bool state)
{
  if (PortalConfig::Hardware::STATUS_LED_ACTIVE_LOW)
  {
    digitalWrite(PortalConfig::Hardware::STATUS_LED_PIN, state ? LOW : HIGH);
  }
  else
  {
    digitalWrite(PortalConfig::Hardware::STATUS_LED_PIN, state ? HIGH : LOW);
  }
}

unsigned long StatusLED::getBlinkInterval()
{
  switch (currentStatus_)
  {
  case PortalConfig::Hardware::WiFiStatus::CONNECTING:
    return PortalConfig::Timing::STATUS_LED_FAST_BLINK_MS;
  case PortalConfig::Hardware::WiFiStatus::CONNECTED:
    return PortalConfig::Timing::STATUS_LED_BLINK_INTERVAL_MS;
  default:
    return PortalConfig::Timing::STATUS_LED_BLINK_INTERVAL_MS;
  }
}