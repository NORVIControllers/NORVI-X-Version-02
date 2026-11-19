#include <Wire.h>
#include <SPI.h>
#include <CST816S.h>

#define LGFX_USE_V1
#include <LovyanGFX.hpp>

// ============================================================
// NORVI X1 + EX-AQ4 | 0-10V ANALOG OUTPUT
// ESP32-S3 | ST7789 | CST816S
// ============================================================

// ---------------- HARDWARE ----------------
#define AQ4_ADDR 0x5F
#define SDA_PIN  8
#define SCL_PIN  9

// ---------------- TOUCHSCREEN ----------------
CST816S touch(8, 9, 47, 48);

bool isTouched = false;
unsigned long lastTouchTime = 0;
int lastPressedButton = 0;

// Debounce / latch / single-shot
const unsigned long TOUCH_LATCH_MS   = 350;
const unsigned long PRESS_VISUAL_MS  = 220;
unsigned long lastReleaseTime = 0;
bool buttonLatched   = false;
bool pressConsumed   = false;

// ---------------- VOLTAGE CONTROL ----------------
float targetVolts = 0.0;

const float MIN_VOLTAGE = 0.0;
const float MAX_VOLTAGE = 10.0;
const float VOLTAGE_STEP = 0.1;

bool dacOK = false;
bool screenNeedsUpdate = true;

// ---------------- COLOR PALETTE (NORVI) ----------------
#define TFT_BG          0x0000
#define TFT_HEADER      0x0000
#define TFT_PANEL       0x18E3
#define TFT_GREEN       0x07E0
#define TFT_GREEN_DIM   0x03E0
#define TFT_WHITE       0xFFFF
#define TFT_GREY        0x8410
#define TFT_RED         0xF800
#define TFT_YELLOW      0xFFE0
#define TFT_BLACK       0x0000

#define TFT_PATTERN_DOT 0x0320

// ---------------- LAYOUT ----------------
constexpr int HDR_H     = 62;
constexpr int CARD_X    = 20;
constexpr int CARD_Y    = 90;
constexpr int CARD_W    = 200;
constexpr int CARD_H    = 118;

constexpr int BTN_W     = 86;
constexpr int BTN_H     = 58;
constexpr int BTN_GAP   = 12;
constexpr int BTN_Y     = 226;
constexpr int BTN_TOTAL = BTN_W * 2 + BTN_GAP;               // 184
constexpr int BTN_LEFT  = 120 - BTN_TOTAL / 2;               // 28
constexpr int BTN_RIGHT = BTN_LEFT + BTN_W + BTN_GAP;        // 126

// ============================================================
// LOVYANGFX DISPLAY CONFIGURATION
// ============================================================

class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ST7789 _panel;
  lgfx::Bus_SPI _bus;

public:
  LGFX(void) {
    {
      auto c = _bus.config();
      c.spi_host = SPI2_HOST;
      c.spi_mode = 0;
      c.freq_write = 27000000;
      c.freq_read = 16000000;
      c.spi_3wire = false;
      c.use_lock = true;
      c.dma_channel = SPI_DMA_CH_AUTO;
      c.pin_sclk = 12;
      c.pin_mosi = 11;
      c.pin_miso = 13;
      c.pin_dc = 46;
      _bus.config(c);
      _panel.setBus(&_bus);
    }
    {
      auto c = _panel.config();
      c.pin_cs = 45;
      c.pin_rst = 47;
      c.pin_busy = -1;
      c.panel_width = 240;
      c.panel_height = 320;
      c.offset_x = 0;
      c.offset_y = 0;
      c.offset_rotation = 0;
      c.readable = false;
      c.invert = true;
      c.rgb_order = false;
      c.dlen_16bit = false;
      c.bus_shared = true;
      _panel.config(c);
    }
    setPanel(&_panel);
  }
};

LGFX tft;

// ============================================================
// DAC
// ============================================================

