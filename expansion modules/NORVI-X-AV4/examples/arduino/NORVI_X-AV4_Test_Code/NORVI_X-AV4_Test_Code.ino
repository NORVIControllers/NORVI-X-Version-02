/*
 * 2026/10/22
 * NPD-NORVI EXPE AV4 
 * N16R2 
 * 
*/

#include <Wire.h>
#include <WiFi.h>
#include <SPI.h>
#include <Adafruit_ADS1X15.h>
#include <CST816S.h>

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
      c.rgb_order  = true;         // BGR order
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

#define SDA   8     
#define SCL   9     

#define MISO 13
#define MOSI 11
#define SCLK 12

#define DSP_CS 45

Adafruit_ADS1115 ads1;
#define VOLTAGE_DIVIDER_RATIO 0.2065

CST816S touch(8, 9, 47, 48);  // sda, scl, rst, irq

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("NPD-NORVI EXPE-AIX TEST");
  delay(1000);


  Wire.begin(SDA, SCL);
  delay(100);


if (!ads1.begin(0x49)) {
    Serial.println("Failed to initialize ADS 1 .");
    // while (1);
  }
 ads1.setGain(GAIN_ONE);

  SPI.begin(SCLK, MISO, MOSI); // Ensure these pin numbers are correct 
  delay(1000);

  tft.init();
  tft.setRotation(0);

  tft.fillScreen(TFT_BLACK);            // Clear screen
  tft.setTextColor(TFT_YELLOW);
  tft.setFont(&fonts::FreeSerifBold12pt7b);
  
  tft.setCursor(60, 20);      //xpos, ypos
  tft.print("NORVI");

  tft.setCursor(20, 40);      //xpos, ypos
  tft.print("EXPE-AV4 TEST");
}

void loop() {
  
  int16_t adc0, adc1, adc2, adc3;

  adc0 = ads1.readADC_SingleEnded(0);
  adc1 = ads1.readADC_SingleEnded(1);
  adc2 = ads1.readADC_SingleEnded(2);
  adc3 = ads1.readADC_SingleEnded(3);

  float Voltage0 = adc0 * 0.125 / 1000.0 / VOLTAGE_DIVIDER_RATIO;
  float Voltage1 = adc1 * 0.125 / 1000.0 / VOLTAGE_DIVIDER_RATIO;
  float Voltage2 = adc2 * 0.125 / 1000.0 / VOLTAGE_DIVIDER_RATIO;
  float Voltage3 = adc3 * 0.125 / 1000.0 / VOLTAGE_DIVIDER_RATIO;

  
  tft.fillScreen(TFT_BLACK);            // Clear screen
  tft.setTextColor(TFT_GREEN);
  tft.setFont(&fonts::FreeSerifBold12pt7b);
  
  tft.setCursor(60, 20);      //xpos, ypos
  tft.print("NORVI");

  tft.setCursor(20, 40);      //xpos, ypos
  tft.print("EXPE-AV4 TEST");

  tft.setTextColor(TFT_YELLOW);
  tft.setCursor(10, 100);      //xpos, ypos
  tft.setFont(&fonts::FreeSerifBold9pt7b);
  tft.print("VOLTAGE 1 :");

  tft.setCursor(10, 130);      //xpos, ypos
  tft.print("VOLTAGE 2 :");
  
  tft.setCursor(10, 160);      //xpos, ypos
  tft.print("VOLTAGE 3 :");
  
  tft.setCursor(10, 190);      //xpos, ypos
  tft.print("VOLTAGE 4 :");
  
//----------------------------------
  tft.setTextColor(TFT_WHITE);
  
  tft.setCursor(170, 100);      //xpos, ypos
  tft.print(String(Voltage0, 2) + " V");
  Serial.println(String("VOLTAGE 1 : ") + String(Voltage0, 2) + " V");
  delay(500);
  
  tft.setCursor(170, 130);      //xpos, ypos
  tft.print(String(Voltage1, 2) + " V");
  Serial.println(String("VOLTAGE 2 : ") + String(Voltage1, 2) + " V");
  delay(500);
  

  tft.setCursor(170, 160);      //xpos, ypos
  tft.print(String(Voltage2, 2) + " V");
  Serial.println(String("VOLTAGE 3 : ") + String(Voltage2, 2) + " V");
  delay(500);
  

  tft.setCursor(170, 190);
  tft.print(String(Voltage3, 2) + " V");
  Serial.println(String("VOLTAGE 4 : ") + String(Voltage3, 2) + " V");
  delay(500);


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
