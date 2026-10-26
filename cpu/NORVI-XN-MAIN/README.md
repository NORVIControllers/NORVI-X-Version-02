# NORVI XN-MAIN – System Test Example

## Overview

The NORVI XN-MAIN is a high-performance programmable controller designed for IoT and industrial automation applications, featuring built-in WiFi, Ethernet, and Bluetooth connectivity.

This controller functions as a central hub for automation systems, supporting RS-485 communication and large-scale expansion with up to 200 digital inputs and outputs through modular I/O expansion.

This example demonstrates how to run a full board bring-up and diagnostics test on the NORVI XN-MAIN, the NORVI X modular controller built around the ESP32-S3-WROOM-1U-N16R2.

The purpose of this example is to show how to:

- Verify the on-board peripherals of the XN-MAIN with a single sketch
- Exercise the 50-pin expansion port outputs
- Display live touch, RTC and I/O status on the TFT display

This example can be used as a starting point for developers building applications with NORVI controllers.

## Product Used

Product: NORVI XN-MAIN (ESP32-S3-WROOM-1U-N16R2)

Platform: NORVI X

https://norvi.io

## Purpose of This Example

The example demonstrates how to:

- Scan the I2C bus and identify the on-board devices
- Read the touch controller and show gestures and X/Y coordinates
- Control the RUN and ERROR indicators and read the P1 / P2 push buttons
- Switch the expansion port outputs one by one (walking test)
- Read the DS3231 real-time clock
- Mount the microSD card and read its type and size
- Obtain an IP address over Ethernet (DHCP) and request the time from an NTP server
- Start a Wi-Fi SoftAP
- Send a test message over RS-485
- Benchmark the TFT display

The program runs the tests in the following order during `setup()`:

| # | Test | Result shown on | 
|---|------|-----------------|
| 1 | I2C scan (touch, PCA9536, DS3231, PCA9539) | Serial Monitor |
| 2 | Touch controller version | Serial Monitor |
| 3 | PCA9536 IO test (RUN / ERROR indicators, P1 / P2 read-back) | Serial Monitor, LEDs |
| 4 | Output test (GPIO and PCA9539 outputs) | Serial Monitor, expansion port pins |
| 5 | DS3231 RTC (5 timestamps) | Serial Monitor |
| 6 | microSD card (mount, card type, size) | Serial Monitor |
| 7 | Ethernet (DHCP + NTP, UTC time) | Serial Monitor |
| 8 | Wi-Fi SoftAP | Serial Monitor |
| 9 | RS-485 test message | Serial Monitor |
| 10 | TFT benchmark (timings in microseconds) | TFT, Serial Monitor |

After the tests, a summary of all results is printed on the Serial Monitor. The TFT then shows the touch test (gesture, points, event, X, Y), a touch counter and the RTC time. The output walking test, the RUN / ERROR indicator test and the RS-485 message continue running in the main loop.

Follow the steps below to run the example:

### 1. Hardware Connections

- Power the XN-MAIN from a 24V DC supply (input range 9–28V DC) using the POWER terminals (− / +).
- Connect a USB cable from the USB port of the XN-MAIN to your computer for programming and the Serial Monitor.
- Insert a microSD card (optional, for the SD card test).
- Connect an Ethernet cable to a network with a DHCP server and internet access (optional, for the Ethernet and NTP test).
- Connect an RS-485 loopback or another RS-485 device that echoes received data to the RS-485 terminals A and B (optional, for the RS-485 echo test).
- To observe the output test, connect LEDs, a logic analyzer or a multimeter to the expansion port output pins listed under **Device Preparation / Configuration**.

### 2. Configure the Program

No additional user configuration is required for this example before upload.

The following settings are defined in the sketch and can be changed if needed:

| Setting | Default |
|---------|---------|
| Serial Monitor baud rate | 115200 |
| RS-485 UART | Serial2, 115200, 8N1 (RX IO16, TX IO15) |
| Ethernet MAC address | DE:AD:BE:EF:FE:ED |
| NTP server | time.nist.gov |
| Wi-Fi SoftAP SSID / password | Device MAC address / 12345678 |
| Output step time / hold time | 150 ms / 1000 ms |

### 3. Upload the Program

- Open the example program in Arduino IDE / PlatformIO
- Select the board **ESP32S3 Dev Module** and the correct port
- Set **Flash Size** to 16MB and **PSRAM** to QSPI PSRAM (the XN-MAIN uses the N16R2 module)
- Enable **USB CDC On Boot** to see the Serial output over the USB port
- For remote login, set USB CDC On Boot to Disabled.
- Upload the program to the NORVI controller

If the upload does not start, hold the BOOT button, press RESET, then release BOOT to enter the programming mode.

### 4. Observe the Serial Monitor and Display

Open the Serial Monitor at **115200 baud**. After upload the controller runs all tests and prints the results.

When the program runs successfully:

- The I2C scan finds 4 devices (0x15, 0x41, 0x68, 0x75)
- The RTC prints 5 timestamps and the SD card is mounted
- Ethernet receives an IP address and the NTP server returns the UTC time
- The Wi-Fi access point starts
- The TFT shows the "NORVI XN" test screen with gesture, points, event, X, Y, touch counter and RTC time
- The outputs switch ON one by one, hold, then switch OFF, and the cycle repeats

## Example Output

