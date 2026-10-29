---
type: Test_program
function: analog_current_read_4-20ma (4-channel ADS1115 analog current input test with TFT display)
compatible_cpu_boards: [NORVI X-CPU-ESPS3-X1, NORVI X-CPU-ESPS3-X2, NORVI X-CPU-ESPS3-X3]
compatible_modules: [NORVI X-AI4 (4-channel analog current input expansion, ADS1115 I2C 16-bit ADC)]
status: completed
last_updated: 2026-10-21
---

# Example: NORVI X-AI4 Analog Current Input Test

## What this does
Reads 4 single-ended analog current channels via an ADS1115 ADC, converts the raw ADC counts to mA using a calibration factor, and continuously displays the live current readings on the TFT screen while printing all four channel values to Serial each cycle.

## Hardware required
- CPU board: NORVI X-CPU-ESPS3-X1 OR X2, X3
- Expansion module(s): NORVI X-AI4 (4-channel analog current input module, ADS1115 I2C ADC)
  - ADS1115 I2C address is DIP-switch selectable: `0x49` (DIP1 ON), `0x4B` (DIP2 ON), `0x4A` (DIP3 ON), `0x48` (DIP4 ON) — this sketch uses `0x49` by default
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

## Libraries required
| Library | Version |
|---|---|
| Wire (ESP32 core) | 3.x (bundled with core) |
| WiFi (ESP32 core) | 3.x (bundled with core) |
| SPI (ESP32 core) | 3.x (bundled with core) |
| Adafruit_ADS1X15 | not specified in sketch |
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
 * NPD-NORVI X-AI4 V2 (N16R2)
 * 2026.08.24
*/

#include <Wire.h>
#include <WiFi.h>
#include <SPI.h>
#include <Adafruit_ADS1X15.h>

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

#define MISO  13
#define MOSI  11
#define SCLK  12

#define DSP_CS 45

/*
 ADS1115 ADDRESS
 DIP1(ON) - 0x49
 DIP2(ON) - 0x4B
 DIP3(ON) - 0x4A
 DIP4(ON) - 0x48
*/

#define ADS_ADDR 0x49
Adafruit_ADS1115 ads1;

const float mA_Factor = 4.096 / 3269.826;


unsigned long lastDisplayUpdate = 0;

const unsigned long DISPLAY_INTERVAL = 100;


float lastDisplay0 = -999.0;
float lastDisplay1 = -999.0;
float lastDisplay2 = -999.0;
float lastDisplay3 = -999.0;


void setup() {

  Serial.begin(115200);
  delay(200);

  Serial.println();
  Serial.println("NPD-NORVI X-AI4 TEST");
  
  // i2c
  Wire.begin(SDA, SCL);

  if (!ads1.begin(ADS_ADDR)) {

    Serial.println("Failed to initialize ADS 1.");

  }
  else {

    Serial.println("ADS1115 OK");

  }

  ads1.setGain(GAIN_ONE);

  // FAST ADC
  ads1.setDataRate(RATE_ADS1115_860SPS);

  Serial.print("ADS1115 ADDRESS: 0x");

  if (ADS_ADDR < 0x10) {
    Serial.print("0");
  }

  Serial.println(ADS_ADDR, HEX);

  Serial.println("ADS1115 DATA RATE = 860 SPS");


  // SPI
  SPI.begin(SCLK, MISO, MOSI);

  delay(100);

  // TFT
  tft.init();
  tft.setRotation(0);

  tft.fillScreen(TFT_BLACK);


  // TITLE
  tft.setTextColor(TFT_GREEN);
  tft.setFont(&fonts::FreeSerifBold12pt7b);
  
  tft.setCursor(60, 20);
  tft.print("NORVI");

  tft.setCursor(40, 40);
  tft.print("X-AI4 TEST");

  // CURRENT LABELS
  tft.setFont(&fonts::FreeSerifBold9pt7b);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setCursor(10, 85);
  tft.print("CURRENT 1 :");

  tft.setCursor(10, 115);
  tft.print("CURRENT 2 :");

  tft.setCursor(10, 145);
  tft.print("CURRENT 3 :");

  tft.setCursor(10, 175);
  tft.print("CURRENT 4 :");

  // INITIAL CURRENT VALUES
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  
  tft.setCursor(145, 85);
  tft.print("0.00 mA");

  tft.setCursor(145, 115);
  tft.print("0.00 mA");

  tft.setCursor(145, 145);
  tft.print("0.00 mA");

  tft.setCursor(145, 175);
  tft.print("0.00 mA");


  // ADS1115 ADDRESS AT BOTTOM

  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setCursor(10, 215);
  tft.print("ADS ADDR : 0x");

  if (ADS_ADDR < 0x10) {
    tft.print("0");
  }

  tft.print(ADS_ADDR, HEX);

  Serial.println("TEST START");
}


