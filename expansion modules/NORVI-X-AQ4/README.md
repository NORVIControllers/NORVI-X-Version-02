# NORVI X Analog Output Expansion Module (AQ4) Example

## Overview
This example demonstrates how to use the **Analog Output Expansion Module (AQ4)** on the **NORVI X** platform.  

The purpose of this example is to show how to:  
- Integrate the **AQ4 Module** into the NORVI X CPU  
- Read the status of 4 analog outputs using a 12-bit Digital-to-Analog Converter (DAC)  
- Setup the display: Initialize **TFT_eSPI** and display pins  

This example can be used as a starting point for developers building applications with NORVI controllers.  

## Product Used

**Product Used:** NORVI X Analog Output Expansion Module (AQ4)  
**Platform:** NORVI X CPU  
More information: https://norvi.io

---

## Purpose of This Example
The example demonstrates how to:  
- Integrate the **AQ4 Module** into the NORVI X CPU
  
- Control and monitor the status of analog outputs
   
- Configure and control 4 channels of analog outputs
  
- Convert raw DAC values into actual voltage/current readings using a defined ratio
  
- Use the **TFT_eSPI** library to display real-time status updates
  
- Implement a basic I2C scanner to detect connected peripherals  

The program continuously reads: **AOUT 1 to AOUT 4**  

The current output states are shown on the built‑in TFT display and Serial Monitor.

---

## Running the Example

### 1. Hardware Connections
Connect the NORVI X CPU and AQ4 expansion module correctly.

**Example:** 

Signal Source (+) → Analog Output

Signal Source (GND) → Controller GND


Connect analog output signals to: **AOUT 1 to AOUT 4**  
Ensure the signal is within the supported output range.

---

### 2. Configure the Program
Ensure the I2C pins are correctly defined:  
**SDA → Pin 8**  
**SCL → Pin 9**

#### Address Selection (via DIP switches)

| DIP 1 | DIP 2 | DIP 3 | DIP 4 | Address |
|-------|-------|-------|-------|---------|
| OFF   | OFF   | OFF   | OFF   | 0x5F    |
| ON    | OFF   | OFF   | OFF   | 0x57    |
| OFF   | ON    | OFF   | OFF   | 0x5B    |
| ON    | ON    | OFF   | OFF   | 0x53    |
| OFF   | OFF   | ON    | OFF   | 0x5D    |
| ON    | OFF   | ON    | OFF   | 0x55    |
| OFF   | ON    | ON    | OFF   | 0x59    |
| ON    | ON    | ON    | OFF   | 0x51    |
| OFF   | OFF   | OFF   | ON    | 0x5E    |
| ON    | OFF   | OFF   | ON    | 0x56    |
| OFF   | ON    | OFF   | ON    | 0x5A    |
| ON    | ON    | OFF   | ON    | 0x52    |
| OFF   | OFF   | ON    | ON    | 0x5C    |
| ON    | OFF   | ON    | ON    | 0x54    |
| OFF   | ON    | ON    | ON    | 0x58    |
| ON    | ON    | ON    | ON    | 0x50    |

---

### 3. Upload the Program
- Open the example program in **Arduino IDE** / **PlatformIO**  
- Select the correct board and port  
- Upload the program to the NORVI controller  

---

### 4. Open Serial Monitor
Set the baud rate to **`115200`**.

#### Serial Command Format

The program accepts commands in the following format: 
CHx=value

where:  
CH identifies an analog output channel
x is the channel number from 1 to 4
value is the requested analog output value

Examples  

For CH1:  

               CH1=5
               This requests:
               CH1 = 5 V  
               

For CH3:  

               CH3=12 
               This requests:
               CH3 = 12 mA 

---

### 5. Observe the Display
When the program runs successfully:
- The controller initializes the display  
- The TFT display will show: `"NORVI EXPE-AQ4 TEST"`  
- The status of each output is displayed  
- Values update continuously as outputs change  
- The Serial Monitor displays the confirmation of the command, requested value, and corresponding 12-bit DAC code (0–4095)  

**Example Output on Serial Monitor:**

After uploading and starting the program, the Serial Monitor displays:

```

====================================
  ESP32 DAC SERIAL CONTROL READY
====================================
CH1=5      (0-10V)
CH2=7.5    (0-10V)
CH3=12     (0-20mA)
CH4=20     (0-20mA)
====================================

```
For example, entering:
CH1=5
produces: 
```
CH1 = 5.00 V | DAC Code = 2047
```
---

## How to Validate the Example
1. Apply a valid output signal to one of the analog channels.  
2. Observe the output on the TFT screen or Serial Monitor to see the voltage/current change.  
3. Increase or decrease the signal and confirm the displayed value updates accordingly.  
4. Confirm that changing the output state reflects correctly in both TFT and Serial Monitor.

---

## Device Preparation / Configuration
Before running the example, ensure:  
- The controller is powered correctly  
- Required signals are connected  
- Any required configuration is completed  

**Example:**  

Power Supply: 24V DC

Analog Output Voltage: 0–10 V / 4–20 mA


---

## Required Libraries
Install the following libraries before compiling:  
- Wire  

**Download Libraries:**  https://github.com/NORVIControllers/NORVI-X-Version-01/tree/main/libraries

**Installation:**
1. Open Arduino IDE  
2. Go to Library Manager  
3. Search and install the required libraries  

---

## Limitations
- This example is provided for demonstration purposes  
- Additional calibration may be required for precise measurements  
- Performance may depend on sensor accuracy and environmental conditions  

---

## Safety Notes
- Do not exceed the rated input voltage/current  
- Ensure proper grounding  
- Incorrect wiring may damage the controller  

---

**Test Date:** 2026-08-31  
**Verified By:** Kaveesha  

---

## Support

Documentation: https://norvi.io/docs

For additional support or inquiries, contact the NORVI support team 

Datasheet: https://norvi.io/docs/norvi-x-aq4-datasheet/

---

## License
This example is provided for development and educational purposes.


