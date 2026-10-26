---
type: Test_program
function: thermocouple_temperature_read_i2c (4-channel thermocouple temperature read over I2C, master side)
compatible_cpu_boards: [NORVI X-CPU-ESPS3-X1, NORVI X-CPU-ESPS3-X2, NORVI X-CPU-ESPS3-X3]
compatible_modules: [NORVI X-TC4 (4-channel thermocouple input expansion, I2C slave co-processor reading the onboard thermocouple-to-digital converters)]
status: completed
last_updated: 2026-10-22
---

# Example: NORVI X-TC4 Thermocouple Temperature Read (I2C, Serial Config)

## What this does
Periodically (every 1 second) requests temperature and fault data for one or more configured thermocouple channels from the X-TC4 module's co-processor over I2C, validates each response with a CRC8 checksum (recovering the I2C bus automatically after repeated transmit failures), and displays temperature (°C) and fault status per channel on the TFT screen and Serial. A Serial-based config mode (triggered by sending `C`) lets the user select which of the 4 channels to poll (single channel, a range like `1-3`, or a comma list like `1,3`). The co-processor firmware is factory-programmed and fixed on the module; this ESP32 sketch is the master-side driver and is the only part intended to be modified/extended.

## Hardware required
- CPU board: NORVI X-CPU-ESPS3-X1 OR X2, X3
- Expansion module(s): NORVI X-TC4 (4-channel thermocouple input module; an onboard co-processor acts as a fixed I2C slave reading the thermocouple channels — its firmware is not user-modifiable)
  - The I2C slave address of the module is `0x18` (`TC_SLAVE_ADDR`) in this sketch. The commented `tcSlaveAddrs[]` line in the sketch lists `0x18`, `0x17` and `0x16` for use when several X-TC4 boards are connected — update the address to match your module.
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
// NPD-NORVI EXPE TC4 – MASTER 

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

#define TC_SLAVE_ADDR 0x18
//uint8_t tcSlaveAddrs[] = {0x18, 0x17, 0x16};   // one entry per physical TC4 board in use (when there is multiple devices)

// ---------------- CONFIG ----------------
uint8_t channelsToRead[4] = {1};    // default channel 1
uint8_t numChannels = 1;

// ---------------- DATA ----------------
float tcTemp[4] = {0};
uint8_t tcFault[4] = {0};
bool configMode = false;

// ---------------- DISPLAY STATE ----------------
// Only rows whose text changed are redrawn, so the screen does not flicker.
#define ROW_COUNT   9             // title + 2 rows (temperature, fault) for each of the 4 channels
#define ROW_HEIGHT  22
String rowCache[ROW_COUNT];
bool forceRedraw = true;          // full clear on start and after a configuration change
uint8_t failCycles = 0;           // consecutive read cycles with no valid reply
bool commFail = false;            // true -> screen shows NO RESPONSE instead of old values

uint8_t i2cFailCount = 0;

