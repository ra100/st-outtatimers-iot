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
bool lastButtonState = HIGH;
unsigned long lastUpdate = 0;
const unsigned long UPDATE_INTERVAL = 10; // 20ms between updates for 50 FPS

// Portal effect variables
float portalAngle = 0.0;
const float PORTAL_SPEED = 4.0 * PI / 500.0; // Complete circle in 10 seconds (500 updates * 20ms = 10s)

// Gradient rotation variables
int gradientPosition = 0;     // Current position of gradient
const int GRADIENT_MOVE = 2;  // LEDs to move per update (2x speed)
const int GRADIENT_STEP = 10; // Generate color every 10th LED

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
  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = effectLeds[(i + gradientPosition) % NUM_LEDS];
  }
  FastLED.show();
}

void loop()
{
  // Check button press (with simple debouncing)
  bool currentButtonState = digitalRead(BUTTON1_PIN);

  if (lastButtonState == HIGH && currentButtonState == LOW)
  {
    // Button just pressed - toggle animation
    animationActive = !animationActive;

    if (animationActive)
    {
      Serial.println("Animation STARTED - Portal effect active");
      portalAngle = 0.0;      // Reset portal angle
      gradientPosition = 0;   // Reset gradient position
      generatePortalEffect(); // Create static portal effect
    }
    else
    {
      Serial.println("Animation STOPPED");
      FastLED.clear(); // Turn off all LEDs
      FastLED.show();
    }

    delay(200); // Simple debounce delay
  }

  lastButtonState = currentButtonState;

  // Run portal effect if active
  if (animationActive && (millis() - lastUpdate >= UPDATE_INTERVAL))
  {
    gradientPosition = (gradientPosition + GRADIENT_MOVE) % NUM_LEDS;
    portalEffect();
    lastUpdate = millis();
  }
}
