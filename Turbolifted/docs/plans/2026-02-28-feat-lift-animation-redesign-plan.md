---
title: "feat: Redesign LIFT_ANIMATION effect with sub-modes, skip regions, and new web UI"
type: feat
status: completed
date: 2026-02-28
origin: docs/brainstorms/2026-02-27-lift-animation-redesign-brainstorm.md
deepened: 2026-02-28
---

# Redesign LIFT_ANIMATION Effect

## Enhancement Summary

**Deepened on:** 2026-02-28
**Research agents used:** Performance Oracle, Architecture Strategist, Security Sentinel, Code Simplicity Reviewer, Pattern Recognition Specialist, Best Practices Researcher

### Key Improvements

1. **Critical bug fixes** in `computeSegments()` underflow guard, stream direction arguments, and fade function reference
2. **Performance**: UPDATE_INTERVAL_MS 10→25 (44 FPS max due to WS2812B wire time), float hoisting, integer-first pulse phase
3. **Simplification**: drop ArduinoJson dependency (use `snprintf`), validate at setters not render path, consider EEPROM over LittleFS JSON
4. **Security**: bounds-check skip parameters before arithmetic, clamp `rightEnd <= NUM_LEDS`, add `default` case in submode switch

### Critical Issues Found

| Issue | Severity | Location |
|-------|----------|----------|
| `computeSegments()` doesn't recompute `available` after mutating `skipM` | CRITICAL | Phase 3a |
| `renderStreamSegment` direction arguments are INVERTED | CRITICAL | Phase 3c |
| `handleLiftFade()` doesn't exist — use `calculateFade()` pattern | HIGH | Phase 3b |
| Stream mode double-brightness (CHSV value + setBrightness) | HIGH | Phase 3c |
| `skipS + skipE >= NUM_LEDS` causes unsigned underflow | HIGH | Phase 3a |
| Pulse phase loses float precision after ~25 days | MEDIUM | Phase 3e |

---

## Overview

Rewrite the LIFT_ANIMATION effect (mode 1) to simulate TNG-style turbolift wall panel lights on a U-shaped 756-LED WS2812B strip. The strip has two vertical segments connected by a horizontal U-bend that should remain dark. The effect needs four sub-modes (stream down, stream up, static, pulse), configurable skip regions for the physical layout, and a new web UI replacing the legacy Classic/Virtual Gradient controls.

## Problem Statement

The current LIFT_ANIMATION implementation has fundamental issues:
- Renders from center outward instead of streaming in one direction
- No awareness of the physical U-shape layout (all 756 LEDs render, including the U-bend)
- Only one visual mode (no static/pulse sub-modes)
- Packet shape is asymmetric linear ramp, not the desired symmetric Gaussian
- Web UI has no controls for lift parameters at all
- No HTTP endpoints exist for lift-specific parameters (speed, width, spacing, etc.)
- Malfunction effect overlays legacy gradient instead of modulating lift colours
- No parameter persistence across reboots

## Proposed Solution

A phased rewrite touching four layers: config defaults, runtime parameter management, effect rendering, HTTP API, and web UI. Sub-modes are dispatched within `liftAnimationEffect()` to keep the top-level mode system unchanged.

(see brainstorm: `docs/brainstorms/2026-02-27-lift-animation-redesign-brainstorm.md`)

## Technical Considerations

### Skip Region Segment Derivation

The U-shaped strip maps to two active segments:

```
Physical:     LED 0 (top-left) ──down── U-bend ──up── LED 755 (top-right)

Segments:
  Left:   [skipStart, leftEnd)           where leftEnd = skipStart + leftLen
  Gap:    [leftEnd, rightStart)          skipMiddle LEDs blanked
  Right:  [rightStart, rightEnd)         where rightEnd = NUM_LEDS - skipEnd

  leftLen = rightLen = (NUM_LEDS - skipStart - skipMiddle - skipEnd) / 2
```

Both segments must have at least 1 LED. Guard: if `skipStart + skipMiddle + skipEnd >= NUM_LEDS - 1`, clamp `skipMiddle` so at least 2 LEDs remain (1 per segment).

#### Research Insights: Segment Computation