```
Scanning I2C...
I2C device found at address 0x15  !  CST816S touch
I2C device found at address 0x41  !  PCA9536 buttons/LEDs
I2C device found at address 0x68  !  DS3231 RTC
I2C device found at address 0x75  !  PCA9539 I/O expander
I2C scan complete: 4 device(s) found
PCA9536 OK (buttons & LEDs ready).
Outputs ready (17 outputs, walking test runs in loop).
RTC Initialized.
Card Mount: success
Ethernet OK, IP address: 192.168.1.50
Wi-Fi Access Point is active!
RS485 echo received: RS485 01 SUCCESS

================ TEST SUMMARY ================
I2C devices : 4 found (expected 4: 0x15, 0x41, 0x68, 0x75)
PCA9536     : OK
RTC DS3231  : OK
SD card     : OK
Ethernet    : OK
NTP         : OK
Wi-Fi AP    : OK
RS485 echo  : OK
==============================================
```

Values such as the IP address, SD card size and timestamps depend on your network and hardware. The summary shows `NOT FOUND`, `FAILED`, `NO RESPONSE` or `NO ECHO` for any test that does not pass.

While the main loop runs, the Serial Monitor also shows:

```
OUTPUT5 ON
OUTPUT6 ON
...
CS4 ON
All outputs OFF
PB1 pressed
PB1 released
SINGLE CLICK    1    0    120    160
```

The touch line shows the gesture, number of points, event, X and Y.

## How to Validate the Example

To verify the example works correctly:

1. Open the Serial Monitor and check that the test summary reports **OK** for all connected peripherals
2. Touch the screen and confirm the gesture (SINGLE CLICK, DOUBLE CLICK, LONG PRESS, SWIPE UP / DOWN / LEFT / RIGHT), X/Y values and touch counter update
3. Press the P1 and P2 push buttons and confirm "PB1 pressed / released" and "PB3 pressed / released" appear on the Serial Monitor
4. Observe the RUN and ERROR indicators turn on alternately every 500 ms
5. Measure the expansion port output pins and confirm they switch ON one by one in the order printed on the Serial Monitor
6. Confirm the RTC time on the TFT updates every second

## Device Preparation / Configuration

Before running the example ensure:

- The controller is powered correctly
- Required peripherals (microSD card, Ethernet cable, RS-485 loopback) are connected for the tests you want to run
- The required libraries are installed

Example:

Power Supply: 24V DC (9–28V DC input range)

Current Consumption: 90 mA – 100 mA

### Pins Used by the Test

| Function | Pins |
|----------|------|
| I2C (touch, PCA9536, DS3231, PCA9539) | SDA IO8, SCL IO9 |
| Shared SPI bus (TFT, microSD, Ethernet) | MOSI IO11, SCLK IO12, MISO IO13 |
| TFT display (ST7789) | CS IO45, DC IO46, RST IO47 |
| Touch controller (CST816S) | RST IO47, INT IO48 |
| microSD card | CS IO42 |
| Ethernet (W5500) | CS IO1 |
| RS-485 | TX IO15, RX IO16 (direction control is automatic) |
| GPIO expander reset (PCA9536 / PCA9539) | IO38 |

### Output Test Pins (Expansion Port)

| Test name on Serial | Source | Expansion port pin |
|---------------------|--------|--------------------|
| OUTPUT5 | IO5 | 21 |
| OUTPUT6 | IO6 | 22 |
| OUTPUT7 | IO7 | 23 |
| OUTPUT10 | IO10 | 24 |
| OUTPUT14 | IO14 | 28 |
| OUTPUT21 | IO21 | 35 |
| OUTPUT39 | IO39 | 41 |
| OUTPUT40 | IO40 | 42 |
| OUTPUT19 | PCA9539 P1_1 | 33 |
| OUTPUT20 | PCA9539 P1_2 | 34 |
| OUTPUT33 | PCA9539 P1_3 | 38 |
| OUTPUT34 | PCA9539 P1_4 | 39 |
| OUTPUT38 | PCA9539 P1_5 | 40 |
| CS1 | PCA9539 P1_0 | 14 |
| CS2 | PCA9539 P0_7 | 15 |
| CS3 | PCA9539 P0_6 | 16 |
| CS4 | PCA9539 P0_5 | 17 |

The PCA9539 I/O expander uses I2C address 0x75. The expansion port pins are not used by the test unless a NORVI expansion module or external load is connected, so disconnect any expansion module before running the output test.

## Required Libraries

Install the following libraries before compiling.

- Wire
- WiFi
- Ethernet (includes EthernetUdp)
- SPI
- FS
- SD
- PCA9539
- PCA9536D
- RTClib
- CST816S
- LovyanGFX

Download Libraries:

https://github.com/NORVIControllers/NORVI-X-Version-01/tree/main/libraries

Installation:

1. Open Arduino IDE
2. Go to Library Manager
3. Search and install the required libraries

## Limitations

- This example is provided for demonstration purposes
- The Ethernet and NTP tests need a DHCP server and internet access; DHCP waits up to 10 seconds before the test fails
- The RS-485 echo test passes only when a loopback or an echoing RS-485 device is connected
- The RTC is set to the compile time of the sketch only if it has lost power
- The microSD card detect pin is not connected, so the card is only detected when the test runs
- The Wi-Fi SoftAP uses a fixed default password; change it before using the code outside of testing
- The output test switches all expansion port outputs, so do not run it with connected modules or loads that are not meant to be switched

## Safety Notes

- Do not exceed the rated supply voltage (maximum 28V DC)
- Ensure proper grounding
- Disconnect expansion modules and external loads before running the output test
- Incorrect wiring may damage the controller

Test Date: 2026-10-23

Verified By: Kaveesha

---

## Support
- Documentation: https://norvi.io/docs  
- For additional support or inquiries, contact the NORVI support team.

---

## License

This example is provided for development and educational purposes.
