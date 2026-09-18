# NORVI X — Analog Input Expansion Module (AI4) Example

## Overview

This example demonstrates how to use the **Analog Input Expansion Module (AI4)** on the **NORVI X** platform.

The purpose of this example is to show how to:

- **Initialize ADS1115** — Set up the 16-bit ADC via I2C
- **Read the status of 4 analog inputs**
- **Setup Display** — Initialize TFT_eSPI and display pins

This example can be used as a starting point for developers building applications with NORVI controllers.

---

## Product Used

Product: NORVI X Analog Input Expansion Module (AI4) 

Platform: NORVI X CPU

More information: [https://norvi.io](https://norvi.io)

---

## Purpose of This Example

The example demonstrates how to:

- Integrate the AI4 Module into the NORVI X CPU
- Control and monitor the status of Analog Inputs
- Convert raw ADC values into actual voltage readings using a defined voltage divider ratio
- Use the TFT_eSPI library to display real-time status updates
- Implement a basic I2C scanner to detect connected peripherals

The program continuously reads:

- **AIN1 to AIN4**

The current input states are shown on the built-in TFT display and Serial Monitor.

---

## Running the Example

Follow the steps below to run the example:

### 1. Hardware Connections

- Connect the NORVI X CPU and AI4 expansion module correctly.

**Example:**
- Signal Source (+) → Analog Input
- Signal Source (GND) → Controller GND

Connect analog input signals to:
- **AIN1 to AIN4**

> Ensure the signal is within the supported input range.

### 2. Configure the Program

Ensure the I2C pins are correctly defined: **SDA (Pin 8)** and **SCL (Pin 9)**.

#### Address Selection

| DIP 1 | DIP 2 | DIP 3 | DIP 4 | Address |
|---|---|---|---|---|
| ON | OFF | OFF | OFF | 0x49 |
| OFF | ON | OFF | OFF | 0x4B |
| OFF | OFF | ON | OFF | 0x4A |
| OFF | OFF | OFF | ON | 0x48 |

### 3. Upload the Program

- Open the example program in Arduino IDE / PlatformIO
- Select the correct board and port
- Upload the program to the NORVI controller

### 4. Open Serial Monitor

- Set the Baud Rate to **115200**

### 5. Observe the Display

When the program runs successfully:

- The controller initializes the display
- The TFT display will initialize, showing **"NORVI EXPE-AI4 TEST"**
- The status of each input (CURRENT 1 – CURRENT 4) is displayed
- Values update continuously as input changes
- The Serial Monitor will print the voltage for each channel every 500 ms

#### Example Output on the TFT Screen

```
CURRENT 1 : 0.05 mA
CURRENT 2 : 0.25 mA
CURRENT 3 : 0.18 mA
CURRENT 4 : 0.26 mA
```

#### Example Output on the Serial Monitor

```
CH1: 0.05 mA  CH2: 0.25 mA  CH3: 0.18 mA  CH4: 0.26 mA
```

---

## How to Validate the Example

To verify the example works correctly:

1. Apply a valid input signal to one of the analog channels.
2. Observe the output on the TFT screen or Serial Monitor to see the voltage change.
3. Increase or decrease the signal and confirm the displayed value updates accordingly.
4. Confirm that changing the output state reflects correctly in both TFT and Serial Monitor.

---

## Device Preparation / Configuration

Before running the example, ensure:

- The controller is powered correctly
- Required signals are connected
- Any required configuration is completed

**Example:**
- Power Supply: 24V DC
- Analog Input: 4–20 mA

---

## Required Libraries

Install the following libraries before compiling:

- `Wire`
- `SPI`
- `Adafruit_ADS1X15`
- `TFT_eSPI`
- `CST816S`

**Download Libraries:**
[https://github.com/NORVIControllers/NORVI-X-Version-02/tree/main/libraries](https://github.com/NORVIControllers/NORVI-X-Version-02/tree/main/libraries)

### Installation

1. Open Arduino IDE
2. Go to Library Manager
3. Search and install the required libraries

---

## Limitations

- This example is provided for demonstration purposes
- Additional calibration may be required for precise measurements
- Performance may depend on sensor accuracy and environmental conditions

---

## Safety Notes

- Do not exceed the rated input voltage
- Ensure proper grounding
- Incorrect wiring may damage the controller

---

## Test Information

Test Date: 2026-08-24

Verified By: Kaveesha

---

## Support

- Documentation: [https://norvi.io/](https://norvi.io/)
- Datasheet: [https://norvi.io/docs/norvi-x-ai4-datasheet/](https://norvi.io/docs/norvi-x-ai4-datasheet/)
- For additional support or inquiries, contact the NORVI support team.

---

## License

This example is provided for development and educational purposes.

