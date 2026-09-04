---
type: expansion_module
product: NORVI X-T4 — 4-Channel Thermistor Input Expansion
compatible_cpu_boards: [X1, X2, X3]
bus: I2C
status: Released
last_updated: 2026-09-04
---

# X-T4 — Expansion Reference

## Overview
The NORVI X-T4 is a 4-channel thermistor input module for the NORVI X industrial controller. It uses ADS1115 ADCs to interface with NTC thermistors for accurate temperature measurement.
Typical use: HVAC, industrial temperature monitoring, refrigeration, and environmental sensing.

## Specs
| Spec | Value |
|---|---|
| Channels | 4 |
| Signal type | wet contact |
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
| 1 | TM1+ | Thermistor1 | via internal ADC | Thermistor1 positive terminal |
| 2 | TM1- | Thermistor1 | via internal ADC | Thermistor1 negative terminal |
| 3 | TM2+ | Thermistor2 | via internal ADC | Thermistor2 positive terminal |
| 4 | TM2- | Thermistor2 | via internal ADC | Thermistor2 negative terminal |
| 5 | TM3+ | Thermistor3 | via internal ADC | Thermistor3 positive terminal |
| 6 | TM3- | Thermistor3 | via internal ADC | Thermistor3 negative terminal |
| 7 | TM4+ | Thermistor4 | via internal ADC | Thermistor4 positive terminal |
| 8 | TM4- | Thermistor4 | via internal ADC | Thermistor4 negative terminal |
| 9 | GND |  | Power ground |


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
|math.h| | Built directly into the underlying compiler|

## Minimal Working Example
```cpp
#include <Wire.h>
#include <ADS1X15.h>
#include <math.h>

#define SDA_PIN 8
#define SCL_PIN 9
ADS1115 ADS(0x48);  // change if DIP switches differ (0x49/0x4B/0x4A/0x48)

// ---------- THERMISTOR / DIVIDER CONFIG ----------
const float SUPPLY_VOLTAGE      = 3.3;    // voltage feeding the top of the divider
const float SERIES_RESISTOR     = 10000.0; // the fixed 10K resistor
const float THERMISTOR_NOMINAL  = 10000.0; // NTC resistance at 25C
const float TEMPERATURE_NOMINAL = 25.0;    // reference temp, deg C
const float B_COEFFICIENT       = 3950.0;  // beta value from your NTC's datasheet -- CHECK/UPDATE THIS

// Converts a voltage reading into temperature (deg C) via the Beta equation
float voltageToTempC(float voltage)
{
  if (voltage <= 0.001 || voltage >= SUPPLY_VOLTAGE - 0.001)
  {
    return NAN; // open circuit / shorted / bad reading
  }

  float ntcResistance = SERIES_RESISTOR * (SUPPLY_VOLTAGE / voltage - 1.0);

  float steinhart = ntcResistance / THERMISTOR_NOMINAL;
  steinhart = log(steinhart);
  steinhart /= B_COEFFICIENT;
  steinhart += 1.0 / (TEMPERATURE_NOMINAL + 273.15);
  steinhart = 1.0 / steinhart;
  steinhart -= 273.15;

  return steinhart;
}

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

  Serial.println("NORVI X-T4 READY");
}

void loop()
{
  int16_t raw;
  float voltage;
  float tempC;

  raw = ADS.readADC(0);
  voltage = ADS.toVoltage(raw);
  tempC = voltageToTempC(voltage);
  Serial.print("T1: ");
  Serial.println(tempC);

  raw = ADS.readADC(1);
  voltage = ADS.toVoltage(raw);
  tempC = voltageToTempC(voltage);
  Serial.print("T2: ");
  Serial.println(tempC);

  raw = ADS.readADC(2);
  voltage = ADS.toVoltage(raw);
  tempC = voltageToTempC(voltage);
  Serial.print("T3: ");
  Serial.println(tempC);

  raw = ADS.readADC(3);
  voltage = ADS.toVoltage(raw);
  tempC = voltageToTempC(voltage);
  Serial.print("T4: ");
  Serial.println(tempC);

  Serial.println("--------------------");

  delay(1000);
}
```

## Related Files
- CPU board(s): `cpu-board_X1.md`, `cpu-board_X2.md`, `cpu-board_X3.md`
- Function-specific examples (MQTT publish, Modbus, etc.): `NORVI X-T4_program.md`
