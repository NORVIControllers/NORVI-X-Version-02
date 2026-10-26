---
type: Test_program
function: ntc_temperature_read (4-channel NTC thermistor temperature input test via ADS1115 with TFT display)
compatible_cpu_boards: [NORVI X-CPU-ESPS3-X1],[NORVI X-CPU-ESPS3-X2],[NORVI X-CPU-ESPS3-X3]
compatible_modules: [NORVI X-T4 (4-channel NTC thermistor temperature input expansion, ADS1115 I2C 16-bit ADC)]
status: completed
last_updated: 2026-10-22
---

# Example: NORVI X-T4 NTC Temperature Input Test

## What this does
Reads 4 single-ended analog channels via an ADS1115 ADC, converts each reading to the NTC thermistor resistance (10K fixed resistor divider) and then to a temperature in °C using the Beta equation, and continuously displays the four temperatures on the TFT screen while printing them to Serial. An open or shorted sensor is shown as `--.-- C`.

## Hardware required
- CPU board: NORVI X-CPU-ESPS3-X1 OR X2, X3
- Expansion module(s): NORVI X-T4 (4-channel NTC thermistor temperature input module, ADS1115 I2C ADC)
  - ADS1115 I2C address is DIP-switch selectable: `0x49` (DIP1 ON), `0x4B` (DIP2 ON), `0x4A` (DIP3 ON), `0x48` (DIP4 ON) — this sketch uses `0x48` by default
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
 *NORVI X-T4
 *2026/08/31
 */

#include <Wire.h>
#include <WiFi.h>
#include <SPI.h>
#include <Adafruit_ADS1X15.h>
#include <math.h>

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

#define ADS_ADDR 0x48
Adafruit_ADS1115 ads1;

// ---------- THERMISTOR / DIVIDER CONFIG ----------
// Set this to the actual voltage feeding the top of the divider (the
// rail the NTC's other end is tied to). Common values: 3.3 or 5.0.
const float SUPPLY_VOLTAGE = 3.3;

const float SERIES_RESISTOR      = 10000.0;  // the fixed 10K resistor
const float THERMISTOR_NOMINAL   = 10000.0;  // NTC resistance at 25C
const float TEMPERATURE_NOMINAL  = 25.0;     // reference temp, deg C
const float B_COEFFICIENT        = 3950.0;   // beta value from your NTC's datasheet -- CHECK/UPDATE THIS

unsigned long lastDisplayUpdate = 0;

const unsigned long DISPLAY_INTERVAL = 100;


float lastDisplay0 = -999.0;
float lastDisplay1 = -999.0;
float lastDisplay2 = -999.0;
float lastDisplay3 = -999.0;

unsigned long lastSerialPrint = 0;
const unsigned long SERIAL_INTERVAL = 500;   // print temperatures to Serial twice a second

bool adsFound = false;                       // set in setup(), shown on the TFT if false


