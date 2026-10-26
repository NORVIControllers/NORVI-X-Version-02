//"NPD-NORVI X-AQ4"
// CH1 / CH2 : 0-10V   (voltage outputs)
// CH3 / CH4 : 0-20mA  (current outputs)

#include <Wire.h>
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

#define PORT_RST 38

// ================= AQ4 slave (DAC) I2C target =================
#define DAC_ADDR 0x5F

CST816S touch(8, 9, 47, 48);   // sda, scl, rst, irq

// =========================================================
// Channel model
// =========================================================
struct Channel {
  const char* label;
  float value;
  float minV;
  float maxV;
  float step;
  const char* unit;
  int   ioChannel;   // 1..4, matches AQ4 register map
};

Channel channels[4] = {
  { "CH1", 0.0, 0.0, 10.0, 0.5, "V",  1 },
  { "CH2", 0.0, 0.0, 10.0, 0.5, "V",  2 },
  { "CH3", 0.0, 0.0, 20.0, 0.5, "mA", 3 },
  { "CH4", 0.0, 0.0, 20.0, 0.5, "mA", 4 },
};

int activeCh = 0;
String lastStatus = "";
uint16_t statusColor = TFT_CYAN;   // cyan = OK, red = I2C failure

int16_t W, H;

struct Rect { int16_t x, y, w, h; };

Rect chBtn[4];
Rect btnMinus, btnPlus;
Rect btnSend;

int valueAreaTop, valueAreaH;
int statusAreaTop, statusAreaH;

unsigned long lastTouchMs = 0;
const unsigned long TOUCH_DEBOUNCE_MS = 220;

uint8_t i2cFailCount = 0;

// =========================================================
void setup() {
  Serial.begin(115200);
  delay(300);

  Wire.begin(SDA, SCL);   // single shared bus: touch IC + AQ4 slave
  delay(100);

  touch.begin();

  pinMode(PORT_RST, OUTPUT);
  digitalWrite(PORT_RST, HIGH);
  delay(100);

  SPI.begin(SCLK, MISO, MOSI);
  delay(200);

  config_dac(DAC_ADDR, 0, 0, 1, 1);   // CH1/2 = voltage, CH3/4 = current

  tft.init();
  tft.setRotation(0);
  tft.setTextDatum(lgfx::textdatum_t::baseline_left);   // cursor Y = text baseline, same layout as before
  W = tft.width();
  H = tft.height();

  tft.fillScreen(TFT_BLACK);

  buildLayout();
  drawScreen();
}

// =========================================================
void loop() {
  if (touch.available()) {
    unsigned long now = millis();
    if (now - lastTouchMs > TOUCH_DEBOUNCE_MS) {
      lastTouchMs = now;
      handleTouch(touch.data.x, touch.data.y);
    }
  }
}

// =========================================================
// I2C BUS RECOVERY — same pattern as the old AQ4 master
// =========================================================
void i2cBusRecover() {
  Wire.end();
  pinMode(SCL, OUTPUT);
  pinMode(SDA, INPUT_PULLUP);
  for (int i = 0; i < 9; i++) {
    digitalWrite(SCL, HIGH); delayMicroseconds(5);
    digitalWrite(SCL, LOW);  delayMicroseconds(5);
    if (digitalRead(SDA) == HIGH) break;
  }
  pinMode(SDA, OUTPUT);
  digitalWrite(SDA, LOW);  delayMicroseconds(5);
  digitalWrite(SCL, HIGH); delayMicroseconds(5);
  digitalWrite(SDA, HIGH); delayMicroseconds(5);
  Wire.begin(SDA, SCL);
  Serial.println("I2C bus recovered");
}

// =========================================================
// WRITE DAC CHANNEL — identical register map to the old master
// =========================================================
bool write_channel(byte device_address, unsigned int channel, unsigned int value) {
  Wire.beginTransmission(device_address);
  Wire.write(1 + (channel * 2));
  Wire.write(value >> 8);
  Wire.write(value & 0xFF);
  byte err = Wire.endTransmission();

  if (err != 0) {
    Serial.print("I2C TX FAIL err="); Serial.println(err);
    if (++i2cFailCount >= 2) { i2cBusRecover(); i2cFailCount = 0; }
    return false;
  }

  i2cFailCount = 0;
  return true;
}