void config_dac(byte device_address, unsigned int c1, unsigned int c2, unsigned int c3, unsigned int c4) {
  unsigned int tft_cfg = 0;
  tft_cfg |= (c4 << 3);
  tft_cfg |= (c3 << 2);
  tft_cfg |= (c2 << 1);
  tft_cfg |= c1;

  Wire.beginTransmission(device_address);
  Wire.write(0x01);
  Wire.write(tft_cfg);
  Wire.endTransmission();
}

bool write_channel(byte device_address, unsigned int channel, unsigned int value) {
  Wire.beginTransmission(device_address);
  Wire.write(1 + (channel * 2));
  Wire.write(value >> 8);
  Wire.write(value & 0xFF);
  byte err = Wire.endTransmission();

  if (err != 0) {
    Serial.print("I2C Write Error: ");
    Serial.println(err);
    return false;
  }
  return true;
}

uint16_t voltageToDAC(float v) {
  float d = (v / 10.0f) * 8191.0f;
  if (d < 0) d = 0;
  if (d > 8191) d = 8191;
  return (uint16_t)d;
}

void applyVoltage() {
  targetVolts = constrain(targetVolts, MIN_VOLTAGE, MAX_VOLTAGE);
  uint16_t dacValue = voltageToDAC(targetVolts);

  Serial.print("Output: ");
  Serial.print(targetVolts, 2);
  Serial.print(" V | DAC: ");
  Serial.println(dacValue);

  dacOK = write_channel(AQ4_ADDR, 1, dacValue);
  if (!dacOK) Serial.println("I2C Write Failed!");

  screenNeedsUpdate = true;
}

void changeVoltage(float amount) {
  float nv = targetVolts + amount;
  nv = roundf(nv * 10.0f) / 10.0f;
  nv = constrain(nv, MIN_VOLTAGE, MAX_VOLTAGE);
  if (nv != targetVolts) {
    targetVolts = nv;
    applyVoltage();
  }
}

// ============================================================
// DISPLAY INIT
// ============================================================

void displayInit() {
  Serial.println("Initializing TFT...");
  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BG);
  Serial.println("TFT ready.");
}

// ============================================================
// TEXT HELPERS
// ============================================================

void drawCenteredText(const char* text, int centerX, int y,
                      const lgfx::IFont* font, uint16_t colour, uint16_t bg) {
  tft.setFont(font);
  tft.setTextColor(colour, bg);
  int x = centerX - tft.textWidth(text) / 2;
  tft.setCursor(x, y);
  tft.print(text);
}

// ============================================================
// RGB565 BLEND
// ============================================================

uint16_t blend565(uint16_t c1, uint16_t c2, float t) {
  t = constrain(t, 0.0f, 1.0f);
  int r1 = (c1 >> 11) & 0x1F, g1 = (c1 >> 5) & 0x3F, b1 = c1 & 0x1F;
  int r2 = (c2 >> 11) & 0x1F, g2 = (c2 >> 5) & 0x3F, b2 = c2 & 0x1F;
  int r = r1 + (int)((r2 - r1) * t);
  int g = g1 + (int)((g2 - g1) * t);
  int b = b1 + (int)((b2 - b1) * t);
  return ((uint16_t)r << 11) | ((uint16_t)g << 5) | (uint16_t)b;
}

// ============================================================
// BACKGROUND PATTERN
// ============================================================

void drawBackgroundPattern() {
  const int spacing = 16;
  const int r       = 1;
  const int yStart  = HDR_H;

  for (int y = yStart; y < 320; y += spacing) {
    for (int x = 8; x < 240; x += spacing) {
      tft.fillCircle(x, y, r, TFT_PATTERN_DOT);
    }
  }
}

// ============================================================
// HEADER
// ============================================================

constexpr int DOT_X = 216;
constexpr int DOT_Y = 16;
constexpr int DOT_R = 6;

uint16_t lastDotColor = 0xFFFF;
unsigned long lastBlinkMs = 0;
const unsigned long BLINK_PERIOD_MS = 1200;

