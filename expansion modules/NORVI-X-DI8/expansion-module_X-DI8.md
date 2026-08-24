---
type: expansion_module
product: NORVI X-DI8 — 8-Channel Digital Input Expansion
compatible_cpu_boards: [X1, X2, X3]
bus: I2C
status: released
last_updated: 2026-08-24
---

# X-DI8 — Expansion Reference

## Overview
The NORVI X-DI8 is an 8-channel isolated 24 V DC digital input expansion module for the NORVI X modular controller. It is suitable for reading switches, sensors, and relay contacts in industrial automation applications.

## Specs
| Spec | Value |
|---|---|
| Channels | 8 |
| Signal type | dry contact |
| Resolution | 1bit |
| Input protection | Opto isolated |
| Power draw | 40mA |

## I2C Address
| Property | Value |
|---|---|
| Default address | 0x73 |
| Address selectable? | Yes via DIP switch |
| Address range if selectable | 0x70-0x73 |
| Conflicts with | X-R8, X-Q8, X-DA8 |

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
| 9 | Digital Com |  | Digital input isolated ground |


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
| PCA9538.h |  | Same library for X-DI8, X-Q8, X-DA8, X-R8  |

## Minimal Working Example
```cpp
#include <PCA9538.h>
#include <Wire.h>

#define PCA9538_ADDR 0x73

#define IN1 1
#define IN2 2
#define IN3 3
#define IN4 4
#define IN5 5
#define IN6 6
#define IN7 7
#define IN8 8

PCA9538 io(PCA9538_ADDR);

void setup()
{
  Serial.begin(115200);
  pinMode(21, OUTPUT);
  digitalWrite(21, HIGH);
  Wire.begin(8, 9);

  io.pinMode(IN1, INPUT);
  io.pinMode(IN2, INPUT);
  io.pinMode(IN3, INPUT);
  io.pinMode(IN4, INPUT);
  io.pinMode(IN5, INPUT);
  io.pinMode(IN6, INPUT);
  io.pinMode(IN7, INPUT);
  io.pinMode(IN8, INPUT);

  Serial.println("NORVI X-DI8 READY");
}

void loop()
{
  Serial.print("IN1="); Serial.print(io.digitalRead(IN1));
  Serial.print("  IN2="); Serial.print(io.digitalRead(IN2));
  Serial.print("  IN3="); Serial.print(io.digitalRead(IN3));
  Serial.print("  IN4="); Serial.print(io.digitalRead(IN4));
  Serial.print("  IN5="); Serial.print(io.digitalRead(IN5));
  Serial.print("  IN6="); Serial.print(io.digitalRead(IN6));
  Serial.print("  IN7="); Serial.print(io.digitalRead(IN7));
  Serial.print("  IN8="); Serial.println(io.digitalRead(IN8));

  delay(1000);
}
```

## Related Files
- CPU board(s): `cpu-board_X1.md`, `cpu-board_X2.md`, `cpu-board_X2.md`
- Function-specific examples (MQTT publish, Modbus, etc.): `NORVI X-DI8_program.md`
