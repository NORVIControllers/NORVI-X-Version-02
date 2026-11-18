---
type: Example_program
function: thermistor_temperature_monitor (NTC read via ADS1115, Beta-equation conversion, compensation offset, 5-min trend, rate-of-rise alarm, over-temp alarm with hysteresis, sensor-fault detection, flicker-free TFT industrial UI)
compatible_cpu_boards: [NORVI X-CPU-ESPS3-X1 (V2, N16R2)]
compatible_modules: [NORVI X-T4 (4-channel NTC thermistor input, ADS1115 16-bit ADC, I2C 0x48),Littelfuse 103JG1K 10k NTC thermistor (R25 = 10k, B = 3892 K), ST7789 TFT 240x320 (LovyanGFX)]
status: completed
last_updated: 2026-10-28
---

# Example: Industrial Furnace / Oven Temperature Monitor (X-T4, Channel 1)

## What this does
Reads a 10k NTC thermistor on channel 1 (TM1+/TM1-) of the NORVI X-T4 through its ADS1115 ADC, converts the divider voltage to resistance and then to °C with the Beta equation, and applies a small compensation offset. The reading is averaged (4 conversions) and smoothed, then drawn on the built-in TFT as a 7-segment value, LED-style bar gauge (0–400 °C), 5-minute trend graph, rate-of-rise panel and a status banner. Alarm states are Normal, Warning (≥ 85 % of the limit), Over-temp (> 300 °C, latched with 5 °C hysteresis), Rate alarm (> 15 °C/min over 60 s) and Sensor fault (open/short after 5 bad reads in a row). One status line is printed on Serial every second.

## Hardware required
- CPU board: NORVI X-CPU-ESPS3-X1 — **V2 "N16R2" ONLY** (ESP32-S3 module, 16MB flash / QSPI PSRAM), 2.0 in ST7789 TFT (240 × 320), 24 V DC supply (1 A or higher)
- Expansion module(s):
  - NORVI X-T4 — 4 NTC channels, ADS1115 16-bit ADC at I2C `0x48` (default), 3.3 V divider with 10k bias resistor, powered through the expansion port (24 V DC, 40 mA)
  - 10k NTC thermistor (R25 = 10k), 2-wire, no polarity, rated for the process temperature
- Optional for commissioning: precision 10k resistor (should read about 25 °C), 3.6k resistor (about 50 °C), melting ice water (about 0 °C)
- X-T4 specifications (datasheet): 24 V DC, 40 mA; 4 two-wire NTC inputs (R25 = 10k, B approx. 3950 K, resistance and B tolerance ±1 % to ±3 %); probe range at least −40 to +125 °C; ADS1115 16-bit, 3.3 V; operating temperature −40 to +85 °C; IP20, DIN rail, push-in terminals (TM1+ … TM4-, AGND)
- X-T4 DIP switches: DIP 4 ON, DIP 1–3 OFF for address 0x48 (DIP 1 ON = 0x49, DIP 3 ON = 0x4A, DIP 2 ON = 0x4B)
- CPU pin facts (CPU guide): display CS IO45 / DC IO46 / RST IO47, SD CS IO42, Ethernet CS IO1 (RST IO2, INT IO4); the X-T4 uses I2C IO8/IO9 and the shared SPI lines on the 50-pin expansion port
- Wiring / pin map (from firmware `#define`s):

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
| ADS1115 (X-T4) address | 0x48 (I2C) |
| Thermistor channel 1 | TM1+ / TM1- (AIN0), AGND = shared analog ground |

> The SPI bus (MOSI 11 / MISO 13 / SCLK 12) is shared with the Ethernet controller (CS IO1) and SD card (CS IO42). If these devices are fitted, hold their CS lines HIGH so they ignore display traffic (see the Note above).

