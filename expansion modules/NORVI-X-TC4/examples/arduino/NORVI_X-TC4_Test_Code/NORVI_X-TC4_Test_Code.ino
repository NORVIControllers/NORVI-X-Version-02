// NPD-NORVI EXPE TC4 – MASTER 

#include <Wire.h>
#include <SPI.h>
#include "TFT_eSPI.h"
#include "Free_Fonts.h"
#include <CST816S.h>

// ---------------- PINS ----------------
#define SDA   8
#define SCL   9

#define MISO 13
#define MOSI 11
#define SCLK 12

#define DSP_CS 45

#define TC_SLAVE_ADDR 0x18
//uint8_t tcSlaveAddrs[] = {0x18, 0x17, 0x16};   // one entry per physical TC4 board in use (when there is multiple devices)

TFT_eSPI tft = TFT_eSPI();

// ---------------- CONFIG ----------------
uint8_t channelsToRead[4] = {1};    // default channel 1
uint8_t numChannels = 1;

// ---------------- DATA ----------------
float tcTemp[4] = {0};
uint8_t tcFault[4] = {0};
bool configMode = false;

uint8_t i2cFailCount = 0;

// ---------------- TIMER ----------------
unsigned long lastRead = 0;
#define READ_INTERVAL 1000

CST816S touch(8, 9, 45, 48);

// ---------------- CRC ----------------
uint8_t crc8(uint8_t *data, int len)
{
  uint8_t crc = 0x00;
  while (len--)
  {
    uint8_t extract = *data++;
    for (uint8_t i = 8; i; i--)
    {
      uint8_t sum = (crc ^ extract) & 0x01;
      crc >>= 1;
      if (sum) crc ^= 0x8C;
      extract >>= 1;
    }
  }
  return crc;
}

// ---------------- DISPLAY ----------------
void drawHeader()
{
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_YELLOW);
  tft.setFreeFont(FSB12);

  tft.setCursor(60, 20);
  tft.print("NORVI");

  tft.setCursor(20, 40);
  tft.print("EXPE-TC4 TEST");
}

void drawValues()
{
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_YELLOW);
  tft.setFreeFont(FSB9);

  int y = 20;
  tft.setCursor(0, y);
  tft.println("NORVI EXPE-TC MASTER");

  y += 20;
  for (int i = 0; i < numChannels; i++)
  {
      uint8_t ch = channelsToRead[i];

      tft.setCursor(0, y);
      tft.setTextColor(TFT_WHITE);
      tft.print("Ch "); tft.print(ch); tft.print(" : ");

      tft.setTextColor(TFT_GREEN);
      tft.print("Temp: "); tft.print(tcTemp[ch-1], 2); tft.println(" C ");

      y += 20;

      tft.setTextColor(TFT_RED);
      tft.print("Fault: "); tft.println(tcFault[ch-1]);

      y += 20;
  }
}

// ---------------- SERIAL ----------------
void handleSerial()
{
  if (Serial.available())
  {
    char c = Serial.read();
    if (c == 'C' || c == 'c') enterConfig();
  }
}

void i2cBusRecover()
{
    Wire.end();
    pinMode(SCL, OUTPUT);
    pinMode(SDA, INPUT_PULLUP);
    for (int i = 0; i < 9; i++) {               // clock out a stuck slave
        digitalWrite(SCL, HIGH); delayMicroseconds(5);
        digitalWrite(SCL, LOW);  delayMicroseconds(5);
        if (digitalRead(SDA) == HIGH) break;
    }
    pinMode(SDA, OUTPUT);                        // manual STOP
    digitalWrite(SDA, LOW);  delayMicroseconds(5);
    digitalWrite(SCL, HIGH); delayMicroseconds(5);
    digitalWrite(SDA, HIGH); delayMicroseconds(5);
    Wire.begin(SDA, SCL);
}

// ---------------- READ SLAVE (CHUNKED) ----------------
#define CHUNK_SIZE 4

