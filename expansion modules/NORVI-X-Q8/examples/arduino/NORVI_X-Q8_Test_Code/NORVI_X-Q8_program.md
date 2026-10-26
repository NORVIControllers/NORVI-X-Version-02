---
type: Test_program
function: transistor_output_test (8-channel transistor output test via PCA9538 with TFT display)
compatible_cpu_boards: [NORVI X-CPU-ESPS3-X1],[NORVI X-CPU-ESPS3-X2],[NORVI X-CPU-ESPS3-X3]
compatible_modules: [NORVI X-Q8 (8-channel transistor output expansion, PCA9538 I2C IO expander)]
status: completed
last_updated: 2026-10-22
---

# Example: NORVI X-Q8 Transistor Output Test

## What this does
Sequentially toggles all 8 transistor outputs (TR1–TR8) ON then OFF via a PCA9538 I2C IO expander, displaying each output's live ON/OFF state on the TFT screen while logging every state change to Serial, then repeats continuously.

## Hardware required
- CPU board: NORVI X-CPU-ESPS3-X1 OR X2, X3
- Expansion module(s): NORVI X-Q8 (8-channel transistor output module, PCA9538 I2C IO expander)
  - PCA9538 I2C address is DIP-switch selectable; this sketch uses `0x73` by default and supported the address range of 0x70-0x73
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
| PCA9538 Reset (PCA_RESET) | 21 |
| TR1–TR8 (via PCA9538) | PCA9538 pins 1, 2, 3, 4, 5, 6, 7, 8 |

## Libraries required
| Library | Version |
|---|---|
| Wire (ESP32 core) | 3.x (bundled with core) |
| WiFi (ESP32 core) | 3.x (bundled with core) |
| SPI (ESP32 core) | 3.x (bundled with core) |
| PCA9538 |
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
 * NORVI-X-Q8 N16R2
 * 2026.08.12
 */

#include <Wire.h>
#include <WiFi.h>
#include <SPI.h>
#include <PCA9538.h>

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

// I2C address of PCA9538
#define PCA9538_ADDR 0x73

#define SDA   8     
#define SCL   9     

#define MISO 13
#define MOSI 11
#define SCLK 12

#define DSP_CS 45

#define PCA_RESET 21

#define TR1   1 
#define TR2   2
#define TR3   3
#define TR4   4
#define TR5   5
#define TR6   6
#define TR7   7
#define TR8   8

PCA9538 io(PCA9538_ADDR);

void setup() {
  Serial.begin(115200);

  Serial.println("NPD-NORVI X-Q8 TEST");
  delay(100);

  pinMode(PCA_RESET, OUTPUT);
  delay(100);
  digitalWrite(PCA_RESET, HIGH);

  Wire.begin(SDA, SCL);
  delay(100);

  I2C_SCAN();
  delay(100);

  io.pinMode(TR1, OUTPUT);
  io.pinMode(TR2, OUTPUT);
  io.pinMode(TR3, OUTPUT);
  io.pinMode(TR4, OUTPUT);
  io.pinMode(TR5, OUTPUT);
  io.pinMode(TR6, OUTPUT);
  io.pinMode(TR7, OUTPUT);
  io.pinMode(TR8, OUTPUT);

  SPI.begin(SCLK, MISO, MOSI); // Ensure these pin numbers are correct 
  delay(100);

  tft.init();
  tft.setRotation(0);
  tft.setTextDatum(lgfx::textdatum_t::baseline_left);   // cursor Y = text baseline, same layout as before

  tft.fillScreen(TFT_BLACK);            // Clear screen
  tft.setTextColor(TFT_YELLOW);
  tft.setFont(&fonts::FreeSerifBold12pt7b);
  
  tft.setCursor(60, 20);      //xpos, ypos
  tft.print("NORVI");

  tft.setCursor(20, 40);      //xpos, ypos
  tft.print("EXPE-Q8 TEST");
}

