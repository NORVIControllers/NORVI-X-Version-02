/*
 * NORVI X-MX4
 * 2026.08.31
*/


#include <Wire.h>
#include <SPI.h>
#include <Adafruit_ADS1X15.h>
#include <PCA9538.h>
#include "TFT_eSPI.h"

#define SDA_PIN   8
#define SCL_PIN   9

#define MISO_PIN  13
#define MOSI_PIN  11
#define SCLK_PIN  12
#define DSP_CS    47

#define ADS_ADDR  0x4A

// PCA9538 I2C expander - IO4-IO7 are digital inputs, IO0-IO3 drive the
// transistor outputs
#define PCA9538_ADDR 0x71
#define PCA_RESET    21

// Digital output pins on the PCA9538
#define DO1_PIN 1
#define DO2_PIN 2
#define DO3_PIN 3
#define DO4_PIN 4

// Digital input pins on the PCA9538
#define DI1_PIN 5
#define DI2_PIN 6
#define DI3_PIN 7
#define DI4_PIN 8

// Walking test timing - one output ON at a time, moves to the next every
// OUT_STEP_MS
const unsigned long OUT_STEP_MS = 1000;

Adafruit_ADS1115 ads1;
PCA9538 io(PCA9538_ADDR);
TFT_eSPI tft = TFT_eSPI();

const float mA_Factor = 4.096 / 3269.826;

// Layout columns
const int LABEL_X   = 10;
const int VALUE_X   = 100;
const int ROW_H     = 18;   // font 2 row height

const int IO_VALUE_X = 45;    // DI status (dot + HIGH/LOW)
const int DO_LABEL_X = 130;   // "DO1:" etc
const int DO_VALUE_X = 165;   // DO status (dot + ON/OFF)

// ANALOG ROW Y POSITIONS (font 2, 26px pitch)
const int CH1_Y = 74;
const int CH2_Y = 100;
const int CH3_Y = 126;
const int CH4_Y = 152;

// DIGITAL I/O ROW Y POSITIONS (each row shows DIn + DOn side by side)
const int IO1_Y = 216;
const int IO2_Y = 242;
const int IO3_Y = 268;
const int IO4_Y = 294;

float old1 = -999;
float old2 = -999;
float old3 = -999;
float old4 = -999;

int oldIn1 = -1;
int oldIn2 = -1;
int oldIn3 = -1;
int oldIn4 = -1;

int outState[4]    = {LOW, LOW, LOW, LOW};
int oldOutState[4] = {-1, -1, -1, -1};
int activeOutput = 0;
unsigned long lastOutSwitch = 0;

void drawStatusDot(int x, int y, bool on, uint16_t onColor) {
  tft.fillCircle(x, y + 8, 5, on ? onColor : TFT_RED);
}

void drawInputRow(int x, int y, bool on) {
  tft.fillRect(x, y - 2, 78, ROW_H + 4, TFT_BLACK);
  drawStatusDot(x + 6, y, on, TFT_GREEN);
  tft.setTextColor(on ? TFT_GREEN : TFT_RED, TFT_BLACK);
  tft.drawString(on ? "HIGH" : "LOW", x + 20, y);
}

void drawOutputRow(int x, int y, bool on) {
  tft.fillRect(x, y - 2, 70, ROW_H + 4, TFT_BLACK);
  drawStatusDot(x + 6, y, on, TFT_GREEN);
  tft.setTextColor(on ? TFT_GREEN : TFT_RED, TFT_BLACK);
  tft.drawString(on ? "ON" : "OFF", x + 20, y);
}

void setup() {

  // PCA9538 RESET must be released before it will respond on I2C
  pinMode(PCA_RESET, OUTPUT);
  digitalWrite(PCA_RESET, HIGH);

  Wire.begin(SDA_PIN, SCL_PIN);

  // FAST I2C
  Wire.setClock(400000);

  // ADS1115
  ads1.begin(ADS_ADDR);

  ads1.setGain(GAIN_ONE);

  // MAX SPEED
  ads1.setDataRate(RATE_ADS1115_860SPS);

  // PCA9538 - digital inputs
  io.pinMode(DI1_PIN, INPUT);
  io.pinMode(DI2_PIN, INPUT);
  io.pinMode(DI3_PIN, INPUT);
  io.pinMode(DI4_PIN, INPUT);

  // PCA9538 - digital outputs, start all OFF
  io.pinMode(DO1_PIN, OUTPUT);
  io.pinMode(DO2_PIN, OUTPUT);
  io.pinMode(DO3_PIN, OUTPUT);
  io.pinMode(DO4_PIN, OUTPUT);
  io.digitalWrite(DO1_PIN, LOW);
  io.digitalWrite(DO2_PIN, LOW);
  io.digitalWrite(DO3_PIN, LOW);
  io.digitalWrite(DO4_PIN, LOW);

  // SPI
  SPI.begin(SCLK_PIN, MISO_PIN, MOSI_PIN);

  pinMode(DSP_CS, OUTPUT);
  digitalWrite(DSP_CS, HIGH);

  // TFT
  tft.init();

  tft.setRotation(0);

  tft.fillScreen(TFT_BLACK);

  // TITLE (only element on font 4 - everything else uses font 2 for a
  // consistent look)
  tft.setTextDatum(TC_DATUM);
  tft.setTextFont(4);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.drawString("NORVI X-MX4", tft.width() / 2, 8);
  tft.setTextDatum(TL_DATUM);

  tft.drawFastHLine(10, 40, tft.width() - 20, TFT_DARKGREY);

  // =========================
  // ANALOG SECTION
  // =========================
  tft.setTextFont(2);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawString("ANALOG INPUTS (mA)", LABEL_X, 50);

  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawString("CH1:", LABEL_X, CH1_Y);
  tft.drawString("CH2:", LABEL_X, CH2_Y);
  tft.drawString("CH3:", LABEL_X, CH3_Y);
  tft.drawString("CH4:", LABEL_X, CH4_Y);

  tft.drawFastHLine(10, 182, tft.width() - 20, TFT_DARKGREY);

  // =========================
  // DIGITAL I/O SECTION (DIn and DOn shown side by side per row)
  // =========================
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawString("DIGITAL I/O", LABEL_X, 192);

  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawString("DI1:", LABEL_X, IO1_Y);
  tft.drawString("DI2:", LABEL_X, IO2_Y);
  tft.drawString("DI3:", LABEL_X, IO3_Y);
  tft.drawString("DI4:", LABEL_X, IO4_Y);

  tft.drawString("DO1:", DO_LABEL_X, IO1_Y);
  tft.drawString("DO2:", DO_LABEL_X, IO2_Y);
  tft.drawString("DO3:", DO_LABEL_X, IO3_Y);
  tft.drawString("DO4:", DO_LABEL_X, IO4_Y);

  lastOutSwitch = millis();
}

void loop() {

  // =========================
  // WALKING OUTPUT TEST - one output ON at a time
  // =========================
  if (millis() - lastOutSwitch >= OUT_STEP_MS) {
    lastOutSwitch = millis();

    for (int i = 0; i < 4; i++) {
      outState[i] = (i == activeOutput) ? HIGH : LOW;
    }

    io.digitalWrite(DO1_PIN, outState[0]);
    io.digitalWrite(DO2_PIN, outState[1]);
    io.digitalWrite(DO3_PIN, outState[2]);
    io.digitalWrite(DO4_PIN, outState[3]);

    activeOutput = (activeOutput + 1) % 4;
  }

  float ch1 = ads1.readADC_SingleEnded(0) * mA_Factor;
  float ch2 = ads1.readADC_SingleEnded(1) * mA_Factor;
  float ch3 = ads1.readADC_SingleEnded(2) * mA_Factor;
  float ch4 = ads1.readADC_SingleEnded(3) * mA_Factor;

  int in1 = !io.digitalRead(DI1_PIN);
  int in2 = !io.digitalRead(DI2_PIN);
  int in3 = !io.digitalRead(DI3_PIN);
  int in4 = !io.digitalRead(DI4_PIN);

  // =========================
  // ANALOG VALUES
  // =========================
  tft.setTextFont(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  if (abs(ch1 - old1) > 0.01) {
    tft.fillRect(VALUE_X, CH1_Y, 100, ROW_H, TFT_BLACK);
    tft.drawFloat(ch1, 2, VALUE_X, CH1_Y);
    old1 = ch1;
  }

  if (abs(ch2 - old2) > 0.01) {
    tft.fillRect(VALUE_X, CH2_Y, 100, ROW_H, TFT_BLACK);
    tft.drawFloat(ch2, 2, VALUE_X, CH2_Y);
    old2 = ch2;
  }

  if (abs(ch3 - old3) > 0.01) {
    tft.fillRect(VALUE_X, CH3_Y, 100, ROW_H, TFT_BLACK);
    tft.drawFloat(ch3, 2, VALUE_X, CH3_Y);
    old3 = ch3;
  }

  if (abs(ch4 - old4) > 0.01) {
    tft.fillRect(VALUE_X, CH4_Y, 100, ROW_H, TFT_BLACK);
    tft.drawFloat(ch4, 2, VALUE_X, CH4_Y);
    old4 = ch4;
  }

  // =========================
  // DIGITAL INPUT VALUES
  // =========================
  tft.setTextFont(2);

  if (in1 != oldIn1) { drawInputRow(IO_VALUE_X, IO1_Y, in1); oldIn1 = in1; }
  if (in2 != oldIn2) { drawInputRow(IO_VALUE_X, IO2_Y, in2); oldIn2 = in2; }
  if (in3 != oldIn3) { drawInputRow(IO_VALUE_X, IO3_Y, in3); oldIn3 = in3; }
  if (in4 != oldIn4) { drawInputRow(IO_VALUE_X, IO4_Y, in4); oldIn4 = in4; }

  // =========================
  // DIGITAL OUTPUT STATUS (reflects the walking test above)
  // =========================
  if (outState[0] != oldOutState[0]) { drawOutputRow(DO_VALUE_X, IO1_Y, outState[0]); oldOutState[0] = outState[0]; }
  if (outState[1] != oldOutState[1]) { drawOutputRow(DO_VALUE_X, IO2_Y, outState[1]); oldOutState[1] = outState[1]; }
  if (outState[2] != oldOutState[2]) { drawOutputRow(DO_VALUE_X, IO3_Y, outState[2]); oldOutState[2] = outState[2]; }
  if (outState[3] != oldOutState[3]) { drawOutputRow(DO_VALUE_X, IO4_Y, outState[3]); oldOutState[3] = outState[3]; }
}
