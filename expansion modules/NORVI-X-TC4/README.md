# NORVI X-TC4 – 4-Channel Thermocouple Input Expansion Module Example

## Overview

This example demonstrates how to use the 4-Channel Thermocouple Input Expansion Module (X-TC4) on the NORVI X platform.

The purpose of this example is to show how to:

- Integrate the X-TC4 module with the NORVI X CPU over I²C
- Read temperature and fault status from up to 4 K-type thermocouple channels
- Validate every reply with a CRC8 checksum and recover the I²C bus automatically after repeated failures
- Select the channels to poll at run time through a Serial configuration mode
- Display the live temperature and fault status on the TFT display and the Serial Monitor

This example can be used as a starting point for developers building temperature monitoring applications with NORVI controllers.

## Product Used

**Product:** NORVI X-TC4 – 4 x Channel Thermocouple Inputs Expansion Module
**Platform:** NORVI X CPU (NORVI X-CPU-ESPS3-X1 / X2 / X3)

https://norvi.io

## Purpose of This Example

The example demonstrates how to:

- Request temperature data from the X-TC4 module every 1 second
- Read the temperature (°C) and fault code of each configured channel
- Show the readings on the built-in TFT display and on the Serial Monitor
- Show a `NO RESPONSE` message on the display when the module stops answering
- Change the polled channels without re-uploading the program

The program continuously reads:

```
CHANNEL 1
CHANNEL 2
CHANNEL 3
CHANNEL 4
```

The X-TC4 module contains an onboard co-processor that reads the four MAX6675 thermocouple-to-digital converters and acts as a fixed I²C slave. Its firmware is factory-programmed and cannot be modified. This ESP32-S3 sketch is the master-side driver and is the only part intended to be modified or extended.

## Hardware Used

| Item | Details |
|---|---|
| CPU board | NORVI X-CPU-ESPS3-X1 / X2 / X3 |
| Expansion module | NORVI X-TC4 |
| Sensors | Up to 4 × K-type thermocouples |
| Power supply | 24V DC |

### Pin Map

| Signal | GPIO |
|---|---|
| I²C SDA | 8 |
| I²C SCL | 9 |
| SPI MISO | 13 |
| SPI MOSI | 11 |
| SPI SCLK | 12 |
| TFT Display CS | 45 |
| TFT Display DC | 46 |
| TFT Display RST | 47 |

> The X-TC4 module uses **IO8 (SDA)** and **IO9 (SCL)** of the expansion port. These pins cannot be used for any other purpose while the module is connected.

### Terminal Configuration

| No. | Terminal | Description |
|---|---|---|
| 1 | TC1+ | Thermocouple Positive (+) Input – Channel 1 |
| 2 | TC1- | Thermocouple Negative (-) Input – Channel 1 |
| 3 | TC2+ | Thermocouple Positive (+) Input – Channel 2 |
| 4 | TC2- | Thermocouple Negative (-) Input – Channel 2 |
| 5 | TC3+ | Thermocouple Positive (+) Input – Channel 3 |
| 6 | TC3- | Thermocouple Negative (-) Input – Channel 3 |
| 7 | TC4+ | Thermocouple Positive (+) Input – Channel 4 |
| 8 | TC4- | Thermocouple Negative (-) Input – Channel 4 |
| 9 | GND | Ground |

### I²C Address Selection

The default address used in this example is `0x18` (all DIP switches OFF).

| DIP 1 | DIP 2 | DIP 3 | DIP 4 | Address |
|---|---|---|---|---|
| OFF | OFF | OFF | OFF | 0x18 |
| ON | OFF | OFF | OFF | 0x17 |
| OFF | ON | OFF | OFF | 0x16 |
| ON | ON | OFF | OFF | 0x14 |
| OFF | OFF | ON | OFF | 0x13 |
| ON | OFF | ON | OFF | 0x12 |
| OFF | ON | ON | OFF | 0x11 |
| ON | ON | ON | OFF | 0x10 |
| OFF | OFF | OFF | ON | 0x0F |
| ON | OFF | OFF | ON | 0x0E |
| OFF | ON | OFF | ON | 0x0D |
| ON | ON | OFF | ON | 0x0C |
| OFF | OFF | ON | ON | 0x0B |
| ON | OFF | ON | ON | 0x0A |
| OFF | ON | ON | ON | 0x09 |
| ON | ON | ON | ON | 0x08 |

