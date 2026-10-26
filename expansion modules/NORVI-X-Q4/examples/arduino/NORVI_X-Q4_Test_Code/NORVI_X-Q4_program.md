---
type: Test_program
function: transistor_output_test (4-channel direct-GPIO transistor output test with TFT display)
compatible_cpu_boards:
  - "NORVI X-CPU-ESPS3-X1 (N16R2, PSRAM-QSPI)"
  - "NORVI X-CPU-ESPS3-X2 (N16R2, PSRAM-QSPI)"
  - "NORVI X-CPU-ESPS3-X3 (N16R2, PSRAM-QSPI)"
compatible_modules: [NORVI X-Q4 (4-channel transistor output expansion, direct GPIO)]
status: completed
last_updated: 2026-10-22
---

# Example: NORVI X-Q4 Transistor Output Test

## What this does
Cycles 4 transistor outputs (TR1–TR4) ON then OFF sequentially via direct GPIO, displaying each output's ON/OFF state on the TFT screen while logging the same state changes to Serial.

## Hardware required
- CPU board: NORVI X-CPU-ESPS3-X1 OR X2, X3 (ESP32-S3, N16R2, PSRAM-QSPI)
- Expansion module(s): NORVI X-Q4 (4-channel transistor output module, direct GPIO — no I2C IO expander)
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
| TR1 | 14 |
| TR2 | 35 |
| TR3 | 41 |
| TR4 | 37 |

## Libraries required
| Library | Version |
|---|---|
| Wire (ESP32 core) | 3.x (bundled with core) |
| WiFi (ESP32 core) | 3.x (bundled with core) |
| SPI (ESP32 core) | 3.x (bundled with core) |
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
//"NPD-NORVI X-Q4"

#include <Wire.h>
#include <WiFi.h>
#include <SPI.h>

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

#define SDA   8     
#define SCL   9     

#define MISO 13
#define MOSI 11
#define SCLK 12

#define DSP_CS 45

#define TR1 14
#define TR2 35
#define TR3 41
#define TR4 37

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("NPD-NORVI EXPE-Q4 TEST");
  delay(1000);

  Wire.begin(SDA, SCL);
  delay(100);

  //I2C_SCAN();
  //delay(1000);

  pinMode(TR1, OUTPUT);
  pinMode(TR2, OUTPUT);
  pinMode(TR3, OUTPUT);
  pinMode(TR4, OUTPUT);

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

  tft.setCursor(20, 40);      //xpos, ypos
  tft.print("EXPE-Q4 TEST");
}

void loop() {

  tft.fillScreen(TFT_BLACK);            // Clear screen
  tft.setTextColor(TFT_GREEN);
  tft.setFont(&fonts::FreeSerifBold12pt7b);
  
  tft.setCursor(60, 20);      //xpos, ypos
  tft.print("NORVI");

  tft.setCursor(20, 40);      //xpos, ypos
  tft.print("EXPE-Q4 TEST");

  tft.setTextColor(TFT_YELLOW);
  tft.setCursor(10, 100);      //xpos, ypos
  tft.setFont(&fonts::FreeSansBold9pt7b);
  tft.print("OUTPUT 1 :");

  tft.setCursor(10, 130);      //xpos, ypos
  tft.print("OUTPUT 2 :");
  
  tft.setCursor(10, 160);      //xpos, ypos
  tft.print("OUTPUT 3 :");
  
  tft.setCursor(10, 190);      //xpos, ypos
  tft.print("OUTPUT 4 :");
  

  tft.setTextColor(TFT_WHITE);
  tft.setCursor(120, 100);      //xpos, ypos
  tft.print("ON");
  Serial.println("OUTPUT 1 : ON");
  digitalWrite(TR1, HIGH);
  delay(500);
  tft.setCursor(170, 100);      //xpos, ypos
  tft.print("OFF");
  Serial.println("OUTPUT 1 : OFF");
  digitalWrite(TR1, LOW);
  delay(500);

  tft.setCursor(120, 130);      //xpos, ypos
  tft.print("ON");
  Serial.println("OUTPUT 2 : ON");
  digitalWrite(TR2, HIGH);
  delay(500);
  tft.setCursor(170, 130);      //xpos, ypos
  tft.print("OFF");
  Serial.println("OUTPUT 2 : OFF");
  digitalWrite(TR2, LOW);
  delay(500);
  
  tft.setCursor(120, 160);      //xpos, ypos
  tft.print("ON");
  Serial.println("OUTPUT 3 : ON");
  digitalWrite(TR3, HIGH);
  delay(500);
  tft.setCursor(170, 160);      //xpos, ypos
  tft.print("OFF");
  Serial.println("OUTPUT 3 : OFF");
  digitalWrite(TR3, LOW);
  delay(500);
  

  tft.setCursor(120, 190);      //xpos, ypos
  tft.print("ON");
  Serial.println("OUTPUT 4 : ON");
  digitalWrite(TR4, HIGH);
  delay(500);
  tft.setCursor(170, 190);      //xpos, ypos
  tft.print("OFF");
  Serial.println("OUTPUT 4 : OFF");
  digitalWrite(TR4, LOW);
  delay(500);


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
- `NPD-NORVI EXPE-Q4 TEST`
- Repeating per-cycle state log: `OUTPUT 1 : ON`, `OUTPUT 1 : OFF`, `OUTPUT 2 : ON`, `OUTPUT 2 : OFF`, `OUTPUT 3 : ON`, `OUTPUT 3 : OFF`, `OUTPUT 4 : ON`, `OUTPUT 4 : OFF`

On the TFT display: "NORVI" / "EXPE-Q4 TEST" header, followed by "OUTPUT 1 :" through "OUTPUT 4 :" labels, each showing "ON" then "OFF" as the outputs are sequentially toggled, refreshing continuously.

## Common Issues
| Symptom | Likely Cause | Fix |
|---|---|---|
| No I2C scan output appears | `I2C_SCAN()` call is commented out in `setup()` in this sketch version | Uncomment the `I2C_SCAN();` call in `setup()` if I2C device confirmation is needed |
| Output pin does not switch connected load even though TFT/Serial shows ON | TR1/TR2/TR3/TR4 (GPIO 14/35/41/37) wiring fault, or load requires external driver/relay stage not toggled by GPIO alone | Verify wiring to the X-Q4 module's terminal map and confirm the transistor driver stage is powered |
| TFT shows nothing / blank screen | SPI pins (MISO 13 / MOSI 11 / SCLK 12) or Display CS (GPIO 45) misconfigured, or `tft.init()` failing silently | Verify SPI wiring and that the pin numbers in the `LGFX` class match the board pinout |
| Display colors wrong or inverted | Panel invert / RGB order setting differs from the panel in use | Toggle `c.invert` or `c.rgb_order` in the `LGFX` class |
| Upload fails / board not detected | Wrong Board/Tools settings | Match settings to: ESP32S3 Dev Module, UART0/Hardware CDC, QSPI PSRAM, 16MB flash, Huge APP partition |

## Related Files
- Module reference: `expansion_module_Q4.md`
- CPU board: `cpu_board_X1.md`
