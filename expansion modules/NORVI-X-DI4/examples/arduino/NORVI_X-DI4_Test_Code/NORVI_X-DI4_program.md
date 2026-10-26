---
type: Test_program
function: digital_input_read (4-channel digital input test with TFT display)
compatible_cpu_boards:
  - "NORVI X-CPU-ESPS3-X1 (N16R2, PSRAM-QSPI)"
  - "NORVI X-CPU-ESPS3-X2 (N16R2, PSRAM-QSPI)"
  - "NORVI X-CPU-ESPS3-X3 (N16R2, PSRAM-QSPI)"
compatible_modules: [NORVI X-DI4 (4-channel digital input expansion)]
status: completed
last_updated: 2026-10-21
---

# Example: NORVI X-DI4 Digital Input Test

## What this does
Reads 4 digital input channels (INPUT1–INPUT4) in a loop and displays their live HIGH/LOW state on the TFT screen, alongside a splash/status header, while also printing a separator line to Serial each cycle.

## Hardware required
- CPU board: NORVI X-CPU-ESPS3-X1 OR X2,X3 (ESP32-S3, N16R2, PSRAM-QSPI)
- Expansion module(s): NORVI X-DI4 (4-channel digital input module)
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
| TFT Display RST (shared with touch RST) | 47 |
| Touch SDA / SCL / RST / IRQ | 8, 9, 47, 48 |
| INPUT1 | 5 |
| INPUT2 | 6 |
| INPUT3 | 7 |
| INPUT4 | 10 |

## Libraries required
| Library | Version |
|---|---|
| Wire (ESP32 core) | 3.x (bundled with core) |
| WiFi (ESP32 core) | 3.x (bundled with core) |
| SPI (ESP32 core) | 3.x (bundled with core) |
| LovyanGFX | 1.2.25 |
| CST816S | 1.3.0 |

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
//"NPD-NORVI EXPE DI4"

#include <Wire.h>
#include <WiFi.h>
#include <SPI.h>
#include <CST816S.h>

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

#define INPUT1  5
#define INPUT2  6
#define INPUT3  7
#define INPUT4  10

CST816S touch(8, 9, 47, 48);  // sda, scl, rst, irq

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("NPD-NORVI EXPE-DI4 TEST");
  delay(1000);
 
  Wire.begin(SDA, SCL);
  delay(100);

  //I2C_SCAN();
  //delay(1000);

  pinMode(INPUT1, INPUT);
  pinMode(INPUT2, INPUT);
  pinMode(INPUT3, INPUT);
  pinMode(INPUT4, INPUT);

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
  tft.print("EXPE-DI4 TEST");
}

void loop() {

  tft.fillScreen(TFT_BLACK);            // Clear screen
  tft.setTextColor(TFT_GREEN);
  tft.setFont(&fonts::FreeSerifBold12pt7b);
  
  tft.setCursor(60, 20);      //xpos, ypos
  tft.print("NORVI");
  

  tft.setCursor(20, 40);      //xpos, ypos
  tft.print("EXPE-DI4 TEST");
 

  tft.setTextColor(TFT_YELLOW);
  tft.setCursor(10, 100);      //xpos, ypos
  tft.setFont(&fonts::FreeSansBold9pt7b);
  tft.print("INPUT 1  :");


  tft.setCursor(10, 130);      //xpos, ypos
  tft.print("INPUT 2  :");
 

  tft.setCursor(10, 160);      //xpos, ypos
  tft.print("INPUT 3  :");
  

  tft.setCursor(10, 190);      //xpos, ypos
  tft.print("INPUT 4  :");
 

  tft.setTextColor(TFT_WHITE);
  tft.setCursor(170, 100);      //xpos, ypos
  tft.print(digitalRead(INPUT1));
  delay(200);
   
  tft.setCursor(170, 130);      //xpos, ypos
  tft.print(digitalRead(INPUT2));
  delay(200);
  
  tft.setCursor(170, 160);      //xpos, ypos
  tft.print(digitalRead(INPUT3));
  delay(200);
  
  tft.setCursor(170, 190);      //xpos, ypos
  tft.print(digitalRead(INPUT4));
  delay(500);

  Serial.println("----------------------------");
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
- `NPD-NORVI EXPE-DI4 TEST`
- I2C scan results, e.g. `I2C device found at address 0x75  !`, `I2C device found at address 0x68  !`, `I2C device found at address 0x41  !`, `I2C device found at address 0x15  !`
- Repeating separator line `----------------------------` printed once per loop cycle

On the TFT display: "NORVI" / "EXPE-DI4 TEST" header, followed by "INPUT 1 :" through "INPUT 4 :" labels each with a live `0` (LOW) or `1` (HIGH) value shown beside them, refreshing continuously.

## Common Issues
| Symptom | Likely Cause | Fix |
|---|---|---|
| Input reads 0 when signal is applied (HIGH) and 1 when no signal is applied | Inputs are internally pulled up in hardware, so they read logic HIGH (1) when idle/open and are pulled LOW (0) when the input signal is active | This is expected behavior — confirm signal wiring matches the DI4 module's terminal map if readings don't match expected state |
| TFT shows nothing / blank screen | SPI pins (MISO 13 / MOSI 11 / SCLK 12) or Display CS (GPIO 45) miswired, or `tft.init()` failing silently | Verify SPI wiring and that the pin numbers in the `LGFX` class match the board pinout |
| Display colors wrong or inverted | Panel invert / RGB order setting differs from the panel in use | Toggle `c.invert` or `c.rgb_order` in the `LGFX` class |
| Upload fails / board not detected | Wrong Board/Tools settings | Match settings to: ESP32S3 Dev Module, UART0/Hardware CDC, QSPI PSRAM, 16MB flash, Huge APP partition |

## Related Files
- Module reference: `expansion_module_DI4.md`
- CPU board: `cpu_board_X1.md`