void loop() {

  tft.fillScreen(TFT_BLACK);            // Clear screen
  tft.setTextColor(TFT_GREEN);
  tft.setFont(&fonts::FreeSerifBold12pt7b);
  
  tft.setCursor(60, 20);      //xpos, ypos
  tft.print("NORVI");

  tft.setCursor(20, 40);      //xpos, ypos
  tft.print("EXPE-Q8 TEST");

  tft.setTextColor(TFT_YELLOW);
  tft.setCursor(10, 70);      //xpos, ypos
  tft.setFont(&fonts::FreeSansBold9pt7b);
  tft.print("TR 1 :");

  tft.setCursor(10, 100);      //xpos, ypos
  tft.print("TR 2 :");
  
  tft.setCursor(10, 130);      //xpos, ypos
  tft.print("TR 3 :");
  
  tft.setCursor(10, 160);      //xpos, ypos
  tft.print("TR 4 :");
  
  tft.setCursor(10, 190);      //xpos, ypos
  tft.print("TR 5 :");
  
  tft.setCursor(10, 220);      //xpos, ypos
  tft.print("TR 6 :");
  
  tft.setCursor(10, 250);      //xpos, ypos
  tft.print("TR 7 :");
  
  tft.setCursor(10, 280);      //xpos, ypos
  tft.print("TR 8 :");
 
  
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(120, 70);      //xpos, ypos
  tft.print("ON");
  Serial.println("TR 1 : ON");
  io.digitalWrite(TR1,HIGH);
  delay(300);
  tft.setCursor(170, 70);      //xpos, ypos
  tft.print("OFF");
  Serial.println("TR 1 : OFF");
  io.digitalWrite(TR1,LOW);
  delay(300);

  tft.setCursor(120, 100);      //xpos, ypos
  tft.print("ON");
  Serial.println("TR 2 : ON");
  io.digitalWrite(TR2,HIGH);
  delay(300);
  tft.setCursor(170, 100);      //xpos, ypos
  tft.print("OFF");
  Serial.println("TR 2 : OFF");
  io.digitalWrite(TR2,LOW);
  delay(300);
  

  tft.setCursor(120, 130);      //xpos, ypos
  tft.print("ON");
  Serial.println("TR 3 : ON");
  io.digitalWrite(TR3,HIGH);
  delay(300);
  tft.setCursor(170, 130);      //xpos, ypos
  tft.print("OFF");
  Serial.println("TR 3 : OFF");
  io.digitalWrite(TR3,LOW);
  delay(300);
  

  tft.setCursor(120, 160);      //xpos, ypos
  tft.print("ON");
  Serial.println("TR 4 : ON");
  io.digitalWrite(TR4,HIGH);
  delay(300);
  tft.setCursor(170, 160);      //xpos, ypos
  tft.print("OFF");
  Serial.println("TR 4 : OFF");
  io.digitalWrite(TR4,LOW);
  delay(300);

  tft.setCursor(120, 190);      //xpos, ypos
  tft.print("ON");
  Serial.println("TR 5 : ON");
  io.digitalWrite(TR5,HIGH);
  delay(300);
  tft.setCursor(170, 190);      //xpos, ypos
  tft.print("OFF");
  Serial.println("TR 5 : OFF");
  io.digitalWrite(TR5,LOW);
  delay(300);

  tft.setCursor(120, 220);      //xpos, ypos
  tft.print("ON");
  Serial.println("TR 6 : ON");
  io.digitalWrite(TR6,HIGH);
  delay(300);
  tft.setCursor(170, 220);      //xpos, ypos
  tft.print("OFF");
  Serial.println("TR 6 : OFF");
  io.digitalWrite(TR6,LOW);
  delay(300);

  tft.setCursor(120, 250);      //xpos, ypos
  tft.print("ON");
  Serial.println("TR 7 : ON");
  io.digitalWrite(TR7,HIGH);
  delay(300);
  tft.setCursor(170, 250);      //xpos, ypos
  tft.print("OFF");
  Serial.println("TR 7 : OFF");
  io.digitalWrite(TR7,LOW);
  delay(300);

  tft.setCursor(120, 280);      //xpos, ypos
  tft.print("ON");
  Serial.println("TR 8 : ON");
  io.digitalWrite(TR8,HIGH);
  delay(300);
  tft.setCursor(170, 280);      //xpos, ypos
  tft.print("OFF");
  Serial.println("TR 8 : OFF");
  io.digitalWrite(TR8,LOW);
  delay(300);
  
  
  Serial.println("........................................................................");

  delay(100);
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
- `NPD-NORVI X-Q8 TEST`
- I2C scan results, e.g. `I2C device found at address 0x73  !`
- Per-channel state log each cycle: `TR 1 : ON`, `TR 1 : OFF`, `TR 2 : ON`, `TR 2 : OFF` ... through `TR 8 : ON`, `TR 8 : OFF`
- Separator line `........................................................................` printed once per full loop cycle

On the TFT display: "NORVI" / "EXPE-Q8 TEST" header, followed by "TR 1 :" through "TR 8 :" labels, each showing "ON" then "OFF" as the outputs are sequentially toggled, refreshing continuously.

## Common Issues
| Symptom | Likely Cause | Fix |
|---|---|---|
| No I2C device found at 0x73 | PCA9538 not detected —  DIP switch address doesn't match `PCA9538_ADDR`, or reset pin not held HIGH | Verify SDA/SCL on GPIO 8/9, confirm PCA_RESET (GPIO 21) is driven HIGH before Wire init, and check the module's DIP switch setting matches `0x73` (or update `PCA9538_ADDR` to match) |
| Outputs don't switch connected load even though TFT/Serial shows ON | PCA9538 output not physically connected to the transistor driver stage, or load requires separate power supply | Check wiring from PCA9538 output pins to the X-Q8 transistor driver circuit and confirm board power |
| TFT shows nothing / blank screen | SPI pins (MISO 13 / MOSI 11 / SCLK 12) or Display CS (GPIO 45) miswired, or `tft.init()` failing silently | Verify SPI wiring and that the pin numbers in the `LGFX` class match the board pinout |
| Display colors wrong or inverted | Panel invert / RGB order setting differs from the panel in use | Toggle `c.invert` or `c.rgb_order` in the `LGFX` class |
| Upload fails / board not detected | Wrong Board/Tools settings | Match settings to: ESP32S3 Dev Module, UART0/Hardware CDC, QSPI PSRAM, 16MB flash, Huge APP partition |

## Related Files
- Module reference: `expansion_module_Q8.md`
- CPU board: `cpu_board_X1.md`