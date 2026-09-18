---
type: expansion_module
product: NORVI X-Q16 — 16-Channel Transistor Output Expansion
compatible_cpu_boards: [X1, X2, X3]
bus: I2C
status: released
last_updated: 2026-08-25
---

# X-Q16 — Expansion Reference

## Overview
The NORVI X-Q16 is a 16-channel transistor output expansion module for the NORVI X industrial controller.
It provides high-side switching (24V DC, 0.5A/channel) with optical isolation and integrated fuse protection.
Typical use: driving actuators, relays, solenoids, or LED arrays in industrial automation setups.

## Specs
| Spec | Value |
|---|---|
| Channels | 16 |
| Signal type | wet contact |
| Resolution | NA |
| Input protection | Resettable Fuse |
| Power draw | 90mA |

## I2C Address
| Property | Value |
|---|---|
| Default address | 0x27 |
| Address selectable? | Yes via DIP switch |
| Address range if selectable | 0x20-0x27 |
| Conflicts with | X-DI16 |

> This address also gets added to `i2c-address-master-table.md` — that table is the conflict-check reference across the whole product line.

## Pin / Terminal Map
| Terminal | Function | Access | Notes |
|---|---|---|---|
| 1 | Transistor Output1 | via internal IO Expander | 0.5A 24V Transistor Output |
| 2 | Transistor Output2 | via internal IO Expander | 0.5A 24V Transistor Output |
| 3 | Transistor Output3 | via internal IO Expander | 0.5A 24V Transistor Output |
| 4 | Transistor Output4 | via internal IO Expander | 0.5A 24V Transistor Output |
| 5 | Transistor Output5 | via internal IO Expander | 0.5A 24V Transistor Output |
| 6 | Transistor Output6 | via internal IO Expander | 0.5A 24V Transistor Output |
| 7 | Transistor Output7 | via internal IO Expander | 0.5A 24V Transistor Output |
| 8 | Transistor Output8 | via internal IO Expander | 0.5A 24V Transistor Output |
| 9 | Transistor Output9 | via internal IO Expander | 0.5A 24V Transistor Output |
| 10 | Transistor Output10 | via internal IO Expander | 0.5A 24V Transistor Output |
| 11 | Transistor Output11 | via internal IO Expander | 0.5A 24V Transistor Output |
| 12 | Transistor Output12 | via internal IO Expander | 0.5A 24V Transistor Output |
| 13 | Transistor Output13 | via internal IO Expander | 0.5A 24V Transistor Output |
| 14 | Transistor Output14 | via internal IO Expander | 0.5A 24V Transistor Output |
| 15 | Transistor Output15 | via internal IO Expander | 0.5A 24V Transistor Output |
| 16 | Transistor Output16 | via internal IO Expander | 0.5A 24V Transistor Output |
| 17 | GND |  | Power Ground |
| 18 | GND |  | Power Ground |


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

PCA9555 io(PCA9555_ADDR);

void setup()
{
  Serial.begin(115200);

  Wire.begin(8, 9);

  for (int i = 0; i < 16; i++)
  {
    io.pinMode(i, OUTPUT);
    io.digitalWrite(i, LOW);
  }

  Serial.println("NORVI X-Q16 READY");
}

void loop()
{
  for (int i = 0; i < 16; i++)
  {
    io.digitalWrite(i, HIGH);

    Serial.print("TR");
    Serial.print(i + 1);
    Serial.println(" ON");

    delay(1000);

    io.digitalWrite(i, LOW);
  }

  Serial.println("All outputs OFF");
  delay(1000);
}
```

## Related Files
- CPU board(s): `cpu-board_X1.md`, `cpu-board_X2.md`, `cpu-board_X3.md`
- Function-specific examples (MQTT publish, Modbus, etc.): `NORVI X-Q16_program.md`
