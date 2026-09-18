---
type: Test_program
function: digital_input_read (16-channel digital input test via PCA9555 with TFT display)
compatible_cpu_boards: [NORVI X-CPU-ESPS3-X1 (N16R2, PSRAM-QSPI)],[NORVI X-CPU-ESPS3-X2 (N16R2, PSRAM-QSPI)],[NORVI X-CPU-ESPS3-X3 (N16R2, PSRAM-QSPI)]
compatible_modules: [NORVI X-DI16 (16-channel digital input expansion, PCA9555 I2C IO expander)]
status: completed
last_updated: 2026-08-17
---

# Example: NORVI X-DI16 Digital Input Test

## What this does
Reads 16 digital input channels (DI1–DI16) via a PCA9555 I2C IO expander in a loop, updating only the channels whose state changed on the TFT screen (green `1` for HIGH, red `0` for LOW), with labels for all 16 channels drawn once at startup.

## Hardware required
- CPU board: NORVI X-CPU-ESPS3-X1 OR X2, X3 (ESP32-S3, N16R2, PSRAM-QSPI)
- Expansion module(s): NORVI X-DI16 (16-channel digital input module, PCA9555 I2C IO expander)
  - PCA9555 I2C address is DIP-switch selectable, default `0x27`, range `0x20`–`0x27` depending on DIP switch setting
- Wiring / pin map (from firmware `#define`s):

| Signal | GPIO |
|---|---|
| I2C SDA | 8 |
| I2C SCL | 9 |
| SPI MISO | 13 |
| SPI MOSI | 11 |
| SPI SCLK | 12 |
| TFT Display CS | 45 |
| Touch SDA / SCL / RST / IRQ | 8, 9, 47, 48 |
| DI1–DI8 (via PCA9555) | PCA9555 pins PCA_A0–A7 (0–7) |
| DI9–DI16 (via PCA9555) | PCA9555 pins PCA_B0–B7 (8–15) |

## Libraries required
| Library | Version |
|---|---|
| Wire (ESP32 core) | 2.0.0 |
| WiFi (ESP32 core) | 2.0.0 |
| SPI (ESP32 core) | 2.0.0 |
| clsPCA9555 |
| TFT_eSPI | 2.5.0 |
| CST816S | 1.3.0 |
| Free_Fonts.h | local header, bundled with sketch |

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

/*
 * NORVI X-DI16 N16R2
 * 2026.07.20
 */

#include <Wire.h>
#include <WiFi.h>
#include <SPI.h>
#include "clsPCA9555.h"
#include "TFT_eSPI.h"
#include <CST816S.h>
#include "Free_Fonts.h" // Include the header file attached to this sketch

PCA9555 ioport(0x27); // Base address starts at 0x74 for A0 = L and A1 = L

#define SDA   8
#define SCL   9

#define MISO 13
#define MOSI 11
#define SCLK 12

#define DSP_CS 45

#define PCA_A0 0
#define PCA_A1 1
#define PCA_A2 2
#define PCA_A3 3
#define PCA_A4 4
#define PCA_A5 5
#define PCA_A6 6
#define PCA_A7 7

#define PCA_B0 8
#define PCA_B1 9
#define PCA_B2 10
#define PCA_B3 11
#define PCA_B4 12
#define PCA_B5 13
#define PCA_B6 14
#define PCA_B7 15


CST816S touch(8, 9, 47, 48);  // sda, scl, rst, irq

TFT_eSPI tft = TFT_eSPI();

int oldState[16] =
{
  -1, -1, -1, -1,
  -1, -1, -1, -1,
  -1, -1, -1, -1,
  -1, -1, -1, -1
};


void setup() {
  Serial.begin(115200);
  delay(100);

  Serial.println("NPD-NORVI X-DI16 TEST");
  delay(1000);


  Wire.begin(SDA, SCL);
  delay(200);

  I2C_SCAN();
  delay(1000);

  ioport.pinMode(PCA_A0, INPUT);
  ioport.pinMode(PCA_A1, INPUT);
  ioport.pinMode(PCA_A2, INPUT);
  ioport.pinMode(PCA_A3, INPUT);
  ioport.pinMode(PCA_A4, INPUT);
  ioport.pinMode(PCA_A5, INPUT);
  ioport.pinMode(PCA_A6, INPUT);
  ioport.pinMode(PCA_A7, INPUT);

  ioport.pinMode(PCA_B0, INPUT);
  ioport.pinMode(PCA_B1, INPUT);
  ioport.pinMode(PCA_B2, INPUT);
  ioport.pinMode(PCA_B3, INPUT);
  ioport.pinMode(PCA_B4, INPUT);
  ioport.pinMode(PCA_B5, INPUT);
  ioport.pinMode(PCA_B6, INPUT);
  ioport.pinMode(PCA_B7, INPUT);

  SPI.begin(SCLK, MISO, MOSI); // Ensure these pin numbers are correct
  delay(1000);

  tft.init();
  tft.begin();
  tft.setRotation(0);
  drawLabel();


}

