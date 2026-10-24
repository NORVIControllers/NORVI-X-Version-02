---
type: Test_program
function: transistor_output_test (16-channel transistor output test via PCA9555 with TFT display)
compatible_cpu_boards: [NORVI X-CPU-ESPS3-X1],[NORVI X-CPU-ESPS3-X2],[NORVI X-CPU-ESPS3-X3]
compatible_modules: [NORVI X-Q16 (16-channel transistor output expansion, PCA9555 I2C IO expander)]
status: completed
last_updated: 2026-10-22
---

# Example: NORVI X-Q16 Transistor Output Test

## What this does
Sequentially toggles all 16 transistor outputs (TR1–TR16) ON then OFF via a PCA9555 I2C IO expander, displaying each output's live ON/OFF state on the TFT screen while logging every state change to Serial, then repeats continuously.

## Hardware required
- CPU board: NORVI X-CPU-ESPS3-X1 OR X2, X3
- Expansion module(s): NORVI X-Q16 (16-channel transistor output module, PCA9555 I2C IO expander)
  - PCA9555 I2C address is DIP-switch selectable, default `0x27`, range `0x20`–`0x27` depending on DIP switch setting
- Wiring / pin map (from firmware `#define`s and the `LGFX` class):

| Signal | GPIO |
|---|---|
| I2C SDA | 8 |
| I2C SCL | 9 |
| SPI MISO | 13 |
| SPI MOSI | 11 |
| SPI SCLK | 12 |
| TFT Display CS | 45 |
| TFT Display DC | 46 |
| TFT Display RST | 47 |
| TR1–TR8 (via PCA9555) | PCA9555 pins 0, 1, 2, 3, 4, 5, 6, 7 |
| TR9–TR16 (via PCA9555) | PCA9555 pins 15, 14, 13, 12, 11, 10, 9, 8 |

## Libraries required
| Library | Version |
|---|---|
| Wire (ESP32 core) | 3.x (bundled with core) |
| WiFi (ESP32 core) | 3.x (bundled with core) |
| SPI (ESP32 core) | 3.x (bundled with core) |
| clsPCA9555 |
| LovyanGFX | 1.2.25 |

**ESP32 Board Package version:** any 3.x version (ESP32 Arduino core 3.x, IDF 5.x). The core-bundled libraries (Wire, WiFi, SPI) use the version of the installed 3.x board package.

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

// ===================== LovyanGFX display setup =====================
#define LGFX_USE_V1
#include <LovyanGFX.hpp>

class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ST7789 _panel;
  lgfx::Bus_SPI      _bus;
public:
  LGFX(void) {
    { // ---- Display shares the main SPI bus with SD + Ethernet ----
      auto c = _bus.config();
      c.spi_host    = SPI2_HOST;   // FSPI - same host the Arduino SPI object uses on the S3
      c.spi_mode    = 0;
      c.freq_write  = 27000000;    // keep modest on a shared bus (SD/W5500 present)
      c.freq_read   = 16000000;
      c.spi_3wire   = false;
      c.use_lock    = true;
      c.dma_channel = SPI_DMA_CH_AUTO;
      c.pin_sclk    = 12;          // shared SCLK
      c.pin_mosi    = 11;          // shared MOSI
      c.pin_miso    = 13;          // shared MISO
      c.pin_dc      = 46;          // TFT_DC (display-only)
      _bus.config(c);
      _panel.setBus(&_bus);
    }
    { auto c = _panel.config();
      c.pin_cs   = 45;             // TFT_CS
      c.pin_rst  = 47;             // TFT_RST (shared with touch RST)
      c.pin_busy = -1;
      c.panel_width  = 240;
      c.panel_height = 320;        // set 240 if your panel is 240x240
      c.offset_x = 0;
      c.offset_y = 0;
      c.offset_rotation = 0;
      c.readable   = false;
      c.invert     = true;         // panel inversion ON
      c.rgb_order  = false;        // RGB order (red/blue were swapped on the panel with true)
      c.dlen_16bit = false;
      c.bus_shared = true;         // IMPORTANT: SD/Ethernet share this bus
      _panel.config(c);
    }
    setPanel(&_panel);
  }
};
LGFX tft;

// LovyanGFX already defines these, but guard just in case
#ifndef TFT_BLACK
#define TFT_BLACK   0x0000
#define TFT_WHITE   0xFFFF
#define TFT_RED     0xF800
#define TFT_GREEN   0x07E0
#define TFT_BLUE    0x001F
#define TFT_YELLOW  0xFFE0
#define TFT_CYAN    0x07FF
#define TFT_MAGENTA 0xF81F
#endif
// ==================================================================

