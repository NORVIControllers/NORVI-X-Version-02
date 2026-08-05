//"NPD-NORVI EXPE AI4-V2 (OPSRAM)

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

#define DSP_CS 47

#define ADS_ADDR 0x49     // DIP1(ON)-0X49  DIP2(ON)-0x4B  DIP3(ON)-0x4A  DIP4(ON)-0x48

Adafruit_ADS1115 ads1;
const float mA_Factor = 4.096 / 3269.826;

CST816S touch(8, 9, 45, 48);  // sda, scl, rst, irq

TFT_eSPI tft = TFT_eSPI();

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("NPD-NORVI X-AI4 TEST");
  delay(1000);

  Wire.begin(SDA, SCL);
  delay(100);


if (!ads1.begin(ADS_ADDR)) {
    Serial.println("Failed to initialize ADS 1 .");
    // while (1);
  }
   ads1.setGain(GAIN_ONE);

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

  tft.setCursor(40, 40);      //xpos, ypos
  tft.print("X-AI4 TEST");
}

void loop() {
  
  int16_t adc0, adc1, adc2, adc3;

  adc0 = ads1.readADC_SingleEnded(0);
  adc1 = ads1.readADC_SingleEnded(1);
  adc2 = ads1.readADC_SingleEnded(2);
  adc3 = ads1.readADC_SingleEnded(3);

  float current0 = adc0 * mA_Factor;
  float current1 = adc1 * mA_Factor;
  float current2 = adc2 * mA_Factor;
  float current3 = adc3 * mA_Factor;

  
  tft.fillScreen(TFT_BLACK);            // Clear screen
  tft.setTextColor(TFT_GREEN);
  tft.setFreeFont(FSB12);
  
  tft.setCursor(60, 20);      //xpos, ypos
  tft.print("NORVI");

  tft.setCursor(40, 40);      //xpos, ypos
  tft.print("X-AI4 TEST");

  tft.setTextColor(TFT_YELLOW);
  tft.setCursor(10, 100);      //xpos, ypos
  tft.setFreeFont(FSB9);
  tft.print("CURRENT 1 :");

  tft.setCursor(10, 130);      //xpos, ypos
  tft.print("CURRENTE 2 :");
  
  tft.setCursor(10, 160);      //xpos, ypos
  tft.print("CURRENT 3 :");
  
  tft.setCursor(10, 190);      //xpos, ypos
  tft.print("CURRENT 4 :");
  
//----------------------------------
  tft.setTextColor(TFT_WHITE);
  
  tft.setCursor(170, 100);      //xpos, ypos
  tft.print(String(current0, 2) + " mA");
  Serial.println(String("CURRENT 4 : ") + String(current0, 2) + " mA");
  delay(500);
  
  tft.setCursor(170, 130);      //xpos, ypos
  tft.print(String(current1, 2) + " mA");
  Serial.println(String("CURRENT 4 : ") + String(current1, 2) + " mA");
  delay(500);
  

  tft.setCursor(170, 160);      //xpos, ypos
  tft.print(String(current2, 2) + " mA");
  Serial.println(String("CURRENT 4 : ") + String(current2, 2) + " mA");
  delay(500);
  

  tft.setCursor(170, 190);
  tft.print(String(current3, 2) + " mA");
  Serial.println(String("CURRENT 4 : ") + String(current3, 2) + " mA");
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
