//"NPD-NORVI X-R4" — clean layout, no progress bar, enlarged header
//2025.10.31

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
    { auto c = _bus.config();
      c.spi_host    = SPI2_HOST;
      c.spi_mode    = 0;
      c.freq_write  = 27000000;
      c.freq_read   = 16000000;
      c.spi_3wire   = false;
      c.use_lock    = true;
      c.dma_channel = SPI_DMA_CH_AUTO;
      c.pin_sclk    = 12;
      c.pin_mosi    = 11;
      c.pin_miso    = 13;
      c.pin_dc      = 46;
      _bus.config(c);
      _panel.setBus(&_bus);
    }
    { auto c = _panel.config();
      c.pin_cs   = 45;
      c.pin_rst  = 47;
      c.pin_busy = -1;
      c.panel_width  = 240;
      c.panel_height = 320;
      c.offset_x = 0;
      c.offset_y = 0;
      c.offset_rotation = 0;
      c.readable   = false;
      c.invert     = true;
      c.rgb_order  = false;
      c.dlen_16bit = false;
      c.bus_shared = true;
      _panel.config(c);
    }
    setPanel(&_panel);
  }
};
LGFX tft;

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

// ===================== Hardware =====================
#define PCA9538_ADDR 0x73
#define SDA   8
#define SCL   9
#define MISO 13
#define MOSI 11
#define SCLK 12
#define DSP_CS 45
#define PCA_RESET 21

const int relayPins[] = {1, 2, 3, 4};
PCA9538 io(PCA9538_ADDR);

// ===================== Screen layout (no progress bar) =====================
// y=  0..49  header band (green, larger title 12pt)
// y= 50      divider
// y= 56..80  phase name
// y= 88..272 4 relay rows (46 px pitch, 42 px tall)
// y=282..312 binary value (cyan, 12pt)

struct RowUI {
  int16_t boxX, boxY, boxW, boxH;
  bool    lastState;
  bool    lastValid;
};

RowUI rowUI[4];

int cycleCount = 0;

// ===================== Setup =====================
void setup() {
  Serial.begin(115200);
  delay(1500);
  Serial.println("\n=== NPD-NORVI X-R4 RELAY TEST ===");

  pinMode(PCA_RESET, OUTPUT);
  digitalWrite(PCA_RESET, HIGH);
  delay(100);

  Wire.begin(SDA, SCL);
  delay(100);

  I2C_SCAN();
  delay(500);

  for (int i = 0; i < 4; i++) {
    io.pinMode(relayPins[i], OUTPUT);
    io.digitalWrite(relayPins[i], LOW);
  }

  SPI.begin(SCLK, MISO, MOSI);
  delay(500);

  tft.init();
  tft.setRotation(0);
  tft.setTextDatum(lgfx::textdatum_t::baseline_left);

  drawStaticLayout();
  refreshRelayRows();

  // Quick self-check blip 1→4
  Serial.println("Self-check 1→4 ...");
  for (int i = 0; i < 4; i++) {
    io.digitalWrite(relayPins[i], HIGH);
    refreshRelayRows();
    delay(200);
    io.digitalWrite(relayPins[i], LOW);
    refreshRelayRows();
    delay(80);
  }
  Serial.println("Self-check done.\n");
}

// ===================== Main loop =====================
void loop() {
  cycleCount++;

  // -------- 1. ALL ON / ALL OFF --------
  drawPhase("1/5  ALL ON / ALL OFF");
  setAllRelays(HIGH);      refreshRelayRows();
  Serial.println("-> ALL ON");
  delay(2000);

  setAllRelays(LOW);       refreshRelayRows();
  Serial.println("-> ALL OFF");
  delay(1000);

  // -------- 2. SEQUENTIAL --------
  drawPhase("2/5  SEQUENTIAL");
  Serial.println("--- SEQUENTIAL ---");
  for (int i = 0; i < 4; i++) {
    io.digitalWrite(relayPins[i], HIGH);
    refreshRelayRows();
    Serial.printf("-> relay %d ON\n", i + 1);
    delay(2000);

    io.digitalWrite(relayPins[i], LOW);
    refreshRelayRows();
    delay(100);
  }

  // -------- 3. ODDS vs EVENS --------
  drawPhase("3/5  ODDS vs EVENS");
  Serial.println("--- ODDS vs EVENS ---");
  for (int r = 0; r < 4; r++) {
    bool odds = (r % 2 == 0);
    for (int i = 0; i < 4; i++) {
      bool s = odds ? (i % 2 == 0) : (i % 2 != 0);
      io.digitalWrite(relayPins[i], s);
    }
    refreshRelayRows();
    Serial.printf("-> %s ON\n", odds ? "ODD (1,3)" : "EVEN (2,4)");
    delay(1500);
  }
  setAllRelays(LOW);      refreshRelayRows();   delay(300);

  // -------- 4. CHASER --------
  drawPhase("4/5  CHASER");
  Serial.println("--- CHASER ---");
  for (int i = 0; i < 4; i++) {
    for (int k = 0; k < 4; k++) io.digitalWrite(relayPins[k], (k == i));
    refreshRelayRows();
    Serial.printf("-> chaser fwd %d\n", i + 1);
    delay(250);
  }
  for (int i = 3; i >= 0; i--) {
    for (int k = 0; k < 4; k++) io.digitalWrite(relayPins[k], (k == i));
    refreshRelayRows();
    Serial.printf("-> chaser rev %d\n", i + 1);
    delay(250);
  }
  setAllRelays(LOW);      refreshRelayRows();   delay(300);

  // -------- 5. BINARY COUNTER --------
  drawPhase("5/5  BINARY COUNTER");
  Serial.println("--- BINARY COUNTER ---");
  for (int count = 0; count <= 15; count++) {
    for (int i = 0; i < 4; i++) {
      io.digitalWrite(relayPins[i], bitRead(count, i));
    }
    refreshRelayRows();
    drawBinaryValue(count);
    Serial.printf("-> val=%2d  bin=%d%d%d%d\n",
                  count, bitRead(count, 3), bitRead(count, 2),
                  bitRead(count, 1), bitRead(count, 0));
    delay(500);
  }
  setAllRelays(LOW);      refreshRelayRows();   drawBinaryValue(-1);
  delay(500);
}

