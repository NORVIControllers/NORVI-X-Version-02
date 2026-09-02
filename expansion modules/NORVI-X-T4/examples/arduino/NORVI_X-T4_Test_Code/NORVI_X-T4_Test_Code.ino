/*
 *NORVI X-T4
 *2026/08/31
 */

#include <Wire.h>
#include <WiFi.h>
#include <SPI.h>
#include <Adafruit_ADS1X15.h>
#include "TFT_eSPI.h"
#include "Free_Fonts.h"
#include <math.h>

#define SDA   8
#define SCL   9

#define MISO  13
#define MOSI  11
#define SCLK  12

#define DSP_CS 47

/*
 ADS1115 ADDRESS
 DIP1(ON) - 0x49
 DIP2(ON) - 0x4B
 DIP3(ON) - 0x4A
 DIP4(ON) - 0x48
*/

#define ADS_ADDR 0x48
Adafruit_ADS1115 ads1;

// ---------- THERMISTOR / DIVIDER CONFIG ----------
// Set this to the actual voltage feeding the top of the divider (the
// rail the NTC's other end is tied to). Common values: 3.3 or 5.0.
const float SUPPLY_VOLTAGE = 3.3;

const float SERIES_RESISTOR      = 10000.0;  // the fixed 10K resistor
const float THERMISTOR_NOMINAL   = 10000.0;  // NTC resistance at 25C
const float TEMPERATURE_NOMINAL  = 25.0;     // reference temp, deg C
const float B_COEFFICIENT        = 3950.0;   // beta value from your NTC's datasheet -- CHECK/UPDATE THIS

TFT_eSPI tft = TFT_eSPI();


unsigned long lastDisplayUpdate = 0;

const unsigned long DISPLAY_INTERVAL = 100;


float lastDisplay0 = -999.0;
float lastDisplay1 = -999.0;
float lastDisplay2 = -999.0;
float lastDisplay3 = -999.0;


void setup() {

  Serial.begin(115200);
  delay(200);

  Serial.println();
  Serial.println("NPD-NORVI X-AI4 NTC TEMPERATURE TEST");
  
  // i2c
  Wire.begin(SDA, SCL);

  if (!ads1.begin(ADS_ADDR)) {

    Serial.println("Failed to initialize ADS 1.");

  }
  else {

    Serial.println("ADS1115 OK");

  }

  ads1.setGain(GAIN_ONE);

  // FAST ADC
  ads1.setDataRate(RATE_ADS1115_860SPS);

  Serial.print("ADS1115 ADDRESS: 0x");

  if (ADS_ADDR < 0x10) {
    Serial.print("0");
  }

  Serial.println(ADS_ADDR, HEX);

  Serial.println("ADS1115 DATA RATE = 860 SPS");


  // SPI
  SPI.begin(SCLK, MISO, MOSI);

  delay(100);

  // TFT
  tft.init();
  tft.begin();
  tft.setRotation(0);
  
  LOGO_PRINT();
  
  tft.fillScreen(TFT_BLACK);


  // TITLE
  tft.setTextColor(TFT_GREEN);
  tft.setFreeFont(FSB12);
  
  tft.setCursor(40, 20);
  tft.print("NORVI X-T4");



  // TEMPERATURE LABELS
  tft.setFreeFont(FSB9);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setCursor(10, 85);
  tft.print("TEMP 1 :");

  tft.setCursor(10, 115);
  tft.print("TEMP 2 :");

  tft.setCursor(10, 145);
  tft.print("TEMP 3 :");

  tft.setCursor(10, 175);
  tft.print("TEMP 4 :");

  // INITIAL TEMPERATURE VALUES
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  
  tft.setCursor(145, 85);
  tft.print("0.00 C");

  tft.setCursor(145, 115);
  tft.print("0.00 C");

  tft.setCursor(145, 145);
  tft.print("0.00 C");

  tft.setCursor(145, 175);
  tft.print("0.00 C");


  // ADS1115 ADDRESS AT BOTTOM

  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setCursor(10, 215);
  tft.print("ADS ADDR : 0x");

  if (ADS_ADDR < 0x10) {
    tft.print("0");
  }

  tft.print(ADS_ADDR, HEX);

  Serial.println("TEST START");
}


