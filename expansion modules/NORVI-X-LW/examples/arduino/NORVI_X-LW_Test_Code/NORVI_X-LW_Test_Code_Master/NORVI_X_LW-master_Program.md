---
type: Test_program
function: lorawan_otaa_serial_bridge (LoRaWAN OTAA uplink of newline-terminated JSON received on Serial1, with LED status indication)
compatible_cpu_boards: [NORVI X-CPU-ESPS3-X1],[NORVI X-CPU-ESPS3-X2],[NORVI X-CPU-ESPS3-X3]
compatible_modules: [NORVI X-LW-915]
status: completed
last_updated: 2026-10-26
---

# Example: NORVI X-LW LoRaWAN Test Mater code

## What this does
Joins a LoRaWAN network using OTAA and then forwards every newline-terminated text line (e.g. a JSON string, max 64 bytes) received on Serial1 as a confirmed uplink on port 2. Two on-board LEDs show the join and uplink status, and all events are logged to the Serial Monitor

## Hardware required
- Expansion module(s): NORVI X-LW
- Serial data output device with newline-terminated text line
- Wiring / pin map (from firmware `#define`s and the `LGFX` class):

| Signal | GPIO |
|---|---|
| LORA_RX | 14 |
| LORA_TX | 40 |

## Libraries required
| Library | Version |
|---|---|
| ArduinoJson.h | 5.13.5 |

**ESP32 Board Package version:** any 3.x version (ESP32 Arduino core 3.x, IDF 5.x). The core-bundled libraries (Wire, WiFi, SPI) use the version of the installed 3.x board package.

**Arduino IDE Board/Tools settings:**
| Setting | Value |
|---|---|
| Board | ESP32S3 Dev Module |
| Upload Speed | 115200 |
| USB Mode | Hardware CDC and JTAG |
| USB CDC On Boot | Enabled |
| USB Firmware MSC On Boot | Disabled |
| USB DFU On Boot | Disabled |
| Upload Mode | UART0 / Hardware CDC |
| CPU Frequency | 240MHz (WiFi) |
| Flash Mode | QIO 80MHz |
| Flash Size | 16MB (128Mb) |
| Partition Scheme | Huge APP (3MB No OTA/1MB SPIFFS) |
| Core Debug Level | None |
| PSRAM | QSPI PSRAM |
| Arduino Runs On | Core 1 |
| Events Run On | Core 1 |
| Erase All Flash Before Sketch Upload | Disabled |
| JTAG Adapter | Disabled |

## Full Sketch
```cpp
#include <ArduinoJson.h>

#define LoRaSerial Serial1

// ESP32-S3 UART pins
#define LORA_RX_PIN 14
#define LORA_TX_PIN 40

void setup()
{
  // UART connection to RAK4631
  LoRaSerial.begin(115200, SERIAL_8N1, LORA_RX_PIN, LORA_TX_PIN);

  // USB/debug serial
  Serial.begin(115200);

  delay(1000);

  Serial.println();
  Serial.println("ESP32-S3 UART JSON Sender");
  Serial.println("-------------------------");

  randomSeed(micros());
}

void loop()
{
  // Generate random values
  int Analog1 = random(0, 101);
  int Analog2 = random(0, 101);

  // Create JSON
  JsonDocument jsonDocument;

  jsonDocument["Analog1"] = Analog1;
  jsonDocument["Analog2"] = Analog2;

  // Convert JSON to String
  String jsonString;
  serializeJson(jsonDocument, jsonString);

  // Send JSON to RAK4631
  LoRaSerial.println(jsonString);

  // Show what was transmitted
  Serial.print("TX -> ");
  Serial.println(jsonString);

  // Check for response from RAK4631
  while (LoRaSerial.available())
  {
    String receivedString = LoRaSerial.readStringUntil('\n');

    Serial.print("RX <- ");
    Serial.println(receivedString);
  }

  delay(30000);
}

```

## Expected Output
On the Serial Monitor (115200 baud, USB CDC On Boot enabled so the output appears on the native USB port):
- `ESP32-S3 UART JSON Sender`
- `-------------------------`
- Every 30 s: `TX -> {"Analog1":57,"Analog2":12}` (random values between 0 and 100)
- `RX <- ...` only if the RAK4631 sends a line back on its Serial1. The provided RAK sketch sends nothing back, so no `RX` lines are expected with it.

On the RAK4631 Serial Monitor: the received JSON line, `Data Packet:` with the hex bytes, `Sending is requested`, then `Successfully sent` (or `Sending failed`). The ChirpStack device shows an uplink on port 2 with the JSON text as payload.

## Common Issues
| Symptom | Likely Cause | Fix |
|---|---|---|
| No output on the Serial Monitor | Wrong COM port, or USB CDC On Boot is not Enabled | Select the correct port and match Tools settings: ESP32S3 Dev Module, USB Mode Hardware CDC and JTAG, USB CDC On Boot Enabled |
| Upload fails / board not detected | Wrong Board/Tools settings, or board not in download mode | Match the settings table (Upload Mode UART0 / Hardware CDC, Upload Speed 115200), try another USB cable or port, and hold BOOT while resetting if needed |
| Compile error on `JsonDocument jsonDocument;` | ArduinoJson 6.x installed | Install ArduinoJson 7.x, or use `StaticJsonDocument<64>` with v6 |
| Compile error or odd UART behaviour | ESP32 Arduino core older than 3.x | Install ESP32 Arduino core 3.x (IDF 5.x) |
| RAK4631 receives the line but sends no uplink | RAK not joined yet, still waiting in `setup()` | Wait for the join (LED1 stops blinking, LED2 on) before expecting uplinks |
| Uplinks are slow or rejected | duty cycle and the data rate (ADR may select DR0) | Keep the 30 s interval or longer and keep the JSON under 64 bytes |


## Related Files
- Module reference: `expansion_module_LW.md`
- CPU board: `cpu_board_X1.md`