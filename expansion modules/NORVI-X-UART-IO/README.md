# NORVI X-UARTIO Multi-Interface Communication Expansion
## Overview
This repository contains example firmware and documentation for the **NORVI X-UARTIO** expansion module. 
This module enables the **NORVI X CPU** to interface with industrial devices via RS485, RS232, and UART protocols.

The purpose of this example is to show how to: 

- Integrate the X-UARTIO module into the NORVI X CPU.
- Configure and switch between communication channels via firmware.
- Utilize shared MCU I/O lines for multi-port serial communication.
- Verify data transmission across different interface groups.

## Product Used

It serves as a starting point for developers building applications with NORVI controllers.

- **Product:** NORVI X-UARTIO Multi-interface Expansion Module 
- **Platform:** NORVI X CPU  
- **More Information:** https://norvi.io

---

## Purpose of This Example
The example demonstrates how to:

- Use RS485, RS232, and UART interfaces
- Configure serial communication
- Connect and interface multiple devices
- Utilize system communication ports effectively

---
## Running the example
### 1. Hardware Connections

Connect the **NORVI X CPU** and **UARTIO expansion module** correctly.  

Example connection:  
   - Signal Source (+) → Analog Input  
   - Signal Source (GND) → Controller GND

- Testing RS485: Connect an RS485-to-USB converter to terminals 1 (RS485-1A) and 2 (RS485-1B).
- Testing RS232: Connect an RS232-to-USB converter to terminals 5 (RS232-1RX) and 6 (RS232-1TX).
- UART Loopback: Use jumper wires to short UART-1RX (Pin 9) to UART-1TX (Pin 10) 
- Common Ground: Connect the ground of your external converters to GND (Pin 17 or 18) to ensure a shared reference.

### 2. Program Configuration

Communication Selection
|P1_3 | P1_4 |Selected Interface |
|------|------|------|
| 0 |  X | RS485  |
| 1  |  X | UART1 | 
|  X | 0  | RS232  | 
|  X  | 1  | UART2 | 

The datasheet defines P1_3 as the selector for RS485/UART1 and P1_4 as the selector for RS232/UART2. 

### 3. Upload the Program

1. Open the example program in **Arduino IDE** or **PlatformIO**  
2. Select the correct board and port  
3. Upload the program to the NORVI controller

---

### 4. Open Serial Monitor

- Set **Baud Rate** to `115200`  

---
### Example Output on Serial Monitor

When the program runs successfully:

Startup
```
========= COMMAND MENU =========
1 -> RS485 / RS232 MODE
2 -> UARTA / UARTB MODE
================================

Scanning I2C...
Found I2C: 0x75
I2C Scan Done

```
If you enter 1
```
MODE SELECTED: 1
MODE CHANGED
MODE 1: RS485 + RS232

RS485 01 SUCCESS
RS232 01 SUCCESS
```
This repeats continuously.

If you enter 2

```
MODE SELECTED: 2
MODE CHANGED
MODE 2: UARTA + UARTB

UARTA SUCCESS
UARTB SUCCESS

```
This also repeats continuously.
## Testing & Validation

### Mode 1: RS485 + RS232
* **Input:** `1`
* **Validation:** Confirm `RS485 01 SUCCESS` and `RS232 01/02 SUCCESS` appear in the monitor.

### Mode 2: UART Loopback
* **Input:** `2`
* **Validation:** Confirm `UARTA` and `UARTB` success messages appear (requires pins 9 and 10 to be shorted).

### Mode 3: RS485 Alternate
* **Input:** `3`
* **Validation:** Confirm `RS485 02 SUCCESS` appears, indicating the alternate RS485 path is active.

---

## Device Preparation / Configuration

Before running the example, ensure:

- The controller is powered correctly  
- Required signals are connected  
- Any required configuration is completed  

**Example:**

- Power Supply: 24 V DC  

---

## Required Libraries

Install the following libraries before compiling:

- Wire  
- PCA9539 

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

- **Test Date:** 2026-08-31 
- **Verified By:** Kaveesha  

---

## Support

Documentation: https://norvi.io/docs 
   
For additional support, contact the NORVI support team  

---

## License

This example is provided for **development and educational purposes**.






