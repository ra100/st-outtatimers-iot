#!/usr/bin/env python3
"""
SKiDL Script to Generate ATTINY10 LED Blinker Circuit
Outtatimers Implant Project

This script uses SKiDL to create a proper electronic circuit description
and generate a KiCad-compatible netlist and schematic.

Install SKiDL: pip install skidl
"""

from skidl import *
import os
import uuid

def create_attiny10_circuit():
    """Create the ATTINY10 LED blinker circuit using SKiDL"""

    # Set the library search path to include KiCad libraries
    lib_search_paths[KICAD].append("/usr/share/kicad/symbols")

    # Create power nets
    vcc, gnd = Net("VCC"), Net("GND")
    vcc.drive = POWER
    gnd.drive = POWER

    # Create signal nets for LED connections
    led1_net = Net("LED1")
    led2_net = Net("LED2")
    led3_net = Net("LED3")

    # Create components
    # ATTINY10 microcontroller - use available symbol
    u1 = Part("MCU_Microchip_ATtiny", "ATtiny10-TS", footprint="Package_TO_SOT_SMD:SOT-23-6")

    # Battery - use generic battery symbol
    b1 = Part("Device", "Battery", footprint="Battery:Battery_Cell_CR2032")

    # Decoupling capacitor
    c1 = Part("Device", "C_Small", value="0.1µF", footprint="Capacitor_SMD:CP_Elec_6.3x5.7")

    # LEDs
    d1 = Part("Device", "LED", value="Blue LED", footprint="LED_SMD:LED_0603_1608Metric")
    d2 = Part("Device", "LED", value="White LED", footprint="LED_SMD:LED_0603_1608Metric")
    d3 = Part("Device", "LED", value="White LED", footprint="LED_SMD:LED_0603_1608Metric")

    # Resistors
    r1 = Part("Device", "R_Small", value="330Ω", footprint="Resistor_SMD:R_0603_1608Metric")
    r2 = Part("Device", "R_Small", value="470Ω", footprint="Resistor_SMD:R_0603_1608Metric")
    r3 = Part("Device", "R_Small", value="1kΩ", footprint="Resistor_SMD:R_0603_1608Metric")

    # Programming pins
    p1 = Part("Connector", "Conn_01x01_Pin", value="VCC", footprint="Connector_PinHeader_2.54mm:PinHeader_1x01_P2.54mm_Vertical")
    p2 = Part("Connector", "Conn_01x01_Pin", value="GND", footprint="Connector_PinHeader_2.54mm:PinHeader_1x01_P2.54mm_Vertical")
    p3 = Part("Connector", "Conn_01x01_Pin", value="RESET", footprint="Connector_PinHeader_2.54mm:PinHeader_1x01_P2.54mm_Vertical")
    p4 = Part("Connector", "Conn_01x01_Pin", value="PB2", footprint="Connector_PinHeader_2.54mm:PinHeader_1x01_P2.54mm_Vertical")
    p5 = Part("Connector", "Conn_01x01_Pin", value="PB1", footprint="Connector_PinHeader_2.54mm:PinHeader_1x01_P2.54mm_Vertical")

    # Connect power distribution
    # Battery connections
    b1["+"] += vcc
    b1["-"] += gnd

    # Capacitor connections (decoupling)
    c1[1] += vcc
    c1[2] += gnd

    # Programming pin connections
    p1[1] += vcc
    p2[1] += gnd
    # Note: ATtiny10-TS doesn't have dedicated CLK/DATA pins
    # We'll use the available GPIO pins for programming if needed
    p3[1] += u1["~{RESET}/PB3"]  # RESET pin
    p4[1] += u1["PB2"]  # Can be used for programming
    p5[1] += u1["PB1"]  # Can be used for programming

    # ATTINY10 power connections
    u1["VCC"] += vcc
    u1["GND"] += gnd

    # LED circuits
    # U1.PB0 → R1 → D1.A → D1.K → GND
    u1["PB0"] += led1_net
    led1_net += r1[1]
    r1[2] += d1["A"]
    d1["K"] += gnd

    # U1.PB1 → R2 → D2.A → D2.K → GND
    u1["PB1"] += led2_net
    led2_net += r2[1]
    r2[2] += d2["A"]
    d2["K"] += gnd

    # U1.PB2 → R3 → D3.A → D3.K → GND
    u1["PB2"] += led3_net
    led3_net += r3[1]
    r3[2] += d3["A"]
    d3["K"] += gnd

    print("✅ Circuit created successfully!")
    print(f"📦 Components: {len(default_circuit.parts)}")
    print(f"🔌 Nets: {len(default_circuit.nets)}")

    return default_circuit

