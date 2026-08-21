---
type: cpu_board
product: NORVI X2
mcu: ESP32-S3
status: draft
last_updated: 2026-08-21
---

# NORVI X2 — CPU Board Reference

## Overview
NORVI X2 is the CPU module for NORVI's modular controller lineup, running on an ESP32-S3-WROOM-1U-N16R2. It mounts on a DIN rail, uses push-in terminals on the bottom, and comes in at 81 x 104 x 37.5 mm. Onboard you get a 2.0" TFT touchscreen, Ethernet, RS-485, RTC (DS3231), and a LTE Modem plus a 50-pin expansion port for adding I/O modules as needed.

## Core Specs
| Spec | Value |
|---|---|
| MCU | ESP32-S3 (dual-core, 240 MHz) |
| Flash | 16MB |
| PSRAM | 2MB QSPI |
| Wi-Fi / BT | Yes |
| Power input | 24VDC |
| Min voltage input | 9VDC |
| Max voltage input | 28VDC |
| Average power consumption | 90mA |
| Operating temp | +85°C |
| Mounting | DIN rail |
| Enclosure rating | IP20 |

## Onboard Peripherals (I2C)
| Peripheral | Interface | Notes |
|---|---|---|
| RTC | DS3231 | I2C addr 0x68 |
| TOUCH PANEL | CST816S | I2C addr 0x15 |

### I2C IO Expander(s)
> Some GPIO Map entries above are physically routed through an I2C expander rather than
> native MCU pins. This section is the electrical/register detail for those — the GPIO
> Map table just flags *which* pins, this section explains *how* to access them.

| Property | Value |
|---|---|
| Chip | PCA9539 |
| I2C address | 0x75 |
| Address selectable? | No, fixed |
| Direction register | Input/Output Configurable |

**Silkscreen-to-expander pin mapping**
| Board Label (as printed / used in GPIO Map) | Expander Local Pin | Notes |
|---|---|---|
| CS4 | A5 | Accessible through Expansion Port |
| CS3 | A6 | Accessible through Expansion Port |
| CS2 | A7 | Accessible through Expansion Port |
| CS1 | B0 | Accessible through Expansion Port |
| IO19 | B1 | Accessible through Expansion Port |
| IO20 | B2 | Accessible through Expansion Port |
| IO33 | B3 | Accessible through Expansion Port |
| IO34 | B4 | Accessible through Expansion Port |
| IO38 | B5 | Accessible through Expansion Port |

| Property | Value |
|---|---|
| Chip | PCA9536 |
| I2C address | 0x41 |
| Address selectable? | No, fixed |
| Direction register | Input/Output Configurable |

**Silkscreen-to-expander pin mapping**
| Board Label (as printed / used in GPIO Map) | Expander Local Pin | Notes |
|---|---|---|
| PB1 | P0 | Push Button 1 Input |
| P1 | P1 | Run Indicator |
| P2 | P2 | Error Indicator |
| PB3 | P3 | Push Button 2 Input |


> If there's more than one expander chip on the board, duplicate this whole subsection
> per chip rather than merging tables — keeps each chip's address and pin numbering unambiguous.

## Onboard Communication Interfaces
> Non-I2C interfaces — kept separate since each has different relevant fields.
> Availability is per-interface, not blanket — see each subsection.

### Ethernet
**Availability:** Standard on all NORVI X CPU boards (X1/X2/X3)
| Property | Value |
|---|---|
| PHY chip | W5500 |
| Speed | 10/100 Mbps |
| MAC source | software-configurable |
| Connector | RJ45 |

### RS-485
**Availability:** Standard on all NORVI X CPU boards (X1/X2/X3)
| Property | Value |
|---|---|
| UART tied to | TX-IO15 RX-IO16 |
| Termination resistor | External |
| Direction control pin | Auto Direction |
| Terminal Isolated | No |
| Default baud | 115200 |

### LTE Modem
**Availability:** SKU-dependent — not present on base X1. Applies to X2 and X3 -LTE variants
| Property | Value |
|---|---|
| Module | SIM A7672G |
| UART pins | TX-IO17 RX-IO18 |
| SIM slot type | micro |
| Power sequencing notes | Require 200ms delay between toggle of GSM_RST pin LOW to HIGH. After tying HIGH it requires minimum 1000ms delay. |
| AT command reference | https://download.kamami.pl/p1189472-A76XX_Series_AT_Command_Manual_V1.09.pdf |