If you change the DIP switch setting, update `TC_SLAVE_ADDR` in the sketch to match. When several X-TC4 modules are connected, give each a different address (the commented `tcSlaveAddrs[]` line in the sketch lists `0x18`, `0x17` and `0x16`).

## How to Run the Example

Follow the steps below to run the example:

### 1. Hardware Connections

- Mount the X-TC4 module next to the NORVI X CPU and connect the expansion port.
- Connect the K-type thermocouples to:
  - TC1+ / TC1-
  - TC2+ / TC2-
  - TC3+ / TC3-
  - TC4+ / TC4-
- Observe the thermocouple polarity (+ / -) on every channel.
- Connect the 24V DC supply and make sure the module's I²C address matches `TC_SLAVE_ADDR` in the sketch.

### 2. Configure the Program

- The default I²C address is `0x18`. Change `TC_SLAVE_ADDR` only if the DIP switches are set to another address.
- By default **Channel 1** is polled. The channels can be changed at run time using the Serial configuration mode (see below).

### 3. Upload the Program

- Open the example program in Arduino IDE.
- Install the required libraries.
- Select the board and port using the settings below.
- Upload the program to the NORVI controller.

**ESP32 Board Package:** any 3.x version (ESP32 Arduino core 3.x, IDF 5.x)

**Arduino IDE Board / Tools settings:**

| Setting | Value |
|---|---|
| Board | ESP32S3 Dev Module |
| Upload Speed | 115200 |
| USB Mode | Hardware CDC and JTAG |
| USB CDC On Boot | Enabled |
| USB Firmware MSC On Boot | Disabled |
| USB DFU On Boot | Disabled |
| Upload Mode | UART0 / Hardware CDC |
| CPU Frequency | 240MHz (WiFi) |
| Flash Mode | QIO 80MHz |
| Flash Size | 16MB (128Mb) |
| Partition Scheme | Huge APP (3MB No OTA/1MB SPIFFS) |
| Core Debug Level | None |
| PSRAM | QSPI PSRAM |
| Arduino Runs On | Core 1 |
| Events Run On | Core 1 |
| Erase All Flash Before Sketch Upload | Disabled |
| JTAG Adapter | Disabled |

### 4. Observe the Display and Serial Monitor

Open the Serial Monitor at **115200 baud**.

When the program runs successfully:

- The controller initializes the display and shows the `NORVI` / `EXPE-TC4 TEST` header at boot
- The TFT screen then shows `NORVI TC4 MASTER` and, for each configured channel, a temperature row (°C) and a fault row
- The Serial Monitor prints the readings every 1 second
- Only the rows whose value changed are redrawn, so the screen does not flicker

### 5. Change the Channels (Serial Configuration Mode)

1. Send `C` (or `c`) in the Serial Monitor. Set the line ending to **Newline**.
2. The program prints `===== CONFIG MODE =====` and `Enter Channels to read:`.
3. Enter the channels to poll:

| Input | Channels polled |
|---|---|
| `1` | Channel 1 |
| `1-3` | Channels 1, 2 and 3 |
| `1,3` | Channels 1 and 3 |

4. The program prints `Configuration Saved:` and the new channel list, and the TFT screen is redrawn.

Input with no valid channel (channels must be 1–4), for example `7` or `1-9`, is rejected with `Invalid input - configuration not changed` and the previous configuration is kept.

## Example Output

**Serial Monitor:**

```
Channels : 1
NORVI TC4 MASTER STARTED
Ch 1 Temp: 24.60 Fault: 0
Ch 1 Temp: 24.75 Fault: 0
Ch 1 Temp: 24.75 Fault: 0
```

**TFT display:**

```
NORVI TC4 MASTER
Ch 1 : Temp: 24.60 C
Fault: 0
```

**Communication error messages (Serial Monitor):**

| Message | Meaning |
|---|---|
| `I2C TX FAIL` | The module did not acknowledge the request (the I²C bus is recovered automatically after 5 consecutive failures) |
| `I2C RX FAIL` | The module did not return the expected number of bytes |
| `CRC ERROR Channel n` | The CRC8 checksum of the reply did not match |

If the module gives no valid reply for 3 read cycles in a row, the TFT display replaces the channel values with a red `NO RESPONSE` message and the I²C address being used. The values return as soon as the module answers again.