def create_fallback_schematic(circuit):
    """Create a basic KiCad schematic template as fallback"""
    print("📝 Creating fallback KiCad schematic template...")

    # Create a minimal KiCad schematic file
    sch_content = f"""(kicad_sch
    (version 20250114)
    (generator "SKiDL")
    (generator_version "2.0.1")
    (uuid "{str(uuid.uuid4())}")
    (paper "A4")
    (lib_symbols)
    (sheet_instances
        (path "/"
            (page "1")
        )
    )
    (symbol
        (lib_id "MCU_Microchip_ATtiny:ATtiny10-TS")
        (at 0 0 0)
        (unit 1)
        (exclude_from_sim no)
        (in_bom yes)
        (on_board yes)
        (dnp no)
        (fields_autoplaced yes)
        (uuid "{str(uuid.uuid4())}")
        (property "Reference" "U1"
            (at 20 10 0)
            (effects
                (font (size 1.27 1.27))
                (justify left)
            )
        )
        (property "Value" "ATtiny10-TS"
            (at 20 25 0)
            (effects
                (font (size 1.27 1.27))
                (justify left)
            )
        )
        (property "Footprint" "Package_TO_SOT_SMD:SOT-23-6"
            (at 0 0 0)
            (effects
                (font (size 1.27 1.27) (italic yes))
                (hide yes)
            )
        )
    )
    (global_label
        (at -150 150)
        (text "VCC")
        (font (size 1.5 1.5) (bold yes))
        (uuid "{str(uuid.uuid4())}")
    )
    (global_label
        (at -150 100)
        (text "GND")
        (font (size 1.5 1.5) (bold yes))
        (uuid "{str(uuid.uuid4())}")
    )
    (embedded_fonts no)
)"""

    # Save the fallback schematic
    fallback_file = "attiny10_implant_fallback.kicad_sch"
    with open(fallback_file, 'w') as f:
        f.write(sch_content)

    print(f"✅ Fallback schematic template saved: {fallback_file}")
    print(f"💡 This is a minimal template - import the netlist for full schematic")

    return fallback_file

def main():
    """Main function to generate the circuit"""
    print("🚀 Generating ATTINY10 LED Blinker Circuit using SKiDL...")
    print("=" * 60)

    try:
        # Create the circuit
        circuit = create_attiny10_circuit()

        # Generate KiCad netlist
        print("\n📄 Generating KiCad netlist...")
        netlist_file = "attiny10_implant_skidl.net"
        generate_netlist(file_=netlist_file, tool=KICAD)
        print(f"✅ Netlist saved: {netlist_file}")

        # Generate KiCad schematic (if supported)
        print("\n📋 Generating KiCad schematic...")
        try:
            # SKiDL can generate schematics in some versions
            sch_file = "attiny10_implant_skidl.kicad_sch"

            # Try to generate schematic in KiCad format
            # Note: SKiDL schematic generation may have limitations
            generate_schematic(file_=sch_file, tool=KICAD)
            print(f"✅ Schematic saved: {sch_file}")

            # Verify the file was created and has content
            if os.path.exists(sch_file):
                file_size = os.path.getsize(sch_file)
                print(f"📏 Schematic file size: {file_size} bytes")

                if file_size > 0:
                    print(f"✅ Schematic file created successfully!")
                else:
                    print(f"⚠ Schematic file is empty - creating fallback")
                    create_fallback_schematic(circuit)
            else:
                print(f"❌ Schematic file was not created - creating fallback")
                create_fallback_schematic(circuit)

        except Exception as e:
            print(f"⚠ Schematic generation failed: {e}")
            print(f"💡 This is common with SKiDL - the netlist is the primary output")
            print(f"💡 Use the netlist ({netlist_file}) to create schematic in KiCad")

            # Create a fallback schematic template
            try:
                print(f"\n📝 Creating fallback schematic template...")
                create_fallback_schematic(circuit)
            except Exception as fallback_error:
                print(f"⚠ Fallback schematic creation also failed: {fallback_error}")

        # Generate XML netlist (alternative format)
        print("\n📄 Generating XML netlist...")
        xml_file = "attiny10_implant_skidl.xml"
        generate_xml(file_=xml_file)
        print(f"✅ XML netlist saved: {xml_file}")

        # Print circuit summary
        print(f"\n📊 Circuit Summary:")
        print(f"  🔌 Total nets: {len(circuit.nets)}")
        print(f"  📦 Total parts: {len(circuit.parts)}")
        print(f"  ⚡ Power nets: VCC, GND")
        print(f"  💡 LED circuits: 3 (Blue, White, White)")
        print(f"  🔧 Programming pins: 5 (VCC, GND, RESET, PB2, PB1)")

        print(f"\n📋 Next steps:")
        print(f"1. Open KiCad and create new project")
        print(f"2. Import the netlist: {netlist_file}")
        print(f"3. KiCad will create schematic with all connections")
        print(f"4. Run ERC to verify electrical rules")
        print(f"5. Generate PCB layout")

    except Exception as e:
        print(f"❌ Error creating circuit: {e}")
        print(f"�� Make sure SKiDL is installed: pip install skidl")
        return False

    return True

if __name__ == "__main__":
    main()