void config_dac(byte device_address, unsigned int c1, unsigned int c2, unsigned int c3, unsigned int c4) {
  unsigned int tft_cfg = 0;
  tft_cfg |= (c4 << 3);
  tft_cfg |= (c3 << 2);
  tft_cfg |= (c2 << 1);
  tft_cfg |= c1;

  Wire.beginTransmission(device_address);
  Wire.write(0x01);
  Wire.write(tft_cfg);
  byte err = Wire.endTransmission();

  if (err != 0) {
    Serial.print("I2C CONFIG FAIL err="); Serial.println(err);
    if (++i2cFailCount >= 2) { i2cBusRecover(); i2cFailCount = 0; }
  } else {
    i2cFailCount = 0;
  }
}

// =========================================================
// Build every rectangle top-to-bottom
// =========================================================
void buildLayout() {
  int margin = 8;

  int chW = (W - margin * 5) / 4;
  int chY = 38;
  int chH = 46;

  for (int i = 0; i < 4; i++) {
    chBtn[i].x = margin + i * (chW + margin);
    chBtn[i].y = chY;
    chBtn[i].w = chW;
    chBtn[i].h = chH;
  }

  valueAreaTop = chY + chH + 8;
  valueAreaH   = 80;

  int stepY = valueAreaTop + valueAreaH + 6;
  int stepH = 44;
  int smallW = (W - margin * 3) / 2;

  btnMinus.x = margin;
  btnMinus.y = stepY;
  btnMinus.w = smallW;
  btnMinus.h = stepH;

  btnPlus.x = margin * 2 + smallW;
  btnPlus.y = stepY;
  btnPlus.w = smallW;
  btnPlus.h = stepH;

  int sendY = stepY + stepH + 8;
  int sendH = 50;
  btnSend.x = margin;
  btnSend.y = sendY;
  btnSend.w = W - margin * 2;
  btnSend.h = sendH;

  statusAreaTop = sendY + sendH + 8;
  statusAreaH   = 24;

  Serial.print("Layout fits, bottom margin = ");
  Serial.println(H - (statusAreaTop + statusAreaH));
}

bool inRect(int16_t x, int16_t y, Rect r) {
  return (x >= r.x && x <= r.x + r.w && y >= r.y && y <= r.y + r.h);
}

// =========================================================
void handleTouch(int16_t x, int16_t y) {
  for (int i = 0; i < 4; i++) {
    if (inRect(x, y, chBtn[i])) {
      activeCh = i;
      drawScreen();
      return;
    }
  }

  Channel &c = channels[activeCh];

  if (inRect(x, y, btnMinus)) { adjustValue(-c.step); return; }
  if (inRect(x, y, btnPlus))  { adjustValue(+c.step); return; }
  if (inRect(x, y, btnSend))  { sendActiveChannel();   return; }
}

void adjustValue(float delta) {
  Channel &c = channels[activeCh];
  c.value += delta;
  if (c.value < c.minV) c.value = c.minV;
  if (c.value > c.maxV) c.value = c.maxV;
  drawValueArea();
}

// =========================================================
// Send the active channel's value straight to the AQ4 slave over I2C
// (same scaling the old master used: 0-10V / 0-20mA -> 13-bit DAC code)
// =========================================================
void sendActiveChannel() {
  Channel &c = channels[activeCh];

  float fullScale = (c.ioChannel <= 2) ? 11.0 : 25.0;   // AD74412R hardware full-scale: 11V / 25mA
  uint16_t dac_value = (uint16_t)((c.value / fullScale) * 8191);
  bool ok = write_channel(DAC_ADDR, c.ioChannel, dac_value);

  if (ok) {
    lastStatus  = "Sent " + String(c.label) + " = " + String(c.value, 2) + " " + c.unit;
    statusColor = TFT_CYAN;
  } else {
    lastStatus  = "I2C FAIL - not sent";
    statusColor = TFT_RED;
  }
  Serial.print(c.label); Serial.print(" = "); Serial.print(c.value);
  Serial.print(" "); Serial.print(c.unit); Serial.print(" | DAC Code = "); Serial.println(dac_value);

  drawStatusArea();
}