## GPIO Map
| Pin / Terminal | Function | Type | Access | Notes |
|---|---|---|---|---|
| 0 | NRST | DIGITAL IN/DIGITAL OUT | Native MCU GPIO | 3.3V Pulled Up |
| 1 | ETHERNET CS/IO1 | DIGITAL IN/DIGITAL OUT/ANALOG IN | Native MCU GPIO | Routed to the Ethernet Module/Accessible through Expansion Port |
| 2 | ETHERNET RESET/IO2 | DIGITAL IN/DIGITAL OUT/ANALOG IN | Native MCU GPIO | Routed to the Ethernet Module/Accessible through Expansion Port |
| 3 | NC | STRAPPING | Native MCU GPIO | Not Used|
| 4 | ETHERNET INTERRUPT/IO4 | DIGITAL IN/DIGITAL OUT/ANALOG IN | Native MCU GPIO | Routed to the Ethernet Module/Accessible through Expansion Port |
| 5 | IO5 | DIGITAL IN/DIGITAL OUT/ANALOG IN | Native MCU GPIO | Accessible through Expansion Port |
| 6 | IO6 | DIGITAL IN/DIGITAL OUT/ANALOG IN | Native MCU GPIO | Accessible through Expansion Port |
| 7 | IO7 | DIGITAL IN/DIGITAL OUT/ANALOG IN | Native MCU GPIO | Accessible through Expansion Port |
| 8 | SDA | COMMUNICATION | Native MCU GPIO | 3.3V Pulled Up/Accessible through Expansion Port |
| 9 | SCL | COMMUNICATION | Native MCU GPIO | 3.3V Pulled Up/Accessible through Expansion Port |
| 10 | IO10 | DIGITAL IN/DIGITAL OUT/ANALOG IN | Native MCU GPIO | Accessible through Expansion Port |
| 11 | MOSI | COMMUNICATION | Native MCU GPIO | Accessible through Expansion Port |
| 12 | SCLK | COMMUNICATION | Native MCU GPIO | Accessible through Expansion Port |
| 13 | MISO | COMMUNICATION | Native MCU GPIO | Accessible through Expansion Port |
| 14 | IO14 | DIGITAL IN/DIGITAL OUT/ANALOG IN | Native MCU GPIO | Accessible through Expansion Port |
| 15 | RS485 TX Line/IO15 | UART/DIGITAL IN/DIGITAL OUT/ANALOG IN | Native MCU GPIO | Routed to the RS485 Tranceiver/Accessible through Expansion Port |
| 16 | RS485 RX Line/IO16 | UART/DIGITAL IN/DIGITAL OUT/ANALOG IN | Native MCU GPIO | Routed to the RS485 Tranceiver/Accessible through Expansion Port |
| 17 | TX1 | UART/DIGITAL IN/DIGITAL OUT/ANALOG IN | Native MCU GPIO | Routed to the Modem RX/Accessible through Expansion Port |
| 18 | RX1 | UART/DIGITAL IN/DIGITAL OUT/ANALOG IN | Native MCU GPIO | Routed to the Modem TX/Accessible through Expansion Port |
| 19 | D_N | USB Communication | Native MCU GPIO | Dedicated USB Line/Accessible through Expansion Port |
| 20 | D_P | USB Communication | Native MCU GPIO | Dedicated USB Line/Accessible through Expansion Port |
| 21 | IO21 | DIGITAL IN/DIGITAL OUT | Native MCU GPIO | Accessible through Expansion Port |
| 35 | IO11 | DIGITAL IN/DIGITAL OUT | Native MCU GPIO | Not supported with N16R8 version of SOC/Accessible through Expansion Port |
| 36 | IO36 | DIGITAL IN/DIGITAL OUT | Native MCU GPIO | Accessible through Expansion Port |
| 37 | IO13 | DIGITAL IN/DIGITAL OUT | Native MCU GPIO | Not supported with N16R8 version of SOC/Accessible through Expansion Port |
| 38 | GPIO EXPANDER RESET | DIGITAL IN/DIGITAL OUT | Native MCU GPIO | Only for CPU |
| 39 | IO39 | DIGITAL IN/DIGITAL OUT | Native MCU GPIO | Routed to the Modem Reset/Accessible through Expansion Port |
| 40 | IO40 | DIGITAL IN/DIGITAL OUT | Native MCU GPIO | Accessible through Expansion Port |
| 41 | IO12 | DIGITAL IN/DIGITAL OUT | Native MCU GPIO | Accessible through Expansion Port |
| 42 | SD Card CS | DIGITAL IN/DIGITAL OUT | Native MCU GPIO | Only for CPU |
| 45 | Display CS | DIGITAL IN/DIGITAL OUT | Native MCU GPIO | Only for CPU |
| 46 | Display DC | DIGITAL IN/DIGITAL OUT | Native MCU GPIO | Only for CPU |
| 47 | Touch Panel Reset/Display Reset | DIGITAL IN/DIGITAL OUT | Native MCU GPIO | Only for CPU |
| 48 | Touch Panel Interrupt | DIGITAL IN/DIGITAL OUT | Native MCU GPIO | Only for CPU |

> Full pin-by-pin map lives here — this is the single source of truth other docs (examples, GPT knowledge files) point back to.
> For any pin marked "via IO Expander," the Access column is the flag — full electrical/register detail lives in the IO Expander subsection below, not repeated here.