#define PCA_ADDR 0x27 // I2C address 0x27 (A0 = A1 = A2 = HIGH); range 0x20 - 0x27

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

// Two columns of 8 rows: TR1-TR8 on the left, TR9-TR16 on the right
const int yPos[8] =
{
   80,110,140,170,
  200,230,260,290
};

const int labelX[2] = { 10, 125 };   // label column x
const int valueX[2] = { 60, 185 };   // ON / OFF column x

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("NPD-NORVI X-Q16 TEST");
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
  tft.setRotation(0);
  tft.setTextDatum(lgfx::textdatum_t::baseline_left);   // cursor Y = text baseline, same layout as before

  tft.fillScreen(TFT_BLACK);            // Clear screen
  tft.setTextColor(TFT_YELLOW);
  tft.setFont(&fonts::FreeSerifBold12pt7b);
  
  tft.setCursor(60, 20);      //xpos, ypos
  tft.print("NORVI");

  tft.setCursor(10, 40);      //xpos, ypos
  tft.print("EXPE-Q16 TEST");
}

void loop() {

  tft.fillScreen(TFT_BLACK);            // Clear screen
  tft.setTextColor(TFT_GREEN);
  tft.setFont(&fonts::FreeSerifBold12pt7b);
  
  tft.setCursor(60, 20);      //xpos, ypos
  tft.print("NORVI");

  tft.setCursor(10, 40);      //xpos, ypos
  tft.print("EXPE-Q16 TEST");

  tft.setTextColor(TFT_YELLOW);
tft.setFont(&fonts::FreeSansBold9pt7b);

for (int i = 0; i < 16; i++)
{
    tft.setCursor(labelX[i / 8], yPos[i % 8]);
    tft.print("TR");
    tft.print(i + 1);
    tft.print(" :");
}
tft.setTextColor(TFT_WHITE);

for (int i = 0; i < 16; i++)
{
    tft.fillRect(valueX[i / 8], yPos[i % 8] - 14, 52, 17, TFT_BLACK);

    tft.setCursor(valueX[i / 8], yPos[i % 8]);
    tft.print("ON");

    Serial.printf("TR %d : ON\n", i + 1);

    ioport.digitalWrite(trPin[i], HIGH);
    delay(200);

    tft.fillRect(valueX[i / 8], yPos[i % 8] - 14, 52, 17, TFT_BLACK);

    tft.setCursor(valueX[i / 8], yPos[i % 8]);
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
- `NPD-NORVI X-Q16 TEST`
- I2C scan results, e.g. `I2C device found at address 0x27  !`
- Per-channel state log each cycle: `TR 1 : ON`, `TR 1 : OFF`, `TR 2 : ON`, `TR 2 : OFF` ... through `TR 16 : ON`, `TR 16 : OFF`
- Separator line `----------------------------------------` printed once per full loop cycle

On the TFT display: "NORVI" / "EXPE-Q16 TEST" header, followed by "TR1 :" through "TR16 :" labels in two columns of 8 rows (TR1–TR8 on the left, TR9–TR16 on the right), each showing "ON" then "OFF" as the outputs are sequentially toggled, refreshing continuously.

## Common Issues
| Symptom | Likely Cause | Fix |
|---|---|---|
| No I2C device found at 0x27 | PCA9555 not detected — wiring fault, or DIP switch address doesn't match `PCA_ADDR` hardcoded in the sketch | Verify SDA/SCL on GPIO 8/9, and confirm the module's DIP switch setting matches `0x27` (or update `PCA_ADDR` to match a different setting in the `0x20`–`0x27` range) |
| Outputs don't switch connected load even though TFT/Serial shows ON | PCA9555 output not physically connected to the transistor driver stage, or load requires separate power supply | Check wiring from PCA9555 output pins to the X-Q16 transistor driver circuit and confirm board power |
| TFT shows nothing / blank screen | SPI pins (MISO 13 / MOSI 11 / SCLK 12) or Display CS (GPIO 45) miswired, or `tft.init()` failing silently | Verify SPI wiring and that the pin numbers in the `LGFX` class match the board pinout |
| Display colors wrong or inverted | Panel invert / RGB order setting differs from the panel in use | Toggle `c.invert` or `c.rgb_order` in the `LGFX` class |
| Upload fails / board not detected | Wrong Board/Tools settings | Match settings to: ESP32S3 Dev Module, UART0/Hardware CDC, QSPI PSRAM, 16MB flash, Huge APP partition |

## Related Files
- Module reference: `expansion_module_Q16.md`
- CPU board: `cpu_board_X1.md`