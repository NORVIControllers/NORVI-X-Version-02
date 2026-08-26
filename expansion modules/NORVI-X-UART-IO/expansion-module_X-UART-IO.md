---
type: expansion_module
product: NORVI X-UART-IO — Communication/IO Expansion
compatible_cpu_boards: [X1, X2, X3]
bus: Direct GPIO
status: relaesed
last_updated: 2026-08-25
---

# X-UART-IO — Expansion Reference

## Overview
The NORVI X-UARTIO is a serial communication expansion module for the NORVI X industrial controller, providing RS485, RS232, and multiplexed UART interfaces via GPIO/CS-controlled switching. Typical use: connecting industrial sensors, meters, PLCs, and legacy RS232 devices, or running Modbus RTU over RS485 in fieldbus setups.

## Specs
| Spec | Value |
|---|---|
| Channels | 4 |
| Signal type | dry contact |
| Resolution | NA |
| Input protection | ESD |
| Power draw | 40mA |

## I2C Address
| Property | Value |
|---|---|
| Default address | NA |
| Address selectable? | No, fixed |
| Address range if selectable | NA |
| Conflicts with | Only one DI4/UART-IO module per system |

> This address also gets added to `i2c-address-master-table.md` — that table is the conflict-check reference across the whole product line.

## Pin / Terminal Map
| Terminal | Function | Access | Notes |
|---|---|---|---|
| 1 | RS485_A | Native to module's main chip through mux | RS485 communication input |
| 2 | RS485_B | Native to module's main chip through mux | RS485 communication input |
| 3 | RS232_RX- | Native to module's main chip through mux | RS232 communication input |
| 4 | RS232_TX- | Native to module's main chip through mux | RS232 communication input |
| 5 | UART1_RX | Native to module's main chip through mux | UART communication / Direct GPIO |
| 6 | UART1_TX | Native to module's main chip through mux | UART communication / Direct GPIO |
| 5 | UART2_RX | Native to module's main chip through mux | UART communication / Direct GPIO |
| 6 | UART2_TX | Native to module's main chip through mux | UART communication / Direct GPIO |
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
| PCA9539.h |  | GPIO Expander |

## Minimal Working Example
```cpp
#include <Wire.h>
#include "PCA9539.h"

PCA9539 ioport(0x75);

uint8_t mode = 1, lastMode = 0;

#define PORT_RST   38
#define OUTPUT1    10
#define OUTPUT2    14

#define UARTA_RXD  5
#define UARTA_TXD  6
#define UARTA_SEL  PCA_B3

#define UARTB_RXD  7
#define UARTB_TXD  10
#define UARTB_SEL  PCA_B4

#define SDA 8
#define SCL 9

void setup()
{
  Serial.begin(115200);
  delay(1000);

  Wire.begin(SDA, SCL);

  pinMode(PORT_RST, OUTPUT);
  digitalWrite(PORT_RST, HIGH);
  delay(100);

  ioport.pinMode(UARTA_SEL, OUTPUT);
  ioport.pinMode(UARTB_SEL, OUTPUT);

  pinMode(OUTPUT1, OUTPUT);
  pinMode(OUTPUT2, OUTPUT);

  Serial.println("1 -> RS485 / RS232   2 -> UART1 / UART2");
}

void loop()
{
  if (Serial.available())
  {
    char input = Serial.read();
    if (input == '1') mode = 1;
    else if (input == '2') mode = 2;

    Serial.print("MODE SELECTED: ");
    Serial.println(mode);
  }

  if (mode != lastMode)
  {
    setupUART();
    lastMode = mode;
  }

  if (mode == 1)
  {
    test(UARTA_SEL, LOW, Serial1, "RS485 01 SUCCESS", 500, 1000);
    test(UARTB_SEL, LOW, Serial2, "RS232 01 SUCCESS", 50, 0);
  }
  else
  {
    test(UARTA_SEL, HIGH, Serial1, "UARTA SUCCESS", 50, 0);
    test(UARTB_SEL, HIGH, Serial2, "UARTB SUCCESS", 50, 0);
  }

  digitalWrite(OUTPUT1, HIGH);
  digitalWrite(OUTPUT2, HIGH);
  delay(200);
  digitalWrite(OUTPUT1, LOW);
  digitalWrite(OUTPUT2, LOW);

  delay(1000);
}

void setupUART()
{
  Serial1.end();
  Serial2.end();

  Serial1.begin(115200, SERIAL_8N1, UARTA_RXD, UARTA_TXD);
  Serial2.begin(115200, SERIAL_8N1, UARTB_RXD, UARTB_TXD);
}

void test(uint8_t sel, uint8_t state, HardwareSerial &uart,
          const char *msg, int waitTime, int extraDelay)
{
  ioport.digitalWrite(sel, state);
  delay(50);

  uart.println(msg);
  uart.flush();

  delay(waitTime);

  while (uart.available())
    Serial.write(uart.read());

  delay(extraDelay);
}
```

## Related Files
- CPU board(s): `cpu-board_X1.md`, `cpu-board_X2.md`, `cpu-board_X3.md`
- Function-specific examples (MQTT publish, Modbus, etc.): `NORVI X-UART-IO_program.md`