## Expansion Bus GPIO Map
| Pin Index | Function | Type |
|---|---|---|
| 1 | MOSI | Native MCU GPIO |
| 2 | MISO | Native MCU GPIO |
| 3 | SCLK | Native MCU GPIO |
| 4 | GND | POWER GROUND |
| 5 | SCL | Native MCU GPIO |
| 6 | SDA | Native MCU GPIO |
| 8 | RX | Native MCU GPIO |
| 9 | TX | Native MCU GPIO |
| 10 | GND | POWER GROUND |
| 11 | D_P | Native MCU GPIO |
| 12 | D_N | Native MCU GPIO |
| 13 | GND | POWER GROUND |
| 14 | CS1 | via IO Expander |
| 15 | CS2 | via IO Expander |
| 16 | CS3 | via IO Expander |
| 17 | CS4 | via IO Expander |
| 18 | IO1 | Native MCU GPIO |
| 19 | IO2 | Native MCU GPIO |
| 20 | IO4 | Native MCU GPIO |
| 21 | IO5 | Native MCU GPIO |
| 22 | IO6 | Native MCU GPIO |
| 23 | IO7 | Native MCU GPIO |
| 24 | IO10 | Native MCU GPIO |
| 25 | IO11 | Native MCU GPIO |
| 26 | IO12 | Native MCU GPIO |
| 27 | IO13 | Native MCU GPIO |
| 28 | IO14 | Native MCU GPIO |
| 29 | IO15 | Native MCU GPIO |
| 30 | GND | POWER GROUND |
| 31 | GND | POWER GROUND |
| 32 | IO16 | Native MCU GPIO |
| 33 | IO19 | via IO Expander |
| 34 | IO20 | via IO Expander |
| 35 | IO21 | Native MCU GPIO |
| 36 | GND | POWER GROUND |
| 37 | GND | POWER GROUND |
| 38 | IO33 | via IO Expander |
| 39 | IO34 | via IO Expander |
| 40 | IO38 | via IO Expander |
| 41 | IO39 | Native MCU GPIO |
| 42 | IO40 | Native MCU GPIO |
| 43 | IO36 | Native MCU GPIO |
| 44 | NC | NO CONNECTION |
| 45 | NC | NO CONNECTION |
| 46 | GND | POWER GROUND |
| 47 | 5V | POWER |
| 48 | GND | POWER GROUND |
| 49 | 24V | POWER |
| 50 | 24V | POWER |



## Required Libraries (base board, before any expansion)
| Library | Version | Install via | Purpose |
|---|---|---|---|
| Wire.h | 2.0.0 | Comes with ESP32 core | I2C Communication |
| WiFi.h | 2.0.0 | Comes with ESP32 core| WiFi Connectivity |
| Ethernet.h | 2.0.2 | Comes with ESP32 core | Ethernet Communication |
| EthernetUdp.h |  | Comes with ESP32 core | Send and receive UDP packets through the Ethernet |
| PCA9539.h |  | Available in Github Repo | GPIO Expander |
| PCA9536D.h | 2.0.1 | Arduino Library Manager/Available in Github Repo | GPIO Expander |
| RTClib.h | 2.1.4 | Arduino Library Manager/Available in Github Repo | RTC |
| SPI.h | 2.0.0 | Arduino Library Manager | SPI Communication |
| FS.h | 2.0.0 | Arduino Library Manager | File System Functions |
| SD.h | 2.0.0 | Arduino Library Manager | SD Card |
| TFT_eSPI.h | 2.5.0 | Arduino Library Manager/Available in Github Repo | Display |
| CST816S.h | 1.3.0 | Arduino Library Manager/Available in Github Repo | Display Touch Panel |
| Free_Fonts.h |  | Attached to the test Firmware | Display Fonts |


## Board Package / Toolchain Setup
1. https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
2. Board:ESP32S3 Dev Module
3. USB CDC On Boot: Enabled
   Flash Size: 16MB(128Mb)
   PSRAM: QSPI PSRAM

## Related Files
- Expansion modules compatible with X2: `expansion-module_X-DI4.md`,`expansion-module_X-R4.md`,`expansion-module_X-Q4.md`,`expansion-module_X-AI4.md`,`expansion-module_X-AV4.md`,`expansion-module_X-AQ4.md`,`expansion-module_X-DI8.md`,`expansion-module_X-DI16.md`,`expansion-module_X-R8.md`,`expansion-module_X-Q16.md`,`expansion-module_X-TC4.md`, `expansion-module_X-Q8.md`,`expansion-module_X-T4.md`,`expansion-module_X-UART-IO.md`,`expansion-module_X-RTD4.md`,`expansion-module_X-BKO.md`,`expansion-module_X-CAN1.md`,`expansion-module_X-DA8.md` (one file per module)
- Master I2C address table: `i2c-address-master-table.md`
- Example programs: `NORVI X-CPU-ESPS3-X2 or X3_program.md` (one file per function)
