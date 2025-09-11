# Product Overview

## Purpose

The Portal Controller is an IoT device designed to create mesmerizing "portal" visual effects on a WS2812B addressable LED strip arranged in a circular pattern. It simulates ethereal, dynamic light portals using gradient-based animations, controllable both locally via physical buttons and remotely through a web interface and HTTP API.

## Problems Solved

- **Complex LED Programming**: Provides pre-built, tunable effects without requiring users to write Arduino code.
- **Remote Control**: Enables wireless control from smartphones or computers via a simple web UI, eliminating the need for direct physical access.
- **Reliable Input Handling**: Handles button debouncing and multiple input sources uniformly, ensuring robust operation.
- **Customization**: Allows runtime adjustment of speed, brightness, colors, and modes without reflashing firmware.
- **Diagnostics**: Includes startup LED tests and status indication for troubleshooting.

## How It Works

- **Hardware**: ESP8266 microcontroller (e.g., Lolin D1 Mini) connected to an 800-LED WS2812B strip, 3 push buttons, and on-board status LED.
- **Core Effect**: Generates random color gradients ("portals") that rotate around the circle. Optional "malfunction" mode adds flickering for dramatic effect.
- **Modes**:
  - Classic: Pre-generated segmented gradients with random colors.
  - Virtual Gradients: Real-time sine-wave based dual rotating gradients for smoother animation.
- **Controls**:
  - Buttons: Toggle effect, trigger malfunction, fade out.
  - Web API: HTTP endpoints for all commands, plus config setting (speed 1-10, brightness 0-255, hue range 0-255, mode 0/1).
- **Web Interface**: Responsive single-page app served from LittleFS, with sliders, color preview, and status fetch.

## User Experience Goals

- **Intuitive**: Simple buttons for local use; clean, mobile-friendly web UI for remote.
- **Immediate Feedback**: Visual LED responses to commands, status messages in UI, blinking status LED for WiFi.
- **Reliable**: Non-blocking operation for smooth 100 FPS updates; debounced inputs prevent glitches.
- **Customizable**: Real-time parameter tweaks with visual hue gradient preview.
- **Accessible**: Works offline (buttons only) or with WiFi; easy setup with credential template.

This product targets LED art enthusiasts, makers, and installers seeking programmable ambient lighting without deep coding knowledge.
