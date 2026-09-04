# NORVI X Q16 Transistor Outputs Example

## Overview
This example demonstrates how to use the Transistor Outputs Expansion Module (Q16) on the NORVI X platform.

The purpose of this example is to show how to:
- Integrate the Q16 Module into the NORVI X CPU
- Read the status of 16 transistor outputs
- Initialize the PCA9555 I/O Expander
- Use the TFT_eSPI library to display real-time status updates
- Implement a basic I2C scanner to detect connected peripherals

This example can be used as a starting point for developers building applications with NORVI controllers.

## Product Used
Product: NORVI X Digital Output Expansion Module (Q16)  
Platform: NORVI X CPU  

Website: https://norvi.io

## Purpose of This Example
The example demonstrates how to:
- Scan the I2C bus to identify connected expansion modules
- Initialize and control 16 transistor outputs (TR1 to TR16) using the PCA9555 I/O expander
- Display real-time status updates on the TFT display

The program continuously reads TR1 to TR16.  
The current output states are shown on the built-in TFT display and Serial Monitor.

## Running the Example

### 1. Hardware Connections
Connect the NORVI X CPU and Q16 expansion module correctly.

Connect transistor output signals to:
TR1 to TR16

### 2. Configure the Program
Ensure the I2C pins are correctly defined:
SDA (Pin 8)  
SCL (Pin 9)
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

### 3. Upload the Program
- Open the example program in Arduino IDE / PlatformIO
- Select the correct board and port
- Upload the program to the NORVI controller

### 4. Open Serial Monitor
Set the baud rate to 115200.

### 5. Observe the Display
When the program runs successfully:
- The controller initializes the display
- The TFT screen shows the Q16 test screen
- The status of each output (TR1 - TR16) is displayed as ON/OFF
- Values update continuously as outputs change

### Example Output on the Serial Monitor
```
TR1 : ON  
TR1 : OFF  
TR2 : ON  
TR2 : OFF  
...  
TR16 : ON  
TR16 : OFF  
```
### Example Output on the TFT Screen
```
TR1 : ON OFF  
TR2 : ON OFF  
...  
TR16 : ON OFF  
```

## How to Validate the Example
To verify the example works correctly:
1. Observe the outputs
2. Check the Serial Monitor for status updates
3. Confirm that changing the output state reflects correctly on both the TFT display and Serial Monitor

## Device Preparation / Configuration
Before running the example ensure:
- The controller is powered correctly
- Required signals are connected
- Any required configuration is completed

Example:

**Power Supply:** 24V DC  

## Required Libraries
Install the following libraries before compiling:
- Wire
- WiFi
- SPI
- PCA9555 Library
- TFT_eSPI
- CST816S
- PCA9539
- PCA9536D
- RTClib

Download Libraries:  https://github.com/NORVIControllers/NORVI-X-Version-02/tree/main/libraries 

## Installation
1. Open Arduino IDE
2. Go to Library Manager
3. Search and install the required libraries

## Limitations
- This example is provided for demonstration purposes
- Additional calibration may be required for precise measurements
- Performance may depend on sensor accuracy and environmental conditions

## Safety Notes
- Do not exceed the rated input voltage
- Ensure proper grounding
- Incorrect wiring may damage the controller

## Test Information
Test Date: 2026-09-04 
Verified By: Kaveesha

## Support
Documentation: https://norvi.io/docs  

Datasheet: https://norvi.io/docs/norvi-x-q16-datasheet/ 

For additional support or inquiries, contact the NORVI support team.

## License
This example is provided for development and educational purposes.

