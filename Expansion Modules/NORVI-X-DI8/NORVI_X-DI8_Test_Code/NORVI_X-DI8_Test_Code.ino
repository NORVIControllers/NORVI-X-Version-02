#include <Wire.h>
#include <SPI.h>
#include <PCA9538.h>
#include <TFT_eSPI.h>
#include "Free_Fonts.h"

#define PCA9538_ADDR 0x73

#define SDA_PIN 8
#define SCL_PIN 9

#define MISO_PIN 13
#define MOSI_PIN 11
#define SCLK_PIN 12

#define PCA_RESET 21

PCA9538 io(PCA9538_ADDR);
TFT_eSPI tft = TFT_eSPI();

int oldState[8] = {-1,-1,-1,-1,-1,-1,-1,-1};

void drawLabel()
{
  tft.fillScreen(TFT_BLACK);

  tft.setTextColor(TFT_YELLOW);
  tft.setFreeFont(FSB12);

  tft.setCursor(50,20);
  tft.print("NORVI");

  tft.setCursor(10,45);
  tft.print("X-DI8 INPUT TEST");

  tft.setFreeFont(FSB9);

  tft.setCursor(0,80);   tft.print("DI1:");
  tft.setCursor(0,110);  tft.print("DI2:");
  tft.setCursor(0,140);  tft.print("DI3:");
  tft.setCursor(0,170);  tft.print("DI4:");
  tft.setCursor(0,200);  tft.print("DI5:");
  tft.setCursor(0,230);  tft.print("DI6:");
  tft.setCursor(0,260);  tft.print("DI7:");
  tft.setCursor(0,290);  tft.print("DI8:");
}

void updateInput(uint8_t ch, uint8_t value)
{
  int y = 80 + (ch * 30);

  tft.fillRect(100, y - 20, 80, 25, TFT_BLACK);

  if(value)
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
  else
    tft.setTextColor(TFT_RED, TFT_BLACK);

  tft.setCursor(100, y);
  tft.print(value);

  Serial.printf("DI%d = %d\n", ch + 1, value);
}

void setup()
{
  Serial.begin(115200);
  delay(1000);

  Serial.println("NORVI X-DI8 TEST");

  pinMode(PCA_RESET, OUTPUT);
  digitalWrite(PCA_RESET, HIGH);

  Wire.begin(SDA_PIN, SCL_PIN);

  io.pinMode(1, INPUT);
  io.pinMode(2, INPUT);
  io.pinMode(3, INPUT);
  io.pinMode(4, INPUT);
  io.pinMode(5, INPUT);
  io.pinMode(6, INPUT);
  io.pinMode(7, INPUT);
  io.pinMode(8, INPUT);

  SPI.begin(SCLK_PIN, MISO_PIN, MOSI_PIN);

  tft.init();
  tft.setRotation(0);

  drawLabel();

  for(int i=0;i<8;i++)
  {
    oldState[i] = -1;
  }
}

void loop()
{
  int state[8];

  state[0] = io.digitalRead(1);
  state[1] = io.digitalRead(2);
  state[2] = io.digitalRead(3);
  state[3] = io.digitalRead(4);
  state[4] = io.digitalRead(5);
  state[5] = io.digitalRead(6);
  state[6] = io.digitalRead(7);
  state[7] = io.digitalRead(8);

  for(int i=0;i<8;i++)
  {
    if(state[i] != oldState[i])
    {
      updateInput(i, state[i]);
      oldState[i] = state[i];
    }
  }

  delay(10);
}
