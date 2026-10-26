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
  tft.print("X-DI8 INPUT TEST");

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