**CRITICAL BUG — recompute `available` after mutating `skipM`:**
The Phase 3a pseudocode mutates `skipM` when `available < 2` but does NOT recompute `available`. The fix:

```cpp
int total = skipS + skipM + skipE;
if (total > NUM_LEDS - 2) {
  skipM = NUM_LEDS - skipS - skipE - 2;
  if (skipM < 0) skipM = 0;
}
int available = NUM_LEDS - skipS - skipM - skipE;
```

**CRITICAL BUG — unsigned underflow:**
If `skipS + skipE >= NUM_LEDS`, the subtraction wraps. Accept skip setters as `int` (not `uint16_t`), validate, and clamp before storing:

```cpp
static void setLiftSkipStart(int v) {
  liftSkipStart = constrain(v, 0, NUM_LEDS / 2);
}
```

**Bounds check**: Always verify `rightEnd <= NUM_LEDS` after computation.

### Right Segment Direction Reversal

The right strip's LED indices increase from bottom to top (physically upward), but "stream down" means visually downward. The rendering loop must iterate the right segment in reverse index order so both strips appear to stream in the same physical direction.

For STREAM_DOWN: left segment iterates `i++` (LED 0 = top), right segment iterates `i--` (LED 755 = top).
For STREAM_UP: reverse of above.

#### Research Insights: Direction Arguments

**CRITICAL BUG — direction arguments are INVERTED in Phase 3c code:**
For STREAM_DOWN (`upward=false`): left should NOT reverse (iterate naturally top→bottom), right SHOULD reverse (iterate high→low). The call should be:

```cpp
// STREAM_DOWN (upward=false): both strips stream top-to-bottom
renderStreamSegment(seg.leftStart, seg.leftEnd, false, ...);   // left: natural order
renderStreamSegment(seg.rightStart, seg.rightEnd, true, ...);  // right: reversed

// STREAM_UP (upward=true): both strips stream bottom-to-top
renderStreamSegment(seg.leftStart, seg.leftEnd, true, ...);    // left: reversed
renderStreamSegment(seg.rightStart, seg.rightEnd, false, ...); // right: natural order
```

So the calls should be `renderStreamSegment(..., upward, ...)` for left and `renderStreamSegment(..., !upward, ...)` for right.

### Gaussian Packet Shape

Replace linear ramp with symmetric Gaussian approximation:

```cpp
// posInPacket: 0 at one edge, width-1 at other edge
// center of packet is at width/2
float centerDist = (float)posInPacket - (float)(width - 1) / 2.0f;
float sigma = (float)width / 4.0f;  // 95% of energy within packet width
float intensity = expf(-(centerDist * centerDist) / (2.0f * sigma * sigma));
```

For ESP8266 performance, consider a lookup table or quadratic approximation: `intensity = 1.0f - 4.0f * centerDist * centerDist / (width * width)` clamped to [0, 1].

#### Research Insights: Envelope Performance

**Pre-compute envelope lookup table** to avoid per-LED float math:

```cpp
// In liftStreamEffect, before the render loops:
uint8_t envelope[width];  // VLA or small fixed buffer (max 50)
float invCenter = (width > 1) ? 2.0f / (float)(width - 1) : 0.0f;
for (int p = 0; p < width; p++) {
  float norm = (float)p * invCenter - 1.0f;
  float val = 1.0f - norm * norm;
  envelope[p] = (val > 0.0f) ? (uint8_t)(val * 255.0f) : 0;
}
```

Then in the inner loop, just index `envelope[posInPacket]` — no floats.

Alternatively, use FastLED's `ease8InOutQuad()` for a hardware-friendly 8-bit curve.

### Double-Brightness Bug

`singleColorEffect()` applies brightness both in CHSV value channel and via `_driver->setBrightness()`. Fix: use `setBrightness(255)` (pass-through) when brightness is already encoded in the colour value, or use `CHSV(hue, sat, 255)` and let `setBrightness(brightness)` handle it. Choose the latter for consistency with other modes.

#### Research Insights

