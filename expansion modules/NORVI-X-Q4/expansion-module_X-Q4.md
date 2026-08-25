---
type: expansion_module
product: NORVI X-Q4 — 4-Channel Transistor Output Expansion
compatible_cpu_boards: [X1, X2, X3]
bus: Direct GPIO
status: Released
last_updated: 2026-08-25
---

# X-Q4 — Expansion Reference

## Overview
The NORVI X-Q4 is a 4-channel transistor output expansion module for the NORVI X platform, using high-side P-channel MOSFET switches with optical isolation and per-channel fuse protection. It's used to drive DC loads like solenoids, valves, contactors, and alarm/tower lights in factory automation setups.

## Specs
| Spec | Value |
|---|---|
| Channels | 4 |
| Signal type | wet contact |
| Resolution | NA |
| Input protection | Resettable Fuse |
| Power draw | 80mA |

## I2C Address
| Property | Value |
|---|---|
| Default address | NA |
| Address selectable? | No, fixed |
| Address range if selectable | NA |
| Conflicts with | Only one Q4 module per system |

> This address also gets added to `i2c-address-master-table.md` — that table is the conflict-check reference across the whole product line.

## Pin / Terminal Map
| Terminal | Function | Access | Notes |
|---|---|---|---|
| 1 | Transistor1 Output | Direct GPIO | 0.5A 24V Transistor Output |
| 1 | Transistor2 Output | Direct GPIO | 0.5A 24V Transistor Output |
| 1 | Transistor3 Output | Direct GPIO | 0.5A 24V Transistor Output |
| 1 | Transistor4 Output | Direct GPIO | 0.5A 24V Transistor Output |
| 5 | GND |  | Power Ground |


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

## Minimal Working Example
```cpp
#define TR1 14
#define TR2 35
#define TR3 41
#define TR4 37

void setup()
{
  Serial.begin(115200);

  pinMode(TR1, OUTPUT);
  pinMode(TR2, OUTPUT);
  pinMode(TR3, OUTPUT);
  pinMode(TR4, OUTPUT);

  // Start with all outputs OFF
  digitalWrite(TR1, LOW);
  digitalWrite(TR2, LOW);
  digitalWrite(TR3, LOW);
  digitalWrite(TR4, LOW);

  Serial.println();
  Serial.println("NORVI X-Q4 Transistor Output Test");
  Serial.println("All outputs OFF");
}

void loop()
{
  int channels[] = {TR1, TR2, TR3, TR4};

  for (int i = 0; i < 4; i++)
  {
    Serial.print("Channel ");
    Serial.print(i + 1);
    Serial.println(" ON");

    digitalWrite(channels[i], HIGH);
    delay(2000);

    Serial.print("Channel ");
    Serial.print(i + 1);
    Serial.println(" OFF");

    digitalWrite(channels[i], LOW);
    delay(1000);
  }

  Serial.println("----------------------------------");
  Serial.println("All 4 channels tested");
  Serial.println();

  delay(2000);
}
```

## Related Files
- CPU board(s): `cpu-board_X1.md`, `cpu-board_X2.md`, `cpu-board_X2.md`
- Function-specific examples (MQTT publish, Modbus, etc.): `NORVI X-Q4_program.md`
