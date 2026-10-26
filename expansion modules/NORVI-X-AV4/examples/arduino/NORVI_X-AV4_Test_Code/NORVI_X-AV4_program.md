---
type: Test_program
function: analog_voltage_read_0-10v (4-channel ADS1115 analog voltage input test with TFT display)
compatible_cpu_boards: [NORVI X-CPU-ESPS3-X1],[NORVI X-CPU-ESPS3-X2],[NORVI X-CPU-ESPS3-X3]
compatible_modules: [NORVI X-AV4 (4-channel analog voltage input expansion, ADS1115 I2C 16-bit ADC)]
status: completed
last_updated: 2026-10-21
---

# Example: NORVI X-AV4 Analog Voltage Input Test

## What this does
Reads 4 single-ended analog voltage channels via an ADS1115 ADC, converts the raw ADC counts to volts using a voltage-divider ratio, and continuously displays the live voltage readings on the TFT screen while printing each channel value to Serial as it's read.

## Hardware required
- CPU board: NORVI X-CPU-ESPS3-X1 OR X2, X3
- Expansion module(s): NORVI X-AV4 (4-channel analog voltage input module, ADS1115 I2C ADC)
  - ADS1115 I2C address is `0x49` in this sketch; it can be `0x48`, `0x49`, `0x4A` or `0x4B` depending on the DIP switch setting (edit the address in the code to match).
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

## Libraries required
| Library | Version |
|---|---|
| Wire (ESP32 core) | 3.x (bundled with core) |
| WiFi (ESP32 core) | 3.x (bundled with core) |
| SPI (ESP32 core) | 3.x (bundled with core) |
| Adafruit_ADS1X15 | 
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
/*
 * 2026/05/13
 * NPD-NORVI EXPE AV4 
 * N16R2 
 * 
*/
// Display driver: LovyanGFX

#include <Wire.h>
#include <WiFi.h>
#include <SPI.h>
#include <Adafruit_ADS1X15.h>
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
      c.rgb_order  = true;         // BGR order
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

Adafruit_ADS1115 ads1;
#define VOLTAGE_DIVIDER_RATIO 0.2065

CST816S touch(8, 9, 47, 48);  // sda, scl, rst, irq

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("NPD-NORVI EXPE-AIX TEST");
  delay(1000);


  Wire.begin(SDA, SCL);
  delay(100);


if (!ads1.begin(0x49)) {
    Serial.println("Failed to initialize ADS 1 .");
    // while (1);
  }
 

  SPI.begin(SCLK, MISO, MOSI); // Ensure these pin numbers are correct 
  delay(1000);

  tft.init();
  tft.setRotation(0);

  tft.fillScreen(TFT_BLACK);            // Clear screen
  tft.setTextColor(TFT_YELLOW);
  tft.setFont(&fonts::FreeSerifBold12pt7b);
  
  tft.setCursor(60, 20);      //xpos, ypos
  tft.print("NORVI");

  tft.setCursor(20, 40);      //xpos, ypos
  tft.print("EXPE-AV4 TEST");
}

