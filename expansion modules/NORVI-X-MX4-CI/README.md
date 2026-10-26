# NORVI X-MX4-CI – Mixed I/O Expansion Module Example

## Overview

This example demonstrates how to use the Mixed I/O Expansion Module (X-MX4-CI) on the NORVI X platform.

The purpose of this example is to show how to:

- Integrate the X-MX4-CI module into the NORVI X CPU
- Read the status of 4 digital inputs
- Read 4 analog (4–20 mA) input channels
- Control 4 transistor outputs
- Display the live input and output states on the TFT display

This example can be used as a starting point for developers building applications with NORVI controllers.

---

## Product Used

**Product:** NORVI X-MX4-CI – 4 Digital Inputs, 4 Transistor Outputs, 4 Analog Inputs Expansion Module

**Platform:** NORVI X CPU (ESP32-S3)

https://norvi.io

---

## Purpose of This Example

The example demonstrates how to:

- Read 4 isolated digital input channels through the PCA9538 I/O expander
- Read 4 analog current inputs (mA) through the ADS1115 16-bit ADC
- Switch the 4 transistor outputs one at a time (walking output test)
- Display the digital input state (ACTIVE / INACTIVE), output state (ON / OFF) and analog values (mA) on the display
- Verify correct operation of the I2C devices on the module (ADS1115 and PCA9538)

The program continuously reads:

```
ANALOG INPUT 1 – 4   (mA)
DIGITAL INPUT 1 – 4
```

and continuously drives:

```
TRANSISTOR OUTPUT 1 – 4
```

The current states are shown on the built-in TFT display and printed to the Serial Monitor.

---

## Hardware Connections

Follow the steps below to run the example.

### 1. Hardware Connections

Connect the NORVI X CPU and the X-MX4-CI expansion module correctly through the 50-pin expansion port.

Connect the field signals to the module terminals:

| Terminal | Label | Function |
|---|---|---|
| 1 | IN1 | Digital Input 1 |
| 2 | IN2 | Digital Input 2 |
| 3 | IN3 | Digital Input 3 |
| 4 | IN4 | Digital Input 4 |
| 5 | DCOM | Isolated Digital Ground |
| 6 | TR1 | Transistor Output 1 |
| 7 | TR2 | Transistor Output 2 |
| 8 | TR3 | Transistor Output 3 |
| 9 | TR4 | Transistor Output 4 |
| 10 | GND | Power Ground |
| 11 | AIN1 | Analog Input 1 (4–20 mA) |
| 12 | AIN2 | Analog Input 2 (4–20 mA) |
| 13 | AIN3 | Analog Input 3 (4–20 mA) |
| 14 | AIN4 | Analog Input 4 (4–20 mA) |
| 15 | AGND | Analog Ground |
| 16 – 18 | GND | Power Ground |

Ensure the signals are within the supported input ranges.

### 2. Configure the Program

The default I2C addresses used by the sketch are:

| Device | Default Address |
|---|---|
| ADS1115 (Analog Inputs) | `0x4A` |
| PCA9538 (Digital Inputs & Transistor Outputs) | `0x71` |

The I2C bus uses **SDA = IO8** and **SCL = IO9**.

If the DIP switch settings on the module are changed, update `ADS_ADDR` and `PCA9538_ADDR` in the sketch accordingly.

**I2C Address Selection (Analog Inputs)**

| DIP 1 | DIP 2 | DIP 3 | DIP 4 | Address |
|---|---|---|---|---|
| ON | OFF | OFF | OFF | 0x49 |
| OFF | ON | OFF | OFF | 0x4B |
| OFF | OFF | ON | OFF | 0x4A (default) |
| OFF | OFF | OFF | ON | 0x48 |

**I2C Address Selection (Digital Inputs & Transistor Outputs)**

| DIP 1 | DIP 2 | DIP 3 | DIP 4 | Address |
|---|---|---|---|---|
| OFF | OFF | × | × | 0x73 |
| ON | OFF | × | × | 0x72 |
| OFF | ON | × | × | 0x71 (default) |
| ON | ON | × | × | 0x70 |

### 3. Upload the Program

- Open the example program in Arduino IDE / PlatformIO
- Select the correct board and port
- Upload the program to the NORVI controller

### 4. Observe the Display

After upload, the TFT display will show the analog values, the digital input states and the transistor output states.

When the program runs successfully:

- The controller initializes the display
- The TFT screen shows the X-MX4-CI test screen
- Each analog channel is displayed in mA
- Each digital input is displayed as ACTIVE or INACTIVE
- The transistor outputs turn ON one at a time, every 1 second
- Values update continuously as inputs change

---

## Example Output

Serial Monitor (115200 baud):

```
NPD-NORVI X-MX4-CI TEST
ADS1115 0x4A OK
PCA9538 0x71 OK
TEST START
CH1: 4.00 mA  CH2: 12.00 mA  CH3: 0.00 mA  CH4: 20.00 mA  |  DI1-4: 0110  DO1-4: 1000
```

TFT display:

```
ANALOG INPUTS (mA)
CH1: 4.00 mA
CH2: 12.00 mA
CH3: 0.00 mA
CH4: 20.00 mA

DIGITAL I/O
DI1: INACTIVE    DO1: ON
DI2: ACTIVE      DO2: OFF
DI3: ACTIVE      DO3: OFF
DI4: INACTIVE    DO4: OFF
```

---

## How to Validate the Example

To verify the example works correctly:

1. Apply a valid input signal (24 V DC to a digital input, or a 4–20 mA loop to an analog input)
2. Observe the output on the display and Serial Monitor
3. Change the input condition
4. Confirm the output changes accordingly
5. Connect a load to the transistor outputs and confirm that each output switches ON in turn

---

## Device Preparation / Configuration

Before running the example ensure:

- The controller is powered correctly
- Required signals are connected
- The I2C address DIP switches match the addresses defined in the sketch
- Any required configuration is completed

Example:

```
Power Supply: 24V DC
Digital Input Range: 0–30V DC (Logic Low < 5V, Logic High > 15V)
Analog Input Range: 4–20 mA / 0–20 mA
Transistor Output: 0.5A per channel, 24V DC (high-side switch)
```

---

## Required Libraries

Install the following libraries before compiling.

- Wire
- SPI
- Adafruit_ADS1X15
- PCA9538
- LovyanGFX

Download Libraries:

https://github.com/NORVIControllers/NORVI-X-Version-02/tree/main/libraries

Installation:

1. Open Arduino IDE
2. Go to Library Manager
3. Search and install the required libraries

---

## Limitations

- This example is provided for demonstration purposes
- Additional calibration may be required for precise measurements
- Performance may depend on sensor accuracy and environmental conditions
- The GPIO pins used by the module (SDA – IO8, SCL – IO9) cannot be used for any other purpose while the module is connected

---

## Safety Notes

- Do not exceed the rated input voltage
- Do not exceed 0.5 A per transistor output channel
- Ensure proper grounding
- Incorrect wiring may damage the controller

---

**Test Date:** 2026-10-23

**Verified By:** Kaveesha

---

## Support
- Documentation: https://norvi.io/docs  
- For additional support or inquiries, contact the NORVI support team.
- Datasheet: https://norvi.io/docs/norvi-x-mx4-ci-datasheet/ 
---

## License

This example is provided for development and educational purposes.
