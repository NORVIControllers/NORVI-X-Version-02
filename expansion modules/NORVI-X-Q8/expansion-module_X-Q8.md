---
type: expansion_module
product: NORVI X-Q8 — 8-Channel Transistor Output Expansion
compatible_cpu_boards: [X1, X2, X3]
bus: I2C
status: released
last_updated: 2026-08-25
---

# X-Q8 — Expansion Reference

## Overview
The NORVI X-Q8 is an 8-channel transistor output expansion module for the NORVI X industrial controller.
It provides high-side switching (24V DC, 0.5A/channel) with optical isolation and per-channel resettable PTC fuse protection.
Typical use: smaller-scale actuator, relay, or contactor control in industrial automation setups.

## Specs
| Spec | Value |
|---|---|
| Channels | 8 |
| Signal type | wet contact |
| Resolution | NA |
| Input protection | Resettable Fuse |
| Power draw | 90mA |

## I2C Address
| Property | Value |
|---|---|
| Default address | 0x73 |
| Address selectable? | Yes via DIP switch |
| Address range if selectable | 0x70-0x73 |
| Conflicts with | X-R8, X-DI8, X-DA8 |

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
| 9 | GND |  | Power Ground |


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
| PCA9538.h |  | Same library for X-DI8, X-Q8, X-DA8, X-R8   |

## Minimal Working Example
```cpp
#include <PCA9538.h>
#include <Wire.h>

#define PCA9538_ADDR 0x73
#define PCA9538_RST  21

PCA9538 io(PCA9538_ADDR);

void setup()
{
  Serial.begin(115200);

  pinMode(PCA9538_RST, OUTPUT);
  digitalWrite(PCA9538_RST, HIGH);

  Wire.begin(8, 9);

  for (int i = 1; i <= 8; i++)
  {
    io.pinMode(i, OUTPUT);
    io.digitalWrite(i, LOW);
  }

  Serial.println("NORVI X-Q8 READY");
}

void loop()
{
  for (int i = 1; i <= 8; i++)
  {
    io.digitalWrite(i, HIGH);

    Serial.print("TR");
    Serial.print(i);
    Serial.println(" ON");

    delay(40000);

    io.digitalWrite(i, LOW);
  }

  Serial.println("All outputs OFF");
  delay(1000);
}
```

## Related Files
- CPU board(s): `cpu-board_X1.md`, `cpu-board_X2.md`, `cpu-board_X2.md`
- Function-specific examples (MQTT publish, Modbus, etc.): `NORVI X-Q8_program.md`
