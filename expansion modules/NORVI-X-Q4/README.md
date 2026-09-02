# NORVI X – Q4 Transistor Output Example

## Overview
This example demonstrates how to use the **Transistor Outputs (Fast Transistor Outputs) Module (Q4)** on the **NORVI X platform**.

The purpose of this example is to show how to:

- Integrate and control the output modules on the **NORVI X CPU**
- Initialize and use the integrated **TFT display** using the **TFT_eSPI library**
- Manage the Direct GPIO digital outputs (TR1–TR4) on the NORVI X CPU.

This example can be used as a starting point for developers building applications with **NORVI controllers**.

---

## Product Used

**Product:** NORVI X Transistor Output Module (Q4)  
**Platform:** NORVI X CPU  

Website: https://norvi.io

---

## Purpose of This Example

The example demonstrates:

- **Control:** The controller cycles four outputs (**TR1 – TR4**) between HIGH and LOW states  
- **Observation:** The user can observe the "ON/OFF" status for each output directly on the **TFT screen** and via the **Serial Monitor**

The program continuously reads:

- OUTPUT 1: ON/OFF  
- OUTPUT 2: ON/OFF  
- OUTPUT 3: ON/OFF  
- OUTPUT 4: ON/OFF  

The current output states are shown on the **built-in TFT display** and **Serial Monitor**.

---

## Running the Example

Follow the steps below to run the example.

### 1. Hardware Connections

Connect the **NORVI X CPU** and **Q4 expansion module** correctly.

Connect transistor output signals to:

- **TR1 – TR4**

Ensure the signal is within the **supported output range**.

---

### 2. Configure the Program

The four transistor outputs are controlled directly using the NORVI X CPU GPIO pins.

| Output | GPIO Pin |
|---|---:|
| TR1 | GPIO 14 |
| TR2 | GPIO 35 |
| TR3 | GPIO 41 |
| TR4 | GPIO 37 |

---

### 3. Upload the Program

1. Open the example program in **Arduino IDE / PlatformIO**
2. Select the correct **board** and **port**
3. Upload the program to the **NORVI controller**

---

### 4. Open Serial Monitor

Set the **Baud Rate** to:

```
115200
```

---

### 5. Observe the Display

After uploading, the **TFT display** will show the status of the four transistor outputs.

When the program runs successfully:

- The controller initializes the display
- The TFT screen shows the **Q4 test screen**
- Values update continuously as outputs change

---

## Example Output

### Serial Monitor

```
OUTPUT 1 : ON
OUTPUT 1 : OFF
OUTPUT 2 : ON
OUTPUT 2 : OFF
OUTPUT 3 : ON
OUTPUT 3 : OFF
OUTPUT 4 : ON
OUTPUT 4 : OFF
```

### TFT Display

```
OUTPUT 1 : ON OFF  
OUTPUT 2 : ON OFF
OUTPUT 3 : ON OFF
OUTPUT 4 : ON OFF
```

---

## How to Validate the Example

To verify the example works correctly:

1. Power on device – TFT displays **"NORVI"** and **"EXPE-QX TEST"**  
2. Observe **TR1 – TR4**: LED/load toggles every 500ms  
3. Check **Serial Monitor**: Status messages appear at **115200 baud**  
4. Monitor **TFT**: Real-time "ON/OFF" indicators update for each output  

---

## Device Preparation / Configuration

Before running the example ensure:

- The controller is **powered correctly**
- Required **signals are connected**
- Any required **configuration is completed**

Example:

**Power Supply:** 24V DC  

---

## Required Libraries

Install the following libraries before compiling:

```
Wire
WiFi
SPI
TFT_eSPI
CST816S
```

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

- Do not exceed the **rated output voltage**  
- Ensure **proper grounding**  
- Incorrect wiring may **damage the controller**  

---

## Test Information

**Test Date:** 2026-09-02  
**Verified By:** Kaveesha  

---

## Documentation

NORVI Documentation:  https://norvi.io/docs   

Datasheet:  https://norvi.io/docs/norvi-x-q4-datasheet/  

---

## Support

For additional support or inquiries, contact the **NORVI support team**.

---

## License

This example is provided for **development and educational purposes**.

