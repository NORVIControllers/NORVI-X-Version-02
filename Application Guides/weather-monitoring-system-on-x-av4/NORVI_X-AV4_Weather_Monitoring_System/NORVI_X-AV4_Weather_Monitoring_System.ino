/*
  NORVI X1 + X-AV4  -  Industrial weather display

    X-AV4 AIN1 -> humidity(DSTHX-3)        (0-10 V = 0-100 %RH)
    X-AV4 AIN2 -> wind speed(WG2/O-10)      (0-10 V = 0-WIND_MAX_MS)
    X-AV4 AIN3 -> wind direction(WRG/O-40)  (0-10 V = 0-360 deg)

*/

#include <Wire.h>
#include <SPI.h>
#include <math.h>
#include <Adafruit_ADS1X15.h>
#define LGFX_USE_V1
#include <LovyanGFX.hpp>

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
  ads.setGain(GAIN_ONE);                      
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
