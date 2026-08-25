---
type: expansion_module
product: NORVI X-AQ4 — 4-Channel Analog Output(0-10V or 4-20mA) Expansion]
compatible_cpu_boards: [X1, X2, X3]
bus: I2C
status: Released
last_updated: 2026-08-25
---

# X-AQ4 — Expansion Reference

## Overview
The NORVI X-AQ4 is a 4-channel analog output expansion module that provides configurable voltage or current outputs for controlling industrial devices. It is suitable for applications such as actuator control, process regulation, and industrial automation.

## Specs
| Spec | Value |
|---|---|
| Channels | 4 |
| Signal type | 4-20mA / 0-10V |
| Resolution | 13bit |
| Input protection | TVS |
| Power draw | 110mA |

## I2C Address
| Property | Value |
|---|---|
| Default address | 0x5F |
| Address selectable? | Yes via DIP switch |
| Address range if selectable | 0x50-0x5F |
| Conflicts with | NA |

> This address also gets added to `i2c-address-master-table.md` — that table is the conflict-check reference across the whole product line.

## Pin / Terminal Map
| Terminal | Function | Access | Notes |
|---|---|---|---|
| 1 | Analog Output1 | via internal DAC and I2C interface(STM32) | 0-10V/4-20mA current output |
| 2 | Analog Output2 | via internal DAC and I2C interface(STM32) | 0-10V/4-20mA current output |
| 3 | Analog Output3 | via internal DAC and I2C interface(STM32) | 0-10V/4-20mA current output |
| 4 | Analog Output4 | via internal DAC and I2C interface(STM32) | 0-10V/4-20mA current output |
| 5 | AGND |  | Analog ground |


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

#define ADDR 0x5F

void setup()
{
  Serial.begin(115200);
  Wire.begin(8, 9);

  // CH1=0-10V, CH2=0-10V, CH3=4-20mA, CH4=4-20mA
  Wire.beginTransmission(ADDR);
  Wire.write(0x01);
  Wire.write(0x0C);
  Wire.endTransmission();

  Serial.println("NORVI X-AQ4 READY");
  Serial.println("CH1/CH2 = 0-10V");
  Serial.println("CH3/CH4 = 4-20mA");
}

void loop()
{
  if (Serial.available())
  {
    String s = Serial.readStringUntil('\n');
    s.trim();
    s.toUpperCase();

    int ch = s.substring(2, 3).toInt();
    int eq = s.indexOf('=');

    if (ch < 1 || ch > 4 || eq < 0)
    {
      Serial.println("Use: CH1=5");
      return;
    }

    float v = s.substring(eq + 1).toFloat();

    if (ch <= 2)
    {
      if (v < 0 || v > 10)
      {
        Serial.println("Range: 0-10V");
        return;
      }

      v = v / 10.0 * 4095;
    }
    else
    {
      if (v < 4 || v > 20)
      {
        Serial.println("Range: 4-20mA");
        return;
      }

      v = (v - 4) / 16.0 * 4095;
    }

    Wire.beginTransmission(ADDR);
    Wire.write(1 + ch * 2);
    Wire.write((int)v >> 8);
    Wire.write((int)v & 255);
    Wire.endTransmission();

    Serial.print("CH");
    Serial.print(ch);
    Serial.print(" = ");
    Serial.println(s.substring(eq + 1));
  }
}
```

## Related Files
- CPU board(s): `cpu-board_X1.md`, `cpu-board_X2.md`, `cpu-board_X2.md`
- Function-specific examples (MQTT publish, Modbus, etc.): `NORVI X-AQ4_program.md`