## Libraries required
| Library | Version |
|---|---|
| Wire (ESP32 core) | core-bundled |
| SPI (ESP32 core) | core-bundled |
| math.h | core-bundled |
| Adafruit ADS1X15 (Adafruit_ADS1X15.h) | _fill in tested version_ |
| Adafruit_BusIO (dependency of ADS1X15) | _fill in tested version_ |
| LovyanGFX (LovyanGFX.hpp, with `#define LGFX_USE_V1`) | _fill in tested version_ |

**ESP32 Board Package version:** _fill in tested version_

**Arduino IDE Board/Tools settings** (from the NORVI X-CPU-ESPS3-X1 user guide; recommended Arduino IDE 1.8.19):
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
| Partition Scheme | Default 4MB with spiffs (1.2MB APP/1.5MB SPIFFS) |
| Core Debug Level | None |
| PSRAM | QSPI PSRAM |
| Arduino Runs On | Core 1 |
| Events Run On | Core 1 |
| Erase All Flash Before Sketch Upload | Disabled |
| JTAG Adapter | Disabled |

## Key settings (top of sketch)
| Constant | Value | Meaning |
|---|---|---|
| `SUPPLY_VOLTAGE` | 3.3 V | X-T4 divider supply |
| `SERIES_RESISTOR` | 10000 Ω | Bias resistor |
| `THERMISTOR_NOMINAL` / `TEMPERATURE_NOMINAL` | 10000 Ω / 25 °C | R25 of the probe |
| `B_COEFFICIENT` | 3892 K | Littelfuse 103JG1K (use your probe's value; X-T4 datasheet quotes about 3950 K) |
| `MAX_SAFE_TEMP` | 300 °C | Over-temp alarm limit |
| `TEMP_HYSTERESIS` | 5 °C | Over-temp clears below 295 °C |
| `WARN_TEMP` | 85 % of limit = 255 °C | Amber warning zone |
| `RATE_OF_RISE_LIMIT` | 15 °C/min | Rate alarm (amber above 70 % of it) |
| `GAUGE_MAX` | 400 °C | Bar gauge full scale |
| `SENSE_INTERVAL` / `TREND_INTERVAL` / `DISPLAY_INTERVAL` | 100 ms / 3 s / 200 ms | Non-blocking schedule |
| `TREND_POINTS` / `ROR_SAMPLES` | 100 / 20 | 5 min of trend; 60 s rate window |

## Full Sketch
```cpp
/*
 * NORVI X-T4 Industrial Furnace Monitor - Channel 1
 * Industrial UI: 7-segment readout, LED bar gauge, live trend, alarm banner
 * Rendering is double-buffered (sprite) -> no flicker, no overlapping text.
 */

#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_ADS1X15.h>
#include <math.h>

#define SDA_PIN   8
#define SCL_PIN   9
#define MISO_PIN  13
#define MOSI_PIN  11
#define SCLK_PIN  12

#define ADS_ADDR 0x48
Adafruit_ADS1115 ads1;

// ---------- DISPLAY DRIVER (LovyanGFX) ----------
class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ST7789 _panel_instance;
  lgfx::Bus_SPI      _bus_instance;
public:
  LGFX(void) {
    {
      auto cfg = _bus_instance.config();
      cfg.spi_host     = SPI2_HOST;
      cfg.spi_mode     = 0;
      cfg.freq_write   = 40000000;
      cfg.freq_read    = 16000000;
      cfg.spi_3wire    = false;
      cfg.use_lock     = true;
      cfg.dma_channel  = SPI_DMA_CH_AUTO;
      cfg.pin_sclk     = SCLK_PIN;
      cfg.pin_mosi     = MOSI_PIN;
      cfg.pin_miso     = MISO_PIN;
      cfg.pin_dc       = 46;
      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }
    {
      auto cfg = _panel_instance.config();
      cfg.pin_cs            = 45;
      cfg.pin_rst           = 47;
      cfg.pin_busy          = -1;
      cfg.panel_width       = 240;
      cfg.panel_height      = 320;
      cfg.offset_x          = 0;
      cfg.offset_y          = 0;
      cfg.offset_rotation   = 0;
      cfg.dummy_read_pixel  = 8;
      cfg.dummy_read_bits   = 1;
      cfg.readable          = true;
      cfg.invert            = true;
      cfg.rgb_order         = false;
      cfg.dlen_16bit        = false;
      cfg.bus_shared        = true;
      _panel_instance.config(cfg);
    }
    setPanel(&_panel_instance);
  }
};

LGFX tft;
LGFX_Sprite canvas(&tft);   // full-screen off-screen buffer

// ---------- CONFIG & CONSTANTS ----------
const float SUPPLY_VOLTAGE      = 3.3;
const float SERIES_RESISTOR     = 10000.0;
const float THERMISTOR_NOMINAL  = 10000.0;
const float TEMPERATURE_NOMINAL = 25.0;
const float B_COEFFICIENT       = 3892.0;   // Littelfuse 103JG1K J-Curve

const float MAX_SAFE_TEMP       = 300.0;    // over-temperature alarm (C)
const float TEMP_HYSTERESIS     = 5.0;      // alarm clears below MAX - this
const float WARN_TEMP           = MAX_SAFE_TEMP * 0.85;  // amber warning zone
const float RATE_OF_RISE_LIMIT  = 15.0;     // C/min
const float GAUGE_MAX           = 400.0;    // bar gauge full scale

// Trend / rate-of-rise timing
const unsigned long TREND_INTERVAL = 3000;  // ms per trend sample
const int  TREND_POINTS = 100;              // 100 x 3 s = 5 min of history
const int  ROR_SAMPLES  = 20;               // 20 x 3 s = 60 s window
const unsigned long SENSE_INTERVAL   = 100; // ADC read period (ms)
const unsigned long DISPLAY_INTERVAL = 200; // screen refresh (ms)

float coldJunctionTemp = 25.0;

// ---------- COLOURS (RGB565) ----------
constexpr uint16_t RGB(uint8_t r, uint8_t g, uint8_t b) {
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}
const uint16_t C_BG       = RGB(8, 10, 14);
const uint16_t C_HEADER   = RGB(24, 34, 48);
const uint16_t C_PANEL    = RGB(16, 22, 30);
const uint16_t C_BORDER   = RGB(60, 74, 92);
const uint16_t C_TEXT     = RGB(232, 237, 242);
const uint16_t C_DIM      = RGB(130, 142, 158);
const uint16_t C_GREEN    = RGB(0, 225, 95);
const uint16_t C_GREEN_DK = RGB(0, 70, 34);
const uint16_t C_AMBER    = RGB(255, 176, 0);
const uint16_t C_AMBER_DK = RGB(80, 56, 0);
const uint16_t C_RED      = RGB(255, 50, 40);
const uint16_t C_RED_DK   = RGB(88, 14, 12);
const uint16_t C_CYAN     = RGB(0, 200, 255);
const uint16_t C_BANNER_OK= RGB(0, 110, 45);
const uint16_t C_BANNER_AL= RGB(190, 0, 0);

// ---------- STATE ----------
float tempF = NAN;             // filtered temperature
float rateOfRise = NAN;        // C/min (NAN = still warming up)
bool  overTempLatched = false;
int   badReadCount = 0;

float hist[TREND_POINTS];
int   histCount = 0;

unsigned long lastSense = 0, lastTrend = 0, lastDisplay = 0, lastSerial = 0;

// ---------- SENSOR ----------
float readThermistorC(int16_t adcRaw) {
  float voltage = ads1.computeVolts(adcRaw);
  if (voltage <= 0.001 || voltage >= SUPPLY_VOLTAGE - 0.001) return NAN;

  float ntcResistance = SERIES_RESISTOR * (SUPPLY_VOLTAGE / voltage - 1.0);
  float s = ntcResistance / THERMISTOR_NOMINAL;
  s = log(s);
  s /= B_COEFFICIENT;
  s += 1.0 / (TEMPERATURE_NOMINAL + 273.15);
  s = 1.0 / s;
  s -= 273.15;

  float cjcOffset = (coldJunctionTemp - 25.0) * 0.02;
  return s + cjcOffset;
}

void senseTemperature() {
  // Average 4 conversions to reduce noise
  long sum = 0;
  for (int i = 0; i < 4; i++) sum += ads1.readADC_SingleEnded(0);
  float raw = readThermistorC((int16_t)(sum / 4));

  if (isnan(raw)) {
    if (++badReadCount >= 5) tempF = NAN;          // confirmed open/short
  } else {
    badReadCount = 0;
    if (isnan(tempF)) tempF = raw;
    else tempF += 0.15f * (raw - tempF);           // smooth display jitter
  }
}

void pushTrend() {
  if (histCount == TREND_POINTS) {
    memmove(hist, hist + 1, sizeof(float) * (TREND_POINTS - 1));
    histCount--;
  }
  hist[histCount++] = tempF;

  // Rate of rise over the last 60 s (first valid result after 60 s)
  if (histCount > ROR_SAMPLES) {
    float a = hist[histCount - 1];
    float b = hist[histCount - 1 - ROR_SAMPLES];
    if (isnan(a) || isnan(b)) rateOfRise = NAN;
    else rateOfRise = (a - b) * (60000.0f / (ROR_SAMPLES * TREND_INTERVAL));
  }
}

// ---------- UI HELPERS ----------
void hazardStripe(int y, int h, uint16_t a, uint16_t b) {
  canvas.fillRect(0, y, 240, h, b);
  canvas.setClipRect(0, y, 240, h);
  for (int x = -h; x < 240; x += 14) {
    canvas.fillTriangle(x, y + h, x + 7, y + h, x + 7 + h, y, a);
    canvas.fillTriangle(x, y + h, x + 7 + h, y, x + h, y, a);
  }
  canvas.clearClipRect();
}

void panel(int x, int y, int w, int h, uint16_t border) {
  canvas.fillRoundRect(x, y, w, h, 6, C_PANEL);
  canvas.drawRoundRect(x, y, w, h, 6, border);
}

void smallText(const char* s, int x, int y, uint16_t col, textdatum_t d) {
  canvas.setFont(&fonts::Font0);
  canvas.setTextSize(1);
  canvas.setTextDatum(d);
  canvas.setTextColor(col);
  canvas.drawString(s, x, y);
}

void drawGauge(int x, int y, int w, int h, float t) {
  int fillW = isnan(t) ? 0 : constrain((int)(w * t / GAUGE_MAX), 0, w);
  for (int i = 0; i < w; i++) {
    float v = GAUGE_MAX * i / w;
    uint16_t bright, dim;
    if (v < WARN_TEMP)           { bright = C_GREEN; dim = C_GREEN_DK; }
    else if (v < MAX_SAFE_TEMP)  { bright = C_AMBER; dim = C_AMBER_DK; }
    else                         { bright = C_RED;   dim = C_RED_DK;   }
    canvas.drawFastVLine(x + i, y, h, i < fillW ? bright : dim);
  }
  // LED-segment look
  for (int i = 7; i < w; i += 8) canvas.drawFastVLine(x + i, y, h, C_BG);

  // Scale ticks + labels
  char b[8];
  for (int v = 0; v <= (int)GAUGE_MAX; v += 100) {
    int px = x + (int)(w * v / GAUGE_MAX);
    if (px > x + w - 1) px = x + w - 1;
    canvas.drawFastVLine(px, y + h, 3, C_DIM);
    snprintf(b, sizeof(b), "%d", v);
    smallText(b, px, y + h + 9, C_DIM, textdatum_t::middle_center);
  }
  // Limit marker
  int lx = x + (int)(w * MAX_SAFE_TEMP / GAUGE_MAX);
  canvas.fillTriangle(lx - 4, y - 6, lx + 4, y - 6, lx, y - 1, C_RED);
}

void drawTrend(int x, int y, int w, int h) {
  panel(x, y, w, h, C_BORDER);
  smallText("TREND  5 MIN", x + 8, y + 9, C_DIM, textdatum_t::middle_left);

  float mn = 1e9, mx = -1e9;
  for (int i = 0; i < histCount; i++) {
    if (isnan(hist[i])) continue;
    if (hist[i] < mn) mn = hist[i];
    if (hist[i] > mx) mx = hist[i];
  }
  if (mn > mx) {
    smallText("NO DATA", x + w / 2, y + h / 2 + 6, C_DIM, textdatum_t::middle_center);
    return;
  }

  char b[24];
  snprintf(b, sizeof(b), "H %.1f  L %.1f", mx, mn);
  smallText(b, x + w - 8, y + 9, C_CYAN, textdatum_t::middle_right);

  float lo = mn, hi = mx;
  if (hi - lo < 10.0f) { float mid = (hi + lo) / 2; lo = mid - 5; hi = mid + 5; }
  else { float pad = (hi - lo) * 0.1f; lo -= pad; hi += pad; }

  int px = x + 8, py = y + 20, pw = w - 16, ph = h - 28;

  // dotted grid
  for (int g = 0; g <= 2; g++) {
    int gy = py + (ph - 1) * g / 2;
    for (int dx = 0; dx < pw; dx += 4) canvas.drawPixel(px + dx, gy, C_BORDER);
  }

  // trace (newest sample at right edge)
  for (int i = 1; i < histCount; i++) {
    if (isnan(hist[i]) || isnan(hist[i - 1])) continue;
    int p0 = TREND_POINTS - histCount + i - 1;
    int p1 = p0 + 1;
    int x0 = px + p0 * (pw - 1) / (TREND_POINTS - 1);
    int x1 = px + p1 * (pw - 1) / (TREND_POINTS - 1);
    int y0 = py + ph - 2 - (int)((hist[i - 1] - lo) / (hi - lo) * (ph - 3));
    int y1 = py + ph - 2 - (int)((hist[i]     - lo) / (hi - lo) * (ph - 3));
    canvas.drawLine(x0, y0, x1, y1, C_CYAN);
    canvas.drawLine(x0, y0 + 1, x1, y1 + 1, C_CYAN);
  }
}

// ---------- MAIN RENDER ----------
void render(unsigned long now) {
  bool blink = (now / 500) % 2;

  // ----- alarm logic -----
  bool sensorFault = isnan(tempF);
  if (!sensorFault) {
    if (tempF > MAX_SAFE_TEMP) overTempLatched = true;
    else if (tempF < MAX_SAFE_TEMP - TEMP_HYSTERESIS) overTempLatched = false;
  }
  bool rateAlarm = !isnan(rateOfRise) && rateOfRise > RATE_OF_RISE_LIMIT;
  bool alarm = sensorFault || overTempLatched || rateAlarm;
  bool warn  = !alarm && !sensorFault && tempF >= WARN_TEMP;

  uint16_t stateCol = alarm ? C_RED : (warn ? C_AMBER : C_GREEN);

  canvas.fillScreen(C_BG);
  char b[32];

  // ----- HEADER -----
  canvas.fillRect(0, 0, 240, 36, C_HEADER);
  canvas.setFont(&fonts::FreeSansBold9pt7b);
  canvas.setTextSize(1);
  canvas.setTextDatum(textdatum_t::middle_left);
  canvas.setTextColor(C_TEXT);
  canvas.drawString("FURNACE CONTROL", 10, 18);
  // status LED
  uint16_t led = alarm ? (blink ? C_RED : C_RED_DK) : (warn ? C_AMBER : C_GREEN);
  canvas.fillCircle(220, 18, 7, led);
  canvas.drawCircle(220, 18, 10, C_BORDER);
  hazardStripe(36, 5, C_AMBER, C_BG);

  // ----- CHANNEL ROW -----
  canvas.setFont(&fonts::FreeSans9pt7b);
  canvas.setTextDatum(textdatum_t::middle_left);
  canvas.setTextColor(C_DIM);
  canvas.drawString("CH1 : OVEN ZONE", 10, 52);
  unsigned long s = now / 1000;
  snprintf(b, sizeof(b), "UP %02lu:%02lu:%02lu", s / 3600, (s / 60) % 60, s % 60);
  smallText(b, 230, 52, C_DIM, textdatum_t::middle_right);

  // ----- TEMPERATURE PANEL -----
  panel(8, 64, 224, 82, stateCol);
  if (alarm) canvas.drawRoundRect(9, 65, 222, 80, 5, stateCol);
  smallText("TEMPERATURE", 18, 75, C_DIM, textdatum_t::middle_left);
  snprintf(b, sizeof(b), "LIMIT %dC", (int)MAX_SAFE_TEMP);
  smallText(b, 222, 75, C_DIM, textdatum_t::middle_right);

  if (sensorFault) {
    canvas.setFont(&fonts::FreeSansBold12pt7b);
    canvas.setTextDatum(textdatum_t::middle_center);
    canvas.setTextColor(blink ? C_RED : C_RED_DK);
    canvas.drawString("SENSOR OPEN", 120, 108);
    smallText("CHECK THERMISTOR WIRING", 120, 132, C_DIM, textdatum_t::middle_center);
  } else {
    snprintf(b, sizeof(b), "%.1f", tempF);
    canvas.setFont(&fonts::Font7);          // 7-segment style digits
    canvas.setTextSize(1);
    canvas.setTextDatum(textdatum_t::middle_right);
    canvas.setTextColor(stateCol);
    canvas.drawString(b, 194, 114);
    // degree sign + unit
    canvas.drawCircle(203, 98, 3, stateCol);
    canvas.drawCircle(203, 98, 2, stateCol);
    canvas.setFont(&fonts::FreeSansBold12pt7b);
    canvas.setTextDatum(textdatum_t::middle_left);
    canvas.drawString("C", 209, 110);
  }

  // ----- BAR GAUGE -----
  drawGauge(8, 156, 224, 10, tempF);

  // ----- TREND -----
  drawTrend(8, 182, 224, 70);

  // ----- RATE OF RISE -----
  uint16_t rc = C_TEXT;
  if (!isnan(rateOfRise)) {
    if (rateOfRise > RATE_OF_RISE_LIMIT) rc = C_RED;
    else if (rateOfRise > RATE_OF_RISE_LIMIT * 0.7f) rc = C_AMBER;
  }
  panel(8, 256, 224, 28, rc == C_TEXT ? C_BORDER : rc);
  canvas.setFont(&fonts::FreeSans9pt7b);
  canvas.setTextDatum(textdatum_t::middle_left);
  canvas.setTextColor(C_DIM);
  canvas.drawString("RATE OF RISE", 16, 270);
  canvas.setFont(&fonts::FreeSansBold9pt7b);
  canvas.setTextDatum(textdatum_t::middle_right);
  canvas.setTextColor(rc);
  if (isnan(rateOfRise)) snprintf(b, sizeof(b), "-- C/min");
  else                   snprintf(b, sizeof(b), "%+.1f C/min", rateOfRise);
  canvas.drawString(b, 224, 270);

  // ----- STATUS BANNER -----
  uint16_t bannerCol = alarm ? (blink ? C_BANNER_AL : C_RED_DK)
                             : (warn ? RGB(190, 120, 0) : C_BANNER_OK);
  canvas.fillRoundRect(8, 288, 224, 24, 6, bannerCol);
  canvas.setFont(&fonts::FreeSansBold9pt7b);
  canvas.setTextDatum(textdatum_t::middle_center);
  canvas.setTextColor(C_TEXT);
  const char* msg;
  if (sensorFault)         msg = "! SENSOR FAULT !";
  else if (overTempLatched) msg = "! OVER TEMP !";
  else if (rateAlarm)       msg = "! RISE TOO FAST !";
  else if (warn)            msg = "TEMP WARNING";
  else                      msg = "SYSTEM NORMAL";
  canvas.drawString(msg, 120, 300);

  // bottom edge: hazard stripe on alarm, thin line otherwise
  if (alarm) hazardStripe(314, 6, C_AMBER, C_BG);
  else       canvas.drawFastHLine(8, 316, 224, C_BORDER);

  canvas.pushSprite(0, 0);
}

// ---------- SETUP / LOOP ----------
void setup() {
  Serial.begin(115200);
  delay(200);

  Wire.begin(SDA_PIN, SCL_PIN);
  if (!ads1.begin(ADS_ADDR)) Serial.println("ADS1115 not found!");
  ads1.setGain(GAIN_ONE);
  ads1.setDataRate(RATE_ADS1115_860SPS);

  SPI.begin(SCLK_PIN, MISO_PIN, MOSI_PIN);
  delay(100);

  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);

  canvas.setColorDepth(16);
  if (!canvas.createSprite(240, 320)) {       // fall back to 8-bit if RAM is tight
    canvas.setColorDepth(8);
    canvas.createSprite(240, 320);
    Serial.println("Sprite: using 8-bit fallback");
  }

  senseTemperature();
  render(millis());
}

void loop() {
  unsigned long now = millis();

  if (now - lastSense >= SENSE_INTERVAL) {
    lastSense = now;
    senseTemperature();
  }

  if (now - lastTrend >= TREND_INTERVAL) {
    lastTrend = now;
    pushTrend();
  }

  if (now - lastDisplay >= DISPLAY_INTERVAL) {
    lastDisplay = now;
    render(now);
  }

  if (now - lastSerial >= 1000) {
    lastSerial = now;
    Serial.printf("Furnace Z1: %.2f C | RoR: %.1f C/min\n", tempF, rateOfRise);
  }
}
```

## How the conversion works
- The X-T4 places the thermistor in a divider with a 10k bias resistor on 3.3 V. The ADS1115 (`GAIN_ONE`, ±4.096 V, 125 µV per count) measures the voltage across the bias resistor: `R_ntc = 10k × (3.3 / V − 1)`.
- Beta equation: `1/T = 1/T0 + ln(R/R0)/B`, with T0 = 298.15 K, R0 = 10k, B = 3892 K, then `T(°C) = T − 273.15`.
- Valid window is 1 mV to 3.299 V. Outside it the read is invalid (about 0 V = open, about 3.3 V = shorted); the fault is declared after 5 bad reads in a row (0.5 s).
- Reference values for commissioning (ideal Beta model, real probes vary with tolerance):

| Temperature | R_ntc | Divider voltage | Expected count | Resolution per count |
|---|---|---|---|---|
| 0 °C | 33.0k | 0.767 V | 6136 | < 0.01 °C |
| 25 °C | 10.0k | 1.650 V | 13200 | < 0.01 °C |
| 50 °C | 3.64k | 2.419 V | 19351 | < 0.01 °C |
| 100 °C | 725 Ω | 3.077 V | 24615 | 0.02 °C |
| 150 °C | 211 Ω | 3.232 V | 25853 | 0.09 °C |
| 300 °C | 19 Ω | 3.294 V | 26350 | 1.7 °C |

## Alarm states
| State | Condition | Display | Header LED |
|---|---|---|---|
| Normal | Below 255 °C | Green value, `SYSTEM NORMAL` | Green |
| Warning | 255 to 300 °C | Amber value, `TEMP WARNING` | Amber |
| Over-temp | Above 300 °C (clears below 295 °C) | Red value, `! OVER TEMP !` | Red, blinking |
| Rate alarm | Rise above 15 °C/min | Red rate, `! RISE TOO FAST !` | Red, blinking |
| Sensor fault | Open or short | `SENSOR OPEN`, `! SENSOR FAULT !` | Red, blinking |

## Expected Output
On the Serial Monitor (115200 baud):
- Nothing at boot if the X-T4 is found, or `ADS1115 not found!` if it is not (check DIP address and seating)
- `Sprite: using 8-bit fallback` only if there is not enough RAM for the 16-bit sprite
- One line per second, for example `Furnace Z1: 25.43 C | RoR: 0.2 C/min`; the rate prints `nan` during the first minute while the display shows `--`

On the TFT display: "FURNACE CONTROL" header with status LED and hazard stripe, `CH1 : OVEN ZONE` and uptime, large 7-segment temperature with limit label, 0–400 °C bar gauge with red limit marker, 5-minute trend with high/low values, rate-of-rise panel and a status banner (hazard stripe along the bottom edge in alarm).

## Commissioning checklist
- Power: 24 V DC stable and correctly wired to the X-CPU before energizing.
- I2C: DIP switches set to 0x48 and no `ADS1115 not found!` message.
- Resistor substitution: a precision 10k resistor in place of the probe reads about 25 °C; 3.6k reads about 50 °C.
- Ice water: probe in melting ice water settles near 0 °C within probe tolerance.
- Thresholds: temporarily set `MAX_SAFE_TEMP` to 40 °C, warm the probe, confirm Warning and Over-temp, then restore the limit.
- Rate of rise: move the probe from room air into about 60 °C water; after the first minute the rate should exceed 15 °C/min and trigger the rate alarm.
- Fault response: disconnect one thermistor lead; `SENSOR OPEN` should appear within about a second.

## Common Issues
| Symptom | Likely Cause | Fix |
|---|---|---|
| Display blank or white | Wrong SPI pins, display bus not initialized, or wrong Tools settings | Confirm CS 45, DC 46, RST 47 in the LGFX class; set USB CDC On Boot, Flash Size and PSRAM as in the table above |
| `ADS1115 not found!` | X-T4 not seated, or DIP address does not match `ADS_ADDR` | Power off and reseat the X-T4; set DIP 4 ON for 0x48; run an I2C scan (expect 0x48) |
| `SENSOR OPEN` / `! SENSOR FAULT !` | Broken lead, loose terminal or shorted leads | Check leads on TM1+ / TM1-; with power off the probe should measure about 10k at 25 °C |
| Negative or erratic temperature | Wrong sensor type, damaged probe or noise pickup | Confirm a 10k NTC; use shielded twisted pair, ground the shield at one end, keep away from motor/VFD/relay wiring |
| Temperature offset from actual | B value or divider constants do not match the probe | Set `B_COEFFICIENT` to the probe value (3892 or 3950 K), compare with a reference thermometer, trim the offset |
| Rate alarm triggers unexpectedly | Draught, poorly seated probe or nearby heat source | Fix probe mounting first, then raise `RATE_OF_RISE_LIMIT` if needed |
| Alarm flickers near 300 °C | Temperature hovers at the limit | Increase `TEMP_HYSTERESIS` |
| Wrong or coarse reading at high temperature | Beyond the probe rating, or limited divider resolution | Use a high-temperature probe, or a thermocouple/RTD module from the NORVI X range |

## Possible extensions
- Monitor all four channels: `readADC_SingleEnded(1)` to `(3)`, with a separate trend buffer, rate and alarm state per zone.
- Publish readings over MQTT, log to the SD card, or switch a relay output on alarm.

## Related Files
- CPU board: `cpu_board_X1.md`
- System diagnostics / board bring-up example: `NORVI_X-CPU-ESPS3-X1_program.md`
- Source documents: NORVI X-T4 datasheet and user guide; NORVI X-CPU-ESPS3-X1 datasheet and user guide; X-T4 test sketch `NORVI_X-T4_Test_Code.ino` in the NORVI-X-Version-02 GitHub repository (expansion modules / NORVI-X-T4 / examples / arduino)
