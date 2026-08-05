//"NPD-NORVI X-Q4"

#include <Wire.h>
#include <WiFi.h>
#include <SPI.h>
#include "TFT_eSPI.h"
#include <CST816S.h>
#include "Free_Fonts.h" // Include the header file attached to this sketch

#define SDA   8     
#define SCL   9     

#define MISO 13
#define MOSI 11
#define SCLK 12

#define DSP_CS 45

#define TR1 14
#define TR2 35
#define TR3 41
#define TR4 37

CST816S touch(8, 9, 47, 48);  // sda, scl, rst, irq

TFT_eSPI tft = TFT_eSPI();

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("NPD-NORVI EXPE-AIX TEST");
  delay(1000);

  Wire.begin(SDA, SCL);
  delay(100);

  //I2C_SCAN();
  //delay(1000);

  pinMode(TR1, OUTPUT);
  pinMode(TR2, OUTPUT);
  pinMode(TR3, OUTPUT);
  pinMode(TR4, OUTPUT);

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
  tft.print("EXPE-QX TEST");
}

void loop() {

  tft.fillScreen(TFT_BLACK);            // Clear screen
  tft.setTextColor(TFT_GREEN);
  tft.setFreeFont(FSB12);
  
  tft.setCursor(60, 20);      //xpos, ypos
  tft.print("NORVI");

  tft.setCursor(20, 40);      //xpos, ypos
  tft.print("EXPE-QX TEST");

  tft.setTextColor(TFT_YELLOW);
  tft.setCursor(0, 100);      //xpos, ypos
  tft.setFreeFont(FSB9);
  tft.print("OUTPUT 1 :");

  tft.setCursor(0, 130);      //xpos, ypos
  tft.print("OUTPUT 2 :");
  
  tft.setCursor(0, 160);      //xpos, ypos
  tft.print("OUTPUT 3 :");
  
  tft.setCursor(0, 190);      //xpos, ypos
  tft.print("OUTPUT 4 :");
  

  tft.setTextColor(TFT_WHITE);
  tft.setCursor(120, 100);      //xpos, ypos
  tft.print("ON");
  Serial.println("OUTPUT 1 : ON");
  digitalWrite(TR1, HIGH);
  delay(500);
  tft.setCursor(170, 100);      //xpos, ypos
  tft.print("OFF");
  Serial.println("OUTPUT 1: OFF");
  digitalWrite(TR1, LOW);
  delay(500);

  tft.setCursor(120, 130);      //xpos, ypos
  tft.print("ON");
  Serial.println("OUTPUT 2 : ON");
  digitalWrite(TR2, HIGH);
  delay(500);
  tft.setCursor(170, 130);      //xpos, ypos
  tft.print("OFF");
  Serial.println("OUTPUT 2 : OFF");
  digitalWrite(TR2, LOW);
  delay(500);
  
  tft.setCursor(120, 160);      //xpos, ypos
  tft.print("ON");
  Serial.println("OUTPUT 3 : ON");
  digitalWrite(TR3, HIGH);
  delay(500);
  tft.setCursor(170, 160);      //xpos, ypos
  tft.print("OFF");
  Serial.println("OUTPUT 3 : OFF");
  digitalWrite(TR3, LOW);
  delay(500);
  

  tft.setCursor(120, 190);      //xpos, ypos
  tft.print("ON");
  Serial.println("OUTPUT 4 : ON");
  digitalWrite(TR4, HIGH);
  delay(500);
  tft.setCursor(170, 190);      //xpos, ypos
  tft.print("OFF");
  Serial.println("OUTPUT 4 : OFF");
  digitalWrite(TR4, LOW);
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
