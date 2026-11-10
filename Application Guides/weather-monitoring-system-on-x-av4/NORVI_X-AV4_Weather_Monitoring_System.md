---
type: Example_program
function: analog_0_10V_weather_display (humidity, wind speed and wind direction read via X-AV4 ADS1115, divider-ratio scaling to engineering units, 8-sample averaging, circular-mean direction, over-range and module-offline fault detection, flicker-free TFT industrial UI)
compatible_cpu_boards: [NORVI X-CPU-ESPS3-X1]
compatible_modules: [NORVI X-AV4 (4-channel 0-10 V input, ADS1115 16-bit ADC, I2C 0x49 with DIP 1 ON), Sentera DSTHF-3 humidity transmitter (0-10 V on AO2), WG2/O-10 wind speed transmitter (0-50 m/s = 0-10 V), WRG/O-40 wind direction transmitter (0-360 deg = 0-10 V), ST7789 TFT 240x320 (LovyanGFX)]
status: completed
last_updated: 2026-11-10
---

# Example: Industrial Weather Monitoring System (X-AV4, Channels 1 to 3)

## What this does
Reads three 0–10 V transmitters on the NORVI X-AV4 through its ADS1115 ADC: humidity on AIN1 (0–10 V = 0–100 %RH), wind speed on AIN2 (0–10 V = 0–50 m/s) and wind direction on AIN3 (0–10 V = 0–360°). Each channel is read 8 times per update and averaged (direction is averaged as a vector so that 359° and 1° give about 0°).The voltage is scaled to %RH, m/s and degrees, and the result is drawn on the built-in TFT: wind speed with a segmented gauge (green / amber / red), a compass dial with needle, 16-point compass name and angle, and humidity with a gauge. A header LED blinks green while all channels are healthy and turns red on any fault. A channel shows `FAULT` when its voltage is above 10.5 V, and all channels show `---` when the X-AV4 does not answer on I2C. One status line is printed on Serial every 500 ms.


## Hardware required
- CPU board: NORVI X-CPU-ESPS3-X1 (ESP32-S3 module, 16MB flash / QSPI PSRAM, 2 MB PSRAM), 2.0 in ST7789 TFT (240 × 320), 24 V DC supply (1 A or higher)
- Expansion module(s):
  - NORVI X-AV4 — 4 analog inputs 0–10 V, ADS1115 16-bit ADC at I2C `0x49` (DIP 1 ON), powered through the expansion port (24 V DC, 40 mA)
  - Sentera DSTHF-3 humidity transmitter — 0–100 %rH, ±3 %rH, 18–34 V DC, 80 mA, humidity on output AO2 set to 0–10 V
  - WG2/O-10 wind speed transmitter — 0–50 m/s = 0–10 V, ±0.5 m/s or ±3 % of reading, 13–30 V DC or 24 V AC/DC, 50 mA
  - WRG/O-40 wind direction transmitter — 0–360° = 0–10 V, ±1.5°, 1° resolution, 15–28 V DC or 24 V AC
- Cable: shielded twisted pair for the signal wires, shield connected at one end
- Optional for commissioning: adjustable 0–10 V source or calibrator, multimeter, compass, reference hygrometer
- X-AV4 specifications: 24 V DC, 40 mA; 4 analog inputs, 0–10 V; ADS1115 16-bit, 3.3 V; operating temperature −40 to +85 °C; IP20, DIN rail, push-in terminals (AIN1 … AIN4, AGND)
- X-AV4 DIP switches: DIP 1 ON, DIP 2–4 OFF for address 0x49 (DIP 4 ON = 0x48, DIP 3 ON = 0x4A, DIP 2 ON = 0x4B)
- CPU pin facts: display CS IO45 / DC IO46 / RST IO47, SD CS IO42, Ethernet CS IO1 (RST IO2, INT IO4); the X-AV4 uses I2C IO8/IO9 on the 50-pin expansion port
- Wiring / pin map (from firmware):

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
| ADS1115 (X-AV4) address | 0x49 (I2C) |
| Humidity | AIN1 (ADS1115 channel 0), transmitter output AO2 |
| Wind speed | AIN2 (ADS1115 channel 1) |
| Wind direction | AIN3 (ADS1115 channel 2) |
| Spare input | AIN4 (ADS1115 channel 3) |
| Common return | AGND, joined to the supply 0 V once at the X-AV4 |

