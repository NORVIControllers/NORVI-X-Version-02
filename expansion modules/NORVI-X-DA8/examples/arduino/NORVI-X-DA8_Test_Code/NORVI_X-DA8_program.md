---
type: Test_program
function: digital_input_read_ac (8-channel AC digital input test via PCA9538 with TFT display)
compatible_cpu_boards: [NORVI X-CPU-ESPS3-X1],[NORVI X-CPU-ESPS3-X2],[NORVI X-CPU-ESPS3-X3]
compatible_modules: [NORVI X-DA8 (8-channel AC digital input expansion, PCA9538 I2C IO expander)]
status: completed
last_updated: 2026-10-21
---

# Example: NORVI X-DA8 AC Digital Input Test

## What this does
Reads 8 AC digital input channels (DI1–DI8) via a PCA9538 I2C IO expander in a loop, updating only the channels whose state changed on the TFT screen (green value for HIGH, red value for LOW), while also printing each changed channel's value to Serial.

## Hardware required
- CPU board: NORVI X-CPU-ESPS3-X1 OR X2, X3
- Expansion module(s): NORVI X-DA8 (8-channel AC digital input module, PCA9538 I2C IO expander)
  - PCA9538 I2C address is DIP-switch selectable, range `0x70`–`0x73`; this sketch uses `0x73` by default
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
| PCA9538 Reset (PCA_RESET) | 21 |
| DI1–DI8 (via PCA9538) | PCA9538 pins 1, 2, 3, 4, 5, 6, 7, 8 |

## Libraries required
| Library | Version |
|---|---|
| Wire (ESP32 core) | 3.x (bundled with core) |
| SPI (ESP32 core) | 3.x (bundled with core) |
| PCA9538 | 
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
#include <Wire.h>
#include <SPI.h>
#include <PCA9538.h>

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

#define PCA9538_ADDR 0x73

#define SDA_PIN 8
#define SCL_PIN 9

#define MISO_PIN 13
#define MOSI_PIN 11
#define SCLK_PIN 12

#define PCA_RESET 21

PCA9538 io(PCA9538_ADDR);

int oldState[8] = {-1,-1,-1,-1,-1,-1,-1,-1};

void drawLabel()
{
  tft.fillScreen(TFT_BLACK);

  tft.setTextColor(TFT_YELLOW);
  tft.setFont(&fonts::FreeSerifBold12pt7b);

  tft.setCursor(50,20);
  tft.print("NORVI");

  tft.setCursor(10,45);
  tft.print("X-DA8 INPUT TEST");

  tft.setFont(&fonts::FreeSansBold9pt7b);

  tft.setCursor(10,80);   tft.print("DI1:");
  tft.setCursor(10,110);  tft.print("DI2:");
  tft.setCursor(10,140);  tft.print("DI3:");
  tft.setCursor(10,170);  tft.print("DI4:");
  tft.setCursor(10,200);  tft.print("DI5:");
  tft.setCursor(10,230);  tft.print("DI6:");
  tft.setCursor(10,260);  tft.print("DI7:");
  tft.setCursor(10,290);  tft.print("DI8:");
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

  Serial.println("NORVI X-DA8 TEST");

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
  tft.setTextDatum(lgfx::textdatum_t::baseline_left);   // cursor Y = text baseline, same layout as before

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
```

## Expected Output
On the Serial Monitor (115200 baud):
- `NORVI X-DA8 TEST`
- Per-channel state log on each change, e.g. `DI1 = 1`, `DI2 = 0`

On the TFT display: "NORVI" / "X-DA8 INPUT TEST" header, followed by "DI1:" through "DI8:" labels, each showing a live value (green for HIGH, red for LOW) that updates only when that channel's state changes.

## Common Issues
| Symptom | Likely Cause | Fix |
|---|---|---|
| No I2C device found at 0x73 | PCA9538 not detected — wiring fault, DIP switch address doesn't match `PCA9538_ADDR`, or reset pin not held HIGH | Verify SDA/SCL on GPIO 8/9, confirm PCA_RESET (GPIO 21) is driven HIGH before Wire init, and check the module's DIP switch setting matches `0x73` (or update `PCA9538_ADDR` to match a different setting in the `0x70`–`0x73` range) |
| Input value never changes on TFT despite AC signal being applied | AC input circuit not detecting the signal — miswired, wrong AC voltage range, or opto-isolator not triggering | Verify wiring to the X-DA8 module's terminal map and confirm the applied AC voltage matches the module's rated input range |
| Input always reads the same value regardless of AC signal presence | Floating/miswired input, or channel's state hasn't actually changed since last read | Verify wiring to the X-DA8 module's terminal map for the affected channel; the display only redraws on a state change, not every loop cycle |
| TFT shows nothing / blank screen | SPI pins (MISO 13 / MOSI 11 / SCLK 12) miswired, or `tft.init()` failing silently | Verify SPI wiring and that the pin numbers in the `LGFX` class match the board pinout |
| Display colors wrong or inverted | Panel invert / RGB order setting differs from the panel in use | Toggle `c.invert` or `c.rgb_order` in the `LGFX` class |
| Upload fails / board not detected | Wrong Board/Tools settings | Match settings to: ESP32S3 Dev Module, UART0/Hardware CDC, QSPI PSRAM, 16MB flash, Huge APP partition |

## Related Files
- Module reference: `expansion_module_DA8.md`
- CPU board: `cpu_board_X1.md`