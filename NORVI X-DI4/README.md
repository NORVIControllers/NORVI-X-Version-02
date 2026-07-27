# NORVI X – DI4 Digital Input Example

## Overview
This example demonstrates how to use the **Digital Input Expansion Module (DI4)** on the **NORVI X platform**.

The purpose of this example is to show how to:

- Integrate the **DI4 Module** with the **NORVI X CPU**
- Read the status of **4 digital inputs**
- Display the **live input states on the TFT display**

This example can be used as a starting point for developers building applications with **NORVI controllers**.

---

## Product Used

**Product:** NORVI X Digital Input Expansion Module (DI4)  
**Platform:** NORVI X CPU  

Website: https://norvi.io

---

## Purpose of This Example

The example demonstrates how to:

- Read **4 digital input channels** from the DI4 module  
- Display the **ON/OFF state as digital values (0 or 1)**  
- Verify the correct operation of the digital input expansion

The program continuously reads:

- INPUT 1  
- INPUT 2  
- INPUT 3  
- INPUT 4  

The current input states are shown on the **built-in TFT display**.

---

## Running the Example

Follow the steps below to run the example.

### 1. Hardware Connections
Connect the **NORVI X CPU** and **DI4 expansion module** correctly.

Connect digital input signals to:

- INPUT 1  
- INPUT 2  
- INPUT 3  
- INPUT 4  

Ensure the signal is within the **supported input range**.

---

### 2. Configure the Program
No additional user configuration is required before uploading this example.

---

### 3. Upload the Program

1. Open the example program in **Arduino IDE / PlatformIO**
2. Select the correct **board** and **port**
3. Upload the program to the **NORVI controller**

---

### 4. Observe the Display

After uploading, the **TFT display** will show the status of the four digital inputs.

When the program runs successfully:

- The controller initializes the display
- The TFT screen shows the **DI4 test screen**
- The status of each input is displayed as **0 or 1**
- Values update continuously as inputs change

#### Example Output

```
INPUT 1 : 0
INPUT 2 : 1
INPUT 3 : 1
INPUT 4 : 1
```

---

## How to Validate the Example

To verify that the example works correctly:

1. Apply a valid digital input signal
2. Observe the displayed output
3. Change the input condition
4. Confirm the output changes accordingly

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

**Test Date:** 2026-07-27  
**Verified By:** Kaveesha

---

## Documentation

NORVI Documentation:  
https://norvi.io/docs

Datasheet:
https://norvi.io/docs/norvi-x-di4-datasheet/ 
---

## Support

For additional support or inquiries, contact the **NORVI support team**.

---

## License

This example is provided for **development and educational purposes**.

