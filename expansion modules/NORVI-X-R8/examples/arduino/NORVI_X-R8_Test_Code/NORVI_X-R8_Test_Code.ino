//"NPD-NORVI X-R8-V2 (N16R2)" — two-column relay layout, non-bold 9pt text
//2025.10.31  (diagnostic build: verify I2C address + readback)

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
#define PCA9538_ADDR 0x72        // change to 0x73 if scan says so
#define SDA   8
#define SCL   9
#define MISO 13
#define MOSI 11
#define SCLK 12
#define DSP_CS 45
#define PCA_RESET 21

const int relayPins[] = {1, 2, 3, 4, 5, 6, 7, 8};   
PCA9538 io(PCA9538_ADDR);

// ===================== Screen layout (two columns) =====================
// y=  0..44   header band (green, 12pt title)
// y= 45       divider
// y= 50..70   phase name (9pt non-bold)
// y= 78..228  8 relay rows in TWO columns (4 rows each, 36 px pitch)
//             left column  : R1..R4   x = 6..116
//             right column : R5..R8   x = 124..234
// y=240..258  progress bar (yellow)
// y=270..300  VAL + binary (9pt non-bold, cyan)

struct RowUI {
  int16_t boxX, boxY, boxW, boxH;
  bool    lastState;
  bool    lastValid;
};

RowUI rowUI[8];

int cycleCount = 0;

// ===================== Setup =====================
void setup() {
  Serial.begin(115200);
  delay(1500);
  Serial.println("\n=== NPD-NORVI X-R8-V2 RELAY TEST ===");

  pinMode(PCA_RESET, OUTPUT);
  digitalWrite(PCA_RESET, HIGH);
  delay(100);

  Wire.begin(SDA, SCL);
  delay(100);

  I2C_SCAN();
  delay(500);

  // ---------- DIAG: PCA9538 ACK check ----------
  Wire.beginTransmission(PCA9538_ADDR);
  int ack = Wire.endTransmission();
  if (ack == 0) {
    Serial.printf("DIAG: PCA9538 ACK at 0x%02X  (OK)\n", PCA9538_ADDR);
  } else {
    Serial.printf("DIAG: PCA9538 NO ACK at 0x%02X  (code %d) <-- wrong address or wiring/reset\n",
                  PCA9538_ADDR, ack);
  }

  for (int i = 0; i < 8; i++) {
    io.pinMode(relayPins[i], OUTPUT);
    io.digitalWrite(relayPins[i], LOW);
  }
  delay(50);

  // ---------- DIAG: write/readback test on relay 0 ----------
  io.digitalWrite(relayPins[0], HIGH);  delay(10);
  int rbH = io.digitalRead(relayPins[0]);
  io.digitalWrite(relayPins[0], LOW);   delay(10);
  int rbL = io.digitalRead(relayPins[0]);
  Serial.printf("DIAG: relay0 write HIGH read=%d (expect 1), write LOW read=%d (expect 0)\n",
                rbH, rbL);

  // ---------- DIAG: dump all 8 readback states at LOW ----------
  Serial.print("DIAG: after ALL LOW, readbacks = ");
  for (int i = 0; i < 8; i++) Serial.printf("%d", io.digitalRead(relayPins[i]) != 0);
  Serial.println("  (expect 00000000)");

  SPI.begin(SCLK, MISO, MOSI);
  delay(500);

  tft.init();
  tft.setRotation(0);
  tft.setTextDatum(lgfx::textdatum_t::baseline_left);

  drawStaticLayout();
  refreshRelayRows();

  // Quick self-check blip 1→8
  Serial.println("Self-check 1→8 ...");
  for (int i = 0; i < 8; i++) {
    io.digitalWrite(relayPins[i], HIGH);
    refreshRelayRows();
    delay(150);
    io.digitalWrite(relayPins[i], LOW);
    refreshRelayRows();
    delay(60);
  }
  Serial.println("Self-check done.\n");
}

