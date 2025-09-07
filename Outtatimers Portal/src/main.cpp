#include <FastLED.h>

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
CRGB leds[NUM_LEDS];
CRGB effectLeds[NUM_LEDS]; // Store the static effect
bool animationActive = false;
bool fadeInActive = false;
unsigned long fadeInStart = 0;
bool fadeOutActive = false;
unsigned long fadeOutStart = 0;
const unsigned long FADE_IN_DURATION_MS = 3000; // Fade in duration in ms
const unsigned long FADE_OUT_DURATION_MS = 200; // Fade out duration in ms
// Gradient rotation variables
int gradientPosition = 0;     // Current position of gradient
const int GRADIENT_MOVE = 2;  // LEDs to move per update (2x speed)
const int GRADIENT_STEP = 10; // Generate color every 10th LED
bool malfunctionActive = false;
bool lastButton1State = HIGH;
bool lastButton2State = HIGH;
unsigned long lastUpdate = 0;
unsigned long malfunctionStart = 0;
int malfunctionStep = 0;
// Malfunction state machine
int malfunctionCycle = 0;
unsigned long malfunctionPhaseStart = 0;
enum MalfunctionPhase
{
  MALF_DIM,
  MALF_RESTORE,
  MALF_ON,
  MALF_OFF,
  MALF_DONE
};
MalfunctionPhase malfunctionPhase = MALF_DONE;

const unsigned long UPDATE_INTERVAL = 10; // 20ms between updates for 50 FPS

// Erratic, continuous power fluctuation malfunction effect
void portalMalfunctionEffect()
{
  unsigned long now = millis();
  static unsigned long lastJump = 0;
  static float targetBrightness = 1.0f;
  static float currentBrightness = 1.0f;
  static int jumpInterval = 100;
  // Always update portal animation for base effect
  gradientPosition = (gradientPosition + GRADIENT_MOVE) % NUM_LEDS;

  // Every so often, pick a new random target brightness and jump interval
  if (now - lastJump > jumpInterval)
  {
    targetBrightness = 0.2f + 1.3f * (random(1000) / 1000.0f); // 0.2 to 1.5
    jumpInterval = 40 + random(160);                           // 40-200ms between jumps
    lastJump = now;
  }
  // Move currentBrightness toward targetBrightness erratically
  float delta = targetBrightness - currentBrightness;
  currentBrightness += delta * (0.3f + 0.5f * (random(1000) / 1000.0f));
  // Add a small random noise
  currentBrightness += (random(-30, 31)) / 255.0f;
  currentBrightness = constrain(currentBrightness, 0.05f, 1.5f);

  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = effectLeds[(i + gradientPosition) % NUM_LEDS];
    leds[i].nscale8((uint8_t)(currentBrightness * 170 + 85));
  }
  FastLED.show();
  // The effect runs continuously until interrupted
}
// Gradient control points
CRGB gradientColors[80]; // 800 LEDs / 10 = 80 control points
int numGradientPoints = 0;

// Circle parameters
const float CIRCLE_RADIUS = NUM_LEDS / (2.0 * PI); // Radius to fit all LEDs in a circle

void setup()
{
  Serial.begin(115200);
  Serial.println("WS2812 Traveling Light Test Starting...");

  // Initialize LED strip
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear();
  FastLED.show();

  // Startup diagnostic: show solid R, G, B to verify wiring and color order (10% brightness)
  delay(100);
  FastLED.setBrightness(25); // 10% of 255
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
  FastLED.setBrightness(BRIGHTNESS); // Restore normal brightness

  // Initialize buttons with internal pull-up resistors
  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);
  pinMode(BUTTON3_PIN, INPUT_PULLUP);

  Serial.println("Setup complete. Press button 1 to start portal effect.");
  Serial.print("Total LEDs: ");
  Serial.println(NUM_LEDS);
  Serial.print("Circle radius: ");
  Serial.print(CIRCLE_RADIUS);
  Serial.println(" LEDs");
}

// Convert LED index to circular coordinates
void getLEDPosition(int ledIndex, float &x, float &y)
{
  // Map LED index to angle around circle
  float angle = (2.0 * PI * ledIndex) / NUM_LEDS;

  // Convert to Cartesian coordinates
  x = CIRCLE_RADIUS * cos(angle);
  y = CIRCLE_RADIUS * sin(angle);
}

// Calculate distance from LED to circle center
float getCircleDistance(float x, float y)
{
  return sqrt(x * x + y * y);
}

// Generate a random color between blue, purple, and almost white
CRGB getRandomDriverColor()
{
  // Blue (hue 160), purple (hue 200), almost white (hue 180, low saturation)
  uint8_t hue = 160 + random(41); // 160 to 200
  uint8_t sat = 180 + random(76); // 180-255 (allow some desaturation for near-white)
  if (random(10) == 0)
    sat = 30 + random(40);        // 10% chance for very low saturation (almost white)
  uint8_t val = 51 + random(205); // 20% to 100% brightness
  return CHSV(hue, sat, val);
}

// Create gradient control points
void createGradientPoints()
{
  numGradientPoints = 0;

  // Generate control points every GRADIENT_STEP LEDs
  for (int i = 0; i < NUM_LEDS; i += GRADIENT_STEP)
  {
    gradientColors[numGradientPoints] = getRandomDriverColor();
    numGradientPoints++;
  }

  Serial.print("Created ");
  Serial.print(numGradientPoints);
  Serial.print(" gradient control points for ");
  Serial.print(NUM_LEDS);
  Serial.println(" LEDs");
}

