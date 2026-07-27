
//"NPD-NORVI X-DI16"
//2025.10.21

#include <Wire.h>
#include <WiFi.h>
#include <SPI.h>
#include "clsPCA9555.h"
#include "TFT_eSPI.h"
#include <CST816S.h>
#include "Free_Fonts.h" // Include the header file attached to this sketch

#define PCA_ADDR 0x27 // Base address starts at 0x74 for A0 = L and A1 = L

PCA9555 ioport(PCA_ADDR);

#define SDA   8     
#define SCL   9     

#define MISO 37
#define MOSI 35
#define SCLK 36

#define DSP_CS 45

#define INPUT1 0
#define INPUT2 1
#define INPUT3 2
#define INPUT4 3
#define INPUT5 4
#define INPUT6 5
#define INPUT7 6
#define INPUT8 7

#define INPUT9  8
#define INPUT10 9
#define INPUT11 10
#define INPUT12 11
#define INPUT13 12
#define INPUT14 13
#define INPUT15 14
#define INPUT16 15


CST816S touch(8, 9, 47, 48);  // sda, scl, rst, irq

TFT_eSPI tft = TFT_eSPI();

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("NPD-NORVI X-DI16 TEST");
  delay(1000);

  Wire.begin(SDA, SCL);
  delay(100);

  I2C_SCAN();
  delay(100);

  ioport.pinMode(INPUT1, INPUT);
  ioport.pinMode(INPUT2, INPUT);
  ioport.pinMode(INPUT3, INPUT);
  ioport.pinMode(INPUT4, INPUT);
  ioport.pinMode(INPUT5, INPUT);
  ioport.pinMode(INPUT6, INPUT);
  ioport.pinMode(INPUT7, INPUT);
  ioport.pinMode(INPUT8, INPUT);
  ioport.pinMode(INPUT9, INPUT);
  ioport.pinMode(INPUT10, INPUT);
  ioport.pinMode(INPUT11, INPUT);
  ioport.pinMode(INPUT12, INPUT);
  ioport.pinMode(INPUT13, INPUT);
  ioport.pinMode(INPUT14, INPUT);
  ioport.pinMode(INPUT15, INPUT);
  ioport.pinMode(INPUT16, INPUT);

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
  tft.print("x-DI16 TEST");
}

void loop() {

  tft.fillScreen(TFT_BLACK);            // Clear screen
  tft.setTextColor(TFT_GREEN);
  tft.setFreeFont(FSB12);
  
  tft.setCursor(60, 20);      //xpos, ypos
  tft.print("NORVI");

  tft.setCursor(20, 40);      //xpos, ypos
  tft.print("X-DI16 TEST");

  tft.setTextColor(TFT_YELLOW);
  tft.setCursor(10, 70);      //xpos, ypos
  tft.setFreeFont(FSB9);
  tft.print("I1      -----------");

  tft.setCursor(10, 85);      //xpos, ypos
  tft.print("I2      -----------");

  tft.setCursor(10, 100);      //xpos, ypos
  tft.print("I3      -----------");

  tft.setCursor(10, 115);      //xpos, ypos
  tft.print("I4      -----------");

  tft.setCursor(10, 130);      //xpos, ypos
  tft.print("I5      -----------");

  tft.setCursor(10, 145);      //xpos, ypos
  tft.print("I6      -----------");

  tft.setCursor(10, 160);      //xpos, ypos
  tft.print("I7      -----------");

  tft.setCursor(10, 175);      //xpos, ypos
  tft.print("I8      -----------");

  tft.setCursor(10, 190);      //xpos, ypos
  tft.print("I9      -----------");

  tft.setCursor(10, 205);      //xpos, ypos
  tft.print("I10    -----------");

  tft.setCursor(10, 220);      //xpos, ypos
  tft.print("I11    -----------");

  tft.setCursor(10, 235);      //xpos, ypos
  tft.print("I12    -----------");

  tft.setCursor(10, 250);      //xpos, ypos
  tft.print("I13    -----------");

  tft.setCursor(10, 265);      //xpos, ypos
  tft.print("I14    -----------");

  tft.setCursor(10, 280);      //xpos, ypos
  tft.print("I15    -----------");

  tft.setCursor(10, 295);      //xpos, ypos
  tft.print("I16   -----------");

  tft.setTextColor(TFT_GREEN);
  tft.setCursor(140, 70);      //xpos, ypos
  tft.print(ioport.digitalRead(INPUT1));
  Serial.printf("Input1: %d\n",ioport.digitalRead(INPUT1));

  tft.setCursor(140, 85);      //xpos, ypos
  tft.print(ioport.digitalRead(INPUT2));
  Serial.printf("Input2: %d\n",ioport.digitalRead(INPUT2));

  tft.setCursor(140, 100);      //xpos, ypos
  tft.print(ioport.digitalRead(INPUT3));
  Serial.printf("Input3: %d\n",ioport.digitalRead(INPUT3));

  tft.setCursor(140, 115);      //xpos, ypos
  tft.print(ioport.digitalRead(INPUT4));
  Serial.printf("Input4: %d\n",ioport.digitalRead(INPUT4));

  tft.setCursor(140, 130);      //xpos, ypos
  tft.print(ioport.digitalRead(INPUT5));
  Serial.printf("Input5: %d\n",ioport.digitalRead(INPUT5));

  tft.setCursor(140, 145);      //xpos, ypos
  tft.print(ioport.digitalRead(INPUT6));
  Serial.printf("Input6: %d\n",ioport.digitalRead(INPUT6));

  tft.setCursor(140, 160);      //xpos, ypos
  tft.print(ioport.digitalRead(INPUT7));
  Serial.printf("Input7: %d\n",ioport.digitalRead(INPUT7));

  tft.setCursor(140, 175);      //xpos, ypos
  tft.print(ioport.digitalRead(INPUT8));
  Serial.printf("Input8: %d\n",ioport.digitalRead(INPUT8));

  tft.setCursor(140, 190);      //xpos, ypos
  tft.print(ioport.digitalRead(INPUT9));
  Serial.printf("Input9: %d\n",ioport.digitalRead(INPUT9));

  tft.setCursor(140, 205);      //xpos, ypos
  tft.print(ioport.digitalRead(INPUT10));
  Serial.printf("Input10: %d\n",ioport.digitalRead(INPUT10));

  tft.setCursor(140, 220);      //xpos, ypos
  tft.print(ioport.digitalRead(INPUT11));
  Serial.printf("Input11: %d\n",ioport.digitalRead(INPUT11));

  tft.setCursor(140, 235);      //xpos, ypos
  tft.print(ioport.digitalRead(INPUT12));
  Serial.printf("Input12: %d\n",ioport.digitalRead(INPUT12));

  tft.setCursor(140, 250);      //xpos, ypos
  tft.print(ioport.digitalRead(INPUT13));
  Serial.printf("Input13: %d\n",ioport.digitalRead(INPUT13));

  tft.setCursor(140, 265);      //xpos, ypos
  tft.print(ioport.digitalRead(INPUT14));
  Serial.printf("Input14: %d\n",ioport.digitalRead(INPUT14));

  tft.setCursor(140, 280);      //xpos, ypos
  tft.print(ioport.digitalRead(INPUT15));
  Serial.printf("Input15: %d\n",ioport.digitalRead(INPUT15));

  tft.setCursor(140, 295);      //xpos, ypos
  tft.print(ioport.digitalRead(INPUT16));
  Serial.printf("Input16: %d\n",ioport.digitalRead(INPUT16));
  Serial.println("........................................................................");

  delay(10);
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
