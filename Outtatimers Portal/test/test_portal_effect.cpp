#include "mock_led_driver.h"
#include "../src/portal_effect.h"
#include "mock_led_driver.h"
#include "mock_led_driver.h"
#include "mock_led_driver.h"
#include <cassert>
#include <iostream>

// Simulated millis() for unit tests
static unsigned long simulated_time = 0;
extern "C" unsigned long millis() { return simulated_time; }

int main()
{
  // Use small N for native test
  const int N = 32;
  MockLEDDriver<N> mock;
  PortalEffectTemplate<N, 4, 1> portal(&mock);

  portal.begin();
  portal.setBrightness(128);
  portal.fillSolid(CRGB::Red());
  // verify buffer filled with red
  for (int i = 0; i < N; ++i)
    assert(mock.buffer[i].r == 255 && mock.buffer[i].g == 0 && mock.buffer[i].b == 0);

  portal.clear();
  for (int i = 0; i < N; ++i)
    assert(mock.buffer[i].r == 0 && mock.buffer[i].g == 0 && mock.buffer[i].b == 0);

  // Test generatePortalEffect()
  CRGB testBuffer[N];
  CRGB *result = portal.testGeneratePortalEffect(testBuffer);
  CRGB *result = portal.testGeneratePortalEffect(testBuffer);
  CRGB *result = portal.generatePortalEffect(testBuffer);
  std::cout << "Testing generatePortalEffect()" << std::endl;
  assert(result == testBuffer);
  // Basic check: all values should be non-zero (indicating some color)
  for (int i = 0; i < N; i++)
  {
    assert(testBuffer[i].r > 0 || testBuffer[i].g > 0 || testBuffer[i].b > 0);
  }
  // Start portal and run a few updates to ensure no crashes
  portal.start();
  unsigned long t = 0;
  for (int k = 0; k < 10; ++k)
  {
    t += 50;
    portal.update(t);
  }

  std::cout << "Portal native test passed" << std::endl;
  return 0;
}