// Interpolate between two colors
CRGB interpolateColor(CRGB color1, CRGB color2, float ratio)
{
  // Clamp ratio between 0 and 1
  ratio = constrain(ratio, 0.0, 1.0);

  // Linear interpolation for each color component
  uint8_t r = color1.r + (color2.r - color1.r) * ratio;
  uint8_t g = color1.g + (color2.g - color1.g) * ratio;
  uint8_t b = color1.b + (color2.b - color1.b) * ratio;

  return CRGB(r, g, b);
}

void generatePortalEffect()
{
  // 1. Place driver LEDs at random intervals (5-15 apart)
  const int minDist = 5;
  const int maxDist = 15;
  int driverIndices[100]; // max possible drivers
  CRGB driverColors[100];
  int numDrivers = 0;
  int idx = 0;
  while (idx < NUM_LEDS - minDist)
  {
    driverIndices[numDrivers] = idx;
    driverColors[numDrivers] = getRandomDriverColor();
    numDrivers++;
    int step = minDist + random(maxDist - minDist + 1); // 5-15
    if (idx + step > NUM_LEDS - minDist)
      break;
    idx += step;
  }
  // Wrap around: add first driver at end for circular gradient
  driverIndices[numDrivers] = NUM_LEDS;
  driverColors[numDrivers] = driverColors[0];
  numDrivers++;

  // 2. Fill between driver LEDs with gradients
  for (int d = 0; d < numDrivers - 1; d++)
  {
    int start = driverIndices[d];
    int end = driverIndices[d + 1];
    CRGB c1 = driverColors[d];
    CRGB c2 = driverColors[d + 1];
    int segLen = end - start;
    for (int i = 0; i < segLen; i++)
    {
      float ratio = (segLen == 1) ? 0.0 : (float)i / (segLen - 1);
      CRGB col = interpolateColor(c1, c2, ratio);
      effectLeds[start + i] = col;
    }
  }
}

void portalEffect()
{
  // Rotate the effectLeds buffer by gradientPosition
  float fadeScale = 1.0f;
  if (fadeInActive)
  {
    unsigned long now = millis();
    float t = (now - fadeInStart) / (float)FADE_IN_DURATION_MS;
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
    float t = (now - fadeOutStart) / (float)FADE_OUT_DURATION_MS;
    fadeScale = 1.0f - constrain(t, 0.0f, 1.0f);
    if (fadeScale <= 0.0f)
    {
      fadeOutActive = false;
      fadeScale = 0.0f;
      animationActive = false;
      FastLED.clear();
      FastLED.show();
      return;
    }
  }
  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = effectLeds[(i + gradientPosition) % NUM_LEDS];
    if (fadeScale < 1.0f)
      leds[i].nscale8((uint8_t)(fadeScale * 255));
  }
  FastLED.show();
}

void loop()
{
  // Check button presses (with simple debouncing)
  bool currentButton1State = digitalRead(BUTTON1_PIN);
  bool currentButton2State = digitalRead(BUTTON2_PIN);
  bool currentButton3State = digitalRead(BUTTON3_PIN);

  // Button 1: normal portal animation
  if (lastButton1State == HIGH && currentButton1State == LOW)
  {
    // Always stop malfunction and return to default animation
    if (!animationActive)
    {
      animationActive = true;
      fadeInActive = true;
      fadeInStart = millis();
      Serial.println("Animation STARTED - Portal effect active (fade in)");
      gradientPosition = 0;
      generatePortalEffect();
    }
    else
    {
      animationActive = false;
      Serial.println("Animation STOPPED");
      FastLED.clear();
      FastLED.show();
    }
    malfunctionActive = false;
    delay(200);
  }
  lastButton1State = currentButton1State;

  // Button 3: fade out animation (works from any mode)
  if (currentButton3State == LOW && !fadeOutActive && (animationActive || malfunctionActive))
  {
    Serial.println("Fade out triggered by button 3");
    fadeOutActive = true;
    fadeOutStart = millis();
    fadeInActive = false;
    animationActive = false;
    malfunctionActive = false;
    // No delay here, fade-out starts immediately and animation keeps rotating
  }

  // Button 2: portal malfunction effect
  if (lastButton2State == HIGH && currentButton2State == LOW && !malfunctionActive)
  {
    Serial.println("Portal MALFUNCTION triggered!");
    malfunctionActive = true;
    animationActive = false;
    malfunctionCycle = 0;
    malfunctionPhase = MALF_DIM;
    malfunctionPhaseStart = millis();
    delay(200);
  }
  lastButton2State = currentButton2State;

  // Run effects
  if (fadeOutActive && (millis() - lastUpdate >= UPDATE_INTERVAL))
  {
    gradientPosition = (gradientPosition + GRADIENT_MOVE) % NUM_LEDS;
    portalEffect();
    lastUpdate = millis();
  }
  else if (malfunctionActive && (millis() - lastUpdate >= UPDATE_INTERVAL))
  {
    portalMalfunctionEffect();
    lastUpdate = millis();
  }
  else if (animationActive && (millis() - lastUpdate >= UPDATE_INTERVAL))
  {
    gradientPosition = (gradientPosition + GRADIENT_MOVE) % NUM_LEDS;
    portalEffect();
    lastUpdate = millis();
  }
}