This bug also exists in the streaming sub-mode code. The `renderStreamSegment` uses `CHSV(hue, sat, 255 * intensity)` which encodes brightness in the value channel, then `liftAnimationEffect` also calls `setBrightness(brightness)`. Fix: either encode brightness into the envelope (multiply `envelope[p] * brightness / 255`) and call `setBrightness(255)`, or pass `CHSV(hue, sat, envelope[p])` and let `setBrightness` handle overall brightness. The latter is simpler.

### Malfunction Interaction

Current `turboliftMalfunctionEffect()` reads from `effectLeds[]` (legacy gradient buffer). For LIFT_ANIMATION mode, malfunction should modulate the already-rendered lift output by applying random brightness flicker to `_driver->getBuffer()` after the lift effect writes to it, rather than overwriting from a different buffer.

#### Research Insights

Apply malfunction overlay BEFORE `show()`, not after. The malfunction should be applied in the same render pass as the lift effect to avoid a visible double-write.

### Frame Rate Constraint

#### Research Insights: WS2812B Wire Time

**UPDATE_INTERVAL_MS must change from 10 to 25:**
WS2812B transmission for 756 LEDs takes ~22.7ms (30µs/LED × 756 + 50µs reset). This means:
- Maximum achievable frame rate: ~44 FPS
- Current 10ms (100 FPS) target is physically impossible
- Set `UPDATE_INTERVAL_MS = 25` (~40 FPS) to avoid back-to-back writes

Also add `#define FASTLED_ALLOW_INTERRUPTS 0` in `platformio.ini` build flags to prevent WiFi interrupts from corrupting LED data on ESP8266.

### Config Persistence

LittleFS is already mounted for serving `index.html`. Add `/config.json` for parameter persistence:
- Write on every `set_*` HTTP call
- Read in `ConfigManager::begin()` if file exists
- Keep compile-time defaults as fallback

#### Research Insights: Persistence Strategy

**Debounce writes to prevent flash wear:**
LittleFS write-on-every-call will wear ESP8266 flash (~10K-100K write cycles). Implement a 5-second deferred write:

```cpp
static unsigned long lastChangeMs = 0;
static bool dirty = false;

static void markDirty() { dirty = true; lastChangeMs = millis(); }

static void flushIfNeeded() {
  if (dirty && (millis() - lastChangeMs >= 5000)) {
    saveConfig();
    dirty = false;
  }
}
```

Call `flushIfNeeded()` from the main loop.

**Skip ArduinoJson — use `snprintf` for JSON building:**
ArduinoJson adds ~5KB flash. The codebase already has manual string construction patterns. Build JSON with `snprintf`:

```cpp
char buf[256];
snprintf(buf, sizeof(buf),
  "{\"mode\":%u,\"liftSubmode\":%u,\"liftSpeed\":%u,...}",
  mode, submode, speed, ...);
server_.send(200, "application/json", buf);
```

For parsing, a simple `parseConfigKV()` function scanning for `"key":value` patterns is sufficient for flat JSON.

**File size guard on read:** Check file size before reading; reject files > 1KB to prevent corrupted data from causing issues.

**Alternative: EEPROM struct** — simpler than LittleFS JSON. Pack all parameters into a struct, write with `EEPROM.put()`, read with `EEPROM.get()`. Add a magic number for validation. Pros: simpler, no parsing. Cons: less flexible for future parameter additions.

## Implementation Phases

### Phase 1: Config Layer (`src/config.h`, `src/config_manager.h`)

Add new parameters to `ConfigManager`:

| Parameter | Type | Default | Range | Notes |
|-----------|------|---------|-------|-------|
| `liftSubmode` | `uint8_t` | 0 | 0-3 | STREAM_DOWN, STREAM_UP, STATIC, PULSE |
| `liftSkipStart` | `uint16_t` | 0 | 0-NUM_LEDS/2 | LEDs to skip at strip start |
| `liftSkipMiddle` | `uint16_t` | 0 | 0-NUM_LEDS/2 | LEDs to blank in U-bend |
| `liftSkipEnd` | `uint16_t` | 0 | 0-NUM_LEDS/2 | LEDs to skip at strip end |
| `liftPulseMin` | `uint8_t` | 30 | 0-255 | Pulse minimum brightness |
| `liftPulseMax` | `uint8_t` | 255 | 0-255 | Pulse maximum brightness |
| `liftPulseSpeed` | `uint8_t` | 3 | 0-10 | Pulse oscillation speed |

