---
type: Test_program
function: rtd_temperature_read_i2c (4-channel PT100/PT1000 RTD temperature read over I2C, master side)
compatible_cpu_boards: [NORVI X-CPU-ESPS3-X1],[NORVI X-CPU-ESPS3-X2],[NORVI X-CPU-ESPS3-X3]
compatible_modules: [NORVI X-RTD4 (4-channel PT100/PT1000 RTD input expansion, STM32F103C8Tx I2C slave co-processor reading onboard RTD-to-digital converters)]
status: completed
last_updated: 2026-10-22
---

# Example: NORVI X-RTD4 RTD Temperature Read (PT100/PT1000, I2C, Serial Config)

## What this does
Periodically (every 1 second; every 100 ms until the first successful read and after any failed read) requests temperature, resistance, and fault data for one or more configured RTD channels from the onboard STM32F103C8Tx co-processor over I2C, validates each response with a CRC8 checksum (recovering the I2C bus automatically after repeated transmit failures), and displays temperature (°C), resistance (Ω), and fault status per channel on the TFT screen and Serial. A Serial-based config mode (triggered by sending `C`) lets the user set the RTD type (PT100 or PT1000) and select which of the 4 channels to poll (single channel, a range like `1-3`, or a comma list like `1,3`) in one line, e.g. `100,1` or `1000,1-3`. The STM32 firmware is factory-programmed and fixed on the module; this ESP32 sketch is the master-side driver and is the only part intended to be modified/extended.

## Hardware required
- CPU board: NORVI X-CPU-ESPS3-X1 OR X2, X3
- Expansion module(s): NORVI X-RTD4 (4-channel PT100/PT1000 RTD input module; onboard STM32F103C8Tx acts as fixed I2C slave co-processor reading the RTD channels — its firmware is not user-modifiable)
  - I2C slave address is DIP-switch selectable, range `0x30`–`0x3F`. This sketch uses `0x3F` (`RTD_SLAVE_ADDR`) by default — update to match the module's DIP switch setting.
- Wiring / pin map (from firmware `#define`s and the `LGFX` class):

| Signal | GPIO |
|---|---|
| I2C SDA | 8 |
| I2C SCL | 9 |
| SPI MISO | 13 |
| SPI MOSI | 11 |
| SPI SCLK | 12 |
| TFT Display CS | 45 |
| TFT Display DC | 46 |
| TFT Display RST | 47 |

## Libraries required
| Library | Version |
|---|---|
| Wire (ESP32 core) | 3.x (bundled with core) |
| SPI (ESP32 core) | 3.x (bundled with core) |
| LovyanGFX | 1.2.25 |

**ESP32 Board Package version:** any 3.x version (ESP32 Arduino core 3.x, IDF 5.x). The core-bundled libraries (Wire, SPI) use the version of the installed 3.x board package.

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
```

## Expected Output
On the Serial Monitor (115200 baud):
- `NORVI RTD MASTER STARTED`, followed by the current config, e.g. `RTD Type : PT1000` and `Channels : 1`
- Every 1 second (READ_INTERVAL; every 100 ms until the first successful read and after a failed read), per configured channel: `Ch 1 Temp: 24.60 Res: 1096.40 Fault: 0`
- `I2C TX FAIL` or `I2C RX FAIL` if the slave doesn't respond correctly (after two consecutive `I2C TX FAIL` the I2C bus is recovered automatically)
- `CRC ERROR Channel n` if a response's CRC8 checksum doesn't match
- Sending `C` (or `c`) over Serial enters config mode: prompts `===== CONFIG MODE =====` and `Enter RTD Type and Channels in one line:` with format examples, accepts input like `100,1` or `1000,1-3`, then prints `Configuration Saved:` and the new RTD type/channel list. Input with no valid channel (channels must be 1–4), e.g. `1000,7` or `1000,1-9`, is rejected with `Invalid input - configuration not changed` and the previous configuration is kept

On the TFT display: "NORVI" / "EXPE-RTD TEST" header at boot, then during normal operation "NORVI RTD MASTER", the configured RTD type (PT100/PT1000), and each configured channel on three rows: live temperature (°C, green), resistance (Ω, orange) and fault code (red). Only the rows whose value changed are redrawn, so the screen does not flicker. If the module gives no valid reply for 3 read cycles in a row, the channel values are replaced by a red "NO RESPONSE" message with the I2C address being used, and the values return as soon as the module answers again.

## Common Issues
| Symptom | Likely Cause | Fix |
|---|---|---|
| `I2C TX FAIL` / `I2C RX FAIL` on every read | `RTD_SLAVE_ADDR` (`0x3F`) doesn't match the module's actual DIP-switch-configured I2C address (`0x30`–`0x3F`) | Check the X-RTD4 module's DIP switch setting and update `RTD_SLAVE_ADDR` in the sketch to match |
| TFT shows "NO RESPONSE" | Module not answering at `RTD_SLAVE_ADDR`, or I2C bus problem | Check the module's DIP switch address against the address shown on screen, power and I2C wiring |
| `CRC ERROR Channel n` repeatedly | Noisy/unstable I2C bus, or slave responding with corrupted data | Check I2C wiring quality/length and confirm the slave's RTD sensor for that channel is connected properly |
| Temperature/resistance readings implausible (e.g. very negative or very high) | RTD type mismatch — sensor is physically PT100 but sketch configured for PT1000, or vice versa | Send the correct RTD type via config mode (`100,...` for PT100, `1000,...` for PT1000) to match the actual sensor connected |
| Fault value non-zero for a channel | RTD open-circuit, short-circuit, or out-of-range resistance detected by the slave's RTD-to-digital converter | Check the physical RTD sensor wiring and connection (2/3/4-wire configuration) for that channel on the X-RTD4 module |
| Config mode doesn't accept input | Input not terminated with a newline, or Serial Monitor line-ending not set to "Newline" | Ensure the Serial Monitor sends a newline after config input (e.g. `1000,1-3` + Enter) |
| `I2C TX FAIL` repeats and the readings stay at 0 | Slave not responding or I2C bus held low | The sketch recovers the bus after two failures; if it keeps failing check the module address, power and I2C wiring |
| TFT shows nothing / blank screen | SPI pins (MISO 13 / MOSI 11 / SCLK 12) or Display CS (GPIO 45) miswired, or `tft.init()` failing silently | Verify SPI wiring and that the pin numbers in the `LGFX` class match the board pinout |
| Display colors wrong or inverted | Panel invert / RGB order setting differs from the panel in use | Toggle `c.invert` or `c.rgb_order` in the `LGFX` class |
| Upload fails / board not detected | Wrong Board/Tools settings | Match settings to: ESP32S3 Dev Module, UART0/Hardware CDC, QSPI PSRAM, 16MB flash, Huge APP partition |

## Related Files
- Module reference: `expansion_module_RTD4.md`
- CPU board: `cpu_board_X1.md`