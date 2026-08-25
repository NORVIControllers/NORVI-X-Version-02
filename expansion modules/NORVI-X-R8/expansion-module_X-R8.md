---
type: expansion_module
product: NORVI X-R8 — 8-Channel Relay Output Expansion
compatible_cpu_boards: [X1, X2, X3]
bus: I2C
status: Released
last_updated: 2026-08-25
---

# X-R8 — Expansion Reference

## Overview
The NORVI X-R8 is an 8-channel relay output expansion module for the NORVI X modular controller, providing electrically isolated relay outputs for switching AC and DC loads. It is suitable for industrial automation, machine control, and PLC I/O expansion applications.

## Specs
| Spec | Value |
|---|---|
| Channels | 8 |
| Signal type | dry contact |
| Resolution | NA |
| Input protection | NA |
| Power draw | 80mA |

## I2C Address
| Property | Value |
|---|---|
| Default address | 0x73 |
| Address selectable? | Yes via DIP switch |
| Address range if selectable | 0x70-0x73 |
| Conflicts with | X-DI8, X-Q8, X-DA8 |

> This address also gets added to `i2c-address-master-table.md` — that table is the conflict-check reference across the whole product line.

## Pin / Terminal Map
| Terminal | Function | Access | Notes |
|---|---|---|---|
| 1 | Relay Output1 | via internal IO Expander | Normally open relay contact |
| 2 | Relay Com1 |  | Relay Common Input |
| 3 | Relay Output2 | via internal IO Expander | Normally open relay contact |
| 4 | Relay Com2 |  | Relay Common Input |
| 5 | Relay Output3 | via internal IO Expander | Normally open relay contact |
| 6 | Relay Com3 |  | Relay Common Input |
| 7 | Relay Output4 | via internal IO Expander | Normally open relay contact |
| 8 | Relay Com4 |  | Relay Common Input |
| 9 | Relay Output5 | via internal IO Expander | Normally open relay contact |
| 10 | Relay Com5 |  | Relay Common Input |
| 11 | Relay Output6 | via internal IO Expander | Normally open relay contact |
| 12 | Relay Com6 |  | Relay Common Input |
| 13 | Relay Output7 | via internal IO Expander | Normally open relay contact |
| 14 | Relay Com7 |  | Relay Common Input |
| 15 | Relay Output8 | via internal IO Expander | Normally open relay contact |
| 16 | Relay Com8 |  | Relay Common Input |
| 17 | GND |  | Internal DC Ground |
| 18 | GND |  | Internal DC Ground |


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
| PCA9538.h |  | Same library for X-DI8, X-Q8, X-DA8, X-R8  |

## Minimal Working Example
```cpp
#include <PCA9538.h>
#include <Wire.h>

#define PCA9538_ADDR 0x73

PCA9538 io(PCA9538_ADDR);

void setup()
{
  Serial.begin(115200);
  pinMode(21, OUTPUT);
  digitalWrite(21, HIGH);
  Wire.begin(8, 9);

  for (int i = 1; i <= 8; i++)
    io.pinMode(i, OUTPUT);

  Serial.println("NORVI X-R8 READY");
}

void loop()
{
  for (int i = 1; i <= 8; i++)
  {
    io.digitalWrite(i, HIGH);

    Serial.print("Relay ");
    Serial.print(i);
    Serial.println(" ON");

    delay(1000);

    io.digitalWrite(i, LOW);
  }
}
```

## Related Files
- CPU board(s): `cpu-board_X1.md`, `cpu-board_X2.md`, `cpu-board_X3.md`
- Function-specific examples (MQTT publish, Modbus, etc.): `NORVI X-R8_program.md`
