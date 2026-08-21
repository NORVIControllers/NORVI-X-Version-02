
//"NPD-NORVI X-R8"
//2025.11.01

#include <Wire.h>
#include <WiFi.h>
#include <SPI.h>
#include <PCA9538.h>
#include "TFT_eSPI.h"
#include <CST816S.h>
#include "Free_Fonts.h" // Include the header file attached to this sketch

// I2C address of PCA9538
#define PCA9538_ADDR 0x73

#define SDA   8     
#define SCL   9     

#define MISO 37
#define MOSI 35
#define SCLK 36

#define DSP_CS 45

#define PCA_RESET 21

#define TR1   1 
#define TR2   2
#define TR3   3
#define TR4   4
#define TR5   5
#define TR6   6
#define TR7   7
#define TR8   8

PCA9538 io(PCA9538_ADDR);

CST816S touch(8, 9, 47, 48);  // sda, scl, rst, irq

TFT_eSPI tft = TFT_eSPI();

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("NPD-NORVI X-DI8 TEST");
  delay(1000);

  pinMode(PCA_RESET, OUTPUT);
  delay(100);
  digitalWrite(PCA_RESET, HIGH);

  Wire.begin(SDA, SCL);
  delay(100);

  I2C_SCAN();
  delay(1000);

  io.pinMode(TR1, OUTPUT);
  io.pinMode(TR2, OUTPUT);
  io.pinMode(TR3, OUTPUT);
  io.pinMode(TR4, OUTPUT);
  io.pinMode(TR5, OUTPUT);
  io.pinMode(TR6, OUTPUT);
  io.pinMode(TR7, OUTPUT);
  io.pinMode(TR8, OUTPUT);

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

  tft.setCursor(10, 40);      //xpos, ypos
  tft.print("EXPE-X-Q8 TEST");
}

void loop() {

  tft.fillScreen(TFT_BLACK);            // Clear screen
  tft.setTextColor(TFT_GREEN);
  tft.setFreeFont(FSB12);
  
  tft.setCursor(60, 20);      //xpos, ypos
  tft.print("NORVI");

  tft.setCursor(20, 40);      //xpos, ypos
  tft.print("EXPE-Q8 TEST");

  tft.setTextColor(TFT_YELLOW);
  tft.setCursor(0, 70);      //xpos, ypos
  tft.setFreeFont(FSB9);
  tft.print("TR 1 :");

  tft.setCursor(0, 100);      //xpos, ypos
  tft.print("TR 2 :");
  
  tft.setCursor(0, 130);      //xpos, ypos
  tft.print("TR 3 :");
  
  tft.setCursor(0, 160);      //xpos, ypos
  tft.print("TR 4 :");
  
  tft.setCursor(0, 190);      //xpos, ypos
  tft.print("TR 5 :");
  
  tft.setCursor(0, 220);      //xpos, ypos
  tft.print("TR 6 :");
  
  tft.setCursor(0, 250);      //xpos, ypos
  tft.print("TR 7 :");
  
  tft.setCursor(0, 280);      //xpos, ypos
  tft.print("TR 8 :");
 
  
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(120, 70);      //xpos, ypos
  tft.print("ON");
  Serial.println("TR 1 : ON");
  io.digitalWrite(TR1,HIGH);
  delay(500);
  tft.setCursor(170, 70);      //xpos, ypos
  tft.print("OFF");
  Serial.println("TR 1 : OFF");
  io.digitalWrite(TR1,LOW);
  delay(500);

  tft.setCursor(120, 100);      //xpos, ypos
  tft.print("ON");
  Serial.println("TR 2 : ON");
  io.digitalWrite(TR2,HIGH);
  delay(500);
  tft.setCursor(170, 100);      //xpos, ypos
  tft.print("OFF");
  Serial.println("TR 2 : OFF");
  io.digitalWrite(TR2,LOW);
  delay(500);
  

  tft.setCursor(120, 130);      //xpos, ypos
  tft.print("ON");
  Serial.println("TR 3 : ON");
  io.digitalWrite(TR3,HIGH);
  delay(500);
  tft.setCursor(170, 130);      //xpos, ypos
  tft.print("OFF");
  Serial.println("TR 3 : OFF");
  io.digitalWrite(TR3,LOW);
  delay(500);
  

  tft.setCursor(120, 160);      //xpos, ypos
  tft.print("ON");
  Serial.println("TR 4 : ON");
  io.digitalWrite(TR4,HIGH);
  delay(500);
  tft.setCursor(170, 160);      //xpos, ypos
  tft.print("OFF");
  Serial.println("TR 4 : OFF");
  io.digitalWrite(TR4,LOW);
  delay(500);

  tft.setCursor(120, 190);      //xpos, ypos
  tft.print("ON");
  Serial.println("TR 5 : ON");
  io.digitalWrite(TR5,HIGH);
  delay(500);
  tft.setCursor(170, 190);      //xpos, ypos
  tft.print("OFF");
  Serial.println("TR 5 : OFF");
  io.digitalWrite(TR5,LOW);
  delay(500);

  tft.setCursor(120, 220);      //xpos, ypos
  tft.print("ON");
  Serial.println("TR 6 : ON");
  io.digitalWrite(TR6,HIGH);
  delay(500);
  tft.setCursor(170, 220);      //xpos, ypos
  tft.print("OFF");
  Serial.println("TR 6 : OFF");
  io.digitalWrite(TR6,LOW);
  delay(500);

  tft.setCursor(120, 250);      //xpos, ypos
  tft.print("ON");
  Serial.println("TR 7 : ON");
  io.digitalWrite(TR7,HIGH);
  delay(500);
  tft.setCursor(170, 250);      //xpos, ypos
  tft.print("OFF");
  Serial.println("TR 7 : OFF");
  io.digitalWrite(TR7,LOW);
  delay(500);

  tft.setCursor(120, 280);      //xpos, ypos
  tft.print("ON");
  Serial.println("TR 8 : ON");
  io.digitalWrite(TR8,HIGH);
  delay(500);
  tft.setCursor(170, 280);      //xpos, ypos
  tft.print("OFF");
  Serial.println("TR 8 : OFF");
  io.digitalWrite(TR8,LOW);
  delay(500);
  
  
  Serial.println("........................................................................");

  delay(100);
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
