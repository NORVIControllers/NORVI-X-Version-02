# NORVI X-LW LoRaWAN Expansion Module Example

## Overview

This example demonstrates how to use the LoRaWAN Expansion Module (X-LW) on the NORVI X platform.

The purpose of this example is to show how to:

- Integrate the X-LW module with the NORVI X CPU
- Send a JSON message from the NORVI X CPU to the X-LW module over UART
- Join a LoRaWAN network (OTAA) and send the received data as a LoRaWAN uplink

This example can be used as a starting point for developers building applications with NORVI controllers.

## Product Used

**Product:** NORVI X-LW LoRaWAN Expansion Module  
**Platform:** NORVI X CPU (ESP32-S3)  
**Website:** https://norvi.io 

## Purpose of This Example

The example consists of two sketches that work together:

| Sketch | Runs on | Board selection (Tools → Board) |
|---|---|---|
| Master test code | NORVI X CPU (ESP32-S3) | ESP32 Arduino → ESP32S3 Dev Module |
| Slave test code | NORVI X-LW (RAK4630) | RAKwireless RUI nRF Boards → WisBlock Core RAK4631 Board |

**Master test code (NORVI X CPU)**

- Initializes UART communication with the X-LW module
- Generates two dummy values (`Analog1`, `Analog2`, 0–100)
- Creates a JSON message and sends it to the X-LW module
- Displays the transmitted data on the USB Serial Monitor
- Checks for a response and repeats every 30 seconds

**Slave test code (NORVI X-LW)**

- Configures the RAK4630 for LoRaWAN (OTAA, Class A)
- Joins the LoRaWAN network
- Receives the JSON message from the NORVI X CPU over UART
- Sends the message as a LoRaWAN uplink and reports the send status

**Data path:**

```
NORVI X CPU → NORVI X-LW → LoRaWAN Gateway → LoRaWAN Network Server (ChirpStack) → Application
```

Example JSON message:

```json
{"Analog1":45,"Analog2":72}
```
## Getting Started

Follow the steps below to run the example.

### 1. Hardware Connections

1. Power off the system.
2. Mount the X-LW on the DIN rail next to the NORVI X CPU (or an adjacent expansion module). The module connects through the 50-pin expansion bus and is powered by the CPU.
3. **Connect the LoRa antenna** before powering the radio module. Never operate the module without an antenna.
4. Power the system on with a regulated 24 V DC supply.

UART connection between the CPU and the X-LW (made through the expansion bus):

| ESP32-S3 | RAK Module |
|---|---|
| GPIO 40 (TX) | RX (P0.15) |
| GPIO 14 (RX) | TX (P0.16) |
| GND | GND |

### 2. Configure the Program

**Master code:** No additional configuration is required before upload.

**Slave code:** Set the LoRaWAN region and OTAA credentials to match your LoRaWAN network server:

```cpp
#define OTAA_BAND   (RAK_REGION_EU868)
#define OTAA_DEVEUI { /* Device EUI */ };
#define OTAA_APPEUI { /* Application EUI / JoinEUI */ };
#define OTAA_APPKEY { /* Application Key */ };
```

| Parameter | Description |
|---|---|
| `OTAA_BAND` | LoRaWAN frequency region (default: EU868). Uncomment the required region and comment out the others. |
| `OTAA_DEVEUI` | Device EUI of the RAK4630 |
| `OTAA_APPEUI` | Application EUI / JoinEUI |
| `OTAA_APPKEY` | Application Key used for OTAA authentication. Keep it confidential. |

The code has been tested with ChirpStack and can also be used with other compatible LoRaWAN network servers.

### 3. Upload the Programs

Each device is uploaded separately. Change the board and port in the Tools menu every time you switch devices.

**Master code to the NORVI X CPU**

1. Connect the NORVI X CPU to your computer using USB.
2. Open the master example in Arduino IDE.
3. Select **Tools → Board → ESP32 Arduino → ESP32S3 Dev Module** and the correct **Port**.
4. Configure the board settings:

| Setting | Selection |
|---|---|
| USB CDC On Boot | Enabled |
| Flash Size | 16MB (128Mb) |
| PSRAM | QSPI PSRAM |

5. Upload the program.

**Slave code to the NORVI X-LW**

1. Open the slave example in Arduino IDE.
2. Select **Tools → Board → RAKwireless RUI nRF Boards → WisBlock Core RAK4631 Board** and the correct **Port**.
3. Upload the program.

For help installing the RAK4630 board package, refer to the RAK4630 Module Quick Start Guide.

