# NORVI X-T4 – Thermistor Input Expansion Module Example

## Overview

This example demonstrates how to use the Thermistor Input Expansion Module on the NORVI X platform.

The purpose of this example is to show how to:

- Integrate the X-T4 Module into the NORVI X CPU
- Read 4 NTC thermistor channels through the on-board ADS1115 ADC
- Convert the readings to temperature (°C) using the Beta equation
- Display the live temperatures on the TFT display and the Serial Monitor

This example can be used as a starting point for developers building multi-zone temperature monitoring applications with NORVI controllers.

## Product Used

Product: NORVI X-T4 Thermistor Input Expansion Module (4 × NTC, 16-bit ADS1115 ADC)

Platform: NORVI X CPU

https://norvi.io

## Purpose of This Example

The example demonstrates how to:

- Read 4 thermistor input channels from the X-T4 module
- Convert raw ADC values to temperature in °C
- Display the temperatures on the built-in TFT display and print them to the Serial Monitor
- Verify correct wiring and operation of the thermistor inputs

The program continuously reads:

- TEMP 1 (TM1+ / TM1-)
- TEMP 2 (TM2+ / TM2-)
- TEMP 3 (TM3+ / TM3-)
- TEMP 4 (TM4+ / TM4-)

The current temperatures are shown on the built-in TFT display (updated every 100 ms) and printed to the Serial Monitor (twice per second).

## Follow the steps below to run the example

### 1. Hardware Connections

Connect the NORVI X CPU and the X-T4 expansion module correctly.

- Power off the system, then mount the X-T4 on the DIN rail next to the CPU module and seat it firmly onto the expansion bus connector.
- Connect each 2-wire 10 kΩ NTC thermistor across the matching terminal pair:

| Channel | Terminals   |
|---------|-------------|
| 1       | TM1+ / TM1- |
| 2       | TM2+ / TM2- |
| 3       | TM3+ / TM3- |
| 4       | TM4+ / TM4- |

- AGND is the shared analog ground reference for all four channels.
- Set the I2C address using the on-board DIP switches (the sketch default is **0x48**, DIP 4 ON):

| DIP 1 | DIP 2 | DIP 3 | DIP 4 | Address |
|-------|-------|-------|-------|---------|
| ON    | OFF   | OFF   | OFF   | 0x49    |
| OFF   | ON    | OFF   | OFF   | 0x4B    |
| OFF   | OFF   | ON    | OFF   | 0x4A    |
| OFF   | OFF   | OFF   | ON    | 0x48    |

The module communicates over I2C using SDA = IO8 and SCL = IO9 of the CPU.

### 2. Configure the Program

Check the following settings at the top of the sketch before upload:

```cpp
#define ADS_ADDR 0x48                       // must match the DIP switch setting

const float SUPPLY_VOLTAGE       = 3.3;     // voltage feeding the thermistor divider
const float SERIES_RESISTOR      = 10000.0; // fixed 10K resistor
const float THERMISTOR_NOMINAL   = 10000.0; // NTC resistance at 25°C
const float TEMPERATURE_NOMINAL  = 25.0;    // reference temperature (°C)
const float B_COEFFICIENT        = 3950.0;  // B-value of your NTC (check its datasheet)
```

If you use a thermistor with a different B-value, update `B_COEFFICIENT` accordingly.

### 3. Upload the Program

- Open the example program in Arduino IDE
- Select the correct board and port:
  - Tools → Board → ESP32 Arduino → **ESP32S3 Dev Module**
  - Tools → Port → select the NORVI controller's COM/serial port
- Configure the following settings under the Tools menu:

| Setting         | Selection    |
|-----------------|--------------|
| USB CDC On Boot | Enabled      |
| Flash Size      | 16MB (128Mb) |
| PSRAM           | QSPI PSRAM   |

- Upload the program to the NORVI controller

### 4. Observe the Display and Serial Monitor

After upload, the TFT display will show the temperature of the four thermistor inputs. Open the Serial Monitor at **115200 baud** to see the same values.

When the program runs successfully:

- The controller initializes the ADS1115 and the display
- The TFT screen shows the NORVI X-T4 test screen with TEMP 1 – TEMP 4 and the ADS1115 address
- The temperature of each channel is displayed in °C
- Values update continuously as the thermistor temperature changes
- A channel with no thermistor connected (open circuit) shows `--.-- C` on the display
- If the ADS1115 is not detected, `ADS1115 NOT FOUND` is shown in red on the display

Example TFT output:

```
NORVI X-T4

TEMP 1 :  27.45 C
TEMP 2 :  27.61 C
TEMP 3 :  26.98 C
TEMP 4 :  --.-- C

ADS ADDR : 0x48
```

Example Serial Monitor output:

```
NPD-NORVI X-T4 NTC TEMPERATURE TEST
ADS1115 OK
ADS1115 ADDRESS: 0x48
ADS1115 DATA RATE = 860 SPS
TEST START
T1: 27.45 C  T2: 27.61 C  T3: 26.98 C  T4: nan C
```

(Temperature values above are examples. A channel with no sensor connected prints `nan` in the Serial Monitor.)

## How to Validate the Example

To verify the example works correctly:

1. Confirm the Serial Monitor shows `ADS1115 OK` and the address matches the DIP switch setting
2. Connect a 10 kΩ NTC thermistor to a channel and check the reading is close to the ambient temperature
3. Warm the thermistor (e.g. hold it between your fingers) or cool it, and observe the reading
4. Confirm the temperature changes accordingly on both the display and the Serial Monitor
5. Disconnect the thermistor and confirm the channel shows `--.-- C`
6. Repeat for the remaining channels

If the ADS1115 is not detected, call `I2C_SCAN();` inside `setup()` (the function is included in the sketch) to list the devices found on the I2C bus, and check the wiring and DIP switch address.

## Device Preparation / Configuration

Before running the example ensure:

- The controller is powered correctly
- The X-T4 is seated firmly on the expansion bus and the DIP switch address is set
- Thermistors are wired to the correct TM_n+ / TM_n- terminals
- `ADS_ADDR`, `SUPPLY_VOLTAGE` and `B_COEFFICIENT` in the sketch match your hardware

Example:

```
Power Supply:          24V DC (shared from the CPU through the expansion port)
Current Consumption:   40 mA @ 24V DC
Thermistor Type:       NTC, 10 kΩ at 25°C, B ≈ 3950 K, 2-wire
ADC:                   ADS1115, 16-bit, GAIN_ONE (±4.096 V), 860 SPS
```

## Required Libraries

Install the following libraries before compiling.

- Wire
- WiFi
- SPI
- Adafruit_ADS1X15
- LovyanGFX

Download Libraries:

https://github.com/NORVIControllers/NORVI-X-Version-02/tree/main/libraries

Installation:

1. Open Arduino IDE
2. Go to Sketch → Include Library → Manage Libraries
3. Search and install the required libraries

## Limitations

- This example is provided for demonstration purposes
- Temperature accuracy depends on the thermistor tolerance (±1% to ±3% for resistance and B-value); additional calibration may be required for precise measurements
- The conversion uses the Beta equation, so the series resistor, R25 and B-value constants in the sketch must match the actual sensor and module design
- Only 2-wire NTC thermistors are supported (4 channels per module)
- Performance may depend on sensor accuracy and environmental conditions

## Safety Notes

- Disconnect all power sources before installing, wiring, or servicing the module
- Do not connect or disconnect the module from the expansion bus while the system is powered
- Do not exceed the rated supply voltage (24V DC)
- Keep thermistor wiring short and separate from AC power, relay, or motor wiring; use shielded twisted-pair cable for long leads and ground the shield at one end only
- The module is rated IP20 and should be mounted on a DIN rail inside a suitably rated enclosure
- Incorrect wiring may damage the controller

Test Date: 2026-08-31

Verified By: Kaveesha

Datasheet: https://norvi.io/docs/norvi-x-t4-datasheet/

User Guide: https://norvi.io/docs/norvi-x-t4-user-guide/

## Support

Documentation: https://norvi.io/docs

Firmware examples & libraries: https://github.com/NORVIControllers/NORVI-X-Version-02/tree/main

Technical support: support@norvi.io

For additional support or inquiries, contact the NORVI support team.

Datasheet: https://norvi.io/docs/norvi-x-t4-datasheet/

## License

This example is provided for development and educational purposes.