Update existing clamp ranges:
- `liftWidth`: change max from 20 to 50 (brainstorm spec)
- `liftSpacing`: change max from 50 to 100 (brainstorm spec)

Add sub-mode enum to `config.h`:
```cpp
namespace TurboliftConfig {
  namespace Effects {
    enum class LiftSubMode : uint8_t {
      STREAM_DOWN = 0,
      STREAM_UP = 1,
      STATIC = 2,
      PULSE = 3
    };
  }
}
```

Add defaults to `config.h`:
```cpp
constexpr uint8_t DEFAULT_SUBMODE = 0;
constexpr uint16_t DEFAULT_SKIP_START = 0;
constexpr uint16_t DEFAULT_SKIP_MIDDLE = 0;
constexpr uint16_t DEFAULT_SKIP_END = 0;
constexpr uint8_t DEFAULT_PULSE_MIN = 30;
constexpr uint8_t DEFAULT_PULSE_MAX = 255;
constexpr uint8_t DEFAULT_PULSE_SPEED = 3;
```

**Note:** `DEFAULT_BRIGHTNESS` already exists at `config.h:120` with value 200 — do not re-add.

#### Research Insights

**Validation at setters, not render path:**
Move all `constrain()` logic into ConfigManager setters so the render loop can trust values are valid:

```cpp
static void setLiftSkipStart(int v) {
  liftSkipStart = constrain(v, 0, Hardware::NUM_LEDS / 2);
}
```

Accept `int` in skip setters (not `uint16_t`) to avoid truncation before `constrain()`.

**Reset `liftOffset` on submode change** to prevent stale offset producing a visual jump:

```cpp
static void setLiftSubmode(uint8_t v) {
  liftSubmode = constrain(v, 0, 3);
  // Effect code should reset liftOffset when submode changes
}
```

**Initialize new params in `begin()`** alongside existing parameters.

**Files:** `src/config.h`, `src/config_manager.h`

### Phase 2: HTTP API (`src/wifi_input_source.h`)

Register new endpoints in `setupWebServerRoutes()`:

```
GET /set_lift_submode?submode=0-3
GET /set_lift_skip_start?count=N
GET /set_lift_skip_middle?count=N
GET /set_lift_skip_end?count=N
GET /set_lift_pulse_min?value=0-255
GET /set_lift_pulse_max?value=0-255
GET /set_lift_pulse_speed?value=0-10
```

Add missing lift parameter endpoints (these have ConfigManager setters but no HTTP handlers):
```
GET /set_lift_speed?speed=0-10
GET /set_lift_width?width=1-50
GET /set_lift_spacing?spacing=0-100
GET /set_lift_hue?hue=0-255
GET /set_lift_saturation?saturation=0-255
GET /set_lift_brightness?brightness=0-255
```

Update `/config` JSON response to include all lift parameters:
```json
{
  "mode": 1,
  "liftSubmode": 0,
  "liftSpeed": 5,
  "liftWidth": 8,
  "liftSpacing": 15,
  "liftHue": 160,
  "liftSaturation": 255,
  "liftBrightness": 200,
  "liftSkipStart": 0,
  "liftSkipMiddle": 100,
  "liftSkipEnd": 0,
  "liftPulseMin": 30,
  "liftPulseMax": 255,
  "liftPulseSpeed": 3
}
```

#### Research Insights

**Use `snprintf` for JSON response** instead of String concatenation:
```cpp
char buf[300];
int n = snprintf(buf, sizeof(buf),
  "{\"mode\":%u,\"liftSubmode\":%u,\"liftSpeed\":%u,"
  "\"liftWidth\":%u,\"liftSpacing\":%u,\"liftHue\":%u,"
  "\"liftSaturation\":%u,\"liftBrightness\":%u,"
  "\"liftSkipStart\":%u,\"liftSkipMiddle\":%u,\"liftSkipEnd\":%u,"
  "\"liftPulseMin\":%u,\"liftPulseMax\":%u,\"liftPulseSpeed\":%u}",
  mode, submode, speed, width, spacing, hue, sat, bri,
  skipStart, skipMiddle, skipEnd, pulseMin, pulseMax, pulseSpeed);
server_.send(200, "application/json", buf);
```

