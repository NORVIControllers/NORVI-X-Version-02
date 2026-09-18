# NORVI X-R8 Relay Expansion Example

## Overview
This example demonstrates how to use the **Relay Expansion Module (R8)** on the **NORVI X CPU**. It serves as a functional starting point for developers building applications with NORVI controllers.

- **Product:** NORVI X Digital Input Expansion Module (R8)  
- **Platform:** NORVI X CPU  
- **Website:** https://norvi.io

---

## Purpose of This Example
This example shows how to:

- Integrate the R8 Module into the NORVI X CPU  
- Control and monitor the status of 8 relay outputs  
- Initialize the PCA9538 I/O Expander at address `0x73`  
- Use the `TFT_eSPI` library to display real-time status updates  
- Implement a basic I2C scanner to detect connected peripherals  

The program continuously reads 

- **NO1 to NO8**

The current input states are shown on the **built-in TFT display** and **Serial Monitor**.

---
## Running the Example

### 1. Hardware Connections

1. Connect the **NORVI X CPU** and **R8 expansion module** correctly.  
2. Connect relay output signals to:

   - **NO1** to **NO8**
     
Ensure the signal is within the **supported output range**.
---

### 2. Program Configuration

- **I2C Pins:**  
  - `SDA` → Pin 8  
  - `SCL` → Pin 9  

#### Address Selection via DIP Switch

| DIP 1 | DIP 2 | DIP 3 | DIP 4 | Address |
|-------|-------|-------|-------|---------|
| OFF   | OFF   | -      | -      | 0x73    |
| ON    | OFF   | -      | -      | 0x72    |
| OFF   | ON    | -      | -      | 0x71    |
| ON    | ON    | -      | -      | 0x70    |

---

### 3.Upload the Program

1. Open the example program in **Arduino IDE** or **PlatformIO**.  
2. Select the correct board and port.  
3. Upload the program to the NORVI controller.  

---

### Open Serial Monitor

- Set **Baud Rate** to `115200`.  

---

### 5. Observe the Display

When the program runs successfully:

- The controller initializes the display  
- The TFT screen shows the **R8 test screen**  
- The status of each output (`RELAY 1 - RELAY 8`) is displayed as **ON/OFF**  
- Values update continuously as outputs change  

### Example Output on Serial Monitor
```
NPD-NORVI X-R8-V2 (N16R2)
Pattern: ALL ON / OFF
Pattern: SEQUENTIAL
Pattern: ODDS vs EVENS
Pattern: CHASER
Pattern: BINARY COUNTER
```

### Example Output on TFT Screen

```
NORVI
X-R8 TEST
----------------
MODE: ALL ON / OFF

ALL ON
```
Sequential

```
MODE: SEQUENTIAL

RELAY 1 : ON
RELAY 2 : OFF
...
RELAY 8 : OFF
```
Each relay turns ON one by one.

Odd / Even
```
MODE: ODDS vs EVENS
```
Odd relays ON:
```
R1 ON   R2 OFF
R3 ON   R4 OFF
R5 ON   R6 OFF
R7 ON   R8 OFF
```

Even relays ON:
```
R1 OFF  R2 ON
R3 OFF  R4 ON
R5 OFF  R6 ON
R7 OFF  R8 ON
```

Chaser
```
Relay 1 → 2 → 3 → ... → 8 → 7 → ... → 1.
```

Binary Counter
```
MODE: BINARY COUNTER

VAL: 0
VAL: 1
VAL: 2
...
VAL: 15
```
---

## How to Validate the Example

To verify the example works correctly:

1. Observe the outputs.  
2. Check the Serial Monitor for status updates.  
3. Confirm that changing the output state reflects correctly on both the TFT and Serial Monitor.  

---

## Device Preparation / Configuration

Before running the example, ensure:

- The controller is powered correctly  
- Required signals are connected  
- Any required configuration is completed

Example:

**Power Supply:** 24V DC  

---

## Required Libraries

Install the following libraries before compiling:

- Wire  
- WiFi  
- SPI  
- PCA9538 Library  
- TFT_eSPI  
- CST816S  

**Download Libraries:**  
https://github.com/NORVIControllers/NORVI-X-Version-02/tree/main/libraries 

**Installation Steps:**

1. Open **Arduino IDE**  
2. Go to **Library Manager**  
3. Search and install the required libraries

---

## Limitations

- This example is provided for **demonstration purposes**  
- Additional calibration may be required for **precise measurements**  
- Performance may depend on **sensor accuracy** and environmental conditions

---

## Safety Notes

- Do **not exceed the rated input voltage**  
- Ensure **proper grounding**  
- Incorrect wiring may **damage the controller**
---

## Test Information

- **Test Date:** 2026-8-31 
- **Verified By:** Kaveesha  

---

## Support

Documentation: https://norvi.io/docs  
  
Datasheet: https://norvi.io/docs/norvi-x-r8-datasheeet/ 

For additional support, contact the NORVI support team.  

---

## License

This example is provided for **development and educational purposes**.

