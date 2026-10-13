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
















