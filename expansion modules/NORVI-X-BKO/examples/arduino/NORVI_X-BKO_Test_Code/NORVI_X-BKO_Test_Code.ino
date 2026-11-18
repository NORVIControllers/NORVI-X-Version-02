// NORVI X-BKO 5475 - 4x10 Grid Pad Map
// Grey = LOW / floating, Green = HIGH (3.3V applied)
// Bottom status block shows: CURRENT: <pin>  STATE: HIGH/LOW

#include <Wire.h>
#include <SPI.h>
#include "PCA9539.h"
#include <PCA9536D.h>
#include <TFT_eSPI.h>

#define SDA 8
#define SCL 9

#define SAFE_HIGH_COLOR  TFT_CYAN      // priority pins
#define OBS_HIGH_COLOR   TFT_GREEN     // everyone else

#define GREY_COLOR  0x7BEF
#define HIGH_COLOR  TFT_GREEN
#define TITLE_COLOR TFT_WHITE
#define STATUS_BG   0x1082
#define STATUS_FG   TFT_WHITE

PCA9539 ioport(0x75);
PCA9536 io;
TFT_eSPI tft = TFT_eSPI();

#define TFT_MISO_PIN 13
#define TFT_MOSI_PIN 11
#define TFT_SCLK_PIN 12

#define CS4      PCA_A5
#define CS3      PCA_A6
#define CS2      PCA_A7
#define CS1      PCA_B0
#define OUTPUT19 PCA_B1
#define OUTPUT20 PCA_B2
#define OUTPUT33 PCA_B3
#define OUTPUT34 PCA_B4
#define OUTPUT38 PCA_B5

enum { LIVE_SAFE = 0, LIVE_OBS = 1, STATIC_PAD = 2, PCA_OUT = 3 };

// =========================================================
//  Priority pins — these show CYAN when HIGH.
//  All other pins show GREEN when HIGH.
//  Names must match the pads[] table exactly.
// =========================================================
const char* cyanPins[] = {
  "CS4", "IO1", "IO2", "IO4",
  "SCL", "SDA", "CS2", "IO38",
  "SCLK", "IO8", "IO12", "CS3"
};
const int numCyanPins = sizeof(cyanPins) / sizeof(cyanPins[0]);

bool isCyanPin(const char* name) {
  for (int i = 0; i < numCyanPins; i++) {
    if (strcmp(name, cyanPins[i]) == 0) return true;
  }
  return false;
}

struct Pad {
  const char* name;
  int pin;
  int cat;
};

Pad pads[] = {
  // ---------- COLUMN 1 ----------
  {"IO11",  11, LIVE_OBS},
  {"IO10",  10, LIVE_SAFE},
  {"IO7",    7, LIVE_SAFE},
  {"IO6",    6, LIVE_SAFE},
  {"IO5",    5, LIVE_SAFE},
  {"IO4",    4, LIVE_SAFE},
  {"IO2",    2, LIVE_SAFE},
  {"IO1",    1, LIVE_SAFE},
  {"CS4",    CS4, PCA_OUT},
  {"CS3",    CS3, PCA_OUT},

  // ---------- COLUMN 2 ----------
  {"CS2",    CS2, PCA_OUT},
  {"CS1",    CS1, PCA_OUT},
  {"URBN",  -1, STATIC_PAD},
  {"USBP",  -1, STATIC_PAD},
  {"TX1",  -1, STATIC_PAD},
  {"RX1",  -1, STATIC_PAD},
  {"SDA",    8, LIVE_OBS},
  {"SCL",    9, LIVE_OBS},
  {"A20",   20, LIVE_OBS},
  {"A19",   19, LIVE_OBS},

  // ---------- COLUMN 3 ----------
  {"A18",   18, LIVE_OBS},
  {"IO40",  40, LIVE_SAFE},
  {"IO39",  39, LIVE_SAFE},
  {"IO38",  38, LIVE_SAFE},
  {"IO34",  34, LIVE_OBS},
  {"IO33",  33, LIVE_OBS},
  {"IO21",  21, LIVE_SAFE},
  {"IO20",  20, LIVE_OBS},
  {"IO19",  19, LIVE_OBS},
  {"IO16",  16, LIVE_OBS},

  // ---------- COLUMN 4 ----------
  {"IO15",  15, LIVE_OBS},
  {"IO14",  14, LIVE_SAFE},
  {"IO13",  13, LIVE_OBS},
  {"IO12",  12, LIVE_OBS},
  {"IO8",    8, LIVE_OBS},
  {"MOSI",  11, LIVE_OBS},
  {"MISO",  13, LIVE_OBS},
  {"SCLK",  12, LIVE_OBS}
};
const int numPads = sizeof(pads) / sizeof(pads[0]);

// =========================================================
//  Grid geometry
// =========================================================
#define COLS        4
#define ROWS        10
#define COL_X0      25
#define COL_STEP    60
#define ROW_Y0      42
#define ROW_STEP    23
#define PAD_R       3

int padX(int index) { return COL_X0 + (index / ROWS) * COL_STEP; }
int padY(int index) { return ROW_Y0 + (index % ROWS) * ROW_STEP; }

// =========================================================
//  Status block
// =========================================================
#define STATUS_H  45
#define STATUS_Y  (tft.height() - STATUS_H)

int  activeIdx = -1;
bool activeState = false;

