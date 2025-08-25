# ATTINY10 LED Blinker Circuit - Connection Guide

## 🎯 Overview

This document describes the complete circuit connections for the Outtatimers Implant project - a minimal coin-cell-powered ATTINY10 LED blinker circuit.

## 📦 Components

- **ATTINY10-TS** microcontroller (SOT-23-6)
- **CR2032** coin cell battery
- **0.1µF** decoupling capacitor
- **3 LEDs**: 1 blue (main blink), 2 white (dim operation)
- **3 resistors**: 330Ω, 470Ω, 1kΩ for LED current limiting
- **5-pin programming header** for USBasp TPI interface

## 🔌 Circuit Connections

### Power Distribution

- **B1(+) → VCC net** (battery positive to VCC)
- **B1(-) → GND net** (battery negative to GND)
- **C1(1) → VCC net** (capacitor to VCC)
- **C1(2) → GND net** (capacitor to GND)
- **P1 → VCC net** (VCC programming pin)
- **P2 → GND net** (GND programming pin)

### ATTINY10 Power Connections

- **U1.VCC → VCC net**
- **U1.GND → GND net**

### LED Circuits

- **U1.PB0 → R1 → D1.A → D1.K → GND** (Blue LED circuit)
- **U1.PB1 → R2 → D2.A → D2.K → GND** (White LED circuit 1)
- **U1.PB2 → R3 → D3.A → D3.K → GND** (White LED circuit 2)

### Programming Interface

- **P3 → U1.RESET** (Reset pin)
- **P4 → U1.PB2** (Can be used for programming)
- **P5 → U1.PB1** (Can be used for programming)

## 📊 Circuit Details

### Power Management

- Ultra-low power design for coin cell operation
- Sleep current: <1µA
- Active current: ~1mA + LED currents

### LED Configuration

- **Blue LED (D1)**: 330Ω → ~9mA at 3V
- **White LED 1 (D2)**: 470Ω → ~6.4mA at 3V
- **White LED 2 (D3)**: 1kΩ → ~3mA at 3V

## 🎨 Design Notes

- **No external timing components needed** - ATTINY10 uses internal WDT
- **SMD footprints only** - Designed for minimal size
- **3V operation** - Optimized for CR2032 coin cell
- **Ultra-low power** - Designed for maximum battery life
- **Use 0603/0805 footprints** for resistors and capacitor to save space
- **Place C1 close to ATTINY10** (decoupling rule)
- **All LED cathodes go to GND**, not MCU

---

_This circuit is designed for the Outtatimers Implant project - creating the smallest possible coin-cell-powered LED blinker circuit._
