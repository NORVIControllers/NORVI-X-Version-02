# NORVI X-CPU-ESPS3-X3 Example

## Overview
This example demonstrates how to use various interfaces including **I2C, RS485, Ethernet, GSM, and a TFT Touch Display** on the **NORVI X-CPU-ESPS3-X3**.

**Product Used**  
- Product: NORVI X-CPU-ESPS3-X3  
- Expansion Module: NORVI X Digital Input Expansion Module  
- More information: https://norvi.io

### Purpose of This Example
The example demonstrates how to:
- Configure and control onboard and expansion IO ports using the PCA9539.  
- Manage I/O expansions using the PCA9539 controller to handle multiple inputs and outputs.  
- Establish RS-485 communication utilizing manual flow control for industrial data exchange.  
- Interface with the integrated TFT display and CST816S touch controller, including benchmark performance testing.  
- Coordinate peripheral hardware, specifically the DS3231 Real-Time Clock (RTC) and SD card storage.  
- Perform system diagnostics through an automated I2C bus scan to detect connected modules.  
- Configure Ethernet with NTP time synchronization and set up a Wi-Fi Access Point.  

This example serves as a comprehensive diagnostic tool and a starting point for building complex applications with NORVI controllers.

---

## Running the Example

### 1. Hardware Connections
- **Power:** Supply 12–24V DC to the controller.  
- **Ethernet:** Plug in an Ethernet cable if testing network and NTP features.  
- **SD Card:** Insert a microSD card into the dedicated slot.  
- **RS485:** Connect your RS485 communication lines to the appropriate terminals.  
- **GSM:** Ensure a compatible GSM module is connected if testing cellular serial communication.

### 2. Configure the Program

### 3. Upload the Program
1. Open the example program in Arduino IDE / PlatformIO.  
2. Select the correct board and port.  
3. Upload the program to the NORVI controller.

### 4. Open Serial Monitor
- Set the **Baud Rate** to 115200.

---

#### Serial Monitor I2C Diagnostics
  When the program runs the `I2C_SCAN()` function, the following devices are detected:

  | I2C Address | Device | Description |
  |---|---|---|
  | 0x15 | CST816S | Touch Controller |
  | 0x41 | PCA9536 | I/O Expander (onboard buttons PB1/PB3 and LEDs) |
  | 0x68 | DS3231 | Real-Time Clock (RTC) |
  | 0x75 | PCA9539 | Base I/O Expander (Expansion Port Address) |

---

## How to Validate the Example
1. **Display & Touch Test** – verify the TFT screen and touch response.
   
2. **Network Port Check (Ethernet/NTP)**  
   - Action: Connect RJ45 cable to the network port.  
   - Result: If the internet is active, Serial Monitor displays:  
     `"The UTC time is [HH:MM:SS]"` retrieved via NTP.
      
4. **RS-485 Communication** – connect an RS-485 to USB converter to a PC.
   
5. **SIM Card Test** – insert a SIM card and check AT command responses in Serial Monitor.
   
 ### AT Commands – Quectel Cellular Engine

  | Command | Output | Description |
  |---|---|---|
  | AT | OK | Checks communication between terminal and modem |
  | AT+CPIN | +CPIN: READY | Shows SIM card status (READY = SIM unlocked and usable) |
  | AT+CSQ | +CSQ: 15,99 | Displays signal strength and quality |
  | AT+CFUN? | +CFUN: 1 | Queries current functionality level of the modem |
  | AT+CFUN=1 | OK | Sets modem functionality level |
  | AT+GMR | EC25EFAR02A09M4G | Displays product firmware/software version |

---

## Device Preparation / Configuration
Before running the example, ensure:
- The controller is powered correctly.  
- Required signals are connected.  
- Any required configuration is completed (e.g., power supply: 24V DC).

---

## Required Libraries
Install the following libraries before compiling:  
- Wire, WiFi, Ethernet, SD, SPI (standard libraries)  
- PCA9539  
- RTClib (for DS3231)  
- TFT_eSPI  
- CST816S  

**Download Libraries:** https://github.com/NORVIControllers/NORVI-X-Version-01/tree/main/libraries

**Installation Steps:**  
1. Open Arduino IDE.  
2. Go to **Library Manager**.  
3. Search and install the required libraries.

---

## Limitations
- This example is provided for demonstration purposes.  
- Additional calibration may be required for precise measurements.  
- Performance may depend on sensor accuracy and environmental conditions.
 
   ### Hardware Compatibility Note
    - Standard Product: ESP32-S3 (N16R2) – PSRAM via QSPI
    - Special Product: ESP32-S3 (N16R8) – PSRAM via OSPI
    - The NORVI X-Q4 expansion module is NOT supported with the Special Product (N16R8 variant)
---

## Safety Notes
- Do not exceed the rated input voltage.  
- Ensure proper grounding.  
- Incorrect wiring may damage the controller.

---

**Test Date:** 2026-03-16  

**Verified By:** Kaveesha  

---

## Support
- Documentation: https://norvi.io/docs  
- For additional support or inquiries, contact the NORVI support team.
- Datasheet: https://norvi.io/docs/norvi-x-cpu-esps3-x1-datasheet/
---

## License
This example is provided for **development and educational purposes**.
