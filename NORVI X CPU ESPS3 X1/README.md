# NORVI X-CPU-ESPS3-X1 Example

## Overview
The **NORVI X-CPU-ESPS3-X1** is a high-performance programmable controller designed for **IoT and industrial automation applications**, featuring built-in **WiFi, Ethernet, and Bluetooth connectivity**.

This controller functions as a **central hub for automation systems**, supporting **RS-485 communication** and large-scale expansion with **up to 200 digital inputs and outputs** through modular I/O expansion.

---

## Product Used
- **Product:** NORVI X-CPU-ESPS3-X1  
 
- **More Information:** https://norvi.io

---

## Purpose of This Example
This example demonstrates how to:

- Configure and control **onboard and expansion I/O ports** using the **PCA9539** I/O expander.
- Manage multiple inputs and outputs through the **PCA9539 expansion controller**.
- Establish **RS-485 communication** using **manual flow control** for industrial data exchange.
- Interface with the integrated **TFT display** and **CST816S touch controller**, including performance benchmark testing.
- Coordinate peripheral hardware including:
  - **DS3231 Real-Time Clock (RTC)**
  - **SD card storage**
- Perform **system diagnostics** using an **automated I2C bus scan** to detect connected peripherals.

---

## Running the Example

Follow the steps below to run the example.

---

### 1. Hardware Connections

- Power Supply: Connect a 24V DC power supply to the controller's power terminals.

- USB Connection: Connect the controller to your PC via USB for programming and to access the Serial Monitor.

---

### 2. Configure the Program
Ensure all required libraries are installed and board settings are correctly configured in **Arduino IDE / PlatformIO**.

---

### 3. Upload the Program
1. Open the example program in **Arduino IDE / PlatformIO**
2. Select the **correct board and COM port**
3. Upload the program to the **NORVI controller**

---

### 4. Open Serial Monitor
Set the **baud rate to 115200**.

---

### Program Operation

When the program runs successfully, the system performs an **I2C bus scan** and displays detected devices in the **Serial Monitor**.

#### Serial Monitor – I2C Diagnostics

The `I2C_SCAN()` function detects the following devices:

| Address | Device |
|-------|-------|
| **0x15** | CST816S Touch Controller |
| **0x41** | PCA9536 I/O Expander (Buttons PB1/PB3 and LEDs) |
| **0x68** | DS3231 Real-Time Clock (RTC) |
| **0x75** | PCA9539 Base I/O Expander |

---

## How to Validate the Example

### 1. Display & Touch Test
Verify that the **TFT display initializes** and the **touch controller responds** correctly.

### 2. Network Port Check (Ethernet / UTC)

**Action**
- Connect an **RJ45 Ethernet cable** to the controller.

**Result**
- If internet access is available, the **Serial Monitor prints UTC time** retrieved via **NTP**:

---

### 3. RS-485 Communication

**Action**
- Connect an **RS-485 to USB converter** between the controller and a PC.

**Result**
- Serial data can be transmitted and monitored through the RS-485 interface.

---

## Device Preparation / Configuration

Before running the example ensure:

- The controller is **powered correctly**
- All **required signals are connected**
- Any necessary **configuration is completed**

**Example Configuration**

- Power Supply: **24V DC**

---

## Required Libraries

Install the following libraries before compiling.

- `Wire`
- `WiFi`
- `Ethernet`
- `SPI`
- `PCA9539`
- `RTClib` (for DS3231 RTC)
- `TFT_eSPI`
- `CST816S`

Download Libraries:  https://github.com/NORVIControllers/NORVI-X-Version-01/tree/main/libraries

---

## Library Installation

1. Open **Arduino IDE**
2. Navigate to **Library Manager**
3. Search and install the required libraries

---

## Limitations

- This example is intended for **demonstration purposes**
- Additional **calibration or optimization** may be required for production systems
- Performance may depend on **sensor accuracy and environmental conditions**

  ### Hardware Compatibility Note
  - Standard Product: ESP32-S3 (N16R2) – PSRAM via QSPI
  - Special Product: ESP32-S3 (N16R8) – PSRAM via OSPI
  - The NORVI X-Q4 expansion module is NOT supported with the Special Product (N16R8 variant)

---

## Safety Notes

- Do **not exceed rated input voltage**
- Ensure **proper grounding**
- Incorrect wiring may **damage the controller**

---

## Test Information

- **Test Date:** 2026-03-10  
- **Verified By:** Kaveesha

---

# Support

Documentation:  https://norvi.io/docs

For additional support or inquiries, contact the **NORVI support team**.

Datasheet:  https://norvi.io/docs/norvi-x-cpu-esps3-x1-datasheet/


---

# License
This example is provided for **development and educational purposes**.
