# ATTINY10 Schematic Generator

## 🎯 Overview

This Python script generates a complete schematic for the Outtatimers Implant project - a minimal coin-cell-powered ATTINY10 LED blinker circuit.

## 📁 Files

- `generate_schematic.py` - Main schematic generation script
- `schematic_requirements.txt` - Dependencies and requirements
- `attiny10_schematic.txt` - Generated schematic output (created when script runs)

## 🚀 Usage

### Standalone Mode (Recommended for initial setup)

```bash
cd "Outtatimers Implant"
python3 generate_schematic.py
```

This will generate a text-based schematic that you can review before importing into KiCad.

### KiCad Integration Mode

1. Open KiCad
2. Go to **Tools** → **Python Console** (or **Scripting Console** in older versions)
3. Navigate to the script directory
4. Run: `exec(open('generate_schematic.py').read())`

**Alternative**: If you can't find the Python Console, run the script externally:

```bash
cd "Outtatimers Implant"
python3 generate_schematic.py
```

## 🔧 What It Generates

The script creates a complete circuit with:

- **ATTINY10-TSHR** microcontroller (SOT-23-6)
- **CR2032** coin cell battery
- **0.1µF** decoupling capacitor
- **3 LEDs**: 1 blue (main blink), 2 white (dim operation)
- **3 resistors**: 330Ω, 470Ω, 1kΩ for LED current limiting
- **5-pin programming header** for USBasp TPI interface

## 📊 Circuit Details

### Power Management

- Ultra-low power design for coin cell operation
- Sleep current: <1µA
- Active current: ~1mA + LED currents

### LED Configuration

- **Blue LED (D1)**: 330Ω → ~9mA at 3V
- **White LED 1 (D2)**: 470Ω → ~6.4mA at 3V
- **White LED 2 (D3)**: 1kΩ → ~3mA at 3V

### Programming Interface

- **VCC**: Power supply
- **GND**: Ground
- **TPICLK**: TPI clock signal
- **TPIDATA**: TPI data signal
- **RESET**: Reset pin

## 🎨 Customization

You can modify the script to:

- Change component values
- Adjust component placement
- Add additional components
- Modify LED configurations
- Change resistor values

## 🔍 Output Files

The script generates:

1. **Text schematic** (`attiny10_schematic.txt`) - Human-readable circuit description
2. **Console output** - Component list and connection details
3. **KiCad-ready data** - When run within KiCad environment

## 📋 Next Steps

After running the script:

1. **Review the generated schematic** for accuracy
2. **Import into KiCad** for PCB design
3. **Adjust component placement** for optimal layout
4. **Generate PCB** and manufacturing files
5. **Order components** based on the BOM

## 🚨 Important Notes

- **No external timing components needed** - ATTINY10 uses internal WDT
- **SMD footprints only** - Designed for minimal size
- **3V operation** - Optimized for CR2032 coin cell
- **Ultra-low power** - Designed for maximum battery life

## 🆘 Troubleshooting

### Script won't run

- Ensure Python 3.4+ is installed
- Check file permissions: `chmod +x generate_schematic.py`

### KiCad integration issues

- **Python Console not found**: Look for `Tools` → `Python Console` (KiCad 6/7) or `Tools` → `Scripting Console` (KiCad 5)
- **Alternative**: Run script externally with `python3 generate_schematic.py`
- Verify KiCad Python environment is properly configured
- Check that pcbnew and kicad modules are available

### Component footprint errors

- Verify KiCad library paths are correct
- Check that required footprints are installed

---

_This schematic generator is part of the Outtatimers Implant project - creating the smallest possible coin-cell-powered LED blinker circuit._