// Converts a raw ADS1115 single-ended reading into a temperature (deg C)
// using the Beta-equation approximation of the Steinhart-Hart formula.
float readThermistorC(int16_t adcRaw) {

  // computeVolts() correctly applies the current gain setting (GAIN_ONE
  // here = +/-4.096V range over 16 bits), so this stays correct even if
  // you change the gain later.
  float voltage = ads1.computeVolts(adcRaw);

  // Guard against divide-by-zero / nonsense readings (e.g. open circuit)
  if (voltage <= 0.001) {
    return NAN;
  }
  if (voltage >= SUPPLY_VOLTAGE - 0.001) {
    return NAN;
  }

  // Solve the divider for NTC resistance.
  // Topology: VCC -> NTC -> node -> 10K -> GND, ADC reads "node".
  float ntcResistance = SERIES_RESISTOR * (SUPPLY_VOLTAGE / voltage - 1.0);

  // *** If your wiring is reversed (NTC from node to GND, fixed resistor
  // from VCC to node), replace the line above with:
  //   float ntcResistance = SERIES_RESISTOR / (SUPPLY_VOLTAGE / voltage - 1.0);

  // Beta equation
  float steinhart;
  steinhart = ntcResistance / THERMISTOR_NOMINAL;      // (R/Ro)
  steinhart = log(steinhart);                          // ln(R/Ro)
  steinhart /= B_COEFFICIENT;                           // 1/B * ln(R/Ro)
  steinhart += 1.0 / (TEMPERATURE_NOMINAL + 273.15);    // + (1/To)
  steinhart = 1.0 / steinhart;                          // invert
  steinhart -= 273.15;                                  // convert to deg C

  return steinhart;
}


void loop() {

  int16_t adc0;
  int16_t adc1;
  int16_t adc2;
  int16_t adc3;

  adc0 = ads1.readADC_SingleEnded(0);
  adc1 = ads1.readADC_SingleEnded(1);
  adc2 = ads1.readADC_SingleEnded(2);
  adc3 = ads1.readADC_SingleEnded(3);

  // ADC TO TEMPERATURE
  float temp0 = readThermistorC(adc0);
  float temp1 = readThermistorC(adc1);
  float temp2 = readThermistorC(adc2);
  float temp3 = readThermistorC(adc3);

  // SERIAL MONITOR
  Serial.printf(
    "T1: %.2f C  "
    "T2: %.2f C  "
    "T3: %.2f C  "
    "T4: %.2f C\n",
    temp0,
    temp1,
    temp2,
    temp3
  );

  // TFT UPDATE
  if (millis() - lastDisplayUpdate >= DISPLAY_INTERVAL) {

    lastDisplayUpdate = millis();

    char buf[20];

    tft.setTextColor(TFT_WHITE, TFT_BLACK);


    // TEMP 1
    if (temp0 != lastDisplay0) {

      tft.fillRect(140, 68, 100, 25, TFT_BLACK);
      if (isnan(temp0)) {
        tft.setCursor(145, 85);
        tft.print("--.-- C");
      } else {
        dtostrf(temp0, 5, 2, buf);
        tft.setCursor(145, 85);
        tft.print(buf);
        tft.print(" C");
      }
      lastDisplay0 = temp0;
    }
    
    // TEMP 2
    if (temp1 != lastDisplay1) {
      tft.fillRect(140, 98, 100, 25, TFT_BLACK);
      if (isnan(temp1)) {
        tft.setCursor(145, 115);
        tft.print("--.-- C");
      } else {
        dtostrf(temp1, 5, 2, buf);
        tft.setCursor(145, 115);
        tft.print(buf);
        tft.print(" C");
      }
      lastDisplay1 = temp1;
    }

    // TEMP 3
    if (temp2 != lastDisplay2) {
      tft.fillRect(140, 128, 100, 25, TFT_BLACK);
      if (isnan(temp2)) {
        tft.setCursor(145, 145);
        tft.print("--.-- C");
      } else {
        dtostrf(temp2, 5, 2, buf);
        tft.setCursor(145, 145);
        tft.print(buf);
        tft.print(" C");
      }
      lastDisplay2 = temp2;
    }

    // TEMP 4
    if (temp3 != lastDisplay3) {
      tft.fillRect(140, 158, 100, 25, TFT_BLACK);
      if (isnan(temp3)) {
        tft.setCursor(145, 175);
        tft.print("--.-- C");
      } else {
        dtostrf(temp3, 5, 2, buf);
        tft.setCursor(145, 175);
        tft.print(buf);
        tft.print(" C");
      }
      lastDisplay3 = temp3;
    }
  }
}


// I2C SCANNER
void I2C_SCAN() {

  byte error;
  byte address;

  int deviceCount = 0;

  Serial.println();
  Serial.println("Scanning...");

  for (address = 1; address < 127; address++) {

    Wire.beginTransmission(address);

    error = Wire.endTransmission();

    if (error == 0) {

      Serial.print("I2C device found at address 0x");

      if (address < 16) {
        Serial.print("0");
      }

      Serial.print(address, HEX);

      Serial.println(" !");

      deviceCount++;

      delay(1);
    }

    else if (error == 4) {

      Serial.print("Unknown error at address 0x");

      if (address < 16) {
        Serial.print("0");
      }

      Serial.println(address, HEX);
    }
  }

  if (deviceCount == 0) {

    Serial.println("No I2C devices found");

  }
  else {

    Serial.println("Scanning complete");

  }
}


//logo
void LOGO_PRINT() {
  tft.setSwapBytes(true);

  // BLACK BACKGROUND
  tft.fillScreen(TFT_WHITE);

  // DISPLAY IMAGE
  tft.pushImage(
    0,
    0,
    IMAGE_WIDTH,
    IMAGE_HEIGHT,
    image_data
  );
delay(2000);
  tft.fillScreen(TFT_BLACK);
}
