# ATTINY10 Random LED Blinker Code

## 🎯 Overview

This code implements a random LED blinking pattern for the ATTINY10 microcontroller, designed for ultra-low power consumption using a coin cell battery.

## ✨ Features

- **Random blinking intervals**: LED on/off times vary randomly
- **Power efficient**: Uses simple delay loops optimized for 1MHz clock
- **Coin cell optimized**: Designed for CR2032 battery operation
- **Simple hardware**: Single LED on PB0 with 330Ω resistor

## 🔧 Hardware Requirements

- **ATTINY10-TS** microcontroller (SOT-23-6)
- **LED** connected to **PB0** through **330Ω resistor**
- **CR2032** coin cell power supply
- **0.1µF decoupling capacitor** across VCC-GND
- **USBasp programmer** with TPI capability

## 📊 Timing Specifications

### LED Intervals (Random)

- **ON time**: 0.2s to 0.5s (short, bright)
- **OFF time**: 0.3s to 2.0s (longer, saves battery)
- **Behavior**: Asymmetric timing for battery optimization

### Distribution

**ON time (200-500ms):**

- 0.2s, 0.25s, 0.3s, 0.35s, 0.4s, 0.45s, 0.5s

**OFF time (300-2000ms):**

- 0.3s, 0.5s, 0.75s, 1.0s, 1.25s, 1.5s, 2.0s

## 🏗️ Code Architecture

### Core Components

1. **Random Generator**: Creates pseudo-random timing variations
2. **Delay Functions**: Simple calibrated loops for precise timing
3. **Port Control**: Direct port manipulation for LED control
4. **Main Loop**: Continuous blinking with random intervals

### Key Functions

- `getRandom()`: Generate pseudo-random numbers using LCG algorithm
- `getRandomOnDelay()`: Get random ON time (200-500ms)
- `getRandomOffDelay()`: Get random OFF time (300-2000ms)
- `delayMilliseconds()`: Power-efficient delay function

## ⚡ Power Optimization

- **Asymmetric timing**: Short ON, long OFF for battery saving
- **Simple delays**: No complex peripherals or interrupts
- **Efficient code**: Optimized for minimal flash usage
- **Expected battery life**: 100+ hours on CR2032

## 🚀 Usage

### Building

```bash
# Using Makefile
make clean && make

# Check toolchain
make check-toolchain
```

### Uploading

```bash
# Upload to ATTINY10
make upload

# Upload and verify
make upload-verify
```

### Programming Interface

- **VCC**: Power supply (3V from coin cell)
- **GND**: Ground
- **RESET**: Reset pin for programming
- **PB0**: LED output pin

## 🔍 Code Details

### Random Number Generation

```cpp
uint16_t getRandom() {
  randomSeed = randomSeed * 214013 + 2531011;
  return randomSeed;
}
```

### Timing Functions

```cpp
// Get random ON delay (200-500ms)
uint16_t getRandomOnDelay() {
  // Maps random values to specific timing ranges
}

// Get random OFF delay (300-2000ms)
uint16_t getRandomOffDelay() {
  // Maps random values to specific timing ranges
}
```

### Main Loop

```cpp
while (1) {
  // Get random ON delay
  // Turn LED on
  // Wait for ON time
  // Get random OFF delay
  // Turn LED off
  // Wait for OFF time
}
```

## 📈 Expected Behavior

The LED will blink with **asymmetric timing**:

- **ON**: Brief flashes (0.2-0.5s) - bright and visible
- **OFF**: Longer periods (0.3-2.0s) - saves battery
- **Pattern**: Creates natural, unpredictable blinking like a firefly

## 🎨 Customization

### Adjusting Timing Ranges

Modify the `getRandomOnDelay()` and `getRandomOffDelay()` functions to change timing distributions.

### Changing Duty Cycle

Adjust the ratio between ON and OFF times for different power consumption profiles.

### Adding More Randomness

Modify the random number generator or add more timing variations.

## 🚨 Important Notes

- **No external timing components** - Uses calibrated delay loops
- **Ultra-low power** - Asymmetric timing maximizes battery life
- **Simple hardware** - Minimal component count
- **TPI programming** - Requires TPI-capable programmer (USBasp)
- **1MHz internal clock** - No external oscillator needed

---

_This code is part of the Outtatimers Implant project - creating the smallest possible coin-cell-powered LED blinker circuit._
