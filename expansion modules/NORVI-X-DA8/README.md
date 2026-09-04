# NORVI X – DA8 AC Digital Input Expansion Module 

## Overview

The DA8 expansion module provides **8 optically isolated 230 V AC digital inputs** sharing a common Neutral (`N`) terminal. This example demonstrates how to initialize communication, monitor input channel states, detect state transitions, and display output on both the onboard TFT screen and the Serial Monitor.

### Key Objectives
* Initialize the PCA9538 I/O expander via I2C.
* Read real-time status for 8 AC input channels (`AC1`–`AC8`).
* Log state transitions to the Serial Monitor (115200 baud).
* Refresh live status on the integrated TFT display.

This example can be used as a starting point for developers building applications with **NORVI controllers**.
---

## Product Used

**Product:** AC Digital Input Expansion Module  (DA8)  
**Platform:** NORVI X CPU  

Website: https://norvi.io

---
## Purpose of This Example

This example demonstrates how to:
- Integrate the DA8 module with the NORVI X CPU.
- Communicate with the PCA9538 through I2C.
- Read the status of AC1 to AC8.
- Detect changes in input states.
- Display the current input states on the TFT display.
- Print changed input states on the Serial Monitor.

---

## Running the Example

Follow the steps below to run the example.

--- 

## 1. Hardware Configuration & Wiring

> ⚠️ **SAFETY WARNING**: The DA8 inputs are designed for **230 V AC** digital signals. Always isolate power sources before touching wiring. Installation should be performed by qualified personnel inside an appropriate enclosure.

### Voltage Specifications
* **NORVI X System Power:** 24 V DC
* **DA8 Input Signal Voltage:** 230 V AC

### Terminal Connections

| Terminal | Description |
| :--- | :--- |
| **AC1 – AC8** | AC Digital Input Channels |
| **AC Supply (Live)** | Connect to target input channel (`AC1`–`AC8`) |
| **AC Supply (Neutral)** | Connect to the common `N` terminal |

---

## 2. I2C Pinout & DIP Switch Configuration

The DA8 communicates with the NORVI X CPU over I2C using the following pin mapping:

* **SDA:** `GPIO 8`
* **SCL:** `GPIO 9`

### Address Selection Table

Configure the onboard DIP switches to match the I2C address defined in your program code.

| DIP 1 | DIP 2 | DIP 3 | DIP 4 | I2C Address |
| :---: | :---: | :---: | :---: | :---: |
| **OFF** | **OFF** | × | × | `0x73` *(Default)* |
| **ON** | **OFF** | × | × | `0x72` |
| **OFF** | **ON** | × | × | `0x71` |
| **ON** | **ON** | × | × | `0x70` |

## 3. Upload the Program

1. Open the example program in **Arduino IDE / PlatformIO**
2. Download the Free_Fonts.h file from the NORVI X Version 02 GitHub repository and place it in the same folder as the Arduino sketch.
   Repository : https://github.com/NORVIControllers/NORVI-X-Version-02 
3. Select the correct **board** and **port**
4. Upload the program to the **NORVI controller**

---
## 4. Open Serial Monitor

Set the **Baud Rate** to:

```
115200
```

---

## 5. Observe the Display

Onboard TFT Display

Initially, all inputs are HIGH:
When operational, the screen renders live state data across all channels:

```
NORVI
X-DA8 INPUT TEST

AC1: 1
AC2: 1
AC3: 1
AC4: 1
AC5: 1
AC6: 1
AC7: 1
AC8: 1

```
When the AC input on AC1 changes:

```
AC1: 0
AC2: 1
AC3: 1
AC4: 1
AC5: 1
AC6: 1
AC7: 1
AC8: 1

```
Example Output on the Serial Monitor 

After uploading, the **Serial Monitor** will show the status of the digital inputs.
Only the input that changes is printed: 
 
```
AC1 = 0

```
The TFT displays the status of all eight inputs, while the Serial Monitor prints only the inputs when their state changes.

---

## How to Validate the Example

To verify the example works correctly:

1. Apply the appropriate AC input signal to one of the AC1-AC8 channels.
2. Observe the corresponding input status on the TFT display.
3. Check the Serial Monitor for the changed input.
4. Repeat the test for the remaining AC input channels.

---

## Device Preparation / Configuration

Before running the example ensure:

- The controller is **powered correctly**
- The DA8 is correctly connected to the NORVI X expansion interface.
- AC input wiring is connected correctly.
- The neutral connection is connected to the N terminal.

Example:

**Power Supply:** 24V DC  
**AC Input:** 230 V AC

---

## Required Libraries

Install the following libraries before compiling:

- Wire
- WiFi
- SPI
- TFT_eSPI
- CST816S
- Free_Fonts

Library Download:  https://github.com/NORVIControllers/NORVI-X-Version-01/tree/main/libraries 

### Installation

1. Open **Arduino IDE**
2. Go to **Library Manager**
3. Search and install the required libraries

---

## Limitations

- This example is provided for **demonstration purposes**
- Additional calibration may be required for **precise measurements**
- Performance may depend on **sensor accuracy and environmental conditions**

---

## Safety Notes

- Do not exceed the **rated input voltage**
- Ensure **proper grounding**
- Incorrect wiring may **damage the controller**

---

## Test Information

**Test Date:** 2026-09-04

**Verified By:** Kaveesha

---

## Documentation

NORVI Documentation:  
https://norvi.io/docs

Datasheet: https://norvi.io/docs/norvi-x-da8-datasheet/  

---

## Support

For additional support or inquiries, contact the **NORVI support team**.

---

## License

This example is provided for **development and educational purposes**.








