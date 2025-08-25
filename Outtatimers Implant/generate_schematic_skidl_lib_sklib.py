from collections import defaultdict
from skidl import Pin, Part, Alias, SchLib, SKIDL, TEMPLATE

from skidl.pin import pin_types

SKIDL_lib_version = '0.0.1'

generate_schematic_skidl_lib = SchLib(tool=SKIDL).add_parts(*[
        Part(**{ 'name':'ATtiny10-TS', 'dest':TEMPLATE, 'tool':SKIDL, 'aliases':Alias({'ATtiny10-TS'}), 'ref_prefix':'U', 'fplist':['Package_TO_SOT_SMD:SOT-23-6', 'Package_TO_SOT_SMD:SOT-23-6'], 'footprint':'Package_TO_SOT_SMD:SOT-23-6', 'keywords':'AVR 8bit Microcontroller tinyAVR', 'description':'', 'datasheet':'http://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-8127-AVR-8-bit-Microcontroller-ATtiny4-ATtiny5-ATtiny9-ATtiny10_Datasheet.pdf', 'pins':[
            Pin(num='5',name='VCC',func=pin_types.PWRIN,unit=1),
            Pin(num='2',name='GND',func=pin_types.PWRIN,unit=1),
            Pin(num='1',name='PB0',func=pin_types.BIDIR,unit=1),
            Pin(num='3',name='PB1',func=pin_types.BIDIR,unit=1),
            Pin(num='4',name='PB2',func=pin_types.BIDIR,unit=1),
            Pin(num='6',name='~{RESET}/PB3',func=pin_types.BIDIR,unit=1)], 'unit_defs':[] }),
        Part(**{ 'name':'Battery', 'dest':TEMPLATE, 'tool':SKIDL, 'aliases':Alias({'Battery'}), 'ref_prefix':'BT', 'fplist':[''], 'footprint':'Battery:Battery_Cell_CR2032', 'keywords':'batt voltage-source cell', 'description':'', 'datasheet':'~', 'pins':[
            Pin(num='1',name='+',func=pin_types.PASSIVE,unit=1),
            Pin(num='2',name='-',func=pin_types.PASSIVE,unit=1)], 'unit_defs':[] }),
        Part(**{ 'name':'C_Small', 'dest':TEMPLATE, 'tool':SKIDL, 'aliases':Alias({'C_Small'}), 'ref_prefix':'C', 'fplist':[''], 'footprint':'Capacitor_SMD:CP_Elec_6.3x5.7', 'keywords':'capacitor cap', 'description':'', 'datasheet':'~', 'pins':[
            Pin(num='1',name='~',func=pin_types.PASSIVE,unit=1),
            Pin(num='2',name='~',func=pin_types.PASSIVE,unit=1)], 'unit_defs':[] }),
        Part(**{ 'name':'LED', 'dest':TEMPLATE, 'tool':SKIDL, 'aliases':Alias({'LED'}), 'ref_prefix':'D', 'fplist':[''], 'footprint':'LED_SMD:LED_0603_1608Metric', 'keywords':'LED diode', 'description':'', 'datasheet':'~', 'pins':[
            Pin(num='1',name='K',func=pin_types.PASSIVE,unit=1),
            Pin(num='2',name='A',func=pin_types.PASSIVE,unit=1)], 'unit_defs':[] }),
        Part(**{ 'name':'R_Small', 'dest':TEMPLATE, 'tool':SKIDL, 'aliases':Alias({'R_Small'}), 'ref_prefix':'R', 'fplist':[''], 'footprint':'Resistor_SMD:R_0603_1608Metric', 'keywords':'R resistor', 'description':'', 'datasheet':'~', 'pins':[
            Pin(num='1',name='~',func=pin_types.PASSIVE,unit=1),
            Pin(num='2',name='~',func=pin_types.PASSIVE,unit=1)], 'unit_defs':[] }),
        Part(**{ 'name':'Conn_01x01_Pin', 'dest':TEMPLATE, 'tool':SKIDL, 'aliases':Alias({'Conn_01x01_Pin'}), 'ref_prefix':'J', 'fplist':[''], 'footprint':'Connector_PinHeader_2.54mm:PinHeader_1x01_P2.54mm_Vertical', 'keywords':'connector', 'description':'', 'datasheet':'~', 'pins':[
            Pin(num='1',name='Pin_1',func=pin_types.PASSIVE,unit=1)], 'unit_defs':[] })])