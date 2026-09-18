---
type: Test_program
function: rtd_temperature_read_i2c (4-channel PT100/PT1000 RTD temperature read over I2C, master side)
compatible_cpu_boards: [NORVI X-CPU-ESPS3-X1],[NORVI X-CPU-ESPS3-X2],[NORVI X-CPU-ESPS3-X3]
compatible_modules: [NORVI X-RTD4 (4-channel PT100/PT1000 RTD input expansion, STM32F103C8Tx I2C slave co-processor reading onboard RTD-to-digital converters)]
status: completed
last_updated: 2026-08-17
---

# Example: NORVI X-RTD4 RTD Temperature Read (PT100/PT1000, I2C, Serial Config)

## What this does
Periodically (every 1 second) requests temperature, resistance, and fault data for one or more configured RTD channels from the onboard STM32F103C8Tx co-processor over I2C, validates each response with a CRC8 checksum, and displays temperature (°C), resistance (Ω), and fault status per channel on the TFT screen and Serial. A Serial-based config mode (triggered by sending `C`) lets the user set the RTD type (PT100 or PT1000) and select which of the 4 channels to poll (single channel, a range like `1-3`, or a comma list like `1,3`) in one line, e.g. `100,1` or `1000,1-3`. The STM32 firmware is factory-programmed and fixed on the module; this ESP32 sketch is the master-side driver and is the only part intended to be modified/extended.

## Hardware required
- CPU board: NORVI X-CPU-ESPS3-X1 OR X2, X3
- Expansion module(s): NORVI X-RTD4 (4-channel PT100/PT1000 RTD input module; onboard STM32F103C8Tx acts as fixed I2C slave co-processor reading the RTD channels — its firmware is not user-modifiable)
  - I2C slave address is DIP-switch selectable, range `0x30`–`0x3F`. This sketch uses `0x3F` (`RTD_SLAVE_ADDR`) by default — update to match the module's DIP switch setting.
- Wiring / pin map (from firmware `#define`s):

| Signal | GPIO |
|---|---|
| I2C SDA | 8 |
| I2C SCL | 9 |
| SPI MISO | 13 |
| SPI MOSI | 11 |
| SPI SCLK | 12 |
| TFT Display CS | 45 |
| Touch SDA / SCL / RST / IRQ | 8, 9, 47, 48 |

## Libraries required
| Library | Version |
|---|---|
| Wire (ESP32 core) | 2.0.0 |
| SPI (ESP32 core) | 2.0.0 |
| TFT_eSPI | 2.5.0 |
| CST816S | 1.3.0 |
| Free_Fonts.h | local header, bundled with sketch |

**ESP32 Board Package version:** 2.0.12

**Arduino IDE Board/Tools settings:**
| Setting | Value |
|---|---|
| Board | ESP32S3 Dev Module |
| Upload Speed | 115200 |
| USB Mode | Hardware CDC and JTAG |
| USB CDC On Boot | Enabled |
| USB Firmware MSC On Boot | Disabled |
| USB DFU On Boot | Disabled |
| Upload Mode | UART0 / Hardware CDC |
| CPU Frequency | 240MHz (WiFi) |
| Flash Mode | QIO 80MHz |
| Flash Size | 16MB (128Mb) |
| Partition Scheme | Huge APP (3MB No OTA/1MB SPIFFS) |
| Core Debug Level | None |
| PSRAM | QSPI PSRAM |
| Arduino Runs On | Core 1 |
| Events Run On | Core 1 |
| Erase All Flash Before Sketch Upload | Disabled |
| JTAG Adapter | Disabled |