void readTC()
{
  for (int chunkStart = 0; chunkStart < numChannels; chunkStart += CHUNK_SIZE)
  {
      uint8_t chunkCount = min(CHUNK_SIZE, numChannels - chunkStart);

      Wire.beginTransmission(TC_SLAVE_ADDR);
      Wire.write(0x01);        // command

      for (int i = 0; i < chunkCount; i++)
      {
          Wire.write(channelsToRead[chunkStart + i]);
      }

      if (Wire.endTransmission() != 0)
      {
          Serial.println("I2C TX FAIL");
          if (++i2cFailCount >= 5) { i2cBusRecover(); i2cFailCount = 0; }
          continue;
      }
      i2cFailCount = 0;

      delay(15);

      uint8_t totalBytes = chunkCount * 8;
      Wire.requestFrom(TC_SLAVE_ADDR, totalBytes);

      if (Wire.available() != totalBytes)
      {
          Serial.println("I2C RX FAIL");
          continue;
      }

      

      for (int idx = 0; idx < chunkCount; idx++)
      {
          uint8_t buf[8];
          for (int i = 0; i < 8; i++) buf[i] = Wire.read();

          if (crc8(buf, 7) != buf[7])
          {
              Serial.print("CRC ERROR Channel ");
              Serial.println(buf[1]);
              continue;
          }

          float temp;
          memcpy(&temp, &buf[2], 4);
          uint8_t ch = buf[1];

          tcTemp[ch-1] = temp;
          tcFault[ch-1] = buf[6];

          Serial.print("Ch ");
          Serial.print(ch);
          Serial.print(" Temp: "); Serial.print(temp);
          Serial.print(" Fault: "); Serial.println(buf[6]);
      }
  }

  drawValues();
}



// ---------------- SETUP ----------------
void setup()
{
  Serial.begin(115200);
  delay(1000);

  Wire.begin(SDA, SCL);
  delay(100);
  SPI.begin(SCLK, MISO, MOSI);
  delay(1000);

  tft.init();
  tft.begin();
  tft.setRotation(0);

  drawHeader();
  printCurrentConfig();

  Serial.println("NORVI TC MASTER STARTED");
}

// ---------------- LOOP ----------------
unsigned long lastChunkRead = 0;
void loop()
{
  handleSerial();

  if (!configMode && millis() - lastChunkRead >= READ_INTERVAL)
  {
      lastChunkRead = millis();
      readTC();
  }
}

// ---------------- CONFIG ----------------
void enterConfig() {
    configMode = true;
    Serial.println("\n===== CONFIG MODE =====");
    Serial.println("Enter Channels to read:");
    Serial.println("Examples: 1 for Ch1, 1-3 for Ch1,2,3, 1,3 for Ch1 and Ch3");

    while (Serial.available()) Serial.read(); // flush buffer
    String input = "";
    while (input.length() == 0)
    {
        if (Serial.available())
        {
            input = Serial.readStringUntil('\n');
            input.trim();
        }
        delay(10);
    }

    numChannels = 0;
    int dashIndex = input.indexOf('-');
    if (dashIndex != -1)
    {
        int startCh = input.substring(0, dashIndex).toInt();
        int endCh   = input.substring(dashIndex + 1).toInt();
        for (int i = startCh; i <= endCh; i++)
            channelsToRead[numChannels++] = i;
    }
    else
    {
        int start = 0;
        while (start < input.length() && numChannels < 4)
        {
            int commaPos = input.indexOf(',', start);
            String chPart = (commaPos == -1) ? input.substring(start) : input.substring(start, commaPos);
            int ch = chPart.toInt();
            if (ch >= 1 && ch <= 4) channelsToRead[numChannels++] = ch;
            if (commaPos == -1) break;
            start = commaPos + 1;
        }
    }

    Serial.println("\nConfiguration Saved:");
    printCurrentConfig();
    configMode = false;
}

// ---------------- PRINT CONFIG ----------------
void printCurrentConfig() {
    Serial.print("Channels : ");
    for (int i = 0; i < numChannels; i++)
    {
        Serial.print(channelsToRead[i]);
        if (i < numChannels - 1) Serial.print(", ");
    }
    Serial.println();
}