// =========================================================
// Drawing
// =========================================================
void drawScreen() {
  tft.fillScreen(TFT_BLACK);

  tft.setTextColor(TFT_YELLOW);
  tft.setFont(&fonts::FreeSansBold12pt7b);
  tft.setCursor(8, 20);
  tft.print("NORVI-X-AQ4");

  drawChannelButtons();
  drawValueArea();
  drawStepButtons();
  drawSendButton();
  drawStatusArea();
}

void drawChannelButtons() {
  for (int i = 0; i < 4; i++) {
    bool active = (i == activeCh);
    uint16_t fill = active ? TFT_BLUE : TFT_DARKGREY;
    tft.fillRoundRect(chBtn[i].x, chBtn[i].y, chBtn[i].w, chBtn[i].h, 6, fill);
    tft.drawRoundRect(chBtn[i].x, chBtn[i].y, chBtn[i].w, chBtn[i].h, 6, TFT_WHITE);

    tft.setTextColor(TFT_WHITE);
    tft.setFont(&fonts::FreeSansBold9pt7b);
    int16_t tw = tft.textWidth(channels[i].label);
    tft.setCursor(chBtn[i].x + (chBtn[i].w - tw) / 2, chBtn[i].y + chBtn[i].h / 2 + 6);
    tft.print(channels[i].label);
  }
}

void drawValueArea() {
  tft.fillRect(0, valueAreaTop, W, valueAreaH, TFT_BLACK);

  Channel &c = channels[activeCh];

  char buf[24];
  snprintf(buf, sizeof(buf), "%.2f %s", c.value, c.unit);

  tft.setTextColor(TFT_GREEN);
  tft.setFont(&fonts::FreeSansBold24pt7b);
  int16_t tw = tft.textWidth(buf);
  tft.setCursor((W - tw) / 2, valueAreaTop + 38);
  tft.print(buf);

  char rangeBuf[32];
  snprintf(rangeBuf, sizeof(rangeBuf), "Range: %.1f - %.1f %s", c.minV, c.maxV, c.unit);
  tft.setTextColor(TFT_LIGHTGREY);
  tft.setFont(&fonts::FreeSans9pt7b);
  tw = tft.textWidth(rangeBuf);
  tft.setCursor((W - tw) / 2, valueAreaTop + 68);
  tft.print(rangeBuf);
}

void drawStepButtons() {
  tft.fillRoundRect(btnMinus.x, btnMinus.y, btnMinus.w, btnMinus.h, 6, TFT_RED);
  tft.drawRoundRect(btnMinus.x, btnMinus.y, btnMinus.w, btnMinus.h, 6, TFT_WHITE);
  tft.fillRoundRect(btnPlus.x, btnPlus.y, btnPlus.w, btnPlus.h, 6, TFT_GREEN);
  tft.drawRoundRect(btnPlus.x, btnPlus.y, btnPlus.w, btnPlus.h, 6, TFT_WHITE);

  tft.setTextColor(TFT_WHITE);
  tft.setFont(&fonts::FreeSansBold18pt7b);
  centerText("-", btnMinus);
  centerText("+", btnPlus);
}

void drawSendButton() {
  tft.fillRoundRect(btnSend.x, btnSend.y, btnSend.w, btnSend.h, 8, TFT_ORANGE);
  tft.drawRoundRect(btnSend.x, btnSend.y, btnSend.w, btnSend.h, 8, TFT_WHITE);
  tft.setTextColor(TFT_BLACK);
  tft.setFont(&fonts::FreeSansBold12pt7b);
  centerText("SEND", btnSend);
}

void drawStatusArea() {
  tft.fillRect(0, statusAreaTop, W, statusAreaH, TFT_BLACK);
  tft.setTextColor(statusColor);
  tft.setFont(&fonts::FreeSans9pt7b);
  tft.setCursor(8, statusAreaTop + 16);
  tft.print(lastStatus);
}

void centerText(const char* txt, Rect r) {
  int16_t tw = tft.textWidth(txt);
  tft.setCursor(r.x + (r.w - tw) / 2, r.y + r.h / 2 + 5);
  tft.print(txt);
}