- Sensor wiring: supply 24 V DC to each transmitter in parallel; each signal output to its AIN terminal; each transmitter GND to AGND; supply 0 V to AGND. For the DSTHF-3, wire the supply GND to the supply 0 V and the GND beside AO2 to AGND as separate wires.

## Libraries required
| Library | Version |
|---|---|
| Wire (ESP32 core) | core-bundled |
| SPI (ESP32 core) | core-bundled |
| math.h | core-bundled |
| Adafruit ADS1X15 (Adafruit_ADS1X15.h) | 2.6.2 |
| Adafruit_BusIO (dependency of ADS1X15) | 1.17.4 |
| LovyanGFX (LovyanGFX.hpp, with `#define LGFX_USE_V1`) | 2.6.2 |

**ESP32 Board Package version:** 3.XX (Above 3)

**Arduino IDE Board/Tools settings** (recommended Arduino IDE 1.8.19):
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
| `ADS_ADDR` | 0x49 | X-AV4 I2C address (DIP 1 ON) |
| `DIVIDER` | 0.3125 | X-AV4 input divider (3.3k / 1.5k), terminal V to ADC V |
| `HUM_MAX_PCT` | 100 %RH | Humidity at 10 V |
| `WIND_MAX_MS` | 50 m/s | Wind speed at 10 V (WG2/O-10 full scale) |
| `OVER_RANGE_V` | 10.5 V | Above this the channel shows FAULT |
| `SAMPLES` | 8 | Readings averaged per channel per update |
| `UPDATE_MS` | 500 ms | Screen and Serial update period |
| ADC gain | `GAIN_ONE` | ±4.096 V, 125 µV per count (13.1 V at the terminal) |

