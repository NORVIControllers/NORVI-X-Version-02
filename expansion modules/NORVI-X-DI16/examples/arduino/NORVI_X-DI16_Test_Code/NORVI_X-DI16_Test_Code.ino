
/*
 * NORVI X-DI16 N16R2
 * 2026.07.20
 */

#include <Wire.h>
#include <WiFi.h>
#include <SPI.h>
#include "clsPCA9555.h"
#include "TFT_eSPI.h"
#include <CST816S.h>
#include "Free_Fonts.h" // Include the header file attached to this sketch

PCA9555 ioport(0x27); // Base address starts at 0x74 for A0 = L and A1 = L

#define SDA   8
#define SCL   9

#define MISO 13
#define MOSI 11
#define SCLK 12

#define DSP_CS 45

#define PCA_A0 0
#define PCA_A1 1
#define PCA_A2 2
#define PCA_A3 3
#define PCA_A4 4
#define PCA_A5 5
#define PCA_A6 6
#define PCA_A7 7

#define PCA_B0 8
#define PCA_B1 9
#define PCA_B2 10
#define PCA_B3 11
#define PCA_B4 12
#define PCA_B5 13
#define PCA_B6 14
#define PCA_B7 15


CST816S touch(8, 9, 47, 48);  // sda, scl, rst, irq

TFT_eSPI tft = TFT_eSPI();

int oldState[16] =
{
  -1, -1, -1, -1,
  -1, -1, -1, -1,
  -1, -1, -1, -1,
  -1, -1, -1, -1
};


void setup() {
  Serial.begin(115200);
  delay(100);

  Serial.println("NPD-NORVI X-DI16 TEST");
  delay(1000);


  Wire.begin(SDA, SCL);
  delay(200);

  I2C_SCAN();
  delay(1000);

  ioport.pinMode(PCA_A0, INPUT);
  ioport.pinMode(PCA_A1, INPUT);
  ioport.pinMode(PCA_A2, INPUT);
  ioport.pinMode(PCA_A3, INPUT);
  ioport.pinMode(PCA_A4, INPUT);
  ioport.pinMode(PCA_A5, INPUT);
  ioport.pinMode(PCA_A6, INPUT);
  ioport.pinMode(PCA_A7, INPUT);

  ioport.pinMode(PCA_B0, INPUT);
  ioport.pinMode(PCA_B1, INPUT);
  ioport.pinMode(PCA_B2, INPUT);
  ioport.pinMode(PCA_B3, INPUT);
  ioport.pinMode(PCA_B4, INPUT);
  ioport.pinMode(PCA_B5, INPUT);
  ioport.pinMode(PCA_B6, INPUT);
  ioport.pinMode(PCA_B7, INPUT);

  SPI.begin(SCLK, MISO, MOSI); // Ensure these pin numbers are correct
  delay(1000);

  tft.init();
  tft.begin();
  tft.setRotation(0);
  drawLabel();


}

void loop()
{
  int state[16];

  // Read Inputs
  state[0]  = ioport.digitalRead(PCA_A0);
  state[1]  = ioport.digitalRead(PCA_A1);
  state[2]  = ioport.digitalRead(PCA_A2);
  state[3]  = ioport.digitalRead(PCA_A3);
  state[4]  = ioport.digitalRead(PCA_A4);
  state[5]  = ioport.digitalRead(PCA_A5);
  state[6]  = ioport.digitalRead(PCA_A6);
  state[7]  = ioport.digitalRead(PCA_A7);

  state[8]  = ioport.digitalRead(PCA_B0);
  state[9]  = ioport.digitalRead(PCA_B1);
  state[10] = ioport.digitalRead(PCA_B2);
  state[11] = ioport.digitalRead(PCA_B3);
  state[12] = ioport.digitalRead(PCA_B4);
  state[13] = ioport.digitalRead(PCA_B5);
  state[14] = ioport.digitalRead(PCA_B6);
  state[15] = ioport.digitalRead(PCA_B7);

  // Update only changed values
  for (int i = 0; i < 16; i++)
  {
    if (state[i] != oldState[i])
    {
      updateInput(i, state[i]);
      oldState[i] = state[i];
    }
  }

  delay(10);
}

void updateInput(uint8_t ch, uint8_t value)
{
  int x, y;

  if(ch < 8)
  {
    x = 60;
    y = 80 + (ch * 30);
  }
  else
  {
    x = 190;
    y = 80 + ((ch - 8) * 30);
  }

  tft.fillRect(x, y - 18, 20, 20, TFT_BLACK);

  if(value)
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
  else
    tft.setTextColor(TFT_RED, TFT_BLACK);

  tft.setCursor(x, y);
  tft.print(value);
}

void drawLabel()
{
  tft.fillScreen(TFT_BLACK);

  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setFreeFont(FSB12);

  tft.setCursor(50,20);
  tft.print("  NORVI");

  tft.setCursor(10,45);
  tft.print("X-DI16 INPUT TEST");

  tft.setFreeFont(FSB9);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  // Left side
  tft.setCursor(0,80);   tft.print("DI1:");
  tft.setCursor(0,110);  tft.print("DI2:");
  tft.setCursor(0,140);  tft.print("DI3:");
  tft.setCursor(0,170);  tft.print("DI4:");
  tft.setCursor(0,200);  tft.print("DI5:");
  tft.setCursor(0,230);  tft.print("DI6:");
  tft.setCursor(0,260);  tft.print("DI7:");
  tft.setCursor(0,290);  tft.print("DI8:");

  // Right side
  tft.setCursor(120,80);   tft.print("DI9:");
  tft.setCursor(120,110);  tft.print("DI10:");
  tft.setCursor(120,140);  tft.print("DI11:");
  tft.setCursor(120,170);  tft.print("DI12:");
  tft.setCursor(120,200);  tft.print("DI13:");
  tft.setCursor(120,230);  tft.print("DI14:");
  tft.setCursor(120,260);  tft.print("DI15:");
  tft.setCursor(120,290);  tft.print("DI16:");
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
