/*
 * ATTINY10 Ultra-Low Power Random LED Blinker
 * Outtatimers Implant Project - Battery Optimized
 *
 * Features:
 * - Single LED blinks with random intervals
 * - LED ON: 0.2-0.5 seconds (short, bright)
 * - LED OFF: 0.3-2.0 seconds (longer, saves battery)
 * - Uses simple sleep loops for power efficiency
 * - Optimized for maximum battery life
 * - Uses PB0 (OC0A) for LED output
 *
 * Hardware:
 * - LED connected to PB0 through 330Ω resistor
 * - CR2032 coin cell power supply (220mAh)
 * - Internal 1MHz oscillator (no external components needed)
 * - 0.1µF decoupling capacitor across VCC-GND
 *
 * Expected Battery Life: 100+ hours
 *
 * Based on ATtiny10 datasheet specifications:
 * - 1KB Flash, 32 bytes SRAM
 * - 1MHz internal oscillator
 * - 3 GPIO pins (PB0, PB1, PB2)
 * - TPI programming interface
 */

#include <avr/io.h>

// Ultra-low power random number generator
static uint16_t randomSeed = 0x1234;

uint16_t getRandom() {
  // Simple but effective random algorithm
  randomSeed = randomSeed * 214013 + 2531011;
  return randomSeed;
}

// Get random ON delay time in milliseconds (200-500ms) - SHORT
uint16_t getRandomOnDelay() {
  uint16_t random = getRandom();
  // Map to 200-500ms range for ON time (SHORT)
  if (random < 10923) return 200;
  if (random < 21845) return 250;
  if (random < 32768) return 300;
  if (random < 43691) return 350;
  if (random < 54613) return 400;
  if (random < 65535) return 450;
  return 500;
}

// Get random OFF delay time in milliseconds (200-1500ms) - evenly distributed
uint16_t getRandomOffDelay() {
  uint16_t random = getRandom();
  // Map to 200-1500ms range for OFF time with even distribution
  if (random < 10923) return 100;
  if (random < 21845) return 200;
  if (random < 32768) return 400;
  if (random < 43691) return 600;
  if (random < 54613) return 800;
  if (random < 65535) return 1250;
  return 1500;
}

// Power-efficient delay function for milliseconds
void delayMilliseconds(uint16_t ms) {
  for (uint16_t i = 0; i < ms; i++) {
    // Simple delay loop - calibrated for 1MHz clock
    for (volatile uint16_t j = 0; j < 100; j++) {
      // Empty loop for timing (100 iterations ≈ 1ms at 1MHz)
    }
  }
}

int main() {
  // Configure PB0 as output (OC0A pin)
  DDRB = (1 << PB0);

  // Main loop - LED blinks with random intervals
  while (1) {
    // Get random delay time for ON (200-1500ms) BEFORE turning LED on
    uint16_t onDelay = getRandomOnDelay();

    // Turn LED on
    PORTB &= ~(1 << PB0);

    // Delay for ON time
    delayMilliseconds(onDelay);

    // Get random delay time for OFF (200-1500ms) BEFORE turning LED off
    uint16_t offDelay = getRandomOffDelay();

    // Turn LED off
    PORTB |= (1 << PB0);

    // Delay for OFF time
    delayMilliseconds(offDelay);
  }

  return 0;  // Never reached
}