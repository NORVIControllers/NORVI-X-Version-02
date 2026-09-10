---
type: expansion_module
product: NORVI X-RTD4 — 4-Channel RTD Input Expansion
compatible_cpu_boards: [X1, X2, X3]
bus: I2C
status: Released
last_updated: 2026-08-26
---

# X-RTD4 — Expansion Reference

## Overview
The NORVI X-RTD4 is a high-precision 4-channel RTD temperature input module for the NORVI X industrial controller, using a 15-bit ADC and onboard STM32 MCU for laboratory-grade PT100/PT1000 sensing.
Typical use: precise temperature monitoring in HVAC, building automation, lab/medical equipment, and energy systems where accuracy matters more than the wide range thermocouples offer.

## Specs
| Spec | Value |
|---|---|
| Channels | 4 |
| Signal type | wet contact |
| Resolution | 15bit |
| Input protection | NA |
| Power draw | 80mA |

## I2C Address
| Property | Value |
|---|---|
| Default address | 0x3F |
| Address selectable? | Yes via DIP switch |
| Address range if selectable | 0x30-0x3F |
| Conflicts with | NA |

> This address also gets added to `i2c-address-master-table.md` — that table is the conflict-check reference across the whole product line.

## Pin / Terminal Map
| Terminal | Function | Access | Notes |
|---|---|---|---|
| 1 | RTD1+ | via internal ADC and I2C interface(STM32) | RTD1 positive terminal |
| 2 | RTD1- | via internal ADC and I2C interface(STM32) | RTD1 negative terminal |
| 3 | RTD2+ | via internal ADC and I2C interface(STM32) | RTD2 positive terminal |
| 4 | RTD2- | via internal ADC and I2C interface(STM32) | RTD2 negative terminal |
| 5 | RTD3+ | via internal ADC and I2C interface(STM32) | RTD3 positive terminal |
| 6 | RTD3- | via internal ADC and I2C interface(STM32) | RTD3 negative terminal |
| 7 | RTD4+ | via internal ADC and I2C interface(STM32) | RTD4 positive terminal |
| 8 | RTD4- | via internal ADC and I2C interface(STM32) | RTD4 negative terminal |
| 5 | GND |  | Power ground |


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
 
#define SDA 8 
#define SCL 9 
#define ADDR 0x3F 
#define CHUNK 2 
 
uint8_t rtdType = 1;   //rtdType = 0 → PT100  ,   rtdType = 1 → PT1000 
uint8_t channels[] = {1,2,3,4}; 
uint8_t numChannels = 4; 
 
float temp[4], res[4]; 
uint8_t fault[4]; 
 
uint8_t crc8(uint8_t *d, uint8_t n) 
{ 
  uint8_t c = 0; 
 
  while (n--) 
  { 
    uint8_t x = *d++; 
 
    for (uint8_t i = 8; i; i--) 
    { 
      uint8_t s = (c ^ x) & 1; 
      c >>= 1; 
      if (s) c ^= 0x8C; 
      x >>= 1; 
    } 
  } 
 
  return c; 
} 
 
void readRTD() 
{ 
  for (uint8_t start = 0; start < numChannels; start += CHUNK) 
  { 
    uint8_t count = min((uint8_t)CHUNK, 
                        (uint8_t)(numChannels - start)); 
 
    Wire.beginTransmission(ADDR); 
    Wire.write(0x01); 
    Wire.write(rtdType); 
 
    for (uint8_t i = 0; i < count; i++) 
      Wire.write(channels[start + i]); 
 
    if (Wire.endTransmission()) 
    { 
      Serial.println("I2C TX FAIL"); 
      continue; 
    } 
 
    delay(15); 
 
    uint8_t bytes = count * 12; 
    Wire.requestFrom(ADDR, bytes); 
 
    if (Wire.available() != bytes) 
    { 
      Serial.println("I2C RX FAIL"); 
      continue; 
    } 
 
    for (uint8_t n = 0; n < count; n++) 
    { 
      uint8_t b[12]; 
 
      for (uint8_t i = 0; i < 12; i++) 
        b[i] = Wire.read(); 
 
      if (crc8(b, 11) != b[11]) 
      { 
        Serial.print("CRC ERROR Channel "); 
        Serial.println(b[1]); 
        continue; 
      } 
 
      uint8_t ch = b[1]; 
 
      memcpy(&temp[ch - 1], b + 2, 4); 
      memcpy(&res[ch - 1], b + 6, 4); 
      fault[ch - 1] = b[10]; 
 
      Serial.print("Ch "); 
      Serial.print(ch); 
      Serial.print(" Temp: "); 
      Serial.print(temp[ch - 1]); 
      Serial.print(" Res: "); 
      Serial.print(res[ch - 1]); 
      Serial.print(" Fault: ");  
      Serial.println(fault[ch - 1]); 
    } 
  } 
} 
 
void setup() 
{ 
  Serial.begin(115200); 
  delay(1000); 
 
  Wire.begin(SDA, SCL); 
 
  Serial.println("NORVI RTD4 STARTED"); 
} 
 
void loop() 
{ 
  readRTD(); 
  delay(1000); 
}
```

## Related Files
- CPU board(s): `cpu-board_X1.md`, `cpu-board_X2.md`, `cpu-board_X3.md`
- Function-specific examples (MQTT publish, Modbus, etc.): `NORVI X-RTD4_program.md`