### 4. Observe the Output

Open the Serial Monitor at **115200 baud**.

**Master (NORVI X CPU):**

```
TX -> {"Analog1":45,"Analog2":72}
```

A new message is sent approximately every 30 seconds.

**Slave (NORVI X-LW):** the Serial Monitor shows the join status, the received data packet, and the send result:

```
Join status: 0
...
Sending is requested
Successfully sent
```

On a successful uplink the green LED pulses; on a failed uplink the blue LED pulses.

## How to Validate the Example

To verify the example works correctly:

1. Power the system and confirm the red power LED on the X-LW is on.
2. Confirm the slave Serial Monitor reports a successful join (`Join status: 0`).
3. Confirm the master Serial Monitor shows a new `TX ->` message every 30 seconds.
4. Confirm the slave Serial Monitor shows the same JSON data and `Successfully sent`.
5. In your LoRaWAN network server (for example ChirpStack), open the device and check **LoRaWAN Frames** and **Device Data** for the join request, join accept, and uplinks.
6. Confirm the payload values change between transmissions.

## Device Preparation / Configuration

Before running the example ensure:

- The controller is powered correctly
- The antenna is connected
- A LoRaWAN gateway is in range and connected to your network server
- The device is registered on the network server with the same DevEUI, AppEUI/JoinEUI and AppKey used in the slave code
- The region in the slave code matches your gateway and network server

**Power Supply:** 24 V DC
**Current Consumption:** 40 mA @ 24 V DC

## Technical Summary

| Parameter | Value |
|---|---|
| Wireless module | RAK4630 (Nordic nRF52840 + Semtech SX1262) |
| Wireless technology | LoRa / LoRaWAN 1.0.3 (also LoRa P2P), Bluetooth 5.0 |
| Host interface | UART / I²C |
| UART settings | 115200 baud, 8 data bits, no parity, 1 stop bit (8-N-1) |
| Operating temperature | −40 °C to +85 °C |
| Protection | IP20 |
| Mounting | DIN rail (NORVI X-N1 enclosure) |

## LED Indicators

| LED | Colour | Behaviour |
|---|---|---|
| Power | Red | Power indicator |
| LED1 | Green | User configurable |
| LED2 | Green | User configurable |

## Required Libraries

Install the following before compiling:

- ESP32 board package (for the NORVI X CPU)
- RAKwireless RUI nRF board package (for the X-LW)
- ArduinoJson
- Wire

**Installation:**

1. Open Arduino IDE.
2. Go to **Sketch → Include Library → Manage Libraries**.
3. Search for and install the required libraries.

Libraries and firmware examples are available here:
https://github.com/NORVIControllers/NORVI-X-Version-02/tree/main

## Limitations

- This example is provided for demonstration purposes
- The master code sends dummy random values, not real sensor data
- The uplink payload buffer is 64 bytes; longer messages are truncated
- Range and reliability depend on antenna, gateway coverage, and environmental conditions
- Local RF regulations for the selected frequency band and transmission power must be followed

## Safety Notes

- Disconnect all power before installing, wiring, or servicing the module
- Do not connect or disconnect the module from the expansion bus while powered
- Connect the antenna before operating the radio module
- Do not exceed the rated supply voltage (24 V DC)
- Ensure proper grounding
- Install in a suitably rated enclosure; the module is IP20 and not for outdoor or wash-down exposure
- Verify wiring and configuration on a test setup before connecting to active industrial equipment

## Troubleshooting

| Problem | Possible Cause | Check |
|---|---|---|
| No output on Serial Monitor | Incorrect COM port or baud rate | Check COM port and 115200 baud |
| ESP32 does not upload | Incorrect board/port selection | Check Arduino IDE settings |
| Garbled UART data | Baud-rate mismatch | Verify 115200 baud on both sides |
| JSON not received correctly | Message handling mismatch | Check newline/message parsing |
| Slave does not join | Wrong credentials or region | Check DevEUI, AppEUI, AppKey and region |
| No wireless transmission | RAK module configuration or hardware issue | Check module, antenna and configuration |
| Intermittent communication | Wiring, power or RF conditions | Check connections and installation |

## Support

**Documentation:** https://norvi.io/docs 
**Datasheet:** https://norvi.io/docs/norvi-x-lw-datasheet/ 
**Technical support:** support@norvi.io 

For additional support or inquiries, contact the NORVI support team.

Test Date: 2026-10-13 
Verified By: Kaveesha

## License

This example is provided for development and educational purposes.