// =========================================================
//  Drawing
// =========================================================
void drawPad(int index, uint16_t color, bool filled) {
  int x = padX(index);
  int y = padY(index);
  const Pad &p = pads[index];

  // Measure the label width (6 px per char at size 1)
  int textWidth = strlen(p.name) * 6;

  // Total width = circle diameter + gap + label width
  int gap = 3;
  int totalWidth = (PAD_R * 2) + gap + textWidth;

  // Center the whole block on the column
  int startX = x - totalWidth / 2;

  // Draw the circle at the left of the block
  int circleX = startX + PAD_R;
  int circleY = y;

  if (filled) {
    tft.fillCircle(circleX, circleY, PAD_R, color);
  } else {
    tft.fillCircle(circleX, circleY, PAD_R, TFT_BLACK);
    tft.drawCircle(circleX, circleY, PAD_R, color);
  }

  // Draw the label to the right of the circle
  tft.setTextColor(color);
  tft.setTextSize(1);
  tft.setCursor(circleX + PAD_R + gap, circleY - 4);
  tft.print(p.name);
}

void drawStaticLayout() {
  tft.fillScreen(TFT_BLACK);

  tft.setTextColor(TITLE_COLOR);
  tft.setTextSize(2);                              // was 1
  const char* title = "NORVI X-BK0 Test";
  int titleWidth = strlen(title) * 12;             // 12 px per char at size 2
  tft.setCursor((tft.width() - titleWidth) / 2, 2);
  tft.print(title);
  tft.drawFastHLine(0, 17, tft.width(), TITLE_COLOR);   // line moved down

  // (C1/C2/C3/C4 headers removed)

  for (int c = 1; c < COLS; c++) {
    int x = COL_X0 + c * COL_STEP - 30;
    tft.drawFastVLine(x, 16, STATUS_Y - 22, GREY_COLOR);
  }
  tft.drawRect(2, 2, tft.width() - 4, tft.height() - 4, GREY_COLOR);

  tft.fillRect(0, STATUS_Y, tft.width(), STATUS_H, STATUS_BG);
  tft.drawFastHLine(0, STATUS_Y, tft.width(), TITLE_COLOR);
}

void drawStatusBlock() {
  tft.fillRect(0, STATUS_Y + 1, tft.width(), STATUS_H - 1, STATUS_BG);

  // Pin name
  tft.setTextSize(2);
  tft.setTextColor(TFT_YELLOW);
  tft.setCursor(50, STATUS_Y + 10);
  tft.print(activeIdx >= 0 ? pads[activeIdx].name : "---");

  // State colour depends on whether the pin is in the cyan list
  uint16_t stateColor = GREY_COLOR;
  if (activeIdx >= 0 && activeState) {
    stateColor = isCyanPin(pads[activeIdx].name) ? SAFE_HIGH_COLOR : OBS_HIGH_COLOR;
  }

  tft.setTextColor(stateColor);
  tft.setCursor(150, STATUS_Y + 10);
  tft.print(activeIdx >= 0 ? (activeState ? "HIGH" : "LOW ") : "----");
}

// =========================================================
void setup() {
  Serial.begin(115200);
  delay(1500);

  SPI.begin(TFT_SCLK_PIN, TFT_MISO_PIN, TFT_MOSI_PIN);
  tft.init();
  tft.setRotation(0);
  drawStaticLayout();

  Wire.begin(SDA, SCL);
  delay(100);

  ioport.pinMode(CS4, INPUT);
  ioport.pinMode(CS3, INPUT);
  ioport.pinMode(CS2, INPUT);
  ioport.pinMode(CS1, INPUT);
  ioport.pinMode(OUTPUT19, INPUT);
  ioport.pinMode(OUTPUT20, INPUT);
  ioport.pinMode(OUTPUT33, INPUT);
  ioport.pinMode(OUTPUT34, INPUT);
  ioport.pinMode(OUTPUT38, INPUT);

  Serial.println("\n=== NORVI 5475 4x10 GRID ===");
  for (int i = 0; i < numPads; i++) {
    if (pads[i].cat == LIVE_SAFE) {
      pinMode(pads[i].pin, INPUT_PULLDOWN);
    }
    drawPad(i, GREY_COLOR, false);
  }

  drawStatusBlock();
  Serial.println("Ready. Apply 3.3V to any safe pin.");
}

// =========================================================
void loop() {
  static bool lastState[sizeof(pads)/sizeof(pads[0])] = {false};
  static bool everDrawn[sizeof(pads)/sizeof(pads[0])] = {false};

  bool statusNeedsRedraw = false;

  for (int i = 0; i < numPads; i++) {
    const Pad &p = pads[i];

    bool state = false;
    bool readable = false;

    switch (p.cat) {
      case LIVE_SAFE:
      case LIVE_OBS:
        state = digitalRead(p.pin);
        readable = true;
        break;
      case PCA_OUT:
        state = ioport.digitalRead(p.pin);
        readable = true;
        break;
      case STATIC_PAD:
      default:
        break;
    }
    if (!readable) continue;

    if (!everDrawn[i] || state != lastState[i]) {
      everDrawn[i] = true;
      lastState[i] = state;

      if (state) {
        // Choose colour: cyan for priority list, green for everything else
        uint16_t highColor = isCyanPin(p.name) ? SAFE_HIGH_COLOR : OBS_HIGH_COLOR;
        drawPad(i, highColor, true);
      } else {
        drawPad(i, GREY_COLOR, false);
      }

      activeIdx = i;
      activeState = state;
      statusNeedsRedraw = true;

      Serial.print("[ACTIVE] "); Serial.print(p.name);
      Serial.println(state ? " HIGH" : " LOW");
    }
  }

  if (statusNeedsRedraw) drawStatusBlock();

  delay(60);
}
