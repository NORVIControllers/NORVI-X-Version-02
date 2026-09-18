# NORVI X-AV4 Analog Input Expansion Example

## Overview
This example demonstrates how to use the **Analog Input Expansion Module (AV4)** on the **NORVI X CPU**.  

The purpose of this example is to show how to: 
- Initialize ADS1115: Set up 16-bit ADC via I2C at 0x48
- Read the status of 4 analog inputs
- Setup Display: Initialize TFT_eSPI and display pins

This example can be used as a starting point for developers building applications with NORVI controllers. 

## Product Used

It serves as a starting point for developers building applications with NORVI controllers.

- **Product:** NORVI X Analog Input Expansion Module (AV4)  
- **Platform:** NORVI X CPU  
- **More Information:** https://norvi.io

---

## Purpose of This Example
The example demonstrates how to:

- Integrate the AV4 Module into the NORVI X CPU  
- Read the status of analog inputs **AIN1 to AIN4**  
- Initialize the **ADS1115** 16-bit ADC via I2C at address `0x48`  
- Convert raw ADC values into actual voltage readings using a defined voltage divider ratio  
- Use the `TFT_eSPI` library to display real-time status updates  
- Implement a basic I2C scanner to detect connected peripherals  

The current input states are displayed on the built-in TFT display and printed to the Serial Monitor.

---
## Running the example
### 1. Hardware Connections

  Connect the **NORVI X CPU** and **AV4 expansion module** correctly.  
  
  Example connection:  
   - Signal Source (+) → Analog Input  
   - Signal Source (GND) → Controller GND  
   
   Connect analog input signals to
    **AIN1 to AIN4**  
   
   Ensure the input signal is within the supported voltage range (0–10 V recommended).  

---

### 2. Program Configuration

- **I2C Pins:**  
  - `SDA` → Pin 8  
  - `SCL` → Pin 9  

### Address Selection via DIP Switch

| DIP 1 | DIP 2 | DIP 3 | DIP 4 | Address |
|-------|-------|-------|-------|---------|
| ON    | OFF   | OFF   | OFF   | 0x49    |
| OFF   | ON    | OFF   | OFF   | 0x4B    |
| OFF   | OFF   | ON    | OFF   | 0x4A    |
| OFF   | OFF   | OFF   | ON    | 0x48    |

---

### 3. Upload the Program

1. Open the example program in **Arduino IDE** or **PlatformIO**  
2. Select the correct board and port  
3. Upload the program to the NORVI controller  

---

### 4. Open Serial Monitor

- Set **Baud Rate** to `115200`  

---

### 5. Observe the Display

When the program runs successfully:

- The controller initializes the TFT display, showing `NORVI EXPE-AV4 TEST`  
- The status of each input (VOLTAGE 1 – VOLTAGE 4) is displayed  
- Values update continuously as input changes  
- The Serial Monitor prints the voltage for each channel every 500 ms  

### Example Output on Serial Monitor

```
VOLTAGE 1 : 0.00V
VOLTAGE 2 : 0.00V
VOLTAGE 3 : 0.00V
VOLTAGE 4 : 0.00V

```

### Example Output on TFT Screen

```
VOLTAGE 1 : 0.00V
VOLTAGE 2 : 0.00V
VOLTAGE 3 : 0.00V
VOLTAGE 4 : 0.00V

```
### Example Output with 5 V Applied to AIN1

```
VOLTAGE 1 : 5.09V
VOLTAGE 2 : 0.00V
VOLTAGE 3 : 0.00V
VOLTAGE 4 : 0.00V

```

When a 5 V analog signal is applied to AIN1, the measured voltage is displayed on both the TFT screen and Serial Monitor. 
The displayed value may vary slightly from the signal generator setting due to ADC accuracy, resistor tolerances, and signal-source accuracy.

The remaining channels show 0.00 V when no input signal is applied.

---

## How to Validate

1. Apply a valid input signal to one of the analog channels  
2. Observe the output on the TFT display or Serial Monitor  
3. Increase or decrease the signal and confirm the displayed values update accordingly  
4. Confirm the values match between TFT and Serial Monitor  

---

## Device Preparation / Configuration

Before running the example, ensure:

- The controller is powered correctly  
- Required signals are connected  
- Any required configuration is completed  

**Example:**

- Power Supply: 24 V DC  
- Analog Input Voltage: 0–10 V  

---

## Required Libraries

Install the following libraries before compiling:

- Wire  
- Adafruit_ADS1X15  
- TFT_eSPI  
- CST816S  

**Download Libraries:**  https://github.com/NORVIControllers/NORVI-X-Version-02/tree/main/libraries 

**Installation Steps:**

1. Open Arduino IDE  
2. Go to **Library Manager**  
3. Search and install the required libraries  

---

## Limitations

- Provided for demonstration purposes only  
- Additional calibration may be required for precise measurements  
- Performance may depend on sensor accuracy and environmental conditions  

---

## Safety Notes

- Do not exceed the rated input voltage  
- Ensure proper grounding  
- Incorrect wiring may damage the controller  

---

## Test Information

- **Test Date:** 2026-09-02   
- **Verified By:** Kaveesha  

---

## Support

Documentation: https://norvi.io/docs
  
Datasheet: https://norvi.io/docs/norvi-x-av4-datasheet/
  
For additional support, contact the NORVI support team  

---

## License

This example is provided for **development and educational purposes**.