**Use `server_.streamFile()` for serving `index.html`** instead of reading char-by-char.

**DRY endpoint registration with helper:**
```cpp
void getIntArg(const char* name, void(*setter)(int), int lo, int hi) {
  if (server_.hasArg(name)) {
    int v = server_.arg(name).toInt();
    setter(constrain(v, lo, hi));
    server_.send(200, "text/plain", "OK");
  } else {
    server_.send(400, "text/plain", "Missing parameter");
  }
}
```

**Naming consistency:** Current HTTP skip endpoints use `count=N` but other lift endpoints use the parameter name (e.g., `speed=N`). Consider using consistent naming: `value=N` for all, or match the parameter name.

**Files:** `src/wifi_input_source.h`

### Phase 3: Effect Engine (`src/turbolift_effect.h`)

#### 3a: Segment computation helper

```cpp
struct LiftSegments {
  int leftStart, leftEnd;    // Left active segment [start, end)
  int rightStart, rightEnd;  // Right active segment [start, end)
};

LiftSegments computeSegments() {
  int skipS = ConfigManager::getLiftSkipStart();
  int skipM = ConfigManager::getLiftSkipMiddle();
  int skipE = ConfigManager::getLiftSkipEnd();

  // Guard: clamp total skip to leave at least 2 active LEDs
  int totalSkip = skipS + skipM + skipE;
  if (totalSkip > NUM_LEDS - 2) {
    skipM = NUM_LEDS - skipS - skipE - 2;
    if (skipM < 0) skipM = 0;
  }

  int available = NUM_LEDS - skipS - skipM - skipE;
  int halfLen = available / 2;

  LiftSegments seg;
  seg.leftStart = skipS;
  seg.leftEnd = skipS + halfLen;
  seg.rightStart = skipS + halfLen + skipM;
  seg.rightEnd = skipS + halfLen + skipM + halfLen;

  // Final bounds safety
  if (seg.rightEnd > NUM_LEDS) seg.rightEnd = NUM_LEDS;

  return seg;
}
```

#### 3b: Sub-mode dispatch

Replace current `liftAnimationEffect()` with:

```cpp
void liftAnimationEffect(unsigned long now) {
  // Use existing calculateFade() pattern (handleLiftFade does NOT exist)
  float fadeScale = 1.0f;
  if (fadeInActive) {
    fadeScale = calculateFade(true, fadeInStart,
                              TurboliftConfig::Timing::FADE_IN_DURATION_MS);
  } else if (fadeOutActive) {
    fadeScale = calculateFade(false, fadeOutStart,
                              TurboliftConfig::Timing::FADE_OUT_DURATION_MS);
    if (fadeScale == 0.0f) return;
  }

  LiftSegments seg = computeSegments();

  // Blank entire strip first
  for (int i = 0; i < NUM_LEDS; i++)
    _driver->setPixel(i, CRGB(0, 0, 0));

  uint8_t submode = ConfigManager::getLiftSubmode();
  switch (submode) {
    case 0: liftStreamEffect(now, seg, fadeScale, false); break; // DOWN
    case 1: liftStreamEffect(now, seg, fadeScale, true);  break; // UP
    case 2: liftStaticEffect(seg, fadeScale);              break;
    case 3: liftPulseEffect(now, seg, fadeScale);          break;
    default: liftStaticEffect(seg, fadeScale);             break; // safety
  }

  _driver->setBrightness(ConfigManager::getLiftBrightness());
  _driver->show();
}
```

#### Research Insights: Phase 3b

- **`handleLiftFade()` does not exist.** Use the inline `calculateFade()` pattern already used in `singleColorEffect()`.
- **Add `default` case** in the switch statement for safety.
- **`liftOffset` should reset** when submode changes to avoid stale position jump.

#### 3c: Streaming sub-mode

