---
type: expansion_module
product: NORVI X-DI4 — 4-Channel Digital Input Expansion
compatible_cpu_boards: [X1, X2, X3]
bus: Direct GPIO
status: relaesed
last_updated: 2026-08-24
---

# X-DI4 — Expansion Reference

## Overview
The NORVI X-DI4 adds four optically isolated 24V DC digital inputs to a NORVI X controller, connected directly to the CPU's GPIO for high-speed pulse, PWM, and encoder signal capture. It's used for fast, reliable sensor/switch interfacing in industrial automation setups.

## Specs
| Spec | Value |
|---|---|
| Channels | 4 |
| Signal type | dry contact |
| Resolution | 1bit |
| Input protection | Opto isolated |
| Power draw | 40mA |

## I2C Address
| Property | Value |
|---|---|
| Default address | NA |
| Address selectable? | No, fixed |
| Address range if selectable | NA |
| Conflicts with | Only one DI4 module per system |

> This address also gets added to `i2c-address-master-table.md` — that table is the conflict-check reference across the whole product line.

## Pin / Terminal Map
| Terminal | Function | Access | Notes |
|---|---|---|---|
| 1 | Digital Input1 | Native to module's main chip | High speed uni directional input |
| 2 | Digital Input2 | Native to module's main chip | High speed uni directional input |
| 3 | Digital Input3 | Native to module's main chip | High speed uni directional input |
| 4 | Digital Input4 | Native to module's main chip | High speed uni directional input |
| 5 | Digital Com | Native to module's main chip | Digital input isolated ground |

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
| [FILL] | [FILL] | [FILL — often the same driver lib as similar modules, note reuse] |

## Minimal Working Example
```cpp
#define DI1 5
#define DI2 6
#define DI3 7
#define DI4 10

void setup() {
  Serial.begin(115200);

  pinMode(DI1, INPUT);
  pinMode(DI2, INPUT);
  pinMode(DI3, INPUT);
  pinMode(DI4, INPUT);

  Serial.println("NORVI X-DI4 Digital Input Test");
  Serial.println("--------------------------------");
}

void loop() {

  int ch1 = digitalRead(DI1);
  int ch2 = digitalRead(DI2);
  int ch3 = digitalRead(DI3);
  int ch4 = digitalRead(DI4);

  Serial.print("Channel 1 (DI1): ");
  Serial.println(ch1);

  Serial.print("Channel 2 (DI2): ");
  Serial.println(ch2);

  Serial.print("Channel 3 (DI3): ");
  Serial.println(ch3);

  Serial.print("Channel 4 (DI4): ");
  Serial.println(ch4);

  Serial.println("--------------------------------");

  delay(1000);
}
```

## Related Files
- CPU board(s): `cpu-board_X1.md`, `cpu-board_X2.md`, `cpu-board_X2.md`
- Function-specific examples (MQTT publish, Modbus, etc.): `NORVI X-DI4_program.md`