// ===================== Main loop =====================
void loop() {
  cycleCount++;

  // -------- 1. ALL ON / ALL OFF --------
  drawPhase("1/5  ALL ON / ALL OFF");
  drawProgress(0);
  setAllRelays(HIGH);      refreshRelayRows();  drawProgress(50);
  Serial.println("-> ALL ON");
  delay(2000);

  setAllRelays(LOW);       refreshRelayRows();  drawProgress(100);
  Serial.println("-> ALL OFF");
  delay(1000);

  // -------- 2. SEQUENTIAL --------
  drawPhase("2/5  SEQUENTIAL");
  drawProgress(0);
  Serial.println("--- SEQUENTIAL ---");
  for (int i = 0; i < 8; i++) {
    io.digitalWrite(relayPins[i], HIGH);
    refreshRelayRows();
    drawProgress(((i + 1) * 100) / 8);
    Serial.printf("-> relay %d ON\n", i + 1);
    delay(1500);

    io.digitalWrite(relayPins[i], LOW);
    refreshRelayRows();
    delay(100);
  }

  // -------- 3. ODDS vs EVENS --------
  drawPhase("3/5  ODDS vs EVENS");
  drawProgress(0);
  Serial.println("--- ODDS vs EVENS ---");
  for (int r = 0; r < 4; r++) {
    bool odds = (r % 2 == 0);
    for (int i = 0; i < 8; i++) {
      bool s = odds ? (i % 2 == 0) : (i % 2 != 0);
      io.digitalWrite(relayPins[i], s);
    }
    refreshRelayRows();
    drawProgress((r + 1) * 25);
    Serial.printf("-> %s ON\n", odds ? "ODD (1,3,5,7)" : "EVEN (2,4,6,8)");
    delay(1000);
  }
  setAllRelays(LOW);      refreshRelayRows();   delay(300);

  // -------- 4. CHASER --------
  drawPhase("4/5  CHASER");
  drawProgress(0);
  Serial.println("--- CHASER ---");
  for (int i = 0; i < 8; i++) {
    for (int k = 0; k < 8; k++) io.digitalWrite(relayPins[k], (k == i));
    refreshRelayRows();
    drawProgress(((i + 1) * 100) / 16);
    Serial.printf("-> chaser fwd %d\n", i + 1);
    delay(150);
  }
  for (int i = 7; i >= 0; i--) {
    for (int k = 0; k < 8; k++) io.digitalWrite(relayPins[k], (k == i));
    refreshRelayRows();
    drawProgress(((16 - i) * 100) / 16);
    Serial.printf("-> chaser rev %d\n", i + 1);
    delay(150);
  }
  setAllRelays(LOW);      refreshRelayRows();   delay(300);

  // -------- 5. BINARY COUNTER 0..255 --------
  drawPhase("5/5  BINARY COUNTER");
  drawProgress(0);
  Serial.println("--- BINARY COUNTER ---");
  for (int count = 0; count <= 255; count++) {
    for (int i = 0; i < 8; i++) {
      io.digitalWrite(relayPins[i], bitRead(count, i));
    }
    refreshRelayRows();
    drawBinaryValue(count);
    drawProgress(((count + 1) * 100) / 256);
    if (count % 16 == 0) {
      Serial.printf("-> val=%3d\n", count);
    }
    delay(100);
  }
  setAllRelays(LOW);      refreshRelayRows();   drawBinaryValue(-1);
  drawProgress(0);
  delay(500);
}

// ===================== Relay helpers =====================
void setAllRelays(bool state) {
  for (int i = 0; i < 8; i++) io.digitalWrite(relayPins[i], state);
}