void loop()
{
  int state[16];

  // Read Inputs
  state[0]  = ioport.digitalRead(PCA_A0);
  state[1]  = ioport.digitalRead(PCA_A1);
  state[2]  = ioport.digitalRead(PCA_A2);
  state[3]  = ioport.digitalRead(PCA_A3);
  state[4]  = ioport.digitalRead(PCA_A4);
  state[5]  = ioport.digitalRead(PCA_A5);
  state[6]  = ioport.digitalRead(PCA_A6);
  state[7]  = ioport.digitalRead(PCA_A7);

  state[8]  = ioport.digitalRead(PCA_B0);
  state[9]  = ioport.digitalRead(PCA_B1);
  state[10] = ioport.digitalRead(PCA_B2);
  state[11] = ioport.digitalRead(PCA_B3);
  state[12] = ioport.digitalRead(PCA_B4);
  state[13] = ioport.digitalRead(PCA_B5);
  state[14] = ioport.digitalRead(PCA_B6);
  state[15] = ioport.digitalRead(PCA_B7);

  // Update only changed values
  for (int i = 0; i < 16; i++)
  {
    if (state[i] != oldState[i])
    {
      updateInput(i, state[i]);
      oldState[i] = state[i];
    }
  }

  delay(10);
}

void updateInput(uint8_t ch, uint8_t value)
{
  int x, y;

  if(ch < 8)
  {
    x = 60;
    y = 80 + (ch * 30);
  }
  else
  {
    x = 190;
    y = 80 + ((ch - 8) * 30);
  }

  tft.fillRect(x, y - 18, 20, 20, TFT_BLACK);

  if(value)
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
  else
    tft.setTextColor(TFT_RED, TFT_BLACK);

  tft.setCursor(x, y);
  tft.print(value);
}

void drawLabel()
{
  tft.fillScreen(TFT_BLACK);

  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setFreeFont(FSB12);

  tft.setCursor(50,20);
  tft.print("  NORVI");

  tft.setCursor(10,45);
  tft.print("X-DI16 INPUT TEST");

  tft.setFreeFont(FSB9);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  // Left side
  tft.setCursor(0,80);   tft.print("DI1:");
  tft.setCursor(0,110);  tft.print("DI2:");
  tft.setCursor(0,140);  tft.print("DI3:");
  tft.setCursor(0,170);  tft.print("DI4:");
  tft.setCursor(0,200);  tft.print("DI5:");
  tft.setCursor(0,230);  tft.print("DI6:");
  tft.setCursor(0,260);  tft.print("DI7:");
  tft.setCursor(0,290);  tft.print("DI8:");

  // Right side
  tft.setCursor(120,80);   tft.print("DI9:");
  tft.setCursor(120,110);  tft.print("DI10:");
  tft.setCursor(120,140);  tft.print("DI11:");
  tft.setCursor(120,170);  tft.print("DI12:");
  tft.setCursor(120,200);  tft.print("DI13:");
  tft.setCursor(120,230);  tft.print("DI14:");
  tft.setCursor(120,260);  tft.print("DI15:");
  tft.setCursor(120,290);  tft.print("DI16:");
}


void I2C_SCAN() {
  byte error, address;
  int deviceCount = 0;

  Serial.println("Scanning...");

  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address < 16) {
        Serial.print("0");
      }
      Serial.print(address, HEX);
      Serial.println("  !");

      deviceCount++;
      delay(1);  // Wait for a moment to avoid overloading the I2C bus
    }
    else if (error == 4) {
      Serial.print("Unknown error at address 0x");
      if (address < 16) {
        Serial.print("0");
      }
      Serial.println(address, HEX);
    }
  }

  if (deviceCount == 0) {
    Serial.println("No I2C devices found\n");
  }
  else {
    Serial.println("Scanning complete\n");
  }
}
```

## Expected Output
On the Serial Monitor (115200 baud):
- `NPD-NORVI X-DI16 TEST`
- I2C scan results, e.g. `I2C device found at address 0x27  !`

On the TFT display: "NORVI" / "X-DI16 INPUT TEST" header, followed by "DI1:" through "DI16:" labels (8 on the left, 8 on the right), each showing a live `1` (green, HIGH) or `0` (red, LOW) that updates only when that channel's state changes.

## Common Issues
| Symptom | Likely Cause | Fix |
|---|---|---|
| No I2C device found at 0x27 | PCA9555 not detected — wiring fault, or DIP switch address doesn't match `0x27` hardcoded in the sketch | Verify SDA/SCL on GPIO 8/9, and confirm the module's DIP switch setting matches `0x27` (or update the address passed to `PCA9555 ioport(...)` to match a different setting in the `0x20`–`0x27` range) |
| Input value never changes on TFT despite toggling the physical signal | Input floating/miswired, or channel's state hasn't actually changed since last read | Verify wiring to the X-DI16 module's terminal map for the affected channel; the display only redraws on a state change, not every loop cycle |
| TFT shows nothing / blank screen | SPI pins (MISO 13 / MOSI 11 / SCLK 12) or Display CS (GPIO 45) miswired, or `tft.init()`/`tft.begin()` failing silently | Verify SPI wiring and TFT_eSPI `User_Setup` pin configuration matches board pinout |
| Upload fails / board not detected | Wrong Board/Tools settings | Match settings to: ESP32S3 Dev Module, UART0/Hardware CDC, QSPI PSRAM, 16MB flash, Huge APP partition |

## Related Files
- Module reference: `expansion_module_DI16.md`
- CPU board: `cpu_board_X1.md`