#include <FastLED.h>

// FastLED timing options for ESP8266 stability with long strips
#define FASTLED_INTERRUPT_RETRY_COUNT 0
#define FASTLED_ALLOW_INTERRUPTS 0

// LED Strip Configuration
#define LED_PIN     4       // GPIO4 (D2 on Lolin D1) - more reliable for WS2812
#define NUM_LEDS    800     // Your 800 LED strip
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define BRIGHTNESS  50      // Start with low brightness (20% of max)

// Button Configuration
#define BUTTON1_PIN 14      // GPIO14 (D5)
#define BUTTON2_PIN 12      // GPIO12 (D6) - for future use
#define BUTTON3_PIN 13      // GPIO13 (D7) - for future use

// Animation Variables
CRGB leds[NUM_LEDS];
int currentLED = 0;
bool animationActive = false;
bool lastButtonState = HIGH;
unsigned long lastUpdate = 0;
const unsigned long UPDATE_INTERVAL = 50; // 50ms between LED moves (adjust for speed)

void setup() {
  Serial.begin(115200);
  Serial.println("WS2812 Traveling Light Test Starting...");

  // Initialize LED strip
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear();
  FastLED.show();

  // Startup diagnostic: show solid R, G, B to verify wiring and color order
  delay(100);
  fill_solid(leds, NUM_LEDS, CRGB::Red);
  FastLED.show();
  delay(500);
  fill_solid(leds, NUM_LEDS, CRGB::Green);
  FastLED.show();
  delay(500);
  fill_solid(leds, NUM_LEDS, CRGB::Blue);
  FastLED.show();
  delay(500);
  FastLED.clear();
  FastLED.show();

  // Initialize buttons with internal pull-up resistors
  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);
  pinMode(BUTTON3_PIN, INPUT_PULLUP);

  Serial.println("Setup complete. Press button 1 to start traveling light animation.");
  Serial.print("Total LEDs: ");
  Serial.println(NUM_LEDS);
}

void travelingLight() {
  // Clear all LEDs
  FastLED.clear();

  // Light up current LED in blue
  leds[currentLED] = CRGB::Blue;

  // Show the changes
  FastLED.show();

  // Move to next LED
  currentLED++;

  // Reset to beginning when we reach the end
  if (currentLED >= NUM_LEDS) {
    currentLED = 0;
    Serial.println("Traveled through all LEDs - starting over");
  }

  // Debug output every 50 LEDs
  if (currentLED % 50 == 0) {
    Serial.print("Currently at LED: ");
    Serial.println(currentLED);
  }
}

void loop() {
  // Check button press (with simple debouncing)
  bool currentButtonState = digitalRead(BUTTON1_PIN);

  if (lastButtonState == HIGH && currentButtonState == LOW) {
    // Button just pressed - toggle animation
    animationActive = !animationActive;

    if (animationActive) {
      Serial.println("Animation STARTED - Traveling light active");
      currentLED = 0;  // Reset to first LED
    } else {
      Serial.println("Animation STOPPED");
      FastLED.clear();  // Turn off all LEDs
      FastLED.show();
    }

    delay(200);  // Simple debounce delay
  }

  lastButtonState = currentButtonState;

  // Run traveling light animation if active
  if (animationActive && (millis() - lastUpdate >= UPDATE_INTERVAL)) {
    travelingLight();
    lastUpdate = millis();
  }
}