```cpp
void liftStreamEffect(unsigned long now, LiftSegments seg, float fadeScale, bool upward) {
  uint8_t speed = ConfigManager::getLiftSpeed();
  uint8_t width = ConfigManager::getLiftWidth();
  uint8_t spacing = ConfigManager::getLiftSpacing();
  uint8_t hue = ConfigManager::getLiftHue();
  uint8_t sat = ConfigManager::getLiftSaturation();

  int period = (int)width + (int)spacing;
  if (period < 1) period = 1;

  // Advance offset (speed 0 = freeze)
  if (speed > 0) {
    unsigned long delayMs = ConfigManager::speedToDelay(speed);
    if (now - liftLastMove >= delayMs) {
      liftOffset = (liftOffset + 1) % period;
      liftLastMove = now;
    }
  }

  // Pre-compute envelope LUT (avoids per-LED float math)
  uint8_t envelope[width];
  float invCenter = (width > 1) ? 2.0f / (float)(width - 1) : 0.0f;
  for (int p = 0; p < width; p++) {
    float norm = (float)p * invCenter - 1.0f;
    float val = 1.0f - norm * norm;
    envelope[p] = (val > 0.0f) ? (uint8_t)(val * 255.0f) : 0;
  }

  // Pre-compute fade as uint8_t
  uint8_t fadeU8 = (uint8_t)(fadeScale * 255.0f);

  // Render left segment: upward=false → natural order, upward=true → reversed
  renderStreamSegment(seg.leftStart, seg.leftEnd, upward,
                      width, period, hue, sat, envelope, fadeU8);

  // Render right segment: upward=false → reversed, upward=true → natural order
  renderStreamSegment(seg.rightStart, seg.rightEnd, !upward,
                      width, period, hue, sat, envelope, fadeU8);
}

void renderStreamSegment(int start, int end, bool reverseDir, int width, int period,
                         uint8_t hue, uint8_t sat, const uint8_t* envelope,
                         uint8_t fadeU8) {
  int segLen = end - start;
  for (int s = 0; s < segLen; s++) {
    int ledIndex = reverseDir ? (end - 1 - s) : (start + s);

    int posInPattern = (s + liftOffset) % period;
    if (posInPattern < width) {
      // Use pre-computed envelope; brightness handled by setBrightness()
      uint8_t val = envelope[posInPattern];
      CRGB ledColor = CHSV(hue, sat, val);
      // Apply fade
      ledColor.nscale8(fadeU8);
      _driver->setPixel(ledIndex, ledColor);
    }
    // else: already black from initial clear
  }
}
```

#### Research Insights: Phase 3c

- **Direction fix**: Left segment uses `upward` as reverseDir, right uses `!upward`. This way STREAM_DOWN (`upward=false`) renders left in natural order (top→bottom) and right reversed (also top→bottom visually).
- **Envelope LUT**: Pre-computed before the inner loop, avoids float math per LED.
- **`nscale8()` for fade**: FastLED's hardware-friendly 8-bit scaling replaces float multiply.
- **No double-brightness**: Envelope encodes the packet shape intensity, `setBrightness()` handles overall brightness. The value channel carries only the envelope, not brightness × envelope.

#### 3d: Static sub-mode

```cpp
void liftStaticEffect(LiftSegments seg, float fadeScale) {
  uint8_t hue = ConfigManager::getLiftHue();
  uint8_t sat = ConfigManager::getLiftSaturation();
  CRGB color = CHSV(hue, sat, 255);  // brightness via setBrightness()
  uint8_t fadeU8 = (uint8_t)(fadeScale * 255.0f);
  color.nscale8(fadeU8);

  for (int i = seg.leftStart; i < seg.leftEnd; i++)
    _driver->setPixel(i, color);
  for (int i = seg.rightStart; i < seg.rightEnd; i++)
    _driver->setPixel(i, color);
}
```

#### 3e: Pulse sub-mode

