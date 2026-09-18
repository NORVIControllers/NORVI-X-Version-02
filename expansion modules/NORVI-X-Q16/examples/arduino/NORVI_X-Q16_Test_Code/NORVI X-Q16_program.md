---
type: Test_program
function: transistor_output_test (16-channel transistor output test via PCA9555 with TFT display)
compatible_cpu_boards: [NORVI X-CPU-ESPS3-X1],[NORVI X-CPU-ESPS3-X2],[NORVI X-CPU-ESPS3-X3]
compatible_modules: [NORVI X-Q16 (16-channel transistor output expansion, PCA9555 I2C IO expander)]
status: completed
last_updated: 2026-08-17
---

# Example: NORVI X-Q16 Transistor Output Test

## What this does
Sequentially toggles all 16 transistor outputs (TR1–TR16) ON then OFF via a PCA9555 I2C IO expander, displaying each output's live ON/OFF state on the TFT screen while logging every state change to Serial, then repeats continuously.

## Hardware required
- CPU board: NORVI X-CPU-ESPS3-X1 OR X2, X3
- Expansion module(s): NORVI X-Q16 (16-channel transistor output module, PCA9555 I2C IO expander)
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
| TR1–TR8 (via PCA9555) | PCA9555 pins 0, 1, 2, 3, 4, 5, 6, 7 |
| TR9–TR16 (via PCA9555) | PCA9555 pins 15, 14, 13, 12, 11, 10, 9, 8 |

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
 * NORVI X-Q16 N16R2
 * 2026.07.20
 */

#include <Wire.h>
#include <WiFi.h>
#include <SPI.h>
#include "clsPCA9555.h"
#include "TFT_eSPI.h"
#include <CST816S.h>
#include "Free_Fonts.h" // Include the header file attached to this sketch

#define PCA_ADDR 0x27 // Base address starts at 0x74 for A0 = L and A1 = L

PCA9555 ioport(PCA_ADDR);

#define SDA   8     
#define SCL   9     

#define MISO 13
#define MOSI 11
#define SCLK 12

#define DSP_CS 45

#define TR1 0
#define TR2 1
#define TR3 2
#define TR4 3
#define TR5 4
#define TR6 5
#define TR7 6
#define TR8 7

#define TR9  15
#define TR10 14
#define TR11 13
#define TR12 12
#define TR13 11
#define TR14 10
#define TR15 9
#define TR16 8

const uint8_t trPin[16] =
{
  TR1, TR2, TR3, TR4,
  TR5, TR6, TR7, TR8,
  TR9, TR10, TR11, TR12,
  TR13, TR14, TR15, TR16
};

const int yPos[16] =
{
   70, 85,100,115,
  130,145,160,175,
  190,205,220,235,
  250,265,280,295
};

CST816S touch(8, 9, 47, 48);  // sda, scl, rst, irq

TFT_eSPI tft = TFT_eSPI();

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("NPD-NORVI X-DI16 TEST");
  delay(1000);

  Wire.begin(SDA, SCL);
  delay(100);

  I2C_SCAN();
  delay(100);

for (int i = 0; i < 16; i++)
{
    ioport.pinMode(trPin[i], OUTPUT);
}

  SPI.begin(SCLK, MISO, MOSI); // Ensure these pin numbers are correct 
  delay(1000);

  tft.init();
  tft.begin();
  tft.setRotation(0);

  tft.fillScreen(TFT_BLACK);            // Clear screen
  tft.setTextColor(TFT_YELLOW);
  tft.setFreeFont(FSB12);
  
  tft.setCursor(60, 20);      //xpos, ypos
  tft.print("NORVI");

  tft.setCursor(20, 40);      //xpos, ypos
  tft.print("x-DI16 TEST");
}

void loop() {

  tft.fillScreen(TFT_BLACK);            // Clear screen
  tft.setTextColor(TFT_GREEN);
  tft.setFreeFont(FSB12);
  
  tft.setCursor(60, 20);      //xpos, ypos
  tft.print("NORVI");

  tft.setCursor(10, 40);      //xpos, ypos
  tft.print("EXPE-Q16 TEST");

  tft.setTextColor(TFT_YELLOW);
tft.setFreeFont(FSB9);

for (int i = 0; i < 16; i++)
{
    tft.setCursor(10, yPos[i]);
    tft.print("TR");
    tft.print(i + 1);
    tft.print(" :");
}
tft.setTextColor(TFT_WHITE);

for (int i = 0; i < 16; i++)
{
    tft.fillRect(120, yPos[i] - 12, 90, 15, TFT_BLACK);

    tft.setCursor(120, yPos[i]);
    tft.print("ON");

    Serial.printf("TR %d : ON\n", i + 1);

    ioport.digitalWrite(trPin[i], HIGH);
    delay(200);

    tft.fillRect(120, yPos[i] - 12, 90, 15, TFT_BLACK);

    tft.setCursor(120, yPos[i]);
    tft.print("OFF");

    Serial.printf("TR %d : OFF\n", i + 1);

    ioport.digitalWrite(trPin[i], LOW);
    delay(200);
}

Serial.println("----------------------------------------");
  
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
- Per-channel state log each cycle: `TR 1 : ON`, `TR 1 : OFF`, `TR 2 : ON`, `TR 2 : OFF` ... through `TR 16 : ON`, `TR 16 : OFF`
- Separator line `----------------------------------------` printed once per full loop cycle

On the TFT display: "NORVI" / "EXPE-Q16 TEST" header, followed by "TR1 :" through "TR16 :" labels, each showing "ON" then "OFF" as the outputs are sequentially toggled, refreshing continuously.

## Common Issues
| Symptom | Likely Cause | Fix |
|---|---|---|
| No I2C device found at 0x27 | PCA9555 not detected — wiring fault, or DIP switch address doesn't match `PCA_ADDR` hardcoded in the sketch | Verify SDA/SCL on GPIO 8/9, and confirm the module's DIP switch setting matches `0x27` (or update `PCA_ADDR` to match a different setting in the `0x20`–`0x27` range) |
| Outputs don't switch connected load even though TFT/Serial shows ON | PCA9555 output not physically connected to the transistor driver stage, or load requires separate power supply | Check wiring from PCA9555 output pins to the X-Q16 transistor driver circuit and confirm board power |
| TFT shows nothing / blank screen | SPI pins (MISO 13 / MOSI 11 / SCLK 12) or Display CS (GPIO 45) miswired, or `tft.init()`/`tft.begin()` failing silently | Verify SPI wiring and TFT_eSPI `User_Setup` pin configuration matches board pinout |
| Upload fails / board not detected | Wrong Board/Tools settings | Match settings to: ESP32S3 Dev Module, UART0/Hardware CDC, QSPI PSRAM, 16MB flash, Huge APP partition |

## Related Files
- Module reference: `expansion_module_Q16.md`
- CPU board: `cpu_board_X1.md`