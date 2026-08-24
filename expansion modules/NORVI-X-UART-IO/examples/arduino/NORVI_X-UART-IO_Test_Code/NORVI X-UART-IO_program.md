---
type: Test_program
function: uart_mode_switch_test (dual-UART RS485/RS232/UARTA/UARTB mode-select test with direct GPIO outputs)
compatible_cpu_boards: [NORVI X-CPU-ESPS3-X1(ESP-N16R2 PSRAM-QSPI)],[NORVI X-CPU-ESPS3-X2(ESP-N16R2 PSRAM-QSPI)],[NORVI X-CPU-ESPS3-X3(ESP-N16R2 PSRAM-QSPI)]
compatible_modules: [NORVI X-UART-IO (dual-UART expansion with mode-select via PCA9539 I2C IO expander)]
status: completed
last_updated: 2026-08-17
---

# Example: NORVI X-UART-IO Dual-UART Mode Test

## What this does
Provides a Serial command menu (`1` or `2`) to switch the module between two operating modes: Mode 1 routes UARTA/UARTB as RS485 + RS232 and sends test strings ("RS485 01 SUCCESS", "RS232 01 SUCCESS") with any response echoed back to Serial; Mode 2 routes the same physical UARTs as UARTA/UARTB and sends test strings ("UARTA SUCCESS", "UARTB SUCCESS") similarly echoed back. UART routing (RS485/RS232 vs UARTA/UARTB) is selected via two PCA9539 I2C-expander output pins (UARTA_SEL/UARTB_SEL). Two direct-GPIO outputs (OUTPUT1/OUTPUT2) are pulsed HIGH for 200ms every loop cycle regardless of mode.

## Hardware required
- CPU board: NORVI X-CPU-ESPS3-X1 OR X2, X3 (ESP-N16R2 PSRAM-QSPI)
- Expansion module(s): NORVI X-UART-IO (dual-UART expansion module; UART interface mode — RS485/RS232 vs UARTA/UARTB — selected via onboard PCA9539 I2C IO expander)
- Wiring / pin map (from firmware `#define`s):

| Signal | GPIO |
|---|---|
| I2C SDA | 8 |
| I2C SCL | 9 |
| IO Expander Reset (PORT_RST) | 38 |
| OUTPUT1 | 10 |
| OUTPUT2 | 14 |
| UARTA RXD | 5 |
| UARTA TXD | 6 |
| UARTB RXD | 7 |
| UARTB TXD | 10 (shared with OUTPUT1 — see note below) |
| UARTA Select (UARTA_SEL) | via PCA9539 pin PCA_B3 (not direct GPIO) |
| UARTB Select (UARTB_SEL) | via PCA9539 pin PCA_B4 (not direct GPIO) |

Note: UARTB_TXD and OUTPUT1 share GPIO 10. This is by design — the module operates in one selected mode at a time (1 = RS485/RS232, 2 = UARTA/UARTB), so GPIO 10 is never used as OUTPUT1 and UARTB TXD simultaneously. Select Mode 2 when UARTB is needed; use Mode 1 (RS485/RS232) otherwise.

## Libraries required
| Library | Version |
|---|---|
| Wire (ESP32 core) | 2.0.0 |
| PCA9539-master | legacy (no version tag) |

**ESP32 Board Package version:** 2.0.12

**Arduino IDE Board/Tools settings:**
| Setting | Value |
|---|---|
| Board | ESP32S3 Dev Module |
| Upload Speed | 115200 |
| USB Mode | Hardware CDC and JTAG |
| USB CDC On Boot | Enabled |
| USB Firmware MSC On Boot | Disabled |
| USB DFU On Boot | Disabled |
| Upload Mode | UART0 / Hardware CDC |
| CPU Frequency | 240MHz (WiFi) |
| Flash Mode | QIO 80MHz |
| Flash Size | 16MB (128Mb) |
| Partition Scheme | Huge APP (3MB No OTA/1MB SPIFFS) |
| Core Debug Level | None |
| PSRAM | QSPI PSRAM |
| Arduino Runs On | Core 1 |
| Events Run On | Core 1 |
| Erase All Flash Before Sketch Upload | Disabled |
| JTAG Adapter | Disabled |