void loop() {
  
  int16_t adc0, adc1, adc2, adc3;

  adc0 = ads1.readADC_SingleEnded(0);
  adc1 = ads1.readADC_SingleEnded(1);
  adc2 = ads1.readADC_SingleEnded(2);
  adc3 = ads1.readADC_SingleEnded(3);

  float Voltage0 = adc0 * 0.125 / 1000.0 / VOLTAGE_DIVIDER_RATIO;
  float Voltage1 = adc1 * 0.125 / 1000.0 / VOLTAGE_DIVIDER_RATIO;
  float Voltage2 = adc2 * 0.125 / 1000.0 / VOLTAGE_DIVIDER_RATIO;
  float Voltage3 = adc3 * 0.125 / 1000.0 / VOLTAGE_DIVIDER_RATIO;

  
  tft.fillScreen(TFT_BLACK);            // Clear screen
  tft.setTextColor(TFT_GREEN);
  tft.setFont(&fonts::FreeSerifBold12pt7b);
  
  tft.setCursor(60, 20);      //xpos, ypos
  tft.print("NORVI");

  tft.setCursor(20, 40);      //xpos, ypos
  tft.print("EXPE-AV4 TEST");

  tft.setTextColor(TFT_YELLOW);
  tft.setCursor(10, 100);      //xpos, ypos
  tft.setFont(&fonts::FreeSerifBold9pt7b);
  tft.print("VOLTAGE 1 :");

  tft.setCursor(10, 130);      //xpos, ypos
  tft.print("VOLTAGE 2 :");
  
  tft.setCursor(10, 160);      //xpos, ypos
  tft.print("VOLTAGE 3 :");
  
  tft.setCursor(10, 190);      //xpos, ypos
  tft.print("VOLTAGE 4 :");
  
//----------------------------------
  tft.setTextColor(TFT_WHITE);
  
  tft.setCursor(170, 100);      //xpos, ypos
  tft.print(String(Voltage0, 2) + " V");
  Serial.println(String("VOLTAGE 1 : ") + String(Voltage0, 2) + " V");
  delay(500);
  
  tft.setCursor(170, 130);      //xpos, ypos
  tft.print(String(Voltage1, 2) + " V");
  Serial.println(String("VOLTAGE 2 : ") + String(Voltage1, 2) + " V");
  delay(500);
  

  tft.setCursor(170, 160);      //xpos, ypos
  tft.print(String(Voltage2, 2) + " V");
  Serial.println(String("VOLTAGE 3 : ") + String(Voltage2, 2) + " V");
  delay(500);
  

  tft.setCursor(170, 190);
  tft.print(String(Voltage3, 2) + " V");
  Serial.println(String("VOLTAGE 4 : ") + String(Voltage3, 2) + " V");
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
- `NPD-NORVI EXPE-AIX TEST`
- `Failed to initialize ADS 1 .` (only if ADS1115 not detected at address `0x49`)
- Per-channel line printed as each reading completes: `VOLTAGE 1 : 5.02 V`, `VOLTAGE 2 : 7.48 V`, `VOLTAGE 3 : 0.00 V`, `VOLTAGE 4 : 9.87 V`

On the TFT display: "NORVI" / "EXPE-AV4 TEST" header, followed by "VOLTAGE 1 :" through "VOLTAGE 4 :" labels, each showing a live voltage value (e.g. `5.02 V`), refreshing continuously.

## Common Issues
| Symptom | Likely Cause | Fix |
|---|---|---|
| `Failed to initialize ADS 1 .` | ADS1115 not detected — wiring fault, or module's I2C address doesn't match the `0x49` used in the sketch | Verify SDA/SCL on GPIO 8/9, and confirm the module's DIP-switch address matches `0x49` (this sketch does not support changing the address without editing the code) |
| Voltage readings show ~0V on all channels regardless of input | No voltage signal connected, or `ads1.begin()` failed silently | Confirm a live voltage signal is wired to the corresponding channel and check ADS1115 initialization status in Serial |
| Voltage readings are inaccurate / scaled incorrectly | `VOLTAGE_DIVIDER_RATIO` (0.2065) not matched to the module's actual resistor-divider network | Verify the divider ratio against the X-AV4 module's voltage-divider spec and recalibrate if needed |
| TFT shows nothing / blank screen | SPI pins (MISO 13 / MOSI 11 / SCLK 12) or Display CS (GPIO 45) miswired, or `tft.init()` failing silently | Verify SPI wiring and that the pin numbers in the `LGFX` class match the board pinout |
| Display colors wrong or inverted | Panel invert / RGB order setting differs from the panel in use | Toggle `c.invert` or `c.rgb_order` in the `LGFX` class |
| Upload fails / board not detected | Wrong Board/Tools settings | Match settings to: ESP32S3 Dev Module, UART0/Hardware CDC, QSPI PSRAM, 16MB flash, Huge APP partition |

## Related Files
- Module reference: `expansion_module_AV4.md`
- CPU board: `cpu_board_X1.md`