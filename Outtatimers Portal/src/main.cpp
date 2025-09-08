#include <FastLED.h>
#include "effects.h"
#include "portal_effect.h"
#include "led_driver.h"
#include "debounce.h"

// FastLED timing options for ESP8266 stability with long strips
#define FASTLED_INTERRUPT_RETRY_COUNT 0
#define FASTLED_ALLOW_INTERRUPTS 0

// LED Strip Configuration
#define LED_PIN 4    // GPIO4 (D2 on Lolin D1) - more reliable for WS2812
#define NUM_LEDS 800 // Your 800 LED strip
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB
#define BRIGHTNESS 255

// Button Configuration
#define BUTTON1_PIN 14 // GPIO14 (D5)
#define BUTTON2_PIN 12 // GPIO12 (D6) - for future use
#define BUTTON3_PIN 13 // GPIO13 (D7) - for future use

// Animation Variables
// PortalEffect will own the LED buffers and state
const int GRADIENT_STEP = 10;             // Generate color every 10th LED
const int GRADIENT_MOVE = 2;              // LEDs to move per update (2x speed)
const unsigned long UPDATE_INTERVAL = 10; // 10ms between updates for ~100 FPS

// Static driver and portal effect using template-based class
static FastLEDDriver<NUM_LEDS> fastDriver(LED_PIN);
static PortalEffectTemplate<NUM_LEDS, GRADIENT_STEP, GRADIENT_MOVE> portal(&fastDriver);
bool lastButton1State = HIGH;
bool lastButton2State = HIGH;
bool portalRunning = false;
Debounce btn1Debounce(50);

// Non-blocking startup state
enum class StartupState
{
  NotStarted,
  WaitingBeforeFlash,
  FlashRed,
  FlashGreen,
  FlashBlue,
  Done
};
static StartupState startupState = StartupState::NotStarted;
static unsigned long startupStateStart = 0;
// We'll use Debounce instances for button guards instead of manual timestamps
Debounce btn2Debounce(50);

// PortalEffect encapsulates malfunction and gradient logic now.

void setup()
{
  Serial.begin(115200);
  Serial.println("WS2812 Traveling Light Test Starting...");

  // Initialize portal effect (which initializes LEDs)
  portal.begin();

  // Initialize non-blocking startup sequence
  portal.setBrightness(25); // ~10% of 255 for diagnostics
  startupState = StartupState::WaitingBeforeFlash;
  startupStateStart = millis();

  // Initialize buttons with internal pull-up resistors
  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);
  pinMode(BUTTON3_PIN, INPUT_PULLUP);

  Serial.println("Setup started; running non-blocking startup diagnostics...");
  Serial.println("Press button 1 to start portal effect.");
  Serial.print("Total LEDs: ");
  Serial.println(NUM_LEDS);
  Serial.print("Circle radius: ");
  Serial.print(NUM_LEDS / (2.0 * PI));
  Serial.println(" LEDs");
}

void loop()
{
  unsigned long now = millis();

  // Handle non-blocking startup diagnostics
  if (startupState != StartupState::Done)
  {
    switch (startupState)
    {
    case StartupState::WaitingBeforeFlash:
      if (now - startupStateStart >= 100)
      {
        portal.fillSolid(CRGB::Red);
        startupState = StartupState::FlashRed;
        startupStateStart = now;
      }
      break;
    case StartupState::FlashRed:
      if (now - startupStateStart >= 500)
      {
        portal.fillSolid(CRGB::Green);
        startupState = StartupState::FlashGreen;
        startupStateStart = now;
      }
      break;
    case StartupState::FlashGreen:
      if (now - startupStateStart >= 500)
      {
        portal.fillSolid(CRGB::Blue);
        startupState = StartupState::FlashBlue;
        startupStateStart = now;
      }
      break;
    case StartupState::FlashBlue:
      if (now - startupStateStart >= 500)
      {
        portal.clear();
        portal.setBrightness(BRIGHTNESS); // Restore normal brightness
        startupState = StartupState::Done;
        Serial.println("Setup complete.");
      }
      break;
    default:
      break;
    }
  }

  // Check button presses (with non-blocking debounce)
  bool currentButton1StateRaw = digitalRead(BUTTON1_PIN);
  bool currentButton2StateRaw = digitalRead(BUTTON2_PIN);
  bool currentButton3StateRaw = digitalRead(BUTTON3_PIN);
  bool changed1 = btn1Debounce.sample(currentButton1StateRaw, now);
  bool currentButton1State = btn1Debounce.getState();
  bool changed2 = btn2Debounce.sample(currentButton2StateRaw, now);
  bool currentButton2State = btn2Debounce.getState();
  bool currentButton3State = currentButton3StateRaw;

  // Button 1: toggle portal animation
  // Use Debounce transition detection for button1 (falling edge)
  if (changed1 && lastButton1State == HIGH && currentButton1State == LOW)
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
  if (currentButton3State == LOW)
  {
    Serial.println("Fade out triggered by button 3");
    portal.triggerFadeOut();
  }

  // Button 2: portal malfunction effect
  // Use Debounce transition detection for button2 (falling edge)
  if (changed2 && lastButton2State == HIGH && currentButton2State == LOW)
  {
    Serial.println("Portal MALFUNCTION triggered!");
    portal.triggerMalfunction();
  }
  lastButton2State = currentButton2State;

  // Run effects
  portal.update(now);
}
