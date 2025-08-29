#!/bin/bash

# Build script for ATTINY10 LED Blinking Project
# Outtatimers Implant - Makefile-based build system

echo "🎯 ATTINY10 LED Blinking Project - Build Script"
echo "================================================"
echo ""

# Check if AVR-GCC is installed
if ! command -v avr-gcc &> /dev/null; then
    echo "❌ AVR-GCC not found!"
    echo ""
    echo "📦 Installation required:"
    echo ""
    echo "Ubuntu/Debian:"
    echo "  sudo apt update"
    echo "  sudo apt install gcc-avr avr-libc avrdude"
    echo ""
    echo "CentOS/RHEL:"
    echo "  sudo yum install avr-gcc avr-libc avrdude"
    echo ""
    echo "macOS:"
    echo "  brew install avr-gcc avr-libc avrdude"
    echo ""
    echo "After installation, run this script again."
    exit 1
fi

# Check if avrdude is installed
if ! command -v avrdude &> /dev/null; then
    echo "❌ AVRDUDE not found!"
    echo "Install with: sudo apt install avrdude"
    exit 1
fi

echo "✅ AVR toolchain found:"
echo "   AVR-GCC: $(avr-gcc --version | head -n1)"
echo "   AVRDUDE: $(avrdude -v 2>&1 | head -n1)"
echo ""

# Check if source file exists
if [ ! -f "src/main.cpp" ]; then
    echo "❌ Source file not found: src/main.cpp"
    echo "Make sure you're in the correct directory."
    exit 1
fi

echo "🔨 Building ATTINY10 program using Makefile..."
echo ""

# Try to build
if make; then
    echo ""
    echo "✅ Build successful!"
    echo ""
    echo "📁 Generated files:"
    ls -la *.elf *.hex 2>/dev/null || echo "   No output files found"
    echo ""
    echo "🚀 Next steps:"
    echo "   1. Connect USBasp programmer to ATTINY10"
    echo "   2. Run: make upload"
    echo "   3. Watch LED blink with random intervals!"
    echo ""
    echo "💡 For help: make help"
    echo "🧹 Clean build: make clean"
else
    echo ""
    echo "❌ Build failed!"
    echo "Check the error messages above."
    exit 1
fi

