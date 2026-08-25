---
type: expansion_module
product: NORVI X-AI4 — 4-Channel 4-20mA Current Input Expansion
compatible_cpu_boards: [X1, X2, X3]
bus: I2C
status: Released
last_updated: 2026-08-25
---

# X-AI4 — Expansion Reference

## Overview
The NORVI X-AI4 is a 4-channel analog input expansion module designed to interface with industrial 4–20 mA sensors. It is suitable for monitoring process variables such as pressure, temperature, flow, and level in industrial automation applications.

## Specs
| Spec | Value |
|---|---|
| Channels | 4 |
| Signal type | 4-20mA |
| Resolution | 16bit |
| Input protection | NA |
| Power draw | 40mA |

## I2C Address
| Property | Value |
|---|---|
| Default address | 0x48 |
| Address selectable? | Yes via DIP switch |
| Address range if selectable | 0x48,0x49,0x4A,0x4B |
| Conflicts with | NA |

> This address also gets added to `i2c-address-master-table.md` — that table is the conflict-check reference across the whole product line.

## Pin / Terminal Map
| Terminal | Function | Access | Notes |
|---|---|---|---|
| 1 | Analog Input1 | via internal ADC | 4-20mA current input |
| 2 | Analog Input2 | via internal ADC | 4-20mA current input |
| 3 | Analog Input3 | via internal ADC | 4-20mA current input |
| 4 | Analog Input4 | via internal ADC | 4-20mA current input |
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
| Adafruit_ADS1X15.h | 2.6.2 | ADC  |

## Minimal Working Example
```cpp
#include <Wire.h>
#include <ADS1X15.h>

#define SDA_PIN 8
#define SCL_PIN 9
ADS1115 ADS(0x48);  // change if DIP switches differ (0x49/0x4B/0x4A/0x48)

void setup()
{
  Serial.begin(115200);

  Wire.begin(SDA_PIN, SCL_PIN);

  if (!ADS.begin())
  {
    Serial.println("ADS1115 NOT detected!");
    while (1);
  }

  ADS.setGain(1);

  Serial.println("NORVI X-AI4 READY");
}

void loop()
{
  int16_t value;

  value = ADS.readADC(0);
  Serial.print("AIN1: ");
  Serial.println(value);

  value = ADS.readADC(1);
  Serial.print("AIN2: ");
  Serial.println(value);

  value = ADS.readADC(2);
  Serial.print("AIN3: ");
  Serial.println(value);

  value = ADS.readADC(3);
  Serial.print("AIN4: ");
  Serial.println(value);

  Serial.println("--------------------");

  delay(1000);
}
```

## Related Files
- CPU board(s): `cpu-board_X1.md`, `cpu-board_X2.md`, `cpu-board_X2.md`
- Function-specific examples (MQTT publish, Modbus, etc.): `NORVI X-AI4_program.md`
