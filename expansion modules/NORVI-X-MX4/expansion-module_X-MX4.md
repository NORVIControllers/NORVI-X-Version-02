---
type: expansion_module
product: NORVI X-MX4 — 4-Channel Digital Input, 4-Channel Analog Input, 4-Channel Transistor Output Expansion
compatible_cpu_boards: [X1, X2, X3]
bus: I2C
status: relaesed
last_updated: 2026-09-04
---

# X-MX4 — Expansion Reference

## Overview
The NORVI X-MX4 is a mixed I/O expansion module that combines 4 digital inputs, 4 transistor outputs, and 4 analog inputs in a single module. It is suitable for industrial automation applications requiring sensor monitoring, digital control, and analog signal measurement through the NORVI X expansion interface.

## Specs: Digital Input
| Spec | Value |
|---|---|
| Channels | 4 |
| Signal type | dry contact |
| Resolution | 1bit |
| Input protection | Opto isolated |
| Power draw | 90mA |

## Specs: Analog Input
| Spec | Value |
|---|---|
| Channels | 4 |
| Signal type | 4-20mA |
| Resolution | 16bit |
| Input protection | NA |
| Power draw | 90mA |

## Specs: Transistor Output
| Spec | Value |
|---|---|
| Channels | 4 |
| Signal type | wet contact |
| Resolution | NA |
| Input protection | Resettable Fuse |
| Power draw | 90mA |

## I2C Address: Analog Input
| Property | Value |
|---|---|
| Default address | 0x4A |
| Address selectable? | Yes via DIP switch |
| Address range if selectable | 0x48,0x49,0x4A,0x4B |
| Conflicts with | NA |

## I2C Address: Digital Input & Transistor Output
| Property | Value |
|---|---|
| Default address | 0x71 |
| Address selectable? | Yes via DIP switch |
| Address range if selectable | 0x70,0x71,0x72,0x73 |
| Conflicts with | NA |

> This address also gets added to `i2c-address-master-table.md` — that table is the conflict-check reference across the whole product line.

## Pin / Terminal Map
| Terminal | Function | Access | Notes |
|---|---|---|---|
| 1 | Digital Input1 | via internal IO Expander | Digital Input (sink or source) |
| 2 | Digital Input2 | via internal IO Expander | Digital Input (sink or source) |
| 3 | Digital Input3 | via internal IO Expander | Digital Input (sink or source) |
| 4 | Digital Input4 | via internal IO Expander | Digital Input (sink or source) |
| 5 | Digital Com |  | Digital input isolated ground |
| 6 | Transistor Output1 | via internal IO Expander | 0.5A 24V Transistor Output |
| 7 | Transistor Output2 | via internal IO Expander | 0.5A 24V Transistor Output |
| 8 | Transistor Output3 | via internal IO Expander | 0.5A 24V Transistor Output |
| 9 | Transistor Output4 | via internal IO Expander | 0.5A 24V Transistor Output |
| 10 | GND |  | Power Ground |
| 11 | Analog Input1 | via internal ADC | 4-20mA current input |
| 12 | Analog Input2 | via internal ADC | 4-20mA current input |
| 13 | Analog Input3 | via internal ADC | 4-20mA current input |
| 14 | Analog Input4 | via internal ADC | 4-20mA current input |
| 15 | AGND |  | Analog ground |
| 16 | GND |  | Power Ground |
| 17 | GND |  | Power Ground |
| 18 | GND |  | Power Ground |

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
| PCA9538.h |  | GPIO Expander  |

## Minimal Working Example
```cpp
#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <PCA9538.h>

#define SDA_PIN   8
#define SCL_PIN   9

#define ADS_ADDR  0x4A

#define PCA9538_ADDR 0x71
#define PCA_RESET    21

// Digital output pins on the PCA9538
#define DO1_PIN 1
#define DO2_PIN 2
#define DO3_PIN 3
#define DO4_PIN 4

// Digital input pins on the PCA9538
#define DI1_PIN 5
#define DI2_PIN 6
#define DI3_PIN 7
#define DI4_PIN 8

// Walking test timing - one output ON at a time, moves to the next every
// OUT_STEP_MS
const unsigned long OUT_STEP_MS = 1000;

Adafruit_ADS1115 ads1;
PCA9538 io(PCA9538_ADDR);

const float mA_Factor = 4.096 / 3269.826;

int activeOutput = 0;
unsigned long lastOutSwitch = 0;

void setup() {

  Serial.begin(115200);
  delay(500);

  // PCA9538 RESET must be released before it will respond on I2C
  pinMode(PCA_RESET, OUTPUT);
  digitalWrite(PCA_RESET, HIGH);

  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(400000);

  ads1.begin(ADS_ADDR);
  ads1.setGain(GAIN_ONE);
  ads1.setDataRate(RATE_ADS1115_860SPS);

  // Digital inputs
  io.pinMode(DI1_PIN, INPUT);
  io.pinMode(DI2_PIN, INPUT);
  io.pinMode(DI3_PIN, INPUT);
  io.pinMode(DI4_PIN, INPUT);

  // Digital outputs, start all OFF
  io.pinMode(DO1_PIN, OUTPUT);
  io.pinMode(DO2_PIN, OUTPUT);
  io.pinMode(DO3_PIN, OUTPUT);
  io.pinMode(DO4_PIN, OUTPUT);
  io.digitalWrite(DO1_PIN, LOW);
  io.digitalWrite(DO2_PIN, LOW);
  io.digitalWrite(DO3_PIN, LOW);
  io.digitalWrite(DO4_PIN, LOW);

  lastOutSwitch = millis();
}

void loop() {

  // Walking output test - one output ON at a time
  if (millis() - lastOutSwitch >= OUT_STEP_MS) {
    lastOutSwitch = millis();

    io.digitalWrite(DO1_PIN, activeOutput == 0 ? HIGH : LOW);
    io.digitalWrite(DO2_PIN, activeOutput == 1 ? HIGH : LOW);
    io.digitalWrite(DO3_PIN, activeOutput == 2 ? HIGH : LOW);
    io.digitalWrite(DO4_PIN, activeOutput == 3 ? HIGH : LOW);

    Serial.printf("OUTPUT %d ON\n", activeOutput + 1);

    activeOutput = (activeOutput + 1) % 4;
  }

  float ch1 = ads1.readADC_SingleEnded(0) * mA_Factor;
  float ch2 = ads1.readADC_SingleEnded(1) * mA_Factor;
  float ch3 = ads1.readADC_SingleEnded(2) * mA_Factor;
  float ch4 = ads1.readADC_SingleEnded(3) * mA_Factor;

  // Inputs are pulled HIGH in the circuit, so invert to get true logical state
  int in1 = !io.digitalRead(DI1_PIN);
  int in2 = !io.digitalRead(DI2_PIN);
  int in3 = !io.digitalRead(DI3_PIN);
  int in4 = !io.digitalRead(DI4_PIN);

  Serial.printf("CH1: %.2f mA  CH2: %.2f mA  CH3: %.2f mA  CH4: %.2f mA  |  DI1: %d DI2: %d DI3: %d DI4: %d\n",
                ch1, ch2, ch3, ch4, in1, in2, in3, in4);

  delay(300);
}

```

## Related Files
- CPU board(s): `cpu-board_X1.md`, `cpu-board_X2.md`, `cpu-board_X3.md`
- Function-specific examples (MQTT publish, Modbus, etc.): `NORVI X-MX4_program.md`
