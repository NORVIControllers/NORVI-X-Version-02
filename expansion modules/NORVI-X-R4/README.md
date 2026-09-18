# NORVI X-R4 Relay Expansion Module Example

This example demonstrates how to use the **Relay Expansion Module (R4)** on the **NORVI X platform**. The purpose of this example is to show how to integrate the R4 Module into the NORVI X CPU. 
This example can be used as a starting point for developers building applications with **NORVI controllers**.

**Product Used:** NORVI X Relay Expansion Module (R4)  
**Platform:** NORVI X CPU  
**Website:** https://norvi.io

## Purpose of This Example

This example demonstrates how to:

- Initialize the **PCA9538 I/O expander** at address `0x73`
- Control four relay outputs (**RELAY 1 – RELAY 4**) via **I2C**
- Display real-time relay status (**ON/OFF**) on the integrated **TFT display** and **Serial Monitor**

The program continuously reads 

- **NO1 to NO4**. 

The current input states are shown on the **built-in TFT display** and **Serial Monitor**.

## Running the Example

Follow the steps below to run the example.

### 1. Hardware Connections

Connect the **NORVI X CPU** and **R4 expansion module** correctly. 

Connect relay output signals to:

- **NO1 – NO4**

Ensure the signal is within the **supported output range**.

---

### 2. Configure the Program

Ensure the I2C pins are correctly defined:  
SDA → Pin 8  
SCL → Pin 9

#### Address Selection

| DIP 1 | DIP 2 | DIP 3 | DIP 4 | I²C Address |
|-------|-------|-------|-------|-------------|
| OFF   | OFF   | -     | -     | 0x73        |
| ON    | OFF   | -     | -     | 0x72        |
| OFF   | ON    | -     | -     | 0x71        |
| ON    | ON    | -     | -     | 0x70        |

---
### 3. Upload the Program

1. Open the example program in **Arduino IDE / PlatformIO**  
2. Select the correct **board and port**  
3. Upload the program to the **NORVI controller**

---
## Open Serial Monitor

Set the **baud rate** to `115200`.

---
### 5. Observe the Display

When the program runs successfully:

- The controller initializes the TFT display.
- The TFT display shows "NORVI X-R4 TEST".
- The relay outputs are switched through different test patterns.
- The current test pattern is displayed on the TFT.
- The Serial Monitor displays the active test pattern.

The program performs the following tests:

ALL ON / OFF 
SEQUENTIAL 
ODDS vs EVENS 
CHASER 
BINARY COUNTER 

### Example Output on the TFT Screen

1. All ON / OFF 
   All four relays are switched ON simultaneously and then switched OFF.

RELAY 1 → ON
RELAY 2 → ON
RELAY 3 → ON
RELAY 4 → ON

```
NORVI
X-R4 TEST
----------------
MODE: ALL ON / OFF
RELAY 1 → OFF
RELAY 2 → OFF
RELAY 3 → OFF
RELAY 4 → OFF
ALL ON

```
2. MODE: SEQUENTIAL 
   Each relay is switched ON and OFF one after another:

```
RELAY 1 → ON → OFF
RELAY 2 → ON → OFF
RELAY 3 → ON → OFF
RELAY 4 → ON → OFF
```
3. Odds vs Evens
   The program alternates between odd and even relay outputs.

```
MODE: ODDS vs EVENS
ODD:  R1 ON   R2 OFF   R3 ON   R4 OFF
EVEN: R1 OFF  R2 ON    R3 OFF  R4 ON

```
4. Chaser
   The relays are switched sequentially from Relay 1 to Relay 4 and then back from Relay 4 to Relay 1.
```
MODE: CHASER
RELAY 1 → ON
RELAY 2 → ON
RELAY 3 → ON
RELAY 4 → ON

```
The previous relay is switched OFF as the next relay turns ON.

5.  Binary Counter
    The four relays represent a 4-bit binary value from 0 to 15.
```
MODE: BINARY COUNTER

VAL: 0
VAL: 1
VAL: 2
...
VAL: 15

```
and continues up to:
VAL: 15

### Example Output on the Serial Monitor
```
Pattern: ALL ON / OFF
Pattern: SEQUENTIAL
Pattern: ODDS vs EVENS
Pattern: CHASER
Pattern: BINARY COUNTER
```
The I2C scan also displays the detected PCA9538: 
I2C device found at address 0x73

## How to Validate the Example

To verify the example works correctly:

1. Observe the output  
2. Check the **Serial Monitor** for status updates  
3. Confirm that changing the output state is reflected correctly in both the **TFT display** and **Serial Monitor**

## Device Preparation / Configuration

Before running the example, ensure:

- The controller is **powered correctly**  
- Required signals are **connected**  
- Any required **configuration** is completed

Example:

**Power Supply:** 24V DC  

## Required Libraries

Install the following libraries before compiling:

- Wire  
- WiFi  
- SPI  
- PCA9538 Library  
- TFT_eSPI  
- CST816S  

Download Libraries:  
https://github.com/NORVIControllers/NORVI-X-Version-02/tree/main/libraries 

### Installation

1. Open **Arduino IDE**  
2. Go to **Library Manager**  
3. Search and install the required libraries

## Limitations

- This example is provided for **demonstration purposes**  
- Additional calibration may be required for **precise measurements**  
- Performance may depend on **sensor accuracy** and environmental conditions

## Safety Notes

- Do **not exceed the rated input voltage**  
- Ensure **proper grounding**  
- Incorrect wiring may **damage the controller**

## Test Information

**Test Date:** 2026-08-31  
**Verified By:** Kaveesha

## Support

Documentation: https://norvi.io/docs  

Datasheet: http://norvi.io/docs/norvi-x-r4-datasheet/ 

For additional support or inquiries, contact the **NORVI support team**.

## License

This example is provided for **development and educational purposes**.