void setup() {

  Serial.begin(115200);
  delay(200);

  Serial.println();
  Serial.println("NPD-NORVI X-T4 NTC TEMPERATURE TEST");
  
  // i2c
  Wire.begin(SDA, SCL);

  if (!ads1.begin(ADS_ADDR)) {

    Serial.println("Failed to initialize ADS 1.");

  }
  else {

    adsFound = true;
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
  tft.setTextDatum(lgfx::textdatum_t::baseline_left);   // cursor Y = text baseline, same layout as before

  tft.fillScreen(TFT_BLACK);


  // TITLE
  tft.setTextColor(TFT_GREEN);
  tft.setFont(&fonts::FreeSerifBold12pt7b);
  
  tft.setCursor(40, 20);
  tft.print("NORVI X-T4");



  // TEMPERATURE LABELS
  tft.setFont(&fonts::FreeSansBold9pt7b);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setCursor(10, 85);
  tft.print("TEMP 1 :");

  tft.setCursor(10, 115);
  tft.print("TEMP 2 :");

  tft.setCursor(10, 145);
  tft.print("TEMP 3 :");

  tft.setCursor(10, 175);
  tft.print("TEMP 4 :");

  // INITIAL TEMPERATURE VALUES
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  
  tft.setCursor(145, 85);
  tft.print("0.00 C");

  tft.setCursor(145, 115);
  tft.print("0.00 C");

  tft.setCursor(145, 145);
  tft.print("0.00 C");

  tft.setCursor(145, 175);
  tft.print("0.00 C");


  // ADS1115 ADDRESS AT BOTTOM

  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setCursor(10, 215);
  tft.print("ADS ADDR : 0x");

  if (ADS_ADDR < 0x10) {
    tft.print("0");
  }

  tft.print(ADS_ADDR, HEX);

  // ADS1115 NOT FOUND WARNING
  if (!adsFound) {
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.setCursor(10, 245);
    tft.print("ADS1115 NOT FOUND");
  }

  Serial.println("TEST START");
}


// True when a displayed value must be redrawn. NAN never equals NAN, so it is compared separately
// (otherwise a missing sensor would be redrawn on every update and flicker).
bool tempChanged(float now, float shown) {
  if (isnan(now) || isnan(shown)) return isnan(now) != isnan(shown);
  return now != shown;
}


// Converts a raw ADS1115 single-ended reading into a temperature (deg C)
// using the Beta-equation approximation of the Steinhart-Hart formula.
float readThermistorC(int16_t adcRaw) {

  // computeVolts() correctly applies the current gain setting (GAIN_ONE
  // here = +/-4.096V range over 16 bits), so this stays correct even if
  // you change the gain later.
  float voltage = ads1.computeVolts(adcRaw);

  // Guard against divide-by-zero / nonsense readings (e.g. open circuit)
  if (voltage <= 0.001) {
    return NAN;
  }
  if (voltage >= SUPPLY_VOLTAGE - 0.001) {
    return NAN;
  }

  // Solve the divider for NTC resistance.
  // Topology: VCC -> NTC -> node -> 10K -> GND, ADC reads "node".
  float ntcResistance = SERIES_RESISTOR * (SUPPLY_VOLTAGE / voltage - 1.0);

  // *** If your wiring is reversed (NTC from node to GND, fixed resistor
  // from VCC to node), replace the line above with:
  //   float ntcResistance = SERIES_RESISTOR / (SUPPLY_VOLTAGE / voltage - 1.0);

  // Beta equation
  float steinhart;
  steinhart = ntcResistance / THERMISTOR_NOMINAL;      // (R/Ro)
  steinhart = log(steinhart);                          // ln(R/Ro)
  steinhart /= B_COEFFICIENT;                           // 1/B * ln(R/Ro)
  steinhart += 1.0 / (TEMPERATURE_NOMINAL + 273.15);    // + (1/To)
  steinhart = 1.0 / steinhart;                          // invert
  steinhart -= 273.15;                                  // convert to deg C

  return steinhart;
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

  // ADC TO TEMPERATURE
  float temp0 = readThermistorC(adc0);
  float temp1 = readThermistorC(adc1);
  float temp2 = readThermistorC(adc2);
  float temp3 = readThermistorC(adc3);

  // SERIAL MONITOR
  if (millis() - lastSerialPrint >= SERIAL_INTERVAL) {

    lastSerialPrint = millis();

    Serial.printf(
      "T1: %.2f C  "
      "T2: %.2f C  "
      "T3: %.2f C  "
      "T4: %.2f C\n",
      temp0,
      temp1,
      temp2,
      temp3
    );
  }

  // TFT UPDATE
  if (millis() - lastDisplayUpdate >= DISPLAY_INTERVAL) {

    lastDisplayUpdate = millis();

    char buf[20];

    tft.setTextColor(TFT_WHITE, TFT_BLACK);


    // TEMP 1
    if (tempChanged(temp0, lastDisplay0)) {

      tft.fillRect(140, 68, 100, 25, TFT_BLACK);
      if (isnan(temp0)) {
        tft.setCursor(145, 85);
        tft.print("--.-- C");
      } else {
        dtostrf(temp0, 5, 2, buf);
        tft.setCursor(145, 85);
        tft.print(buf);
        tft.print(" C");
      }
      lastDisplay0 = temp0;
    }
    
    // TEMP 2
    if (tempChanged(temp1, lastDisplay1)) {
      tft.fillRect(140, 98, 100, 25, TFT_BLACK);
      if (isnan(temp1)) {
        tft.setCursor(145, 115);
        tft.print("--.-- C");
      } else {
        dtostrf(temp1, 5, 2, buf);
        tft.setCursor(145, 115);
        tft.print(buf);
        tft.print(" C");
      }
      lastDisplay1 = temp1;
    }

    // TEMP 3
    if (tempChanged(temp2, lastDisplay2)) {
      tft.fillRect(140, 128, 100, 25, TFT_BLACK);
      if (isnan(temp2)) {
        tft.setCursor(145, 145);
        tft.print("--.-- C");
      } else {
        dtostrf(temp2, 5, 2, buf);
        tft.setCursor(145, 145);
        tft.print(buf);
        tft.print(" C");
      }
      lastDisplay2 = temp2;
    }

    // TEMP 4
    if (tempChanged(temp3, lastDisplay3)) {
      tft.fillRect(140, 158, 100, 25, TFT_BLACK);
      if (isnan(temp3)) {
        tft.setCursor(145, 175);
        tft.print("--.-- C");
      } else {
        dtostrf(temp3, 5, 2, buf);
        tft.setCursor(145, 175);
        tft.print(buf);
        tft.print(" C");
      }
      lastDisplay3 = temp3;
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
- `NPD-NORVI X-T4 NTC TEMPERATURE TEST`
- `ADS1115 OK` (or `Failed to initialize ADS 1.` only if ADS1115 not detected at the configured address)
- `ADS1115 ADDRESS: 0x48` and `ADS1115 DATA RATE = 860 SPS`, then `TEST START`
- Twice a second (every 500 ms): `T1: %.2f C  T2: %.2f C  T3: %.2f C  T4: %.2f C` (e.g. `T1: 25.31 C  T2: 24.87 C  T3: 26.02 C  T4: 25.10 C`; an open or shorted sensor is printed as `nan`)

On the TFT display: "NORVI X-T4" header, followed by "TEMP 1 :" through "TEMP 4 :" labels, each showing a live temperature in °C (e.g. `25.31 C`, or `--.-- C` for an open/shorted sensor), and "ADS ADDR : 0x48" at the bottom, refreshing continuously (a row is redrawn only when its value changes). If the ADS1115 is not found at start-up, a red "ADS1115 NOT FOUND" message is shown below the address line.

## Common Issues
| Symptom | Likely Cause | Fix |
|---|---|---|
| `Failed to initialize ADS 1.` | ADS1115 not detected — wiring fault, or DIP-switch address doesn't match `ADS_ADDR` (`0x48`) hardcoded in the sketch | Verify SDA/SCL on GPIO 8/9, and confirm the module's DIP switch setting matches `0x48`, or update `ADS_ADDR` to match the actual DIP configuration (`0x49`/`0x4A`/`0x4B`) |
| Red "ADS1115 NOT FOUND" on the TFT (all channels show `--.-- C`) | ADS1115 not detected at start-up — wiring fault or DIP-switch address doesn't match `ADS_ADDR` | See the `Failed to initialize ADS 1.` row above |
| A channel shows `--.-- C` (and `nan` on Serial) | Sensor open-circuit (reading at the supply rail) or shorted (reading ~0 V), or no NTC connected to that channel | Check the NTC wiring and connection for that channel |
| Temperature readings are wrong or offset | `B_COEFFICIENT`, `THERMISTOR_NOMINAL`, `SERIES_RESISTOR` or `SUPPLY_VOLTAGE` do not match the NTC and the module | Set the constants at the top of the sketch from the NTC datasheet (10K at 25 °C, B = 3950 by default) and the actual divider supply voltage |
| Temperature goes the wrong way (hotter reads colder) | NTC and fixed resistor wired the other way round in the divider | Use the alternative `ntcResistance` formula given in the comment inside `readThermistorC()` |
| TFT shows nothing / blank screen | SPI pins (MISO 13 / MOSI 11 / SCLK 12) or Display CS (GPIO 45) miswired, or `tft.init()` failing silently | Verify SPI wiring and that the pin numbers in the `LGFX` class match the board pinout |
| Display colors wrong or inverted | Panel invert / RGB order setting differs from the panel in use | Toggle `c.invert` or `c.rgb_order` in the `LGFX` class |
| Upload fails / board not detected | Wrong Board/Tools settings | Match settings to: ESP32S3 Dev Module, UART0/Hardware CDC, QSPI PSRAM, 16MB flash, Huge APP partition |

## Related Files
- Module reference: `expansion_module_T4.md`
- CPU board: `cpu_board_X1.md`