---
type: expansion_module
product: NORVI X-R4 — 4-Channel Relay Output Expansion
compatible_cpu_boards: [X1, X2, X3]
bus: I2C
status: Released
last_updated: 2026-08-25
---

# X-R4 — Expansion Reference

## Overview
The NORVI X-R4 is a 4-channel electromechanical relay expansion module for the NORVI X controller platform, controlled via I²C (PCA9538). It's used to add isolated relay outputs for switching AC/DC loads like pumps, valves, or contactors in industrial automation.

## Specs
| Spec | Value |
|---|---|
| Channels | 4 |
| Signal type | dry contact |
| Resolution | NA |
| Input protection | Opto isolated |
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
| 1 | Relay1 Output | via internal IO Expander | Normally open relay contact |
| 2 | Relay1 Com |  | Relay Common Input |
| 3 | Relay2 Output | via internal IO Expander | Normally open relay contact |
| 4 | Relay2 Com |  | Relay Common Input |
| 5 | Relay3 Output | via internal IO Expander | Normally open relay contact |
| 6 | Relay3 Com |  | Relay Common Input |
| 7 | Relay4 Output | via internal IO Expander | Normally open relay contact |
| 8 | Relay4 Com |  | Relay Common Input |
| 9 | NC |  | No internal connection |


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

#define RELAY1 1
#define RELAY2 2
#define RELAY3 3
#define RELAY4 4

PCA9538 io(PCA9538_ADDR);

void setup()
{
  Serial.begin(115200);

  Wire.begin(8, 9);

  io.pinMode(RELAY1, OUTPUT);
  io.pinMode(RELAY2, OUTPUT);
  io.pinMode(RELAY3, OUTPUT);
  io.pinMode(RELAY4, OUTPUT);

  io.digitalWrite(RELAY1, LOW);
  io.digitalWrite(RELAY2, LOW);
  io.digitalWrite(RELAY3, LOW);
  io.digitalWrite(RELAY4, LOW);

  Serial.println("NORVI X-R4 READY");
}


void loop()
{
  io.digitalWrite(RELAY1, HIGH);
  Serial.println("Relay 1 ON");
  delay(1000);
  io.digitalWrite(RELAY1, LOW);

  io.digitalWrite(RELAY2, HIGH);
  Serial.println("Relay 2 ON");
  delay(1000);
  io.digitalWrite(RELAY2, LOW);

  io.digitalWrite(RELAY3, HIGH);
  Serial.println("Relay 3 ON");
  delay(1000);
  io.digitalWrite(RELAY3, LOW);

  io.digitalWrite(RELAY4, HIGH);
  Serial.println("Relay 4 ON");
  delay(1000);
  io.digitalWrite(RELAY4, LOW);
}
```

## Related Files
- CPU board(s): `cpu-board_X1.md`, `cpu-board_X2.md`, `cpu-board_X2.md`
- Function-specific examples (MQTT publish, Modbus, etc.): `NORVI X-R4_program.md`
