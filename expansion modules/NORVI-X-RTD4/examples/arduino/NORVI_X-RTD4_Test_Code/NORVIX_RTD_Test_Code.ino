// NPD-NORVI EXPE RTD – MASTER

#include <Wire.h>
#include <SPI.h>

// ===================== LovyanGFX display setup =====================
#define LGFX_USE_V1
#include <LovyanGFX.hpp>

class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ST7789 _panel;
  lgfx::Bus_SPI      _bus;
public:
  LGFX(void) {
    { // ---- Display shares the main SPI bus with SD + Ethernet ----
      auto c = _bus.config();
      c.spi_host    = SPI2_HOST;   // FSPI - same host the Arduino SPI object uses on the S3
      c.spi_mode    = 0;
      c.freq_write  = 27000000;    // keep modest on a shared bus (SD/W5500 present)
      c.freq_read   = 16000000;
      c.spi_3wire   = false;
      c.use_lock    = true;
      c.dma_channel = SPI_DMA_CH_AUTO;
      c.pin_sclk    = 12;          // shared SCLK
      c.pin_mosi    = 11;          // shared MOSI
      c.pin_miso    = 13;          // shared MISO
      c.pin_dc      = 46;          // TFT_DC (display-only)
      _bus.config(c);
      _panel.setBus(&_bus);
    }
    { auto c = _panel.config();
      c.pin_cs   = 45;             // TFT_CS
      c.pin_rst  = 47;             // TFT_RST (shared with touch RST)
      c.pin_busy = -1;
      c.panel_width  = 240;
      c.panel_height = 320;        // set 240 if your panel is 240x240
      c.offset_x = 0;
      c.offset_y = 0;
      c.offset_rotation = 0;
      c.readable   = false;
      c.invert     = true;         // panel inversion ON
      c.rgb_order  = false;        // RGB order (red/blue were swapped on the panel with true)
      c.dlen_16bit = false;
      c.bus_shared = true;         // IMPORTANT: SD/Ethernet share this bus
      _panel.config(c);
    }
    setPanel(&_panel);
  }
};
LGFX tft;

// LovyanGFX already defines these, but guard just in case
#ifndef TFT_BLACK
#define TFT_BLACK   0x0000
#define TFT_WHITE   0xFFFF
#define TFT_RED     0xF800
#define TFT_GREEN   0x07E0
#define TFT_BLUE    0x001F
#define TFT_YELLOW  0xFFE0
#define TFT_CYAN    0x07FF
#define TFT_MAGENTA 0xF81F
#endif
// ==================================================================

// ---------------- PINS ----------------
#define SDA   8
#define SCL   9

#define MISO 13
#define MOSI 11
#define SCLK 12

#define DSP_CS 45

#define RTD_SLAVE_ADDR 0x3F

// ---------------- CONFIG ----------------
uint8_t rtdType = 1;                 // 0 = PT100 | 1 = PT1000
uint8_t channelsToRead[4] = {1};    // default channel 1
uint8_t numChannels = 1;

// ---------------- DATA ----------------
float rtdTemp[4] = {0};
float rtdRes[4]  = {0};
uint8_t rtdFault[4] = {0};
bool configMode = false;

// ---------------- DISPLAY STATE ----------------
// Only rows whose text changed are redrawn, so the screen does not flicker.
#define ROW_COUNT   14            // title + RTD type + 3 rows for each of the 4 channels
#define ROW_HEIGHT  22
#define COLOR_ORANGE 0xFD20       // readable colour for the resistance value
String rowCache[ROW_COUNT];
bool forceRedraw = true;          // full clear on start and after a configuration change
uint8_t failCycles = 0;           // consecutive read cycles with no valid reply
bool commFail = false;            // true -> screen shows NO RESPONSE instead of old values

// ---------------- TIMER ----------------
#define READ_INTERVAL 1000

uint8_t i2cFailCount = 0;
bool i2cEverSynced = false;
bool lastReadFailed = false;

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
  tft.setFont(&fonts::FreeSerifBold12pt7b);

  tft.setCursor(60, 20);
  tft.print("NORVI");

  tft.setCursor(20, 40);
  tft.print("EXPE-RTD TEST");
}