void refreshRelayRows() {
  static uint32_t lastLog = 0;
  bool log = (millis() - lastLog) > 2000;   // log readback every ~2s
  for (int i = 0; i < 8; i++) {
    bool st = (io.digitalRead(relayPins[i]) != 0);
    drawRelayState(i, st);
    if (log) Serial.printf("%d", st ? 1 : 0);
  }
  if (log) { Serial.println("   <- readback (relay 1..8)"); lastLog = millis(); }
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

// ===================== UI: static layout =====================
void drawStaticLayout() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(lgfx::textdatum_t::baseline_left);

  // -------- Header band: GREEN, larger title (12pt) --------
  uint16_t greenBg = tft.color565(0, 130, 70);
  tft.fillRect(0, 0, 240, 44, greenBg);
  tft.setTextColor(TFT_WHITE, greenBg);

  tft.setFont(&fonts::FreeSansBold12pt7b);
  const char* title = "NORVI EX-R8";
  int tw = tft.textWidth(title, &fonts::FreeSansBold12pt7b);
  tft.setCursor((240 - tw) / 2, 32);
  tft.print(title);

  tft.drawFastHLine(0, 45, 240, TFT_CYAN);

  // -------- Two-column relay layout --------
  // Left  column: index 0..3   x = 6..116
  // Right column: index 4..7   x = 124..234
  for (int i = 0; i < 8; i++) {
    int col = i / 4;
    int row = i % 4;

    int16_t xLeft = (col == 0) ? 6 : 124;
    int16_t top   = 78 + row * 36;

    tft.fillRect(xLeft, top, 110, 32, tft.color565(18, 18, 18));
    tft.drawRect(xLeft, top, 110, 32, tft.color565(60, 60, 60));

    tft.setTextColor(TFT_WHITE, tft.color565(18, 18, 18));
    tft.setFont(&fonts::FreeSans9pt7b);
    tft.setCursor(xLeft + 6, top + 22);
    tft.printf("R%d", i + 1);

    rowUI[i].boxX  = xLeft + 48;
    rowUI[i].boxY  = top + 5;
    rowUI[i].boxW  = 56;
    rowUI[i].boxH  = 22;
    rowUI[i].lastValid = false;

    drawRelayState(i, false);
  }

  tft.drawRect(5, 240, 230, 18, tft.color565(80, 80, 80));
}

// ===================== UI: per-relay state =====================
void drawRelayState(int i, bool on) {
  RowUI& r = rowUI[i];
  if (r.lastValid && r.lastState == on) return;
  r.lastValid = true;
  r.lastState = on;

  tft.fillRect(r.boxX, r.boxY, r.boxW, r.boxH, TFT_BLACK);
  tft.drawRect(r.boxX, r.boxY, r.boxW, r.boxH,
               on ? TFT_CYAN : tft.color565(80, 80, 80));
  tft.setTextColor(on ? TFT_GREEN : TFT_RED, TFT_BLACK);
  tft.setFont(&fonts::FreeSans9pt7b);
  tft.setCursor(r.boxX + 10, r.boxY + 16);
  tft.print(on ? "ON" : "OFF");
}

// ===================== UI: phase name + progress =====================
void drawPhase(const char* name) {
  tft.fillRect(0, 50, 240, 24, TFT_BLACK);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setFont(&fonts::FreeSans9pt7b);
  tft.setCursor(8, 68);
  tft.print(name);
  Serial.print("Pattern: ");
  Serial.println(name);
}

void drawProgress(int percent) {
  if (percent < 0)   percent = 0;
  if (percent > 100) percent = 100;
  int w = (226 * percent) / 100;
  tft.fillRect(6, 241, 226, 16, TFT_BLACK);
  if (w > 0) tft.fillRect(6, 241, w, 16, TFT_YELLOW);
}

// ===================== UI: binary value =====================

void drawBinaryValue(int v) {
  tft.fillRect(0, 270, 240, 32, TFT_BLACK);
  if (v < 0) return;

  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setFont(&fonts::FreeSans9pt7b);

  char buf[32];
  snprintf(buf, sizeof(buf), "VAL: %d %d%d%d%d%d%d%d%d",
           v,
           bitRead(v, 7), bitRead(v, 6), bitRead(v, 5), bitRead(v, 4),
           bitRead(v, 3), bitRead(v, 2), bitRead(v, 1), bitRead(v, 0));

  int tw = tft.textWidth(buf, &fonts::FreeSans9pt7b);
  tft.setCursor((240 - tw) / 2, 292);
  tft.print(buf);
}