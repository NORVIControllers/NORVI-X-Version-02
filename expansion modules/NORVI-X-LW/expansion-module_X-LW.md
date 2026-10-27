---
type: expansion_module
product: NORVI X-LW — LoraWan Expansion
compatible_cpu_boards: [X1, X2, X3, XN]
bus: UART
use_summary: LoRaWAN node (RAK4630): sends JSON text from Serial1 as LoRaWAN uplinks
keywords: [LoRa, LoRaWAN, wireless, radio]
status: released
last_updated: 2026-08-25
---

# X-LW — Expansion Reference

## Overview
The RAK4630 (WisBlock Core) combines a Nordic nRF52840 MCU with a Semtech SX1262 LoRa transceiver. In this setup it runs RAK's RUI3 firmware (RAK4631-R) and is programmed from the Arduino IDE. It is used as a LoRaWAN OTAA node that forwards newline-terminated JSON text received on Serial1 as confirmed uplinks. Typical use: LoRaWAN gateway-connected sensor/data node, bridging an main CPU device's UART output to a network server such as ChirpStack.

## Specs
| Spec | Value |
|---|---|
| Power draw | 40mA |

## I2C Address
| Property | Value |
|---|---|
| [FILL] | [FILL] |

> This address also gets added to `i2c-address-master-table.md` — that table is the conflict-check reference across the whole product line.

## Pin / Terminal Map
| Terminal | Function | Access | Notes |
|---|---|---|---|
| [FILL] | [FILL] | [FILL] | [FILL] |


> If any terminal is routed through its own IO expander chip on this module (separate from
> the module's primary I2C device), document that chip the same way as the CPU board's
> IO Expander subsection — address, direction register, and a local pin mapping table.

## Register Map (if applicable)
| Register | Address | R/W | Description |
|---|---|---|---|
| [FILL] | [FILL] | [FILL] | [FILL] |

## Required Libraries
| Library | Version | Notes |
|---|---|---|
| [FILL] | [FILL] | [FILL] |

## Minimal Working Example
```cpp
uint8_t deveui[8]  = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};  // replace with your DevEUI
uint8_t appeui[8]  = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t appkey[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};  // replace with your AppKey

void setup()
{
  Serial.begin(115200, RAK_AT_MODE);
  Serial1.begin(115200);

  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);

  if (api.lorawan.nwm.get() != 1) {
    api.lorawan.nwm.set();          // switch to LoRaWAN mode, then restart
    api.system.reboot();
  }

  api.lorawan.band.set(RAK_REGION_EU868);
  api.lorawan.deviceClass.set(RAK_LORA_CLASS_A);
  api.lorawan.njm.set(RAK_LORA_OTAA);
  api.lorawan.deui.set(deveui, 8);
  api.lorawan.appeui.set(appeui, 8);
  api.lorawan.appkey.set(appkey, 16);

  Serial.println("Joining...");
  api.lorawan.join(1, 0, 10, 8);    // join now, no auto-join, retry every 10 s, 8 attempts

  while (api.lorawan.njs.get() == 0) {   // blink blue until joined
    digitalWrite(LED_BLUE, !digitalRead(LED_BLUE));
    delay(250);
  }
  digitalWrite(LED_BLUE, LOW);
  digitalWrite(LED_GREEN, HIGH);    // joined
  Serial.println("Joined");

  api.lorawan.adr.set(true);
  api.lorawan.cfm.set(1);           // confirmed uplinks
}

void loop()
{
  if (Serial1.available()) {
    String line = Serial1.readStringUntil('\n');
    line.trim();
    if (line.length() == 0) return;

    uint8_t buf[64];
    uint8_t len = line.length() > 64 ? 64 : line.length();
    memcpy(buf, line.c_str(), len);

    Serial.println(line);
    if (api.lorawan.send(len, buf, 2)) Serial.println("Sending is requested");
    else Serial.println("Sending failed");
  }
}
```

## Related Files
- CPU board(s): `cpu-board_X1.md`, `cpu-board_X2.md`, `cpu-board_X3.md`
- Function-specific examples (MQTT publish, Modbus, etc.): `NORVI X-Q8_program.md`