## Full Sketch
```cpp
// NPD-NORVI EXPE RTD – MASTER (CHUNKED READ)

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

#define RTD_SLAVE_ADDR 0x3F

TFT_eSPI tft = TFT_eSPI();

// ---------------- CONFIG ----------------
uint8_t rtdType = 1;                 // 0 = PT100 | 1 = PT1000
uint8_t channelsToRead[4] = {1};    // default channel 1
uint8_t numChannels = 1;

// ---------------- DATA ----------------
float rtdTemp[4] = {0};
float rtdRes[4]  = {0};
uint8_t rtdFault[4] = {0};
bool configMode = false;

// ---------------- TIMER ----------------
unsigned long lastRead = 0;
#define READ_INTERVAL 1000

CST816S touch(8, 9, 47, 48);

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
  tft.print("EXPE-RTD TEST");
}

void drawValues()
{
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_YELLOW);
  tft.setFreeFont(FSB9);

  int y = 20;
  tft.setCursor(0, y);
  tft.println("NORVI EXPE-RTD MASTER");

  y += 20;
  tft.setCursor(0, y);
  tft.setTextColor(TFT_CYAN);
  tft.print("RTD Type: "); tft.println(rtdType == 0 ? "PT100" : "PT1000");

  y += 20;
  for (int i = 0; i < numChannels; i++)
  {
      uint8_t ch = channelsToRead[i];

      tft.setCursor(0, y);
      tft.setTextColor(TFT_WHITE);
      tft.print("Ch "); tft.print(ch); tft.print(" : ");

      tft.setTextColor(TFT_GREEN);
      tft.print("Temp: "); tft.print(rtdTemp[ch-1], 2); tft.println(" C ");

      y += 20;

      tft.setTextColor(TFT_BLUE);
      tft.print("Res: "); tft.print(rtdRes[ch-1], 2); tft.print(" Ohm ");

      tft.setTextColor(TFT_RED);
      tft.print("Fault: "); tft.println(rtdFault[ch-1]);

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

// ---------------- READ SLAVE (CHUNKED) ----------------
#define CHUNK_SIZE 2

void readRTD()
{
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
          continue;
      }

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

  drawValues();
}

// ---------------- SETUP ----------------
void setup()
{
  Serial.begin(115200);
  delay(10000);

  Wire.begin(SDA, SCL);
  delay(100);
  SPI.begin(SCLK, MISO, MOSI);
  delay(1000);

  tft.init();
  tft.begin();
  tft.setRotation(0);

  drawHeader();
  printCurrentConfig();

  Serial.println("NORVI RTD MASTER STARTED");
}

// ---------------- LOOP ----------------
unsigned long lastChunkRead = 0;
void loop()
{
  handleSerial();

  if (!configMode && millis() - lastChunkRead >= READ_INTERVAL)
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

    int commaIndex = input.indexOf(',');
    if (commaIndex != -1)
    {
        String typeStr = input.substring(0, commaIndex);
        String chStr   = input.substring(commaIndex + 1);

        int typeInput = typeStr.toInt();
        rtdType = (typeInput == 100) ? 0 : 1;

        numChannels = 0;
        int dashIndex = chStr.indexOf('-');
        if (dashIndex != -1)
        {
            int startCh = chStr.substring(0, dashIndex).toInt();
            int endCh   = chStr.substring(dashIndex + 1).toInt();
            for (int i = startCh; i <= endCh; i++)
                channelsToRead[numChannels++] = i;
        }
        else
        {
            int start = 0;
            while (start < chStr.length() && numChannels < 4)
            {
                int commaPos = chStr.indexOf(',', start);
                String chPart = (commaPos == -1) ? chStr.substring(start) : chStr.substring(start, commaPos);
                int ch = chPart.toInt();
                if (ch >= 1 && ch <= 4) channelsToRead[numChannels++] = ch;
                if (commaPos == -1) break;
                start = commaPos + 1;
            }
        }
    }
    Serial.println("\nConfiguration Saved:");
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
```

## Expected Output
On the Serial Monitor (115200 baud):
- `NORVI RTD MASTER STARTED`, followed by the current config, e.g. `RTD Type : PT1000` and `Channels : 1`
- Every 1 second (READ_INTERVAL), per configured channel: `Ch 1 Temp: 24.60 Res: 1096.40 Fault: 0`
- `I2C TX FAIL` or `I2C RX FAIL` if the slave doesn't respond correctly
- `CRC ERROR Channel n` if a response's CRC8 checksum doesn't match
- Sending `C` (or `c`) over Serial enters config mode: prompts `===== CONFIG MODE =====` and `Enter RTD Type and Channels in one line:` with format examples, accepts input like `100,1` or `1000,1-3`, then prints `Configuration Saved:` and the new RTD type/channel list

On the TFT display: "NORVI" / "EXPE-RTD TEST" header at boot, then during normal operation "NORVI EXPE-RTD MASTER", the configured RTD type (PT100/PT1000), and each configured channel's live temperature (°C, green), resistance (Ω, blue), and fault code (red).

## Common Issues
| Symptom | Likely Cause | Fix |
|---|---|---|
| `I2C TX FAIL` / `I2C RX FAIL` on every read | `RTD_SLAVE_ADDR` (`0x3F`) doesn't match the module's actual DIP-switch-configured I2C address (`0x30`–`0x3F`) | Check the X-RTD4 module's DIP switch setting and update `RTD_SLAVE_ADDR` in the sketch to match |
| `CRC ERROR Channel n` repeatedly | Noisy/unstable I2C bus, or slave responding with corrupted data | Check I2C wiring quality/length and confirm the slave's RTD sensor for that channel is connected properly |
| Temperature/resistance readings implausible (e.g. very negative or very high) | RTD type mismatch — sensor is physically PT100 but sketch configured for PT1000, or vice versa | Send the correct RTD type via config mode (`100,...` for PT100, `1000,...` for PT1000) to match the actual sensor connected |
| Fault value non-zero for a channel | RTD open-circuit, short-circuit, or out-of-range resistance detected by the slave's RTD-to-digital converter | Check the physical RTD sensor wiring and connection (2/3/4-wire configuration) for that channel on the X-RTD4 module |
| Config mode doesn't accept input | Input not terminated with a newline, or Serial Monitor line-ending not set to "Newline" | Ensure the Serial Monitor sends a newline after config input (e.g. `1000,1-3` + Enter) |
| Long delay before any Serial output appears at boot | Sketch has a 10-second startup delay (`delay(10000)`) in `setup()` | This is expected — wait ~10 seconds after upload/reset before Serial output appears |
| TFT shows nothing / blank screen | SPI pins (MISO 13 / MOSI 11 / SCLK 12) or Display CS (GPIO 45) miswired, or `tft.init()`/`tft.begin()` failing silently | Verify SPI wiring and TFT_eSPI `User_Setup` pin configuration matches board pinout |
| Upload fails / board not detected | Wrong Board/Tools settings | Match settings to: ESP32S3 Dev Module, UART0/Hardware CDC, QSPI PSRAM, 16MB flash, Huge APP partition |

## Related Files
- Module reference: `expansion_module_RTD4.md`
- CPU board: `cpu_board_X1.md`