void loop() {

  int16_t adc0;
  int16_t adc1;
  int16_t adc2;
  int16_t adc3;

  adc0 = ads1.readADC_SingleEnded(0);
  adc1 = ads1.readADC_SingleEnded(1);
  adc2 = ads1.readADC_SingleEnded(2);
  adc3 = ads1.readADC_SingleEnded(3);

  // ADC TO CURRENT
  float current0 = adc0 * mA_Factor;
  float current1 = adc1 * mA_Factor;
  float current2 = adc2 * mA_Factor;
  float current3 = adc3 * mA_Factor;

  // SERIAL MONITOR
  Serial.printf(
    "CH1: %.2f mA  "
    "CH2: %.2f mA  "
    "CH3: %.2f mA  "
    "CH4: %.2f mA\n",
    current0,
    current1,
    current2,
    current3
  );

  // TFT UPDATE
  if (millis() - lastDisplayUpdate >= DISPLAY_INTERVAL) {

    lastDisplayUpdate = millis();

    char buf[20];

    tft.setTextColor(TFT_WHITE, TFT_BLACK);


    // CURRENT 1
    if (current0 != lastDisplay0) {

      tft.fillRect(140, 68, 100, 25, TFT_BLACK);
      dtostrf(current0, 5, 2, buf);
      tft.setCursor(145, 85);
      tft.print(buf);
      tft.print(" mA");
      lastDisplay0 = current0;
    }
    
    // CURRENT 2
    if (current1 != lastDisplay1) {
      tft.fillRect(140, 98, 100, 25, TFT_BLACK);
      dtostrf(current1, 5, 2, buf);
      tft.setCursor(145, 115);
      tft.print(buf);
      tft.print(" mA");
      lastDisplay1 = current1;
    }

    // CURRENT 3
    if (current2 != lastDisplay2) {
      tft.fillRect(140, 128, 100, 25, TFT_BLACK);
      dtostrf(current2, 5, 2, buf);
      tft.setCursor(145, 145);
      tft.print(buf);
      tft.print(" mA");
      lastDisplay2 = current2;
    }

    // CURRENT 4
    if (current3 != lastDisplay3) {
      tft.fillRect(140, 158, 100, 25, TFT_BLACK);
      dtostrf(current3, 5, 2, buf);
      tft.setCursor(145, 175);
      tft.print(buf);
      tft.print(" mA");
      lastDisplay3 = current3;
    }
  }
}


// I2C SCANNER
void I2C_SCAN() {

  byte error;
  byte address;

  int deviceCount = 0;

  Serial.println();
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

      Serial.println(" !");

      deviceCount++;

      delay(1);
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

    Serial.println("No I2C devices found");

  }
  else {

    Serial.println("Scanning complete");

  }
}
```

## Expected Output
On the Serial Monitor (115200 baud):
- `NPD-NORVI X-AI4 TEST`
- `ADS1115 OK` (or `Failed to initialize ADS 1.` only if ADS1115 not detected at the configured address)
- Continuous per-cycle line: `CH1: %.2f mA  CH2: %.2f mA  CH3: %.2f mA  CH4: %.2f mA` (e.g. `CH1: 4.02 mA  CH2: 8.15 mA  CH3: 12.33 mA  CH4: 20.01 mA`)

On the TFT display: "NORVI" / "X-AI4 TEST" header, followed by "CURRENT 1 :" through "CURRENT 4 :" labels, each showing a live current value in mA (e.g. `4.02 mA`), refreshing continuously.

## Common Issues
| Symptom | Likely Cause | Fix |
|---|---|---|
| `Failed to initialize ADS 1.` | ADS1115 not detected — wiring fault, or DIP-switch address doesn't match `ADS_ADDR` (`0x49`) hardcoded in the sketch | Verify SDA/SCL on GPIO 8/9, and confirm the module's DIP switch setting matches `0x49`, or update `ADS_ADDR` to match the actual DIP configuration (`0x48`/`0x4A`/`0x4B`) |
| Current readings show ~0 mA on all channels regardless of input | No 4-20mA signal source connected, or `ads1.begin()` failed silently | Confirm a live current loop signal is wired to the corresponding channel and check ADS1115 initialization status in Serial |
| Current readings are inaccurate / scaled incorrectly | `mA_Factor` calibration constant not matched to the module's actual shunt resistor value | Verify the `mA_Factor` (4.096 / 3269.826) against the X-AI4 module's shunt resistor spec and recalibrate if needed |
| TFT shows nothing / blank screen | SPI pins (MISO 13 / MOSI 11 / SCLK 12) or Display CS (GPIO 45) miswired, or `tft.init()` failing silently | Verify SPI wiring and that the pin numbers in the `LGFX` class match the board pinout |
| Display colors wrong or inverted | Panel invert / RGB order setting differs from the panel in use | Toggle `c.invert` or `c.rgb_order` in the `LGFX` class |
| Upload fails / board not detected | Wrong Board/Tools settings | Match settings to: ESP32S3 Dev Module, UART0/Hardware CDC, QSPI PSRAM, 16MB flash, Huge APP partition |

## Related Files
- Module reference: `expansion_module_AI4.md`
- CPU board: `cpu_board_X1.md`
