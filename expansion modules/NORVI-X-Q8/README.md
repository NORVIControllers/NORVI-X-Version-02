# NORVI X – Q8 Transistor Output Expansion Example

## Overview
This example demonstrates how to use the **Transistor Outputs Expansion Module (Q8)** on the **NORVI X platform**.

The purpose of this example is to show how to:

- Integrate the **Q8 Module** into the NORVI X CPU  
- Read the status of **8 transistor outputs**  
- Integrate the **PCA9538 I/O expander** to control transistor outputs  
- Use the **TFT_eSPI library** to display real-time status updates  
- Implement a basic **I2C scanner** to detect connected peripherals  

This example can be used as a starting point for developers building applications with **NORVI controllers**.

---

## Product Used

**Product:** NORVI X Transistor Output Expansion Module (Q8)  
**Platform:** NORVI X CPU  

Website: [https://norvi.io](https://norvi.io)

---

## Purpose of This Example

The example demonstrates:

- **Control Outputs:** Sequentially toggle eight transistor outputs (**TR1 – TR8**) via the PCA9538 expander  
- **Visual Feedback:** Display the current "ON/OFF" status of each output on the integrated TFT screen  
- **System Diagnostics:** Print the status of each output and I2C scan results to the Serial Monitor  

The program continuously updates:

- **TR1 to TR8**  

The current output states are shown on the **built-in TFT display** and **Serial Monitor**.

---

## Running the Example

### 1. Hardware Connections

1. Connect the **NORVI X CPU** and **Q8 expansion module** correctly  
2. Connect transistor output signals to **TR1 – TR8**  
3. Ensure the I2C is correctly mapped

---
Load Connection

Since the Q8 uses high-side switching, connect the load between the transistor output and GND.

For TR1:
Q8 TR1  →  Load (+)
Q8 GND  →  Load (-)

For TR2:
Q8 TR2  →  Load (+)
Q8 GND  →  Load (-)

The same connection applies to TR3–TR8.

Important: Ensure that the connected load does not exceed the specified output current. For inductive loads such as solenoids and relays, use appropriate suppression/protection as required by the load manufacturer.

### 2. Configure the Program

Ensure the **I2C pins** are correctly defined:

- **SDA → Pin 8**  
- **SCL → Pin 9**

---

### 3. I2C Address Selection

The I2C address of the module can be configured using the DIP switches:

| DIP 1 | DIP 2 | DIP 3 | DIP 4 | I2C Address |
|-------|-------|-------|-------|-------------|
| OFF   | OFF   | –     | –     | 0x73        |
| ON    | OFF   | –     | –     | 0x72        |
| OFF   | ON    | –     | –     | 0x71        |
| ON    | ON    | –     | –     | 0x70        |


---

### 4. Upload the Program

1. Open the example program in **Arduino IDE / PlatformIO**  
2. Select the correct **board** and **port**  
3. Upload the program to the **NORVI controller**

---

### 5. Open Serial Monitor

Set the **Baud Rate** to:

```
115200
```

---

### 6. Observe the Display

When the program runs successfully:

- The controller initializes the TFT display  
- The screen shows **Q8 test screen**  
- The status of each output (**TR1 – TR8**) is displayed as **ON/OFF**  
- Values update continuously as outputs change  

---

## Example Output

### Serial Monitor

```
TR1 : ON
TR1 : OFF
TR2 : ON
TR2 : OFF
...
TR8 : ON
TR8 : OFF
```
The complete sequence repeats continuously.

### TFT Display

```
NORVI
EXPE-Q8 TEST

TR 1 :  ON   OFF
TR 2 :  ON   OFF
TR 3 :  ON   OFF
TR 4 :  ON   OFF
TR 5 :  ON   OFF
TR 6 :  ON   OFF
TR 7 :  ON   OFF
TR 8 :  ON   OFF

```
The ON and OFF indications appear sequentially as each output is tested.

---

## How to Validate the Example
 
1. Observe the outputs on the **TFT screen**  
2. Check the **Serial Monitor** for status updates  
3. Confirm that changing the output state reflects correctly in both TFT and Serial Monitor  

---

## Device Preparation / Configuration

Before running the example:

- Ensure the controller is **powered correctly**  
- Required signals are **connected**  
- Any necessary configuration is **completed**

Example:

**Power Supply:** 24V DC  

---

## Required Libraries

Install the following libraries before compiling:

```
Wire
WiFi
SPI
PCA9538
TFT_eSPI
CST816S
Free_Fonts
```

Library download:  https://github.com/NORVIControllers/NORVI-X-Version-02/tree/main/libraries 

**Installation Steps:**

1. Open **Arduino IDE**  
2. Go to **Library Manager**  
3. Search and install the required libraries  

---

## Limitations

- This example is provided for **demonstration purposes**  
- Additional calibration may be required for **precise operation**  
- Performance depends on **sensor accuracy and environmental conditions**  

---

## Safety Notes

- Do not exceed the **rated input/output voltage**  
- Ensure proper **grounding**  
- Incorrect wiring may **damage the controller**

---

## Test Information

**Test Date:** 2026-09-04   
**Verified By:** Kaveesha  

---

## Documentation

NORVI Documentation:  https://norvi.io/docs/norvi-x-r8-datasheeet/ 

Datasheet:   https://norvi.io/docs/norvi-x-q8-datasheet/ 


---

## Support

For additional support or inquiries, contact the **NORVI support team**.

---

## License

This example is provided for **development and educational purposes**.

