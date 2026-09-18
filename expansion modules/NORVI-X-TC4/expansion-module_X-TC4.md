---
type: expansion_module
product: NORVI X-TC4 — 4-Channel Thermocouple Input Expansion
compatible_cpu_boards: [X1, X2, X3]
bus: I2C
status: Released
last_updated: 2026-08-25
---

# X-TC4 — Expansion Reference

## Overview
The NORVI X-TC4 is a 4-channel thermocouple input module for the NORVI X industrial controller, to interface K-type thermocouples. Typical use: monitoring high-temperature processes such as furnaces, ovens, exhaust systems, or industrial motors where wide-range, rugged temperature sensing is needed.

## Specs
| Spec | Value |
|---|---|
| Channels | 4 |
| Signal type | wet contact  |
| Resolution | 12bit |
| Input protection | NA |
| Power draw | 80mA |

## I2C Address
| Property | Value |
|---|---|
| Default address | 0x18 |
| Address selectable? | Yes via DIP switch |
| Address range if selectable | 0x08-0x0F,0x10-0x18 |
| Conflicts with | NA |

> This address also gets added to `i2c-address-master-table.md` — that table is the conflict-check reference across the whole product line.

## Pin / Terminal Map
| Terminal | Function | Access | Notes |
|---|---|---|---|
| 1 | TC1+ | via internal thermocuple amplifier and I2C interface(STM32) | Thermocouple1 positive terminal |
| 2 | TC1- | via internal thermocuple amplifier and I2C interface(STM32) | Thermocouple1 negative terminal |
| 3 | TC2+ | via internal thermocuple amplifier and I2C interface(STM32) | Thermocouple2 positive terminal |
| 4 | TC2- | via internal thermocuple amplifier and I2C interface(STM32) | Thermocouple2 negative terminal |
| 5 | TC3+ | via internal thermocuple amplifier and I2C interface(STM32) | Thermocouple3 positive terminal |
| 6 | TC3- | via internal thermocuple amplifier and I2C interface(STM32) | Thermocouple3 negative terminal |
| 7 | TC4+ | via internal thermocuple amplifier and I2C interface(STM32) | Thermocouple4 positive terminal |
| 8 | TC4- | via internal thermocuple amplifier and I2C interface(STM32) | Thermocouple4 negative terminal |
| 9 | GND |  | Power ground |


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
| Wire.h | 2.0.0 | Comes with ESP32 core/I2C Communication |

## Minimal Working Example
```cpp
#include <Wire.h>

#define SDA_PIN 8
#define SCL_PIN 9
#define TC_ADDR 0x18

uint8_t crc8(uint8_t *d, int n)
{
  uint8_t c = 0;

  while (n--)
  {
    uint8_t x = *d++;

    for (int i = 0; i < 8; i++)
    {
      uint8_t s = (c ^ x) & 1;
      c >>= 1;
      if (s) c ^= 0x8C;
      x >>= 1;
    }
  }

  return c;
}

void setup()
{
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);

  Serial.println("NORVI EXPE-TC4 READY");
}

void loop()
{
  // Ask TC4 for channels 1-4
  Wire.beginTransmission(TC_ADDR);
  Wire.write(0x01);
  Wire.write(1);
  Wire.write(2);
  Wire.write(3);
  Wire.write(4);

  if (Wire.endTransmission() != 0)
  {
    Serial.println("I2C TX FAIL");
    delay(1000);
    return;
  }

  delay(15);

  if (Wire.requestFrom(TC_ADDR, 32) != 32)
  {
    Serial.println("I2C RX FAIL");
    delay(1000);
    return;
  }

  for (int ch = 0; ch < 4; ch++)
  {
    uint8_t b[8];

    for (int i = 0; i < 8; i++)
      b[i] = Wire.read();

    if (crc8(b, 7) != b[7])
    {
      Serial.print("CRC ERROR CH");
      Serial.println(b[1]);
      continue;
    }

    float temp;
    memcpy(&temp, &b[2], 4);

    Serial.print("CH");
    Serial.print(b[1]);
    Serial.print("  Temp = ");
    Serial.print(temp, 2);
    Serial.print(" C  Fault = ");
    Serial.println(b[6]);
  }

  Serial.println("----------------");
  delay(1000);
}
```

## Related Files
- CPU board(s): `cpu-board_X1.md`, `cpu-board_X2.md`, `cpu-board_X3.md`
- Function-specific examples (MQTT publish, Modbus, etc.): `NORVI X-TC4_program.md`
