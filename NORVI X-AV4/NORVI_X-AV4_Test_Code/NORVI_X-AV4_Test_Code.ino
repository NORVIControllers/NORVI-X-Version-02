/*
 * 2026/05/13
 * NPD-NORVI EXPE AV4 
 * N16R2 
 * 
*/

#include <Wire.h>
#include <WiFi.h>
#include <SPI.h>
#include <Adafruit_ADS1X15.h>
#include "TFT_eSPI.h"
#include <CST816S.h>
#include "Free_Fonts.h" // Include the header file attached to this sketch

#define SDA   8     
#define SCL   9     

#define MISO 13
#define MOSI 11
#define SCLK 12

#define DSP_CS 45

Adafruit_ADS1115 ads1;
#define VOLTAGE_DIVIDER_RATIO 0.2065

CST816S touch(8, 9, 47, 48);  // sda, scl, rst, irq

TFT_eSPI tft = TFT_eSPI();

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("NPD-NORVI EXPE-AIX TEST");
  delay(1000);


  Wire.begin(SDA, SCL);
  delay(100);


if (!ads1.begin(0x48)) {
    Serial.println("Failed to initialize ADS 1 .");
    // while (1);
  }
 

  SPI.begin(SCLK, MISO, MOSI); // Ensure these pin numbers are correct 
  delay(1000);

  tft.init();
  tft.begin();
  tft.setRotation(0);

  tft.fillScreen(TFT_BLACK);            // Clear screen
  tft.setTextColor(TFT_YELLOW);
  tft.setFreeFont(FSB12);
  
  tft.setCursor(60, 20);      //xpos, ypos
  tft.print("NORVI");

  tft.setCursor(20, 40);      //xpos, ypos
  tft.print("EXPE-AV4 TEST");
}

void loop() {
  
  int16_t adc0, adc1, adc2, adc3;

  adc0 = ads1.readADC_SingleEnded(0);
  adc1 = ads1.readADC_SingleEnded(1);
  adc2 = ads1.readADC_SingleEnded(2);
  adc3 = ads1.readADC_SingleEnded(3);

  float Voltage0 = adc0 * 0.125 / 1000.0 / VOLTAGE_DIVIDER_RATIO;
  float Voltage1 = adc1 * 0.125 / 1000.0 / VOLTAGE_DIVIDER_RATIO;
  float Voltage2 = adc2 * 0.125 / 1000.0 / VOLTAGE_DIVIDER_RATIO;
  float Voltage3 = adc3 * 0.125 / 1000.0 / VOLTAGE_DIVIDER_RATIO;

  
  tft.fillScreen(TFT_BLACK);            // Clear screen
  tft.setTextColor(TFT_GREEN);
  tft.setFreeFont(FSB12);
  
  tft.setCursor(60, 20);      //xpos, ypos
  tft.print("NORVI");

  tft.setCursor(20, 40);      //xpos, ypos
  tft.print("EXPE-AV4 TEST");

  tft.setTextColor(TFT_YELLOW);
  tft.setCursor(10, 100);      //xpos, ypos
  tft.setFreeFont(FSB9);
  tft.print("VOLTAGE 1 :");

  tft.setCursor(10, 130);      //xpos, ypos
  tft.print("VOLTAGE 2 :");
  
  tft.setCursor(10, 160);      //xpos, ypos
  tft.print("VOLTAGE 3 :");
  
  tft.setCursor(10, 190);      //xpos, ypos
  tft.print("VOLTAGE 4 :");
  
//----------------------------------
  tft.setTextColor(TFT_WHITE);
  
  tft.setCursor(170, 100);      //xpos, ypos
  tft.print(String(Voltage0, 2) + " V");
  Serial.println(String("VOLTAGE 1 : ") + String(Voltage0, 2) + " V");
  delay(500);
  
  tft.setCursor(170, 130);      //xpos, ypos
  tft.print(String(Voltage1, 2) + " V");
  Serial.println(String("VOLTAGE 2 : ") + String(Voltage1, 2) + " V");
  delay(500);
  

  tft.setCursor(170, 160);      //xpos, ypos
  tft.print(String(Voltage2, 2) + " V");
  Serial.println(String("VOLTAGE 3 : ") + String(Voltage2, 2) + " V");
  delay(500);
  

  tft.setCursor(170, 190);
  tft.print(String(Voltage3, 2) + " V");
  Serial.println(String("VOLTAGE 4 : ") + String(Voltage3, 2) + " V");
  delay(500);


}

void I2C_SCAN() {
  byte error, address;
  int deviceCount = 0;

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
      Serial.println("  !");

      deviceCount++;
      delay(1);  // Wait for a moment to avoid overloading the I2C bus
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
    Serial.println("No I2C devices found\n");
  }
  else {
    Serial.println("Scanning complete\n");
  }
}