// ---------------- TIMER ----------------
#define READ_INTERVAL 1000

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
  tft.print("EXPE-TC4 TEST");
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
    tft.print("NORVI TC4 MASTER");
  }

  if (commFail)
  {
    // module not answering - do not show old values as if they were live
    if (rowBegin(1, "nores"))
    {
      tft.setTextColor(TFT_RED);
      tft.print("NO RESPONSE");
    }
    if (rowBegin(2, "nores2"))
    {
      tft.setTextColor(TFT_WHITE);
      tft.print("Check address 0x"); tft.print(TC_SLAVE_ADDR, HEX);
    }
    for (int row = 3; row < ROW_COUNT; row++) rowBegin(row, "blank");   // clear the old channel rows
    return;
  }

  for (int i = 0; i < numChannels; i++)
  {
      uint8_t ch = channelsToRead[i];
      int row = 1 + i * 2;

      if (rowBegin(row, String(ch) + "|T|" + String(tcTemp[ch-1], 2)))
      {
        tft.setTextColor(TFT_WHITE);
        tft.print("Ch "); tft.print(ch); tft.print(" : ");

        tft.setTextColor(TFT_GREEN);
        tft.print("Temp: "); tft.print(tcTemp[ch-1], 2); tft.print(" C");
      }

      if (rowBegin(row + 1, String(ch) + "|F|" + String(tcFault[ch-1])))
      {
        tft.setTextColor(TFT_RED);
        tft.print("Fault: "); tft.print(tcFault[ch-1]);
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
  bool anyValid = false;

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
          if (ch < 1 || ch > 4) continue;     // ignore a reply for a channel that does not exist
          anyValid = true;

          tcTemp[ch-1] = temp;
          tcFault[ch-1] = buf[6];

          Serial.print("Ch ");
          Serial.print(ch);
          Serial.print(" Temp: "); Serial.print(temp);
          Serial.print(" Fault: "); Serial.println(buf[6]);
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

  Serial.println("NORVI TC4 MASTER STARTED");
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

    uint8_t newCh[4];
    uint8_t newCount = 0;

    int dashIndex = input.indexOf('-');
    if (dashIndex != -1)
    {
        int startCh = input.substring(0, dashIndex).toInt();
        int endCh   = input.substring(dashIndex + 1).toInt();
        if (startCh >= 1 && endCh <= 4 && startCh <= endCh)          // channels 1-4 only
        {
            for (int i = startCh; i <= endCh; i++)
                newCh[newCount++] = i;
        }
    }
    else
    {
        int start = 0;
        while (start < input.length() && newCount < 4)
        {
            int commaPos = input.indexOf(',', start);
            String chPart = (commaPos == -1) ? input.substring(start) : input.substring(start, commaPos);
            int ch = chPart.toInt();
            if (ch >= 1 && ch <= 4) newCh[newCount++] = ch;
            if (commaPos == -1) break;
            start = commaPos + 1;
        }
    }

    if (newCount > 0)
    {
        numChannels = newCount;
        for (int i = 0; i < newCount; i++) channelsToRead[i] = newCh[i];

        Serial.println("\nConfiguration Saved:");
        forceRedraw = true;      // channel list may have changed - redraw the whole screen
    }
    else
    {
        Serial.println("\nInvalid input - configuration not changed (channels must be 1-4).");
        Serial.println("Examples: 1  or  1-3  or  1,3");
        Serial.println("Current configuration:");
    }
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
```

## Expected Output
On the Serial Monitor (115200 baud):
- `NORVI TC4 MASTER STARTED`, preceded by the current config, e.g. `Channels : 1`
- Every 1 second (READ_INTERVAL), per configured channel: `Ch 1 Temp: 24.60 Fault: 0`
- `I2C TX FAIL` or `I2C RX FAIL` if the slave doesn't respond correctly (after five consecutive `I2C TX FAIL` the I2C bus is recovered automatically)
- `CRC ERROR Channel n` if a response's CRC8 checksum doesn't match
- Sending `C` (or `c`) over Serial enters config mode: prompts `===== CONFIG MODE =====` and `Enter Channels to read:` with format examples, accepts input like `1`, `1-3` or `1,3`, then prints `Configuration Saved:` and the new channel list. Input with no valid channel (channels must be 1–4), e.g. `7` or `1-9`, is rejected with `Invalid input - configuration not changed` and the previous configuration is kept

On the TFT display: "NORVI" / "EXPE-TC4 TEST" header at boot, then during normal operation "NORVI TC4 MASTER" and, for each configured channel, two rows: live temperature (°C, green) and fault code (red). Only the rows whose value changed are redrawn, so the screen does not flicker. If the module gives no valid reply for 3 read cycles in a row, the channel values are replaced by a red "NO RESPONSE" message with the I2C address being used, and the values return as soon as the module answers again.

## Common Issues
| Symptom | Likely Cause | Fix |
|---|---|---|
| `I2C TX FAIL` / `I2C RX FAIL` on every read, TFT shows "NO RESPONSE" | `TC_SLAVE_ADDR` (`0x18`) doesn't match the module's actual I2C address, or I2C wiring problem | Check the address shown on screen against the X-TC4 module and update `TC_SLAVE_ADDR` in the sketch; check SDA/SCL wiring |
| `CRC ERROR Channel n` repeatedly | Noisy/unstable I2C bus, or slave responding with corrupted data | Check I2C wiring quality/length and confirm the thermocouple for that channel is connected properly |
| Fault value non-zero for a channel | Thermocouple open-circuit, short-circuit, or out-of-range reading detected by the module | Check the thermocouple wiring and polarity for that channel on the X-TC4 module |
| Temperature readings implausible | Wrong thermocouple type connected for the module's configuration, or poor connection | Confirm the thermocouple type supported by the X-TC4 module and check the connection |
| Config mode doesn't accept input | Input not terminated with a newline, or Serial Monitor line-ending not set to "Newline" | Ensure the Serial Monitor sends a newline after config input (e.g. `1-3` + Enter) |
| TFT shows nothing / blank screen | SPI pins (MISO 13 / MOSI 11 / SCLK 12) or Display CS (GPIO 45) miswired, or `tft.init()` failing silently | Verify SPI wiring and that the pin numbers in the `LGFX` class match the board pinout |
| Display colors wrong or inverted | Panel invert / RGB order setting differs from the panel in use | Toggle `c.invert` or `c.rgb_order` in the `LGFX` class |
| Upload fails / board not detected | Wrong Board/Tools settings | Match settings to: ESP32S3 Dev Module, UART0/Hardware CDC, QSPI PSRAM, 16MB flash, Huge APP partition |

## Related Files
- Module reference: `expansion_module_TC4.md`
- CPU board: `cpu_board_X1.md`
