# 🔨 Building and Uploading ATTINY10 Project

## 📋 Overview

This project uses a **Makefile-based AVR-GCC toolchain** for building and uploading the ATTINY10 LED blinking program. This approach is specifically optimized for the ATtiny10's constraints and provides reliable compilation.

## 🛠️ Prerequisites

### **Required Tools**

- **AVR-GCC**: C compiler for AVR microcontrollers
- **AVR-LIBC**: C library for AVR microcontrollers
- **AVRDUDE**: Program uploader for AVR chips

### **Hardware**

- **USBasp programmer** with TPI-capable firmware
- **ATTINY10** microcontroller (SOT-23-6 package)
- **Programming connections**: VCC, GND, TPICLK, TPIDATA, RESET

## 🚀 Quick Start

### **1. Install AVR Toolchain**

**Ubuntu/Debian:**

```bash
sudo apt update
sudo apt install gcc-avr avr-libc avrdude
```

**CentOS/RHEL:**

```bash
sudo yum install avr-gcc avr-libc avrdude
```

**macOS:**

```bash
brew install avr-gcc avr-libc avrdude
```

### **2. Build the Project**

```bash
cd "Outtatimers Implant"

# Use the build script (recommended)
./build.sh

# Or use make directly
make
```

### **3. Upload to ATTINY10**

```bash
# Upload via USBasp
make upload
```

## 📁 Build Output

After successful build, you'll get:

- **`attiny10_implant.elf`**: Executable file
- **`attiny10_implant.hex`**: Hex file for upload

## 🔧 Build Commands

### **Makefile Targets**

```bash
make              # Compile program (default)
make size         # Show program size information
make upload       # Upload to ATTINY10 via USBasp
make clean        # Remove build files
make help         # Show help message
```

### **Build Script**

```bash
./build.sh        # Check tools and build automatically
```

## 🚨 Troubleshooting

### **AVR-GCC Not Found**

```bash
# Install AVR toolchain
sudo apt install gcc-avr avr-libc avrdude
```

### **Compilation Errors**

```bash
# Check source file
ls -la src/main.cpp

# Clean and rebuild
make clean
make
```

### **Upload Issues**

```bash
# Check USBasp connection
lsusb | grep usbasp

# Check avrdude version
avrdude -v

# Test connection
avrdude -c usbasp -p attiny10
```

## 📊 Program Information

### **Target Specifications**

- **Microcontroller**: ATTINY10 (SOT-23-6)
- **Clock Speed**: 8 MHz internal
- **Flash Memory**: 1KB
- **SRAM**: 32 bytes
- **GPIO Pins**: 3 (PB0, PB1, PB2)

### **Expected Behavior**

- **LED on PB0** blinks with random intervals
- **Intervals**: 0.1s to 1.0s (equal ON/OFF times)
- **Power**: Ultra-low power using sleep modes
- **Timing**: Internal watchdog timer

## 🔄 Development Workflow

### **1. Code Changes**

```bash
# Edit source code
nano src/main.cpp

# Rebuild
make clean && make
```

### **2. Test Changes**

```bash
# Upload to test
make upload

# Monitor behavior
# LED should blink with new timing/behavior
```

### **3. Iterate**

```bash
# Make more changes
# Rebuild and upload
# Test again
```

## 📈 Next Steps

1. **Install AVR toolchain** if not already installed
2. **Run build script** to verify setup
3. **Build the program** to generate hex file
4. **Connect hardware** (USBasp + ATTINY10)
5. **Upload and test** LED blinking behavior

## ✅ **Why This Approach Works**

- **Optimized for ATtiny10**: Direct AVR-GCC compilation without Arduino framework overhead
- **Memory efficient**: Only 164 bytes (16% of Flash), 2 bytes (6% of RAM)
- **Reliable**: No dependency on PlatformIO's Arduino framework compatibility
- **Standard**: This is the traditional and recommended approach for very small AVR microcontrollers

---

_This Makefile-based build system provides reliable ATtiny10 development without the complexity of larger frameworks!_