// ===================== Relay helpers =====================
void setAllRelays(bool state) {
  for (int i = 0; i < 4; i++) io.digitalWrite(relayPins[i], state);
}

void refreshRelayRows() {
  for (int i = 0; i < 4; i++) {
    bool st = (io.digitalRead(relayPins[i]) != 0);
    drawRelayState(i, st);
  }
}

// ===================== I2C scan =====================
void I2C_SCAN() {
  byte error, address;
  int deviceCount = 0;
  Serial.println("Scanning...");
  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      Serial.printf("I2C device found at address 0x%02X !\n", address);
      deviceCount++;
      delay(1);
    } else if (error == 4) {
      Serial.printf("Unknown error at address 0x%02X\n", address);
    }
  }
  if (deviceCount == 0) Serial.println("No I2C devices found\n");
  else                  Serial.println("Scanning complete\n");
}

void drawStaticLayout() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(lgfx::textdatum_t::baseline_left);

  uint16_t greenBg = tft.color565(0, 130, 70);
  tft.fillRect(0, 0, 240, 49, greenBg);
  tft.setTextColor(TFT_WHITE, greenBg);

  tft.setFont(&fonts::FreeSansBold12pt7b);
  const char* title = "NORVI EX-R4";
  int tw = tft.textWidth(title, &fonts::FreeSansBold12pt7b);
  tft.setCursor((240 - tw) / 2, 34);
  tft.print(title);

  for (int i = 0; i < 4; i++) {
    int16_t top = 88 + i * 46;

    tft.fillRect(5, top, 230, 42, tft.color565(18, 18, 18));
    tft.drawRect(5, top, 230, 42, tft.color565(60, 60, 60));

    tft.setTextColor(TFT_WHITE, tft.color565(18, 18, 18));
    tft.setFont(&fonts::FreeSansBold9pt7b);
    tft.setCursor(12, top + 28);
    tft.print("RELAY ");
    tft.print(i + 1);

    rowUI[i].boxX  = 160;
    rowUI[i].boxY  = top + 8;
    rowUI[i].boxW  = 68;
    rowUI[i].boxH  = 26;
    rowUI[i].lastValid = false;

    drawRelayState(i, false);
  }
}

void drawRelayState(int i, bool on) {
  RowUI& r = rowUI[i];
  if (r.lastValid && r.lastState == on) return;
  r.lastValid = true;
  r.lastState = on;

  tft.fillRect(r.boxX, r.boxY, r.boxW, r.boxH, TFT_BLACK);
  tft.drawRect(r.boxX, r.boxY, r.boxW, r.boxH,
               on ? TFT_CYAN : tft.color565(80, 80, 80));
  tft.setTextColor(on ? TFT_GREEN : TFT_RED, TFT_BLACK);
  tft.setFont(&fonts::FreeSansBold9pt7b);
  tft.setCursor(r.boxX + 12, r.boxY + 19);
  tft.print(on ? "ON" : "OFF");
}

void drawPhase(const char* name) {
  tft.fillRect(0, 56, 240, 26, TFT_BLACK);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setFont(&fonts::FreeSans9pt7b);
  tft.setCursor(8, 76);
  tft.print(name);
  Serial.print("Pattern: ");
  Serial.println(name);
}

// ---- Binary value: "VAL: 10" and the bits use the SAME font & size ----
void drawBinaryValue(int v) {
  tft.fillRect(0, 282, 240, 30, TFT_BLACK);
  if (v < 0) return;

  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setFont(&fonts::FreeSans9pt7b);

  tft.setCursor(65, 304);
  tft.printf("VAL: %2d", v);

  tft.setCursor(135, 304);
  tft.printf("%d%d%d%d",
             bitRead(v, 3), bitRead(v, 2), bitRead(v, 1), bitRead(v, 0));
}