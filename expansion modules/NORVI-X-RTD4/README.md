
# NORVI X-RTD4 Example
## Overview

This example demonstrates how to use the NORVI X-RTD4 on the NORVI X platform. The NORVI X-RTD4 is a high-precision 4-channel RTD expansion module for the NORVI X modular controller platform. It utilizes the 15-bit ADC to provide laboratory-grade temperature digitization for Platinum RTDs.

The purpose of this example is to show how to:

- Integrate the RTD4 Module into the NORVI X CPU.
- Read temperature values from up to 4 independent RTD sensors (PT100 or PT1000).
- Display the measured temperature for each channel on the Serial Monitor.
- Setup Display: Initialize TFT_eSPI and display pins.

This example can be used as a starting point for developers building applications with NORVI controllers.

## Product Used

**Product:** NORVI X RTD Input Expansion Module (4-Channel)

**Platform:** NORVI X CPU

**More information:** https://norvi.io

## Purpose of This Example

The example demonstrates how to:

- Communicate with the X-RTD4 module over I2C via the expansion bus.
- Sequentially read temperature data from Channels 1–4.
- Display the measured RTD temperature values on a TFT display.

**Example:**
This example reads temperature data from up to 4 RTD sensors connected to the X-RTD4 module and displays the measured values on a TFT display connected to the NORVI X CPU.

## Running the Example

Follow the steps below to run the example.

### 1. Hardware Connections

Connect the required hardware according to the example.

**Example:**

- RTD1 (+) → Terminal 1, RTD1 (−) → Terminal 2
- RTD2 (+) → Terminal 3, RTD2 (−) → Terminal 4
- RTD3 (+) → Terminal 5, RTD3 (−) → Terminal 6
- RTD4 (+) → Terminal 7, RTD4 (−) → Terminal 8
- GND → Terminal 9

Ensure each RTD sensor type (PT100 or PT1000) matches the RTD Type Selector setting on the module.

### 2. Configure the Program

- **I2C Address Selector (DIP 1–4):** Select the desired I2C address for the module (e.g., all OFF = 0x3F). Refer to the address selection table below if multiple X-RTD4 modules are used on the same bus.
- Ensure the I2C pins are correctly defined: SDA (Pin 8) and SCL (Pin 9).

**Address Selection**

| DIP 1 | DIP 2 | DIP 3 | DIP 4 | Address |
|---|---|---|---|---|
| OFF | OFF | OFF | OFF | 0x3F |
| ON | OFF | OFF | OFF | 0x3E |
| OFF | ON | OFF | OFF | 0x3D |
| ON | ON | OFF | OFF | 0x3C |
| OFF | OFF | ON | OFF | 0x3B |
| ON | OFF | ON | OFF | 0x3A |
| OFF | ON | ON | OFF | 0x39 |
| ON | ON | ON | OFF | 0x38 |
| OFF | OFF | OFF | ON | 0x37 |
| ON | OFF | OFF | ON | 0x36 |
| OFF | ON | OFF | ON | 0x35 |
| ON | ON | OFF | ON | 0x34 |
| OFF | OFF | ON | ON | 0x33 |
| ON | OFF | ON | ON | 0x32 |
| OFF | ON | ON | ON | 0x31 |
| ON | ON | ON | ON | 0x30 |

**RTD Type Selector:**

- OFF = PT1000
- ON = PT100

### 3. Configure the Program (if required)

Update any required parameters in the code before uploading.

**Examples:**

- I2C address of the X-RTD4 module (must match DIP switch setting)
- RTD type (PT100 / PT1000)
- Number of active channels
- TFT display driver/model and pin definitions (CS, DC, RST)
- Display refresh interval

### 4. Upload the Program

- Open the example program in Arduino IDE / PlatformIO
- Select the correct board and port
- Upload the program to the NORVI controller

### 5. Open Serial Monitor

Set the Baud Rate to 115200.

The example uses the Serial Monitor to configure which RTD sensor type and terminals to read, using the following command format:

1. Enter `c` to start the configuration command.
2. Enter the sensor type and terminal number(s), separated by a comma: `<sensor_type>,<terminal(s)>`
   - `sensor_type`: `100` for PT100, `1000` for PT1000
   - `terminal(s)`: a single channel number, or a range (e.g., `1-3`)

**Example — read Channel 1 only (PT100):**
c
100,1

**Example — read Channels 1, 2, and 3 (PT1000):**
c
1000,1-3

### 6. Observe the Display

After the configuration command is entered, the controller will read the configured RTD channel(s) and show the measured temperature values on the TFT display.

## What the User Should Expect as a Result

When the program runs successfully:

- The controller initializes the display.
- The TFT display will initialize, showing only the channel(s) connected/configured.

**Example TFT display layout:**
```
NORVI EXPE-RTD MASTER
RTD Type: PT1000

Ch 1 : Temp: 25.30 C
Res: 111.41 Ohm Fault: 0

Ch 2 : Temp: 0.00 C
Res: 429.99 Ohm Fault: 168
```
**Example Serial Monitor output:**

```
Ch 1 Temp: 25.30 Res: 111.41 Fault: 0

Ch 2 Temp: 0.00 Res: 429.99 Fault: 168
```
## How to Validate the Example

To verify the example works correctly:

1. Connect a valid RTD sensor to one or more terminals on the X-RTD4 module.
2. In the Serial Monitor, enter `c` (or `C`), then enter the sensor type and channel(s), e.g. `100,1`.
3. Observe the corresponding temperature and resistance reading on the TFT display and Serial Monitor.
4. Change the temperature at the sensor (e.g., touch the sensor or apply heat/cold).
5. Confirm the displayed value changes accordingly.
6. Disconnect a sensor and confirm the `Fault` field changes to indicate a fault condition.
7. Repeat with a different sensor type/channel combination (e.g., `1000,1-3`) to confirm multi-channel configuration works.

## Device Preparation / Configuration

Before running the example ensure:

- The NORVI X CPU and X-RTD4 module are properly connected via the 50-pin expansion bus.
- The controller is powered with 24V DC.
- RTD sensors are wired according to the terminal configuration.
- The RTD Type Selector and I2C Address Selector DIP switches on the X-RTD4 module are set correctly (default address `0x3F`).
- A TFT display (compatible with TFT_eSPI) is connected to the SPI pins defined in the code.

## Required Libraries

Install the following libraries before compiling.

- `Wire` (I2C communication)
- `SPI`
- `TFT_eSPI` (TFT display driver)
- `Free_Fonts.h` (font definitions used with TFT_eSPI)
- `CST816S` (capacitive touch controller driver)

**Download Libraries:**
https://github.com/NORVIControllers/NORVI-X-Version-02/tree/main/libraries

**Installation:**

1. Open Arduino IDE
2. Go to Library Manager
3. Search and install the required libraries

## Limitations

- This example is provided for demonstration purposes.
- Additional calibration may be required for precise measurements.
- Performance may depend on sensor accuracy and environmental conditions.

## Safety Notes

- Do not exceed the rated input voltage.
- Ensure proper grounding.
- Incorrect wiring may damage the controller.

## Tested Hardware

**Test Date:** 2026-08-06

**Verified By:** Kaveesha

## Support

**Documentation:** https://norvi.io/docs

For additional support or inquiries, contact the NORVI support team.

**Datasheet:** https://norvi.io/docs/norvi-x-rtd4-datasheet/

## License

This example is provided for development and educational purposes.





