```cpp
void liftPulseEffect(unsigned long now, LiftSegments seg, float fadeScale) {
  uint8_t hue = ConfigManager::getLiftHue();
  uint8_t sat = ConfigManager::getLiftSaturation();
  uint8_t pulseMin = ConfigManager::getLiftPulseMin();
  uint8_t pulseMax = ConfigManager::getLiftPulseMax();
  uint8_t pulseSpeed = ConfigManager::getLiftPulseSpeed();

  // Enforce min <= max
  if (pulseMin > pulseMax) { uint8_t tmp = pulseMin; pulseMin = pulseMax; pulseMax = tmp; }

  // Sine wave: map pulseSpeed 0-10 to period 10000ms-500ms
  unsigned long periodMs = 10000UL - (unsigned long)pulseSpeed * 950UL;
  if (periodMs < 500) periodMs = 500;

  // Integer modulo FIRST, then float — avoids precision loss after 25+ days uptime
  unsigned long phaseMs = now % periodMs;
  float phase = (float)phaseMs / (float)periodMs;
  float sine = 0.5f + 0.5f * sinf(phase * 2.0f * TurboliftConfig::Math::PI_F);
  uint8_t val = pulseMin + (uint8_t)((pulseMax - pulseMin) * sine);

  CRGB color = CHSV(hue, sat, val);
  uint8_t fadeU8 = (uint8_t)(fadeScale * 255.0f);
  color.nscale8(fadeU8);

  for (int i = seg.leftStart; i < seg.leftEnd; i++)
    _driver->setPixel(i, color);
  for (int i = seg.rightStart; i < seg.rightEnd; i++)
    _driver->setPixel(i, color);
}
```

#### Research Insights: Phase 3e

**Precision fix:** Using `fmodf((float)now / periodMs, 1.0f)` loses float precision after ~25 days because `millis()` exceeds float32 mantissa range (~16M). Solution: `now % periodMs` first (integer, exact), then divide.

#### 3f: Fix malfunction interaction

In `update()`, when mode is LIFT_ANIMATION and malfunction is active, apply brightness flicker to the already-rendered buffer instead of overwriting from `effectLeds`:

```cpp
if (malfunctionActive) {
  if (mode == (uint8_t)TurboliftConfig::Effects::EffectMode::LIFT_ANIMATION) {
    liftMalfunctionOverlay();  // flicker the current buffer
  } else {
    turboliftMalfunctionEffect();  // legacy path
  }
}
```

#### Research Insights: Phase 3f

Apply malfunction overlay **before** `show()`, not after, so the flickered frame is what actually gets sent to the strip.

**Files:** `src/turbolift_effect.h`

### Phase 4: Config Persistence

Add LittleFS-based JSON save/load to `ConfigManager`:

- `saveConfig()`: serialize all lift parameters to `/config.json` using `snprintf`
- `loadConfig()`: read `/config.json` on boot, parse, call setters
- Mark dirty on each `set_*` call; flush after 5-second idle
- Call `loadConfig()` in `ConfigManager::begin()` if file exists

#### Research Insights: Phase 4

**Debounced writes (5-second idle flush):**
```cpp
static unsigned long lastChangeMs = 0;
static bool dirty = false;

static void markDirty() { dirty = true; lastChangeMs = millis(); }
static void flushIfNeeded() {
  if (dirty && (millis() - lastChangeMs >= 5000)) {
    saveConfig();
    dirty = false;
  }
}
```

**File size guard:** Reject config files > 1KB on read to prevent corrupted data issues.

**No ArduinoJson needed.** Use `snprintf` to build JSON, simple string scanning (`strstr` + `atoi`) to parse. This saves ~5KB flash.

**EEPROM alternative** (simpler): Pack all params into a struct with magic number, use `EEPROM.put()`/`EEPROM.get()`. Simpler code, no parsing. Consider this if LittleFS JSON feels over-engineered.

**Files:** `src/config_manager.h`, `src/wifi_input_source.h`

### Phase 5: Web UI (`data/index.html`)

Replace the current Classic/Virtual Gradient controls with:

**Preset buttons section:**
```html
<div class="preset-buttons">
  <button class="preset active" data-submode="0" onclick="setPreset(0)">Moving Down</button>
  <button class="preset" data-submode="1" onclick="setPreset(1)">Moving Up</button>
  <button class="preset" data-submode="2" onclick="setPreset(2)">Stopped</button>
  <button class="preset" data-submode="3" onclick="setPreset(3)">Pulse</button>
</div>
```

**Basic controls (always visible):**
- Hue slider (0-255) with canvas colour preview
- Saturation slider (0-255)
- Brightness slider (0-255)
- Toggle on/off button
- Fade out button