// Clears one text row and returns true when its content changed since last time
bool rowBegin(int row, String sig)
{
  if (sig == rowCache[row]) return false;
  rowCache[row] = sig;

  int y = 20 + row * ROW_HEIGHT;                              // text baseline of this row
  tft.fillRect(0, y - 15, 240, ROW_HEIGHT, TFT_BLACK);
  tft.setCursor(10, y);
  return true;
}

void drawValues()
{
  if (forceRedraw)
  {
    tft.fillScreen(TFT_BLACK);
    for (int i = 0; i < ROW_COUNT; i++) rowCache[i] = "";
    forceRedraw = false;
  }

  tft.setFont(&fonts::FreeSansBold9pt7b);

  if (rowBegin(0, "title"))
  {
    tft.setTextColor(TFT_YELLOW);
    tft.print("NORVI RTD MASTER");
  }

  if (rowBegin(1, String("type") + rtdType))
  {
    tft.setTextColor(TFT_CYAN);
    tft.print("RTD Type: "); tft.print(rtdType == 0 ? "PT100" : "PT1000");
  }

  if (commFail)
  {
    // module not answering - do not show old values as if they were live
    if (rowBegin(2, "nores"))
    {
      tft.setTextColor(TFT_RED);
      tft.print("NO RESPONSE");
    }
    if (rowBegin(3, "nores2"))
    {
      tft.setTextColor(TFT_WHITE);
      tft.print("Check address 0x"); tft.print(RTD_SLAVE_ADDR, HEX);
    }
    for (int row = 4; row < ROW_COUNT; row++) rowBegin(row, "blank");   // clear the old channel rows
    return;
  }

  for (int i = 0; i < numChannels; i++)
  {
      uint8_t ch = channelsToRead[i];
      int row = 2 + i * 3;

      if (rowBegin(row, String(ch) + "|T|" + String(rtdTemp[ch-1], 2)))
      {
        tft.setTextColor(TFT_WHITE);
        tft.print("Ch "); tft.print(ch); tft.print(" : ");

        tft.setTextColor(TFT_GREEN);
        tft.print("Temp: "); tft.print(rtdTemp[ch-1], 2); tft.print(" C");
      }

      if (rowBegin(row + 1, String(ch) + "|R|" + String(rtdRes[ch-1], 2)))
      {
        tft.setTextColor(COLOR_ORANGE);
        tft.print("Res: "); tft.print(rtdRes[ch-1], 2); tft.print(" Ohm");
      }

      if (rowBegin(row + 2, String(ch) + "|F|" + String(rtdFault[ch-1])))
      {
        tft.setTextColor(TFT_RED);
        tft.print("Fault: "); tft.print(rtdFault[ch-1]);
      }
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
    for (int i = 0; i < 9; i++) {
        digitalWrite(SCL, HIGH); delayMicroseconds(5);
        digitalWrite(SCL, LOW);  delayMicroseconds(5);
        if (digitalRead(SDA) == HIGH) break;
    }
    pinMode(SDA, OUTPUT);
    digitalWrite(SDA, LOW);  delayMicroseconds(5);
    digitalWrite(SCL, HIGH); delayMicroseconds(5);
    digitalWrite(SDA, HIGH); delayMicroseconds(5);
    Wire.begin(SDA, SCL);
}

// ---------------- READ SLAVE (CHUNKED) ----------------
#define CHUNK_SIZE 2

void readRTD()
{
  bool anyValid = false;

  for (int chunkStart = 0; chunkStart < numChannels; chunkStart += CHUNK_SIZE)
  {
      uint8_t chunkCount = min(CHUNK_SIZE, numChannels - chunkStart);

      Wire.beginTransmission(RTD_SLAVE_ADDR);
      Wire.write(0x01);        // command
      Wire.write(rtdType);     // RTD type

      for (int i = 0; i < chunkCount; i++)
      {
          Wire.write(channelsToRead[chunkStart + i]);
      }

      if (Wire.endTransmission() != 0)
      {
          Serial.println("I2C TX FAIL");
          if (++i2cFailCount >= 2) { i2cBusRecover(); i2cFailCount = 0; }
          lastReadFailed = true;
          continue;
      }
      i2cFailCount = 0;
      lastReadFailed = false;

      delay(15);

      uint8_t totalBytes = chunkCount * 12;
      Wire.requestFrom(RTD_SLAVE_ADDR, totalBytes);

      if (Wire.available() != totalBytes)
      {
          Serial.println("I2C RX FAIL");
          continue;
      }

      for (int idx = 0; idx < chunkCount; idx++)
      {
          uint8_t buf[12];
          for (int i = 0; i < 12; i++) buf[i] = Wire.read();

          if (crc8(buf, 11) != buf[11])
          {
              Serial.print("CRC ERROR Channel ");
              Serial.println(buf[1]);
              continue;
          }
          i2cEverSynced = true;   
          anyValid = true;

          float temp, res;
          memcpy(&temp, &buf[2], 4);
          memcpy(&res, &buf[6], 4);
          uint8_t ch = buf[1];

          rtdTemp[ch-1] = temp;
          rtdRes[ch-1] = res;
          rtdFault[ch-1] = buf[10];

          Serial.print("Ch ");
          Serial.print(ch);
          Serial.print(" Temp: "); Serial.print(temp);
          Serial.print(" Res: "); Serial.print(res);
          Serial.print(" Fault: "); Serial.println(buf[10]);
      }
  }

  if (anyValid) failCycles = 0;
  else if (failCycles < 255) failCycles++;
  commFail = (failCycles >= 3);          // 3 failed reads in a row

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
  tft.setRotation(0);
  tft.setTextDatum(lgfx::textdatum_t::baseline_left);   // cursor Y = text baseline, same layout as before

  drawHeader();
  printCurrentConfig();

  Serial.println("NORVI RTD MASTER STARTED");
}

// ---------------- LOOP ----------------
unsigned long lastChunkRead = 0;
void loop()
{
  handleSerial();

  uint16_t interval;
  if (!i2cEverSynced) interval = 100;
  else if (lastReadFailed) interval = 100;
  else interval = READ_INTERVAL;

  if (!configMode && millis() - lastChunkRead >= interval)
  {
      lastChunkRead = millis();
      readRTD();
  }
}

// ---------------- CONFIG ----------------
void enterConfig() {
    configMode = true;
    Serial.println("\n===== CONFIG MODE =====");
    Serial.println("Enter RTD Type and Channels in one line:");
    Serial.println("Examples: 100,1 for PT100 Ch1, 1000,1-3 for PT1000 Ch1,2,3");

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

    uint8_t newType = rtdType;
    uint8_t newCh[4];
    uint8_t newCount = 0;

    int commaIndex = input.indexOf(',');
    if (commaIndex != -1)
    {
        String typeStr = input.substring(0, commaIndex);
        String chStr   = input.substring(commaIndex + 1);

        int typeInput = typeStr.toInt();
        newType = (typeInput == 100) ? 0 : 1;

        int dashIndex = chStr.indexOf('-');
        if (dashIndex != -1)
        {
            int startCh = chStr.substring(0, dashIndex).toInt();
            int endCh   = chStr.substring(dashIndex + 1).toInt();
            if (startCh >= 1 && endCh <= 4 && startCh <= endCh)      // channels 1-4 only
            {
                for (int i = startCh; i <= endCh; i++)
                    newCh[newCount++] = i;
            }
        }
        else
        {
            int start = 0;
            while (start < chStr.length() && newCount < 4)
            {
                int commaPos = chStr.indexOf(',', start);
                String chPart = (commaPos == -1) ? chStr.substring(start) : chStr.substring(start, commaPos);
                int ch = chPart.toInt();
                if (ch >= 1 && ch <= 4) newCh[newCount++] = ch;
                if (commaPos == -1) break;
                start = commaPos + 1;
            }
        }
    }

    if (newCount > 0)
    {
        rtdType = newType;
        numChannels = newCount;
        for (int i = 0; i < newCount; i++) channelsToRead[i] = newCh[i];

        Serial.println("\nConfiguration Saved:");
        forceRedraw = true;      // channel list may have changed - redraw the whole screen
    }
    else
    {
        Serial.println("\nInvalid input - configuration not changed (channels must be 1-4).");
        Serial.println("Examples: 100,1  or  1000,1-3  or  1000,1,3");
        Serial.println("Current configuration:");
    }
    printCurrentConfig();
    configMode = false;
}

// ---------------- PRINT CONFIG ----------------
void printCurrentConfig() {
    Serial.print("RTD Type : "); Serial.println(rtdType == 0 ? "PT100" : "PT1000");
    Serial.print("Channels : ");
    for (int i = 0; i < numChannels; i++)
    {
        Serial.print(channelsToRead[i]);
        if (i < numChannels - 1) Serial.print(", ");
    }
    Serial.println();
}