## How to Validate the Example

To verify the example works correctly:

1. Connect a K-type thermocouple to a channel and select that channel in the configuration mode
2. Observe the temperature on the display and the Serial Monitor (it should match the ambient or reference temperature)
3. Warm the thermocouple tip (for example by holding it between your fingers) or move it to a different temperature
4. Confirm the temperature changes accordingly
5. Disconnect the thermocouple and confirm that the fault value changes to a non-zero value
6. Reconnect the thermocouple and confirm that the fault value returns to `0`

## Device Preparation / Configuration

Before running the example ensure:

- The controller and the X-TC4 module are powered correctly
- The module is firmly connected to the NORVI X expansion port
- The thermocouples are connected with the correct polarity
- The I²C address set on the DIP switches matches `TC_SLAVE_ADDR` in the sketch

Example:

```
Power Supply: 24V DC
Current Consumption: 40mA @ 24VDC
Thermocouple Type: K-type
Temperature Range: 0°C to +1024°C
```

## Required Libraries

Install the following libraries before compiling.

| Library | Version |
|---|---|
| Wire (ESP32 core) | 3.x (bundled with the core) |
| SPI (ESP32 core) | 3.x (bundled with the core) |
| LovyanGFX | 1.2.25 |

**Installation:**

1. Open Arduino IDE
2. Go to Library Manager
3. Search for **LovyanGFX** and install version 1.2.25
4. Install the ESP32 board package (3.x) from the Boards Manager. Wire and SPI are installed with it.

## Troubleshooting

| Symptom | Likely Cause | Fix |
|---|---|---|
| `I2C TX FAIL` / `I2C RX FAIL` on every read, TFT shows `NO RESPONSE` | `TC_SLAVE_ADDR` does not match the module's I²C address, or I²C wiring problem | Compare the address shown on the screen with the DIP switch setting and update `TC_SLAVE_ADDR`; check SDA / SCL wiring |
| `CRC ERROR Channel n` repeatedly | Noisy or unstable I²C bus, or corrupted data | Check the I²C wiring quality and length; confirm the thermocouple of that channel is connected properly |
| Fault value is non-zero for a channel | Thermocouple open-circuit, short-circuit or out-of-range reading | Check the thermocouple wiring and polarity on that channel |
| Temperature readings are implausible | Wrong thermocouple type, or poor connection | Use a K-type thermocouple and check the connection |
| Config mode does not accept input | Input is not terminated with a newline | Set the Serial Monitor line ending to **Newline** and press Enter after the input (e.g. `1-3`) |
| TFT shows nothing / blank screen | SPI pins (MISO 13 / MOSI 11 / SCLK 12) or Display CS (GPIO 45) miswired, or `tft.init()` failing | Verify the SPI wiring and that the pin numbers in the `LGFX` class match the board pinout |
| Display colors wrong or inverted | Panel invert / RGB order setting differs from the panel in use | Toggle `c.invert` or `c.rgb_order` in the `LGFX` class |
| Upload fails / board not detected | Wrong Board / Tools settings | Match the settings listed in step 3 |

## Limitations

- This example is provided for demonstration purposes
- The module supports **K-type thermocouples only**
- Measurement range is 0°C to +1024°C with 12-bit resolution; accuracy is ±8 LSB from 0°C to +700°C
- Temperature below 0°C cannot be measured by the MAX6675 converters
- Additional calibration may be required for precise measurements
- Performance may depend on thermocouple accuracy, cable routing and environmental conditions
- The co-processor firmware on the module is fixed and cannot be modified

## Safety Notes

- Do not exceed the rated supply voltage (24V DC)
- Ensure proper grounding
- Do not connect voltage sources to the thermocouple inputs
- Incorrect wiring may damage the controller or the module
- Operating temperature of the module: -40°C to +85°C (the thermocouple tip can be at a much higher temperature, but the module itself must stay within this range)

## Test Information

Test Date: 2026-10-22
Verified By: <Kaveesha>

## Datasheet

Datasheet: https://norvi.io/docs/norvi-x-tc4-datasheet/

## Support

Documentation: https://norvi.io/docs

Technical Support: support@norvi.io

For additional support or inquiries, contact the NORVI support team.

## License

This example is provided for development and educational purposes.
