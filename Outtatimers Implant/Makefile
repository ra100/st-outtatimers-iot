# Makefile for ATTINY10 LED Always On Project
# Outtatimers Implant - Optimized for ATtiny10 Datasheet Specifications

# AVR-GCC settings
CC = avr-gcc
OBJCOPY = avr-objcopy
SIZE = avr-size

# Target microcontroller (ATTINY10) - Based on official datasheet
MCU = attiny10
F_CPU = 1000000L

# Optimized compiler flags for ATTINY10 (1KB Flash, 32 bytes SRAM)
CFLAGS = -mmcu=$(MCU) -DF_CPU=$(F_CPU)UL -Os -Wall -std=c99 -fno-builtin -fno-tree-loop-optimize -fno-split-wide-types -fno-move-loop-invariants -fno-unwind-tables
LDFLAGS = -mmcu=$(MCU) -Wl,--gc-sections -Wl,--relax

# Source files
SOURCES = src/main.cpp
TARGET = attiny10_implant

# Default target
all: $(TARGET).hex

# Compile the program
$(TARGET).elf: $(SOURCES)
	@echo "🔨 Compiling ATTINY10 LED always-on program..."
	@echo "Optimized for ATtiny10 datasheet specifications:"
	@echo "  - 1MHz internal oscillator"
	@echo "  - 1KB Flash, 32 bytes SRAM"
	@echo "  - TPI programming interface"
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^
	@echo "✅ Compilation successful!"

# Create hex file for upload
$(TARGET).hex: $(TARGET).elf
	@echo "📦 Creating hex file..."
	$(OBJCOPY) -O ihex -R .eeprom $< $@
	@echo "✅ Hex file created!"

# Show size information
size: $(TARGET).elf
	@echo "📏 Program size information:"
	$(SIZE) --format=avr --mcu=$(MCU) $<
	@echo ""
	@echo "📊 ATtiny10 Memory Limits (per datasheet):"
	@echo "  Flash: 1KB (1024 bytes)"
	@echo "  SRAM: 32 bytes"
	@echo "  EEPROM: Not available"

# Clean build files
clean:
	@echo "🧹 Cleaning build files..."
	rm -f *.elf *.hex *.o readback.hex
	@echo "✅ Clean complete!"

# Upload to ATTINY10 using USBasp with TPI
upload: $(TARGET).hex
	@echo "🚀 Uploading to ATTINY10..."
	@echo "Using TPI (Tiny Programming Interface) as per datasheet"
	@echo "Make sure USBasp is connected and ATTINY10 is in programming mode"
	avrdude -c usbasp -p $(MCU) -B 62.5kHz -U flash:w:$<:i
	@echo "✅ Upload completed!"

# Verify uploaded code matches our hex file
verify: $(TARGET).hex
	@echo "🔍 Verifying uploaded code..."
	@echo "Reading flash memory from ATTINY10..."
	avrdude -c usbasp -p $(MCU) -B 62.5kHz -U flash:r:readback.hex:i
	@echo "Comparing with original hex file..."
	@if diff $(TARGET).hex readback.hex > /dev/null 2>&1; then \
		echo "✅ Verification successful! Uploaded code matches exactly."; \
	else \
		echo "❌ Verification failed! Uploaded code differs from original."; \
		echo "Differences found:"; \
		diff $(TARGET).hex readback.hex || true; \
		exit 1; \
	fi

# Upload and verify in one command
upload-verify: upload verify

# Show help
help:
	@echo "🎯 ATTINY10 LED Always On Project - Makefile"
	@echo "==========================================="
	@echo ""
	@echo "Available targets:"
	@echo "  all             - Compile program (default)"
	@echo "  size            - Show program size information"
	@echo "  upload          - Upload to ATTINY10 via USBasp (TPI)"
	@echo "  verify          - Verify uploaded code matches hex file"
	@echo "  upload-verify   - Upload and verify in one command"
	@echo "  clean           - Remove build files"
	@echo "  help            - Show this help message"
	@echo ""
	@echo "✅ ATtiny10 Specifications (per datasheet):"
	@echo "  - 1MHz internal oscillator (F_CPU=1000000L)"
	@echo "  - 1KB Flash memory"
	@echo "  - 32 bytes SRAM"
	@echo "  - 3 GPIO pins (PB0, PB1, PB2)"
	@echo "  - TPI programming interface"
	@echo "  - SOT-23-6 package"
	@echo ""
	@echo "Prerequisites:"
	@echo "  - AVR-GCC toolchain installed"
	@echo "  - USBasp programmer with TPI firmware"
	@echo "  - ATTINY10 connected to programming pins"
	@echo ""
	@echo "Examples:"
	@echo "  make                    # Just compile"
	@echo "  make size              # Show program size"
	@echo "  make upload            # Compile and upload"
	@echo "  make verify            # Verify uploaded code"
	@echo "  make upload-verify     # Upload and verify"
	@echo "  make clean             # Clean build files"

.PHONY: all size upload verify upload-verify clean help
