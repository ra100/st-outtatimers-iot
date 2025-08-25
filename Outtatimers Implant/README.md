# Outtatimers Implant

## 🎯 Project Overview

**Goal**: Create the smallest possible coin-cell-powered circuit with blinking LED(s).

**Microcontroller**: ATTINY10 (SOT-23-6)

- Ultra-low-power design suitable for 1 × CR2032 (3V) battery
- Can drive one blue LED to blink and two white LEDs dimly or intermittently

## 🔧 Programming Setup

**Programmer**: USBasp with TPI-capable firmware
**Connection**: 5 pads required

- VCC
- GND
- TPICLK
- TPIDATA
- RESET

## 💡 Blink Pattern Design

- Use ATTINY10 sleep and wake from WDT to pulse LEDs briefly
- Short pulses (5–10ms) at low duty cycle to maximize battery life
- Ultra-low power consumption for extended coin cell operation

## 📋 Components / BOM (from Farnell CZ)

| Purpose                  | Component                 | Notes                              |
| ------------------------ | ------------------------- | ---------------------------------- |
| **MCU**                  | ATTINY10-TSHR             | SOT-23-6 package                   |
| **LED(s)**               | 3mm LEDs                  | Blue for blink, two white for dim  |
| **LED resistors**        | 0402/0603 SMD             | ~330Ω for blue, 470Ω–1kΩ for white |
| **Decoupling capacitor** | 0.1µF, 0603 MLCC          | Across VCC–GND near ATTINY10       |
| **Battery**              | CR2032 coin cell + holder | 3V, 220mAh                         |
| **Programmer**           | USBasp                    | With TPI firmware for ATTINY10     |

## ✅ Completed

- [x] Project concept and requirements defined
- [x] Component selection and BOM created
- [x] Programming approach determined (USBasp + TPI)

## 🚧 Still Missing / Needed

### Full Schematic Design

- [ ] Battery connection circuit
- [ ] ATTINY10 VCC/GND connections
- [ ] Decoupling capacitor placement
- [ ] LED(s) + series resistors circuit
- [ ] Complete circuit diagram

### Hardware Implementation

- [ ] PCB layout or breadboard plan (optional, but useful for tiny assembly)
- [ ] Programming header / pads (5 small pads for USBasp TPI)
- [ ] Component placement optimization for minimal size

### Optional Extras

- [ ] Small switch to test LEDs without removing battery
- [ ] Test points for measuring voltage/current if debugging
- [ ] Tiny protection diode (optional, to prevent reverse battery insertion)

## 🎯 Next Steps

1. **Design complete schematic** showing all components and connections
2. **Create PCB layout** optimized for minimal size
3. **Assemble prototype** with selected components
4. **Test programming** via USBasp TPI interface
5. **Verify power consumption** and battery life
6. **Optimize blink patterns** for maximum efficiency

## 🔍 Technical Notes

- **Power Management**: Focus on ultra-low power consumption
- **Size Optimization**: Minimize PCB footprint for coin cell applications
- **Programming**: TPI interface requires specific USBasp firmware
- **Timing**: Internal WDT oscillator - no external timing components needed
- **Simplified Design**: Uses ATTINY10's built-in timing capabilities for maximum compactness

---

_This project aims to create the most compact, power-efficient LED blinker circuit possible while maintaining programmability and reliability._