## Full Sketch
```cpp
/*
  NORVI X1 + X-AV4  -  Industrial weather monitoring system

    X-AV4 AIN1 -> humidity        (0-10 V = 0-100 %RH)
    X-AV4 AIN2 -> wind speed      (0-10 V = 0-WIND_MAX_MS)
    X-AV4 AIN3 -> wind direction  (0-10 V = 0-360 deg)

*/

#include <Wire.h>
#include <SPI.h>
#include <math.h>
#include <Adafruit_ADS1X15.h>
#define LGFX_USE_V1
#include <LovyanGFX.hpp>

// Channel state. Declared before any function so the Arduino auto-prototypes can see it.
enum ChState { ST_OK, ST_FAULT, ST_NOADC };

// ---------------- Settings ----------------
static const uint8_t  ADS_ADDR     = 0x49;     // X-AV4 DIP1 ON (0x48/0x4A/0x4B on other DIP settings)
static const float    DIVIDER      = 0.3125f;  // X-AV4 input divider (terminal V -> ADC V)
static const float    HUM_MAX_PCT  = 100.0f;   // humidity at 10 V
static const float    WIND_MAX_MS  = 50.0f;    // wind speed at 10 V (check your sensor datasheet)
static const float    OVER_RANGE_V = 10.5f;    // above this the input is treated as a fault
static const int      SAMPLES      = 8;        // readings averaged per update
static const uint32_t UPDATE_MS    = 500;

// ---------------- Display (NORVI X1 TFT, ST7789 240x320) ----------------
class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ST7789 _panel;
  lgfx::Bus_SPI      _bus;
public:
  LGFX() {
    auto b = _bus.config();
    b.spi_host = SPI2_HOST;  b.spi_mode = 0;
    b.freq_write = 27000000; b.freq_read = 16000000;
    b.spi_3wire = false;     b.use_lock = true;
    b.dma_channel = SPI_DMA_CH_AUTO;
    b.pin_sclk = 12; b.pin_mosi = 11; b.pin_miso = 13; b.pin_dc = 46;
    _bus.config(b);
    _panel.setBus(&_bus);

    auto p = _panel.config();
    p.pin_cs = 45; p.pin_rst = 47; p.pin_busy = -1;
    p.panel_width = 240; p.panel_height = 320;
    p.offset_x = 0; p.offset_y = 0; p.offset_rotation = 0;
    p.readable = false; p.invert = true; p.rgb_order = true;
    p.dlen_16bit = false; p.bus_shared = true;
    _panel.config(p);
    setPanel(&_panel);
  }
};

static LGFX        tft;
static LGFX_Sprite canvas(&tft);
static Adafruit_ADS1115 ads;
static bool adsReady = false;
static bool canvasOk = false;

// ---------------- Colours ----------------
static constexpr uint16_t rgb(uint8_t r, uint8_t g, uint8_t b) {
  return (uint16_t)(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
}
static const uint16_t C_BG     = rgb(14, 17, 22);
static const uint16_t C_HDR    = rgb(28, 34, 42);
static const uint16_t C_PANEL  = rgb(24, 30, 38);
static const uint16_t C_BORDER = rgb(52, 63, 77);
static const uint16_t C_TRACK  = rgb(10, 12, 16);
static const uint16_t C_TEXT   = rgb(240, 244, 248);
static const uint16_t C_DIM    = rgb(130, 143, 158);
static const uint16_t C_AMBER  = rgb(255, 176, 0);
static const uint16_t C_CYAN   = rgb(53, 184, 255);
static const uint16_t C_TEAL   = rgb(46, 216, 182);
static const uint16_t C_GREEN  = rgb(0, 210, 106);
static const uint16_t C_RED    = rgb(255, 59, 48);

// ---------------- Sensor helpers ----------------
static bool adsPresent() {
  Wire.beginTransmission(ADS_ADDR);
  return Wire.endTransmission() == 0;
}

static bool startAds() {
  if (!ads.begin(ADS_ADDR, &Wire)) return false;
  ads.setGain(GAIN_ONE);                       // +/-4.096 V at the ADC = 19.8 V at the terminal
  return true;
}

static float readVolts(uint8_t ch) {           // terminal volts
  float v = ads.computeVolts(ads.readADC_SingleEnded(ch)) / DIVIDER;
  return v < 0 ? 0 : v;
}

static const char* compass(float deg) {
  static const char* names[16] = {"N","NNE","NE","ENE","E","ESE","SE","SSE",
                                  "S","SSW","SW","WSW","W","WNW","NW","NNW"};
  return names[(int)((deg + 11.25f) / 22.5f) % 16];
}

// ---------------- Drawing helpers ----------------
static void panel(int y, int h, const char* label, uint16_t accent) {
  canvas.fillRoundRect(6, y, 228, h, 4, C_PANEL);
  canvas.drawRoundRect(6, y, 228, h, 4, C_BORDER);
  canvas.fillRect(6, y + 6, 4, h - 12, accent);
  canvas.setFont(&fonts::FreeSansBold9pt7b);
  canvas.setTextColor(C_DIM);
  canvas.drawString(label, 16, y + 5);
}

// Segmented horizontal gauge, frac 0..1
static void gauge(int x, int y, int w, int h, float frac, uint16_t col) {
  canvas.fillRect(x, y, w, h, C_TRACK);
  if (frac < 0) frac = 0;
  if (frac > 1) frac = 1;
  canvas.fillRect(x, y, (int)(frac * w), h, col);
  for (int i = 1; i < 20; i++) canvas.drawFastVLine(x + w * i / 20, y, h, C_PANEL);
  canvas.drawRect(x, y, w, h, C_BORDER);
}

// Big value with a small unit next to it
static void bigValue(int x, int y, const char* v, const char* unit, uint16_t col) {
  canvas.setFont(&fonts::FreeSansBold18pt7b);
  canvas.setTextColor(col);
  canvas.drawString(v, x, y);
  const int w = canvas.textWidth(v);
  canvas.setFont(&fonts::FreeSansBold9pt7b);
  canvas.setTextColor(C_DIM);
  canvas.drawString(unit, x + w + 6, y + 18);
}

static void dial(int cx, int cy, int r, bool ok, float deg) {
  const float D2R = 0.0174532925f;
  canvas.fillCircle(cx, cy, r, C_TRACK);
  canvas.drawCircle(cx, cy, r, C_BORDER);
  canvas.drawCircle(cx, cy, r - 1, C_BORDER);

  for (int a = 0; a < 360; a += 30) {          // tick marks
    const float s = sinf(a * D2R), c = cosf(a * D2R);
    canvas.drawLine(cx + (int)((r - 1) * s), cy - (int)((r - 1) * c),
                    cx + (int)((r - 6) * s), cy - (int)((r - 6) * c), C_DIM);
  }

  canvas.setFont(&fonts::FreeSansBold9pt7b);   // N E S W
  const char* lbl[4] = {"N", "E", "S", "W"};
  for (int i = 0; i < 4; i++) {
    const float a = i * 90 * D2R;
    const int lx = cx + (int)((r - 14) * sinf(a));
    const int ly = cy - (int)((r - 14) * cosf(a));
    canvas.setTextColor(i == 0 ? C_RED : C_DIM);
    canvas.drawString(lbl[i], lx - canvas.textWidth(lbl[i]) / 2, ly - canvas.fontHeight() / 2);
  }

  if (ok) {                                    // needle
    const float a = deg * D2R, s = sinf(a), c = cosf(a);
    const int tipX = cx + (int)(24 * s),  tipY = cy - (int)(24 * c);
    const int tlX  = cx - (int)(12 * s),  tlY  = cy + (int)(12 * c);
    const int b1X  = cx + (int)(6 * c),   b1Y  = cy + (int)(6 * s);
    const int b2X  = cx - (int)(6 * c),   b2Y  = cy - (int)(6 * s);
    canvas.fillTriangle(tipX, tipY, b1X, b1Y, b2X, b2Y, C_AMBER);
    canvas.fillTriangle(tlX,  tlY,  b1X, b1Y, b2X, b2Y, C_DIM);
  }
  canvas.fillCircle(cx, cy, 4, C_BG);
  canvas.drawCircle(cx, cy, 4, C_BORDER);
}

static void drawUI(ChState sW, float wind, ChState sD, float deg, ChState sH, float hum, bool beat) {
  char v[24];
  canvas.fillScreen(C_BG);

  // ----- header -----
  canvas.fillRect(0, 0, 240, 28, C_HDR);
  canvas.fillRect(0, 28, 240, 2, C_AMBER);
  canvas.setFont(&fonts::FreeSansBold9pt7b);
  canvas.setTextColor(C_TEXT);
  canvas.drawString("WEATHER STATION", 10, 4);
  const bool allOk = (sW == ST_OK && sD == ST_OK && sH == ST_OK);
  const uint16_t led = allOk ? (beat ? C_GREEN : rgb(0, 90, 45)) : C_RED;
  canvas.fillCircle(222, 14, 6, led);
  canvas.drawCircle(222, 14, 6, C_BORDER);

  // ----- wind speed -----
  panel(34, 84, "WIND SPEED", C_CYAN);
  if (sW == ST_OK) {
    const float f = wind / WIND_MAX_MS;
    snprintf(v, sizeof v, "%.1f", wind);
    bigValue(16, 54, v, "m/s", C_TEXT);
    gauge(16, 100, 208, 10, f, f < 0.5f ? C_GREEN : (f < 0.8f ? C_AMBER : C_RED));
  } else {
    bigValue(16, 54, sW == ST_NOADC ? "---" : "FAULT", sW == ST_NOADC ? "no X-AV4" : "check wiring", C_RED);
    gauge(16, 100, 208, 10, 0, C_RED);
  }

  // ----- wind direction -----
  panel(122, 108, "WIND DIRECTION", C_AMBER);
  dial(62, 184, 40, sD == ST_OK, deg);
  if (sD == ST_OK) {
    canvas.setFont(&fonts::FreeSansBold18pt7b);
    canvas.setTextColor(C_AMBER);
    canvas.drawString(compass(deg), 118, 150);
    snprintf(v, sizeof v, "%.0f", deg);
    bigValue(118, 190, v, "deg", C_TEXT);
  } else {
    bigValue(118, 170, sD == ST_NOADC ? "---" : "FAULT", "", C_RED);
  }

  // ----- humidity -----
  panel(234, 84, "HUMIDITY", C_TEAL);
  if (sH == ST_OK) {
    snprintf(v, sizeof v, "%.1f", hum);
    bigValue(16, 254, v, "%RH", C_TEXT);
    gauge(16, 300, 208, 10, hum / HUM_MAX_PCT, C_TEAL);
  } else {
    bigValue(16, 254, sH == ST_NOADC ? "---" : "FAULT", sH == ST_NOADC ? "no X-AV4" : "check wiring", C_RED);
    gauge(16, 300, 208, 10, 0, C_RED);
  }

  canvas.pushSprite(&tft, 0, 0);
}

// ---------------- Arduino ----------------
void setup() {
  Serial.begin(115200);
  Wire.begin(8, 9);
  Wire.setClock(400000);
  SPI.begin(12, 13, 11);                       // SCLK, MISO, MOSI
  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);

  canvas.setColorDepth(16);
  canvas.setPsram(true);
  canvasOk = canvas.createSprite(240, 320) != nullptr;
  if (!canvasOk) {
    Serial.println("Could not allocate display buffer (is PSRAM enabled?)");
    tft.setTextColor(TFT_RED);
    tft.drawString("Display buffer failed", 10, 10);
  }
  adsReady = startAds();
}

void loop() {
  static uint32_t last = 0;
  static bool beat = false;
  if (millis() - last < UPDATE_MS) return;
  last = millis();
  beat = !beat;

  if (!adsPresent()) {
    adsReady = false;
    Serial.println("X-AV4 not found");
    if (canvasOk) drawUI(ST_NOADC, 0, ST_NOADC, 0, ST_NOADC, 0, beat);
    return;
  }
  if (!adsReady) adsReady = startAds();
  if (!adsReady) return;

  // Average several readings. Direction is averaged as a vector so that
  // 359 deg and 1 deg give ~0 deg instead of 180 deg.
  float vHum = 0, vWind = 0, vDir = 0, sx = 0, sy = 0;
  for (int i = 0; i < SAMPLES; i++) {
    const float h = readVolts(0), w = readVolts(1), d = readVolts(2);
    vHum += h; vWind += w; vDir += d;
    const float a = d / 10.0f * 2.0f * (float)M_PI;
    sx += cosf(a); sy += sinf(a);
  }
  vHum /= SAMPLES; vWind /= SAMPLES; vDir /= SAMPLES;

  const ChState sH = vHum  <= OVER_RANGE_V ? ST_OK : ST_FAULT;
  const ChState sW = vWind <= OVER_RANGE_V ? ST_OK : ST_FAULT;
  const ChState sD = vDir  <= OVER_RANGE_V ? ST_OK : ST_FAULT;

  float deg = atan2f(sy, sx) * 180.0f / (float)M_PI;
  if (deg < 0) deg += 360.0f;
  const float wind = vWind / 10.0f * WIND_MAX_MS;
  const float hum  = vHum  / 10.0f * HUM_MAX_PCT;

  if (canvasOk) drawUI(sW, wind, sD, deg, sH, hum, beat);

  Serial.printf("wind=%.1f m/s  dir=%.0f deg %s  hum=%.1f %%\n", wind, deg, compass(deg), hum);
}
```

## How the conversion works
- The X-AV4 divides each 0–10 V input down so it fits the ADS1115 (3.3 V supply). The ADC runs at `GAIN_ONE` (±4.096 V, 125 µV per count) and the sketch undoes the divider: `V_terminal = V_adc / DIVIDER` (clamped at 0 V). With `DIVIDER` = 0.3125 (3.3k series, 1.5k to ground: 1.5 / (3.3 + 1.5)) a full-scale 10 V signal reaches the ADC as 3.125 V.
- Linear scaling: `value = V_terminal / 10 × span`, with spans of 100 %RH, 50 m/s and 360°. For an offset range such as 2–10 V use `min + (V − 2) / 8 × (max − min)`.
- Each channel is read `SAMPLES` (8) times per update. Each reading is a single-shot conversion at the library default rate (128 SPS, about 8 ms), so one update of three channels takes roughly 0.2 s.
- Direction is averaged as a vector: each reading becomes an angle `a = V / 10 × 2π`, the cosines and sines are summed, and `atan2(Σsin, Σcos)` gives the mean angle (add 360° if negative). The compass name comes from 16 sectors of 22.5°.
- Valid window: 0 to 10.5 V per channel. Above 10.5 V the channel is a fault. If the ADS1115 does not answer at `ADS_ADDR`, all channels show `---` and the sketch restarts the ADC when it answers again. A 0–10 V signal has no live zero, so an open signal wire reads as 0 V and is not detected.
- Reference values for commissioning (ideal values for `DIVIDER` = 0.3125, real modules vary with resistor tolerance):

| Terminal | ADC input | Expected count | Humidity | Wind speed | Direction |
|---|---|---|---|---|---|
| 0.0 V | 0.000 V | 0 | 0 %RH | 0 m/s | 0° |
| 2.5 V | 0.781 V | 6250 | 25 %RH | 12.5 m/s | 90° |
| 5.0 V | 1.563 V | 12500 | 50 %RH | 25 m/s | 180° |
| 7.5 V | 2.344 V | 18750 | 75 %RH | 37.5 m/s | 270° |
| 10.0 V | 3.125 V | 25000 | 100 %RH | 50 m/s | 360° |
| 10.5 V (limit) | 3.281 V | 26250 | FAULT | FAULT | FAULT |

- Resolution: one count is 0.4 mV at the terminal (125 µV ÷ 0.3125), which is 0.004 %RH, 0.002 m/s or 0.014°. This is finer than the transmitters themselves (±3 %rH, ±0.5 m/s or ±3 %, ±1.5°).
- Worked examples: 5.85 V = 58.5 %RH (count 14625); 3.42 V = 17.1 m/s (count 8550); 6.20 V = 223.2° = SW (count 15500).
- Calibrating `DIVIDER`: apply a known voltage (for example 5.00 V and 10.00 V) between an input and AGND, convert the displayed value back to volts, and set `DIVIDER (new) = DIVIDER (old) × displayed volts / applied volts`. Example: 10.00 V shows 9.70 V-equivalent, so the new value is 0.3125 × 0.97 = 0.3031. A correct setting shows 25.0 m/s at 5.00 V on the wind speed input.

## Display states
| State | Condition | Display | Header LED |
|---|---|---|---|
| Normal | All channels 0 to 10.5 V | Values, gauges and compass needle | Green, blinking each update |
| Wind speed zone | Gauge colour | Green below 25 m/s, amber 25 to 40 m/s, red from 40 m/s | Unchanged |
| Channel fault | A channel above 10.5 V | `FAULT` and `check wiring` in red, gauge empty | Red |
| X-AV4 offline | ADS1115 does not answer | `---` and `no X-AV4` on all channels | Red |

## Expected Output
On the Serial Monitor (115200 baud):
- One line every 500 ms, for example `wind=17.1 m/s  dir=223 deg SW  hum=58.5 %`
- `X-AV4 not found` every 500 ms if the module does not answer (check DIP address and seating)

On the TFT display: "WEATHER STATION" header with a status LED, a WIND SPEED panel with value in m/s and a segmented gauge, a WIND DIRECTION panel with a compass dial, 16-point name and angle, and a HUMIDITY panel with value in %RH and a teal gauge.

## Commissioning checklist
- Power: 24 V DC stable and correctly wired to the X-CPU and to the three transmitters before energizing.
- I2C: DIP 1 ON (0x49) and no `X-AV4 not found` message. An I2C scan should list 0x49, 0x41, 0x15, 0x68 and 0x75.
- Voltage injection: with the transmitters disconnected, apply 0, 2.5, 5, 7.5 and 10 V between each input and AGND and compare with the reference table. Do not exceed 10 V.
- Divider calibration: if the error is a constant percentage, set `DIVIDER` as described above.
- Wind direction: turn the vane to N, E, S and W and compare with a compass. Align the north mark as the installation instructions describe (the sketch has no direction offset).
- Wind speed: spin the cups by hand; the value rises and returns to about 0 m/s when they stop.
- Humidity: the voltage at AIN1 should equal %RH ÷ 10 (58.5 %RH is 5.85 V); compare with a reference hygrometer in the same air.
- Fault response: power off, remove the X-AV4, power on and confirm `---` and `no X-AV4` with a red LED; power off, reseat the module and power on to confirm normal operation.

## Common Issues
| Symptom | Likely Cause | Fix |
|---|---|---|
| Display blank or white | Wrong SPI pins, display bus not initialized, buffer not allocated, or wrong Tools settings | Confirm CS 45, DC 46, RST 47 in the LGFX class; set USB CDC On Boot, Flash Size and PSRAM as in the table above |
| Display flickers or shows noise | Ethernet or SD CS lines floating on the shared SPI bus | In `setup()` before `tft.init()`, `pinMode(1, OUTPUT)` and `pinMode(42, OUTPUT)`, then drive both HIGH |
| `---` / `no X-AV4` on all channels | X-AV4 not seated, or DIP address does not match `ADS_ADDR` | Power off and reseat the X-AV4; set DIP 1 ON for 0x49; run an I2C scan (expect 0x49) |
| `FAULT` on one channel | More than 10.5 V on that input: wrong sensor variant, supply voltage on the signal wire, or a faulty transmitter | Power off and check the wiring; use the 0–10 V variants (-10 or -40) and measure the terminal voltage |
| Reading stuck at zero | Signal wire open, transmitter unpowered, or sensor 0 V not tied to AGND | Check the supply at the transmitter and the voltage between AIN and AGND |
| All values wrong by the same percentage | `DIVIDER` differs from the module | Calibrate with a known voltage |
| Humidity low at high humidity | Output loaded by the X-AV4 input (the DSTHx-3 datasheet specifies a 50 kΩ minimum load), or wrong output mode or range | Check 0–10 V mode and the 0–100 %RH range; compare the AIN1 voltage with the transmitter output; add a 0–10 V buffer if needed |
| Noisy or erratic values | Cable pickup, unshielded cable or a ground loop | Use shielded twisted pair, ground the shield at one end, keep away from motor/VFD/relay wiring, raise `SAMPLES` |
| Direction wrong by a fixed angle | North mark of the vane not aligned | Rotate the mounting, or add an offset to the angle in the sketch |
| Reset or supply sag when heaters switch on | Supply too small for the sensor heaters | Size the supply for up to 20 W per heater, or use a separate heater supply |

## Possible extensions
- Read the fourth transmitter on AIN4 with `readVolts(3)` and your own span.
- Add a slower exponential filter (`x = x + alpha × (new − x)`) or 10-minute mean and gust values; for direction, filter the sine and cosine sums, not the angle.
- Add a low-limit check on humidity (for example below 1 %RH) to flag an open signal wire.
- Publish readings over MQTT or Modbus, log to the SD card, or switch a relay output when wind speed passes a limit.

