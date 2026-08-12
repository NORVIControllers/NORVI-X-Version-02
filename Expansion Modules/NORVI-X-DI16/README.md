
# NORVI X – DI16 Digital Input Example

## Overview
This example demonstrates how to use the **Digital Input Expansion Module (DI16)** on the **NORVI X platform**.

The purpose of this example is to show how to:

- Integrate the **DI16 Module** with the **NORVI X CPU**
- Read the status of **16 digital inputs**
- Display the **live input states on the TFT display and Serial Monitor**

This example can be used as a starting point for developers building applications with **NORVI controllers**.

---

## Product Used

**Product:** NORVI X Digital Input Expansion Module (DI16)  
**Platform:** NORVI X CPU  

Website: https://norvi.io

---

## Purpose of This Example

The example demonstrates how to:

- Interface with the **PCA9555 I/O expander** at **I2C address 0x27**
  
- Read **16 digital input channels (INPUT1 – INPUT16)**
- Verify correct operation of the **digital input expansion module**

The program continuously reads:

- **I1 to I16**

The current input states are shown on the **built-in TFT display and Serial Monitor**.

---

## Running the Example

Follow the steps below to run the example.

---

## 1. Hardware Connections

Connect the **NORVI X CPU** and **DI16 expansion module** correctly.

Connect digital input signals to:

- **IN1 to IN16**

Ensure the signal is within the **supported input range**.

---

## 2. Configure the Program

Ensure the **I2C pins** are correctly defined:

- **SDA → Pin 8**
- **SCL → Pin 9**

I2C Address Selection

The I2C address of the module can be configured using the DIP switches.

| DIP 1 | DIP 2 | DIP 3 | DIP 4 | I2C Address |
|------|------|------|------|-------------|
| OFF | OFF | OFF | – | 0x27 |
| ON  | OFF | OFF | – | 0x26 |
| OFF | ON  | OFF | – | 0x25 |
| ON  | ON  | OFF | – | 0x24 |
| OFF | OFF | ON  | – | 0x23 |
| ON  | OFF | ON  | – | 0x22 |
| OFF | ON  | ON  | – | 0x21 |
| ON  | ON  | ON  | – | 0x20 |


---

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

After uploading, the **Serial Monitor** will show the status of the digital inputs.

When the program runs successfully:

- The controller initializes successfully.
- The Serial Monitor displays the test status and input readings.
- The status of each input is displayed as 0 or 1 in the Serial Monitor.
- Input values update continuously as the input signals change.
---

## Example Output

### Serial Monitor

```
INPUT 1 : 0
INPUT 2 : 1
INPUT 3 : 0
...
INPUT 16 : 1
```

## How to Validate the Example

To verify the example works correctly:

1. Apply a **valid input signal**
2. Observe the **output**
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
- clsPCA9555
- TFT_eSPI
- CST816S

Library Download:  
https://github.com/NORVIControllers/NORVI-X-Version-01/tree/main/libraries

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

**Test Date:** 2026-07-27

**Verified By:** Kaveesha

---

## Documentation

NORVI Documentation:  
https://norvi.io/docs

Datasheet:  
https://norvi.io/docs/norvi-x-di16-datasheet/

---

## Support

For additional support or inquiries, contact the **NORVI support team**.

---

## License

This example is provided for **development and educational purposes**.
