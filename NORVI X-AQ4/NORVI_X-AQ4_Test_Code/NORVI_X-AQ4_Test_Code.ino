#include <Wire.h>
#include <SPI.h>
#include "TFT_eSPI.h"
#include "Free_Fonts.h"

// ================= I2C =================
#define SDA_PIN 8     // Change if needed
#define SCL_PIN 9
#define DAC_ADDR 0x5F

// ================= TFT =================
#define MISO 13
#define MOSI 11
#define SCLK 12
#define DSP_CS 45

TFT_eSPI tft = TFT_eSPI();

// ================= DISPLAY VALUES =================
int lastChannel = 0;
float lastValue = 0;
uint16_t lastDAC = 0;
bool isVoltage = true;

float val;
uint16_t dac_value = 0;
int ch = 0;


// =========================================================
// DRAW HEADER
// =========================================================
void drawHeader()
{
  tft.fillScreen(TFT_BLACK);

  tft.setTextColor(TFT_YELLOW);
  tft.setFreeFont(FSB12);

  tft.setCursor(20, 20);
  tft.print("NORVI X AQ4");

}


// =========================================================
// DRAW VALUES
// =========================================================
void drawValues()
{
  tft.fillScreen(TFT_BLACK);

  tft.setTextColor(TFT_YELLOW);
  tft.setFreeFont(FSB9);

  tft.setCursor(20,20);
  tft.println("NORVI X AQ4");

  tft.setTextColor(TFT_WHITE);
  tft.setCursor(20,70);

  tft.print("CH");
  tft.print(lastChannel);
  tft.print(" = ");
  tft.print(val,2);

  if(isVoltage)
    tft.print(" V");
  else
    tft.print(" mA");

  tft.setTextColor(TFT_CYAN);
  tft.setCursor(20,110);

  tft.print("DAC Code = ");
  tft.println(dac_value);
}


// =========================================================
void setup() {

  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);

  delay(300);

  SPI.begin(SCLK, MISO, MOSI);

  tft.init();
  tft.begin();
  tft.setRotation(0);

  drawHeader();

  delay(1500);

  config_dac(DAC_ADDR, 0, 0, 1, 1);

  Serial.println("====================================");
  Serial.println("  ESP32 DAC SERIAL CONTROL READY");
  Serial.println("====================================");
  Serial.println("CH1=5      (0-10V)");
  Serial.println("CH2=7.5    (0-10V)");
  Serial.println("CH3=12     (0-20mA)");
  Serial.println("CH4=20     (0-20mA)");
  Serial.println("====================================");
}

// =========================================================
void loop() {

  if (Serial.available()) {

    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    cmd.toUpperCase();

    processCommand(cmd);
  }
}

// =========================================================
// PROCESS COMMAND
// =========================================================
void processCommand(String cmd) {

  if (!cmd.startsWith("CH")) {
    Serial.println("Invalid command!");
    return;
  }

  ch = cmd.substring(2, 3).toInt();
  int eqIndex = cmd.indexOf('=');

  if (ch < 1 || ch > 4 || eqIndex < 0) {
    Serial.println("Format: CHx=value");
    return;
  }

  val = cmd.substring(eqIndex + 1).toFloat();
  dac_value = 0;

  // -------- CH1 & CH2 (0–10V) --------
  if (ch == 1 || ch == 2) {

    if (val < 0 || val > 10) {
      Serial.println("Voltage range: 0-10V");
      return;
    }

    dac_value = (val / 10.0) * 4095.0;
  }

  // -------- CH3 & CH4 (0–20mA) --------
  if (ch == 3 || ch == 4) {

    if (val < 0 || val > 20) {
      Serial.println("Current range: 0-20mA");
      return;
    }

    dac_value = (val / 20.0) * 4095.0;
  }

  write_channel(DAC_ADDR, ch, dac_value);

  Serial.print("CH");
  Serial.print(ch);
  Serial.print(" = ");
  Serial.print(val);

  if (ch <= 2)
    Serial.print(" V");
  else
    Serial.print(" mA");

  Serial.print(" | DAC Code = ");
  Serial.println(dac_value);
  drawValues();
}

// =========================================================
// WRITE DAC CHANNEL
// =========================================================
void write_channel(byte device_address, unsigned int channel, unsigned int value) {

  Wire.beginTransmission(device_address);
  Wire.write(1 + (channel * 2));
  Wire.write(value >> 8);
  Wire.write(value & 0xFF);
  Wire.endTransmission();
}

// =========================================================
// DAC CONFIG
// =========================================================
void config_dac(byte device_address,
                unsigned int c1,
                unsigned int c2,
                unsigned int c3,
                unsigned int c4) {

  unsigned int tftc = 0;
  tftc |= (c4 << 3);
  tftc |= (c3 << 2);
  tftc |= (c2 << 1);
  tftc |= c1;

  Wire.beginTransmission(device_address);
  Wire.write(0x01);
  Wire.write(tftc);
  Wire.endTransmission();
}
