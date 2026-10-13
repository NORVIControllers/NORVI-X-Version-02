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
