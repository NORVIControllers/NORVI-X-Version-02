---
type: expansion_module
product: NORVI X-DI16 — 16-Channel Digital Input Expansion
compatible_cpu_boards: [X1, X2, X3]
bus: I2C
status: released
last_updated: 2026-08-24
---

# X-DI16 — Expansion Reference

## Overview
The NORVI X-DI16 is a 16-channel isolated 24 V DC digital input expansion module for the NORVI X Modular Controller. It is designed for monitoring industrial switches, sensors, and control signals in automation and machine applications.

## Specs
| Spec | Value |
|---|---|
| Channels | 16 |
| Signal type | dry contact |
| Resolution | 1bit |
| Input protection | Opto isolated |
| Power draw | 90mA |

## I2C Address
| Property | Value |
|---|---|
| Default address | 0x27 |
| Address selectable? | Yes via DIP switch |
| Address range if selectable | 0x20-0x27 |
| Conflicts with | X-Q16 |

> This address also gets added to `i2c-address-master-table.md` — that table is the conflict-check reference across the whole product line.

## Pin / Terminal Map
| Terminal | Function | Access | Notes |
|---|---|---|---|
| 1 | Digital Input1 | via internal IO Expander | Digital Input (sink or source) |
| 2 | Digital Input2 | via internal IO Expander | Digital Input (sink or source) |
| 3 | Digital Input3 | via internal IO Expander | Digital Input (sink or source) |
| 4 | Digital Input4 | via internal IO Expander | Digital Input (sink or source) |
| 5 | Digital Input5 | via internal IO Expander | Digital Input (sink or source) |
| 6 | Digital Input6 | via internal IO Expander | Digital Input (sink or source) |
| 7 | Digital Input7 | via internal IO Expander | Digital Input (sink or source) |
| 8 | Digital Input8 | via internal IO Expander | Digital Input (sink or source) |
| 9 | Digital Input9 | via internal IO Expander | Digital Input (sink or source) |
| 10 | Digital Input10 | via internal IO Expander | Digital Input (sink or source) |
| 11 | Digital Input11 | via internal IO Expander | Digital Input (sink or source) |
| 12 | Digital Input12 | via internal IO Expander | Digital Input (sink or source) |
| 13 | Digital Input13 | via internal IO Expander | Digital Input (sink or source) |
| 14 | Digital Input14 | via internal IO Expander | Digital Input (sink or source) |
| 15 | Digital Input15 | via internal IO Expander | Digital Input (sink or source) |
| 16 | Digital Input16 | via internal IO Expander | Digital Input (sink or source) |
| 17 | Digital Com |  | Digital input isolated ground |
| 18 | Digital Com |  | Digital input isolated ground |


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
| clsPCA9555.h | V1.0 | Same library for X-DI16 and X-Q16  |

## Minimal Working Example
```cpp
#include <Wire.h>
#include "clsPCA9555.h"

#define PCA9555_ADDR 0x27

#define SDA_PIN 8
#define SCL_PIN 9

PCA9555 ioport(PCA9555_ADDR);

void setup()
{
  Serial.begin(115200);

  Wire.begin(SDA_PIN, SCL_PIN);

  // DI1-DI8
  for (int i = 0; i < 8; i++)
    ioport.pinMode(i, INPUT);

  // DI9-DI16
  for (int i = 8; i < 16; i++)
    ioport.pinMode(i, INPUT);

  Serial.println("NORVI X-DI16 READY");
  Serial.println("--------------------");
}

void loop()
{
  for (int i = 0; i < 16; i++)
  {
    Serial.print("DI");
    Serial.print(i + 1);
    Serial.print("=");

    Serial.print(ioport.digitalRead(i));

    if (i < 15)
      Serial.print("  ");
  }

  Serial.println();

  delay(1000);
}
```

## Related Files
- CPU board(s): `cpu-board_X1.md`, `cpu-board_X2.md`, `cpu-board_X3.md`
- Function-specific examples (MQTT publish, Modbus, etc.): `NORVI X-DI16_program.md`