**Advanced section (collapsible `<details>`):**
- Speed (0-10)
- Packet width (1-50)
- Packet spacing (0-100)
- Pulse min brightness (0-255)
- Pulse max brightness (0-255)
- Pulse speed (0-10)
- Skip Start count
- Skip Middle count
- Skip End count

**JavaScript updates:**
- `fetchConfig()` populates all new fields from expanded `/config` JSON
- `setPreset(submode)` sends `/set_lift_submode` and highlights active button
- All sliders use existing `oninput`/`onmouseup` pattern calling new lift endpoints
- Remove legacy controls: rotation speed, portal mode dropdown, hue min/max, sat min/max, gradient canvases

**Design continuity:**
- Keep dark theme (#1a1a1a, green #4CAF50 accents)
- Keep Status tab and device IP configuration
- Keep localStorage persistence for device IP

**Files:** `data/index.html`

## Acceptance Criteria

### Functional

- [x] Four sub-modes render correctly: STREAM_DOWN, STREAM_UP, STATIC, PULSE
- [x] Skip regions blank the correct LEDs (U-bend section dark)
- [x] Both vertical strips stream in the same physical direction
- [x] Gaussian-shaped packets (symmetric, peak at center)
- [x] Speed=0 freezes streaming animation
- [x] Pulse oscillates smoothly with configurable min/max/speed
- [x] All parameters adjustable via HTTP API
- [x] Web UI preset buttons switch sub-modes
- [x] Web UI advanced section controls all parameters
- [x] Parameters survive power cycle (persistence)
- [x] Fade in/out works with all sub-modes
- [x] Malfunction effect flickers lift colours, not legacy gradient

### Non-Functional

- [x] Compiles without warnings: `pio run -e d1`
- [x] Existing tests pass: `./run_tests.sh`
- [x] RAM usage stays under 60% (53.6% used, ~44KB)
- [x] Web UI loads and populates from `/config` on page open
- [x] Frame rate ~40 FPS (UPDATE_INTERVAL_MS = 25)
- [x] Config writes debounced (max once per 5 seconds)

## Dependencies & Risks

- **Physical measurement needed**: Default skip values require measuring the installed strip to determine segment boundaries. Can start with zeros (full strip) and configure via API after installation.
- **ESP8266 float performance**: `expf()` is slow on ESP8266. Using quadratic approximation `1 - x^2` with pre-computed LUT instead.
- **Flash write wear**: Debounced writes (5-second idle) mitigate LittleFS/EEPROM wear.
- **No ArduinoJson needed**: Use `snprintf` for JSON building and simple string parsing for reading. Saves ~5KB flash.
- **WS2812B wire time**: 756 LEDs × 30µs = ~22.7ms. Frame rate capped at ~44 FPS regardless of software speed.
- **ESP8266 WiFi interrupts**: Add `FASTLED_ALLOW_INTERRUPTS=0` build flag to prevent LED data corruption.

## Sources & References

### Origin

- **Brainstorm document:** [docs/brainstorms/2026-02-27-lift-animation-redesign-brainstorm.md](docs/brainstorms/2026-02-27-lift-animation-redesign-brainstorm.md)
  - Key decisions carried forward: U-shape skip regions, 4 sub-modes, same-direction streaming, Gaussian packets, preset-based web UI

### Internal References

- ConfigManager pattern: `src/config_manager.h:198-355`
- HTTP endpoint registration: `src/wifi_input_source.h:244-280`
- Current lift effect: `src/turbolift_effect.h:753-813`
- Single color effect (static reference): `src/turbolift_effect.h:696-748`
- Web UI: `data/index.html`
- Speed-to-delay mapping: `src/config_manager.h:329-335`

### Research Sources

- Performance Oracle: WS2812B timing constraints, float optimization, flash wear mitigation
- Architecture Strategist: Segment computation bugs, direction inversion, fade function patterns
- Security Sentinel: Integer overflow guards, input validation, LittleFS safety
- Code Simplicity Reviewer: YAGNI analysis, EEPROM alternative, LOC reduction opportunities
- Pattern Recognition Specialist: Namespace conventions, naming consistency, initialization gaps
- Best Practices Researcher: FastLED interrupt config, snprintf patterns, envelope LUT approach
