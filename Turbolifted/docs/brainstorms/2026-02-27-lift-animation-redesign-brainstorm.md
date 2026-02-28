# Brainstorm: LIFT_ANIMATION Effect Redesign

**Date:** 2026-02-27
**Status:** Draft

## What We're Building

A TNG-style turbolift wall panel effect for a 756-LED WS2812B strip mounted in a U-shape: two vertical strips (~1m apart) connected by a horizontal section. The effect simulates lights streaming past turbolift windows to convey motion.

### Physical Layout

```
LED 0                                LED 755
  |                                    |
  |  Left vertical    Right vertical   |
  |  (active)         (active)         |
  |                                    |
  +--- U-bend (horizontal, blanked) ---+
       LEDs skipped (configurable)
```

The strip is one continuous WS2812B chain. Three configurable skip counts control which LEDs are active:

- **skipStart**: LEDs to skip at the beginning (LED 0 to skipStart-1 = off)
- **skipMiddle**: LEDs to blank in the U-bend section (center gap)
- **skipEnd**: LEDs to skip at the end (last N LEDs = off)

This yields two active segments (left strip and right strip) that display the effect.

### Sub-Modes

The LIFT_ANIMATION effect (mode 1) supports multiple sub-modes, selectable via API parameter. The sub-mode system is designed to be extensible for future additions without code changes.

| Sub-mode | Name        | Description                                                                              |
| -------- | ----------- | ---------------------------------------------------------------------------------------- |
| 0        | STREAM_DOWN | Light packets stream downward on both strips (turbolift moving up)                       |
| 1        | STREAM_UP   | Light packets stream upward on both strips (turbolift moving down)                       |
| 2        | STATIC      | Solid single color, adjustable hue/saturation/brightness                                 |
| 3        | PULSE       | Slowly pulsing single color (sine wave breathe), adjustable min/max brightness and speed |

### Streaming Packet Visual (sub-modes 0 & 1)

- **Packet shape**: Gaussian/smooth blob - gradually brightens, peaks at center, gradually dims. Symmetric soft glow.
- **Color model**: Single uniform color across all packets (configurable hue, saturation, brightness via API)
- **Flow direction**: Both vertical strips stream in the same direction simultaneously
- **Spacing**: Configurable dark gap between packets
- **Width**: Configurable packet width (number of LEDs)
- **Speed**: Configurable via existing speed-to-delay mapping

Both strips show the same pattern simultaneously. The U-bend section remains dark.

### Static Mode (sub-mode 2)

- All active LEDs display the same color
- Hue, saturation, and brightness adjustable via API
- Essentially what SINGLE_COLOR (mode 0) does but within the lift animation's skip-aware rendering

### Pulse Mode (sub-mode 3)

- Smooth sine wave oscillation of brightness
- Configurable: min brightness, max brightness, pulse speed
- Same color parameters as static (hue, saturation)
- Starts with sine wave, can be extended later

## Why This Approach

**U-shape with skip regions** matches the physical installation: a single continuous LED strip that wraps between two vertical mounting surfaces. Rather than hardcoding segment boundaries, configurable skip counts let the same firmware work with different physical arrangements.

**Sub-modes within LIFT_ANIMATION** keep the top-level mode count manageable (4 modes) while providing the flexibility needed on-set. All sub-modes share the same parameter set (speed, width, spacing, hue, sat, brightness) and add only a sub-mode selector. This is controllable entirely via the existing WiFi API pattern.

**Gaussian packet shape** looks more natural than hard-edged blocks or comet tails for simulating ambient turbolift panel lighting seen through windows.

## Key Decisions

1. **Both strips flow the same direction** - this creates the correct motion illusion (scenery passing by)
2. **Direction is switchable** (up/down) via sub-mode, not a separate parameter - keeps the API simple
3. **Single uniform color** for all packets - clean, controllable, matches TNG aesthetic
4. **Skip regions** (start, middle, end) rather than arbitrary segment definitions - simpler configuration, covers the U-shape use case
5. **Sub-mode system** within LIFT_ANIMATION for extensibility without firmware changes
6. **Gaussian packet shape** for smooth, organic appearance
7. **Pulse uses sine wave** with configurable min/max/speed

## API Surface

New/modified endpoints needed:

```
GET /set_lift_submode?submode=0-3     (stream_down, stream_up, static, pulse)
GET /set_lift_skip_start?count=N      (LEDs to skip at strip start)
GET /set_lift_skip_middle?count=N     (LEDs to blank in U-bend)
GET /set_lift_skip_end?count=N        (LEDs to skip at strip end)
GET /set_lift_pulse_min?value=0-255   (minimum brightness for pulse mode)
GET /set_lift_pulse_max?value=0-255   (maximum brightness for pulse mode)
GET /set_lift_pulse_speed?value=0-10  (pulse oscillation speed)
```

Existing endpoints remain unchanged:

```
GET /set_lift_speed?speed=0-10
GET /set_lift_width?width=1-50
GET /set_lift_spacing?spacing=0-100
GET /set_lift_hue?hue=0-255
GET /set_lift_saturation?saturation=0-255
GET /set_lift_brightness?brightness=0-255
```

## Web UI

The current web UI (Classic/Virtual Gradient controls) will be **replaced** with a Lift Animation-focused interface. The existing `data/index.html` single-page app pattern is retained.

### Layout: Presets + Advanced

**Preset Buttons** (top section, always visible):
| Button | Action |
|--------|--------|
| Moving Down | Sets sub-mode 0 (STREAM_DOWN) with sensible defaults |
| Moving Up | Sets sub-mode 1 (STREAM_UP) with sensible defaults |
| Stopped | Sets sub-mode 2 (STATIC) |
| Pulse | Sets sub-mode 3 (PULSE) |

Each preset button sends the sub-mode change plus appropriate default parameters in one action. The currently active preset is visually highlighted.

**Basic Controls** (always visible below presets):

- Hue slider (0-255) with color preview
- Saturation slider (0-255)
- Brightness slider (0-255)
- Toggle on/off button
- Fade out button

**Advanced Section** (expandable/collapsible):

- Speed slider (0-10)
- Packet width slider (1-50)
- Packet spacing slider (0-100)
- Pulse min brightness (0-255)
- Pulse max brightness (0-255)
- Pulse speed (0-10)
- Skip Start count
- Skip Middle count
- Skip End count

### Design Continuity

- Keep the existing dark theme (#1a1a1a background, green accents)
- Keep the Status tab
- Remove the Classic/Virtual Gradient mode selector and related controls (hue range, sat range, rotation speed)
- Keep the device IP configuration and localStorage persistence

## Key Decisions

8. **Web UI replaces legacy mode controls** - Classic/Virtual Gradient UI removed, Lift Animation becomes primary
9. **Preset buttons for quick on-set operation** - Moving Up, Moving Down, Stopped, Pulse
10. **Advanced section is collapsible** - skip regions and fine-tuning hidden by default
11. **Basic color controls always visible** - hue, saturation, brightness are the most-used adjustments

## Open Questions

_None - all questions resolved through brainstorming._
