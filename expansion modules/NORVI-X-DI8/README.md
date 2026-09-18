# NORVI X – DI8 Digital Input Example

## Overview
This example demonstrates how to use the **Digital Input Expansion Module (DI8)** on the **NORVI X platform**.

The purpose of this example is to show how to:

- Integrate the **DI8 Module** with the **NORVI X CPU**
- Initialize the **I2C-based PCA9538 GPIO expander**
- Read the **real-time status of 8 digital input channels**
- Display **live input states on the TFT display and Serial Monitor**

This example can be used as a starting point for developers building applications with **NORVI controllers**.

---

## Product Used

**Product:** NORVI X Digital Input Expansion Module (DI8)  
**Platform:** NORVI X CPU  

Website: https://norvi.io

---

## Purpose of This Example

This example demonstrates the following technical implementations:

- **I2C Scanning:** Automatically detects the DI8 module at address **0x73**
  I2C Address Selection

The I2C address of the module can be configured using the DIP switches.

| DIP 1 | DIP 2 | DIP 3 | DIP 4 | I2C Address |
|------|------|------|------|-------------|
| OFF | OFF | – | – | 0x73 |
| ON | OFF  | – | – | 0x72 |
| OFF  | ON | – | – | 0x71 |
| ON  | ON  | – | – | 0x70 |

  The DI8 module supports multiple I2C addresses using the DIP switch configuration.


Set the DIP switches on the module according to the desired I2C address before powering the device.
- **Input Reading:** Polls **8 digital input channels**
- **Visual Feedback:** Maps digital states (**0 or 1**) to a dedicated UI on the **TFT screen**

The program continuously reads:

- INPUT 1  
- INPUT 2  
- INPUT 3  
- INPUT 4  
- INPUT 5  
- INPUT 6  
- INPUT 7  
- INPUT 8  

The current input states are shown on the **built-in TFT display and Serial Monitor**.

---

## Running the Example

Follow the steps below to run the example.

---

## 1. Hardware Connections

Connect the **NORVI X CPU** and **DI8 expansion module** correctly.

Connect digital input signals as follows:

- **Signal Source (+)** → Digital Input (**IN1 – IN8**)  
- **Signal Source (GND)** → **Controller GND**

Ensure the signal is within the **supported input range**.

---

## 2. Configure the Program

- No specific software configuration (like WiFi credentials) is required for this basic test.

- Verify the I2C pins are defined as SDA (8) and SCL (9) in the code.

---

## 3. Upload the Program

1. Open the example program in **Arduino IDE / PlatformIO**
2. Select the correct **board** and **port**
3. Upload the program to the **NORVI controller**

---
## 4. Open Serial Monitor

Set the **Baud Rate** to:

```
115200
```

---
## 5. Observe the Display

After uploading, the **TFT display** will show the status of the digital inputs.

When the program runs successfully:

- The controller initializes the display
- The TFT screen shows the **DI8 test screen**
- The status of each input is displayed as **0 or 1**
- Values update continuously as inputs change

---

## Example Output

Initially, all inputs are HIGH:
### TFT Display

```
NORVI
X-DI8 INPUT TEST
DI1: 1
DI2: 1
DI3: 1
DI4: 1
DI5: 1
DI6: 1
DI7: 1
DI8: 1
```
When the DC input is applied/changed on DI1, it changes to LOW:

```
NORVI
X-DI8 INPUT TEST
DI1: 0
DI2: 1
DI3: 1
DI4: 1
DI5: 1
DI6: 1
DI7: 1
DI8: 1
```

### Serial Monitor
Only the input that changes is printed:
```
DI1 = 0

```
The TFT displays the status of all eight inputs, while the Serial Monitor prints only the inputs when their state changes.

---

## How to Validate the Example

To verify the example works correctly:

1. Apply a **valid input signal**
2. Observe the **displayed output**
3. Change the **input condition**
4. Confirm the **output changes accordingly**

---

## Device Preparation / Configuration

Before running the example ensure:

- The controller is **powered correctly**
- Required **signals are connected**
- Any required **configuration is completed**

Example:

**Power Supply:** 24V DC  
**Digital Input Range:** 24V

---

## Required Libraries

Install the following libraries before compiling:

- Wire
- WiFi
- SPI
- TFT_eSPI
- CST816S
- Free_Fonts.h

Library Download:  
https://github.com/NORVIControllers/NORVI-X-Version-02/tree/main/libraries 

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

**Test Date:** 2026-08-31  
**Verified By:** Kaveesha

---

## Support

Documentation:  
https://norvi.io/docs

Datasheet:  
https://norvi.io/docs/norvi-x-di8-datasheet/ 

For additional support or inquiries, contact the **NORVI support team**.

---

## License

This example is provided for **development and educational purposes**.