## Full Sketch
```cpp
//"NPD-NORVI X-UART-IO-V2 (OPSRAM)"
 
#include <Wire.h>
#include "PCA9539.h"

PCA9539 ioport(0x75);

// ================= MODE =================
uint8_t mode = 1;
uint8_t lastMode = 0;

// ================= PINS =================
#define PORT_RST 38
#define OUTPUT1 10
#define OUTPUT2 14
#define UARTA_RXD 5
#define UARTA_TXD 6
#define UARTA_SEL PCA_B3

#define UARTB_RXD 7
#define UARTB_TXD 10
#define UARTB_SEL PCA_B4  

#define SDA 8     
#define SCL 9 

// ================= SETUP =================
void setup() {
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
  printCommands();
  I2C_SCAN();
}

// ================= LOOP =================
void loop() {

  // ----- READ COMMAND -----
  if (Serial.available()) {
    char input = Serial.read();

    if (input == '1') mode = 1;
    else if (input == '2') mode = 2;

    Serial.print("MODE SELECTED: ");
    Serial.println(mode);
  }

  // ----- MODE CHANGE -----
  if (mode != lastMode) {
    Serial.println("MODE CHANGED");

    switch (mode) {
      case 1: setupFirstCode(); break;
      case 2: setupSecondCode(); break;
    }
    lastMode = mode;
  }

  // ----- RUN MODE -----
  switch (mode) {
    case 1: firstCode(); break;
    case 2: secondCode(); break;
  }
   digitalWrite(OUTPUT1, HIGH);
  digitalWrite(OUTPUT2, HIGH);
    delay(200);
  digitalWrite(OUTPUT1, LOW);
  digitalWrite(OUTPUT2, LOW);
}

// ================= MODE 1 =================
void setupFirstCode() {
  Serial.println("MODE 1: RS485 + RS232");

  Serial1.end();
  Serial2.end();

  Serial1.begin(115200, SERIAL_8N1, UARTA_RXD, UARTA_TXD);
  Serial2.begin(115200, SERIAL_8N1, UARTB_RXD, UARTB_TXD);
}

void firstCode() {
  RS485_TEST();
  RS232_TEST();
  delay(1000);
}

// ================= MODE 2 =================
void setupSecondCode() {
  Serial.println("MODE 2: UARTA + UARTB");


  Serial1.end();
  Serial2.end();

  Serial1.begin(115200, SERIAL_8N1, UARTA_RXD, UARTA_TXD);
  Serial2.begin(115200, SERIAL_8N1, UARTB_RXD, UARTB_TXD);
}

void secondCode() {
  UARTA_TEST();
  UARTB_TEST();
  delay(1000);
}

// ================= TEST FUNCTIONS =================
void RS485_TEST() {
  
  ioport.digitalWrite(UARTA_SEL, LOW);
  delay(50);

  Serial1.println("RS485 01 SUCCESS");
  Serial1.flush();

  delay(500);
  while (Serial1.available()) {
    Serial.write(Serial1.read());
  }
   delay(1000);
}

void RS232_TEST() {
  ioport.digitalWrite(UARTB_SEL, LOW);
  delay(50);

  Serial2.println("RS232 01 SUCCESS");
  Serial2.flush();

  delay(50);
  while (Serial2.available()) {
    Serial.write(Serial2.read());
  }
}

void UARTA_TEST() {
  ioport.digitalWrite(UARTA_SEL, HIGH);
  delay(50);

  Serial1.println("UARTA SUCCESS");
  Serial1.flush();

  delay(50);
  while (Serial1.available()) {
    Serial.write(Serial1.read());
  }
}

void UARTB_TEST() {
  ioport.digitalWrite(UARTB_SEL, HIGH);
  delay(50);

  Serial2.println("UARTB SUCCESS");
  Serial2.flush();

  delay(50);
  while (Serial2.available()) {
    Serial.write(Serial2.read());
  }
}

// ================= I2C SCAN =================
void I2C_SCAN() {
  Serial.println("Scanning I2C...");
  for (byte addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.print("Found I2C: 0x");
      Serial.println(addr, HEX);
    }
  }
  Serial.println("I2C Scan Done");
}

// ================= MENU =================
void printCommands() {
  Serial.println();
  Serial.println("========= COMMAND MENU =========");
  Serial.println("1 -> RS485 / RS232 MODE");
  Serial.println("2 -> UARTA / UARTB MODE");
  Serial.println("================================");
}
```

## Expected Output
On the Serial Monitor (115200 baud):
- Boot menu:
========= COMMAND MENU =========
1 -> RS485 / RS232 MODE
2 -> UARTA / UARTB MODE

- `Scanning I2C...`, `Found I2C: 0x75`, `I2C Scan Done`
- Sending `1` or `2` prints `MODE SELECTED: 1` (or `2`), then `MODE CHANGED` and `MODE 1: RS485 + RS232` (or `MODE 2: UARTA + UARTB`) on the first cycle after switching
- In Mode 1: echoed-back bytes forming `RS485 01 SUCCESS` and `RS232 01 SUCCESS` (looped back through the transceiver)
- In Mode 2: echoed-back bytes forming `UARTA SUCCESS` and `UARTB SUCCESS`

## Common Issues
| Symptom | Likely Cause | Fix |
|---|---|---|
| No I2C device found at 0x75 | PCA9539 not detected — wiring fault, or PORT_RST (GPIO 38) not held HIGH | Verify SDA/SCL on GPIO 8/9 and confirm PORT_RST is driven HIGH before Wire init |
| No echoed data in Mode 1 or Mode 2 | UARTA_SEL/UARTB_SEL not switching correctly on the PCA9539, or nothing looped back on the physical UART lines | Confirm the module's UART select lines respond to the PCA9539 I2C writes, and verify wiring/loopback on UARTA/UARTB terminals |
| UARTB doesn't work as expected | Board is in Mode 1 (RS485/RS232), where GPIO 10 is being driven as OUTPUT1 instead of UARTB TXD | Send 2 over Serial to switch to Mode 2 (UARTA/UARTB) before testing UARTB |
| Mode doesn't switch when sending `1` or `2` | Serial Monitor sending characters with unexpected line-endings, or a different key was sent | Send exactly the character `1` or `2` (no newline required, since the sketch reads a single char) |
| Upload fails / board not detected | Wrong Board/Tools settings | Match settings to: ESP32S3 Dev Module, UART0/Hardware CDC, QSPI PSRAM, 16MB flash, Huge APP partition |

## Related Files
- Module reference: `expansion_module_UART-IO.md`
- CPU board: `cpu_board_X1.md`