void drawHeader() {
  tft.fillRect(0, 0, 240, HDR_H, TFT_HEADER);

  tft.setFont(&fonts::FreeSansBold12pt7b);
  tft.setTextColor(TFT_WHITE, TFT_HEADER);
  int titleW = tft.textWidth("NORVI X1");
  tft.setCursor((240 - titleW) / 2, 8);
  tft.print("NORVI X1");

  tft.setFont(&fonts::FreeSans9pt7b);
  tft.setTextColor(TFT_GREEN, TFT_HEADER);
  int subW = tft.textWidth("EX-AQ4  |  0-10V OUTPUT");
  tft.setCursor((240 - subW) / 2, 38);
  tft.print("EX-AQ4  |  0-10V OUTPUT");

  tft.fillCircle(DOT_X, DOT_Y, DOT_R, TFT_GREEN);
  tft.drawCircle(DOT_X, DOT_Y, DOT_R + 1, TFT_WHITE);
  lastDotColor = TFT_GREEN;
}

void updateStatusDot() {
  unsigned long now = millis();
  if (now - lastBlinkMs < 40) return;
  lastBlinkMs = now;

  float phase = (float)(now % BLINK_PERIOD_MS) / (float)BLINK_PERIOD_MS;
  float tri = phase < 0.5f ? phase * 2.0f : (1.0f - phase) * 2.0f;
  float amt = tri * tri * (3.0f - 2.0f * tri);

  uint16_t base = dacOK ? TFT_GREEN : TFT_RED;
  uint16_t col  = blend565(TFT_HEADER, base, 0.25f + 0.75f * amt);

  if (col != lastDotColor) {
    lastDotColor = col;
    tft.fillCircle(DOT_X, DOT_Y, DOT_R, col);
    tft.drawCircle(DOT_X, DOT_Y, DOT_R + 1, TFT_WHITE);
  }
}

// ============================================================
// BUTTON
// ============================================================

void drawButton(int x, int y, int w, int h, const char* symbol, bool pressed) {
  tft.fillRoundRect(x, y, w, h, 8, TFT_PANEL);

  if (pressed) {
    tft.drawRoundRect(x,     y,     w,     h,     8, TFT_GREEN);
    tft.drawRoundRect(x + 1, y + 1, w - 2, h - 2, 7, TFT_GREEN);
    tft.drawRoundRect(x + 4, y + 4, w - 8, h - 8, 5, TFT_GREEN_DIM);
  } else {
    tft.drawRoundRect(x, y, w, h, 8, TFT_GREY);
  }

  tft.setFont(&fonts::FreeSansBold24pt7b);
  tft.setTextColor(pressed ? TFT_GREEN : TFT_WHITE, TFT_PANEL);
  int symX = x + (w - tft.textWidth(symbol)) / 2;
  int symY = y + (h - tft.fontHeight()) / 2 - 2;
  tft.setCursor(symX, symY);
  tft.print(symbol);
}

// ============================================================
// READOUT PANEL
// ============================================================

void drawReadoutPanel() {
  tft.fillRoundRect(CARD_X, CARD_Y, CARD_W, CARD_H, 8, TFT_PANEL);
  tft.drawRoundRect(CARD_X, CARD_Y, CARD_W, CARD_H, 8, TFT_GREY);
  // REMOVED: tft.fillRect(CARD_X, CARD_Y, CARD_W, 3, TFT_GREEN);

  drawCenteredText("AOUT1:", CARD_X + CARD_W / 2, CARD_Y + 10,
                   &fonts::FreeSans9pt7b, TFT_GREY, TFT_PANEL);
}

// ============================================================
// DYNAMIC UPDATE
// ============================================================

void updateDynamicDisplay() {
  // Value band (inside card)
  tft.fillRect(CARD_X + 4, CARD_Y + 34, CARD_W - 8, 64, TFT_PANEL);

  char voltageText[16];
  snprintf(voltageText, sizeof(voltageText), "%.2f", targetVolts);

  char fullText[20];
  snprintf(fullText, sizeof(fullText), "%s V", voltageText);

  tft.setFont(&fonts::FreeSansBold24pt7b);
  tft.setTextColor(TFT_YELLOW, TFT_PANEL);
  int x = 120 - tft.textWidth(fullText) / 2;
  tft.setCursor(x, CARD_Y + 42);
  tft.print(fullText);

  // Buttons — centered under card
  drawButton(BTN_LEFT,  BTN_Y, BTN_W, BTN_H, "-", lastPressedButton == 1);
  drawButton(BTN_RIGHT, BTN_Y, BTN_W, BTN_H, "+", lastPressedButton == 2);

  screenNeedsUpdate = false;
}

// ============================================================
// MAIN UI
// ============================================================

void drawMainUI() {
  tft.fillScreen(TFT_BG);
  drawBackgroundPattern();
  drawHeader();
  drawReadoutPanel();
  screenNeedsUpdate = true;
  updateDynamicDisplay();
}

// ============================================================
// TOUCH — single-shot per press
// ============================================================

void touchTask() {
  unsigned long now = millis();

  if (!touch.available()) {
    if (isTouched) {
      isTouched = false;
      lastReleaseTime = now;
      buttonLatched = false;
      pressConsumed = false;
    }
    if (lastPressedButton != 0 && (now - lastTouchTime) >= PRESS_VISUAL_MS) {
      lastPressedButton = 0;
      screenNeedsUpdate = true;
    }
    return;
  }

  int x = touch.data.x;
  int y = touch.data.y;

  if (!isTouched) {
    isTouched = true;
    lastTouchTime = now;

    if (pressConsumed) return;
    if (buttonLatched && (now - lastReleaseTime) < TOUCH_LATCH_MS) {
      pressConsumed = true;
      return;
    }

    bool fired = false;

    // Hit zones padded by 6 px around BTN_LEFT / BTN_RIGHT / BTN_Y
    if (x >= BTN_LEFT - 6 && x <= BTN_LEFT + BTN_W + 6 &&
        y >= BTN_Y - 8    && y <= BTN_Y + BTN_H + 8) {
      lastPressedButton = 1;
      changeVoltage(-VOLTAGE_STEP);
      fired = true;
    }
    else if (x >= BTN_RIGHT - 6 && x <= BTN_RIGHT + BTN_W + 6 &&
             y >= BTN_Y - 8     && y <= BTN_Y + BTN_H + 8) {
      lastPressedButton = 2;
      changeVoltage(VOLTAGE_STEP);
      fired = true;
    }

    if (fired) {
      buttonLatched = true;
      pressConsumed = true;
    }
  }

  if (screenNeedsUpdate) updateDynamicDisplay();
}

// ============================================================
// SERIAL
// ============================================================

void serialTask() {
  if (Serial.available() <= 0) return;
  String s = Serial.readStringUntil('\n');
  s.trim();
  if (s.length() == 0) return;

  float nv = s.toFloat();
  nv = constrain(nv, MIN_VOLTAGE, MAX_VOLTAGE);
  nv = roundf(nv * 10.0f) / 10.0f;
  targetVolts = nv;
  applyVoltage();
  Serial.println("Enter voltage (0-10 V):");
}

// ============================================================
// SETUP
// ============================================================

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(30);
  delay(2000);

  Serial.println();
  Serial.println("NORVI X1 | EX-AQ4 VOLTAGE CONTROLLER");

  Wire.begin(SDA_PIN, SCL_PIN);
  delay(100);

  Serial.println("Initializing touchscreen...");
  touch.begin();

  displayInit();

  Serial.println("Configuring EX-AQ4...");
  config_dac(AQ4_ADDR, 0, 0, 1, 1);

  targetVolts = 0.0;
  dacOK = write_channel(AQ4_ADDR, 1, 0);

  if (dacOK) Serial.println("AOUT1 initialized to 0 V.");
  else      Serial.println("DAC initialization failed!");

  drawMainUI();

  Serial.println("Controller ready.");
  Serial.println("Touch +/- to adjust by 0.1 V.");
  Serial.println("Serial input: 0 to 10 V.");
}

// ============================================================
// MAIN LOOP
// ============================================================

void loop() {
  touchTask();
  serialTask();
  updateStatusDot();

  unsigned long now = millis();
  if (lastPressedButton != 0 && (now - lastTouchTime) >= PRESS_VISUAL_MS) {
    lastPressedButton = 0;
    screenNeedsUpdate = true;
  }

  if (screenNeedsUpdate) updateDynamicDisplay();

  delay(5);
}