---
type: Test_program
function: analog_voltage_read_0-10v (4-channel ADS1115 analog voltage input test with TFT display)
compatible_cpu_boards: [NORVI X-CPU-ESPS3-X1],[NORVI X-CPU-ESPS3-X2],[NORVI X-CPU-ESPS3-X3]
compatible_modules: [NORVI X-AV4 (4-channel analog voltage input expansion, ADS1115 I2C 16-bit ADC)]
status: completed
last_updated: 2026-08-17
---

# Example: NORVI X-AV4 Analog Voltage Input Test

## What this does
Reads 4 single-ended analog voltage channels via an ADS1115 ADC, converts the raw ADC counts to volts using a voltage-divider ratio, and continuously displays the live voltage readings on the TFT screen while printing each channel value to Serial as it's read.

## Hardware required
- CPU board: NORVI X-CPU-ESPS3-X1 OR X2, X3
- Expansion module(s): NORVI X-AV4 (4-channel analog voltage input module, ADS1115 I2C ADC)
  - ADS1115 I2C address is hardcoded to `0x48` in this sketch and can have addresses of 0x48,0x49,0x4A,0x4B by changing dip switch.
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
| WiFi (ESP32 core) | 2.0.0 |
| SPI (ESP32 core) | 2.0.0 |
| Adafruit_ADS1X15 | 
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
/*
 * 2026/05/13
 * NPD-NORVI EXPE AV4 
 * N16R2 
 * 
*/

#include <Wire.h>
#include <WiFi.h>
#include <SPI.h>
#include <Adafruit_ADS1X15.h>
#include "TFT_eSPI.h"
#include <CST816S.h>
#include "Free_Fonts.h" // Include the header file attached to this sketch

#define SDA   8     
#define SCL   9     

#define MISO 13
#define MOSI 11
#define SCLK 12

#define DSP_CS 45

Adafruit_ADS1115 ads1;
#define VOLTAGE_DIVIDER_RATIO 0.2065

CST816S touch(8, 9, 47, 48);  // sda, scl, rst, irq

TFT_eSPI tft = TFT_eSPI();

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("NPD-NORVI EXPE-AIX TEST");
  delay(1000);


  Wire.begin(SDA, SCL);
  delay(100);


if (!ads1.begin(0x48)) {
    Serial.println("Failed to initialize ADS 1 .");
    // while (1);
  }
 

  SPI.begin(SCLK, MISO, MOSI); // Ensure these pin numbers are correct 
  delay(1000);

  tft.init();
  tft.begin();
  tft.setRotation(0);

  tft.fillScreen(TFT_BLACK);            // Clear screen
  tft.setTextColor(TFT_YELLOW);
  tft.setFreeFont(FSB12);
  
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
  tft.setFreeFont(FSB12);
  
  tft.setCursor(60, 20);      //xpos, ypos
  tft.print("NORVI");

  tft.setCursor(20, 40);      //xpos, ypos
  tft.print("EXPE-AV4 TEST");

  tft.setTextColor(TFT_YELLOW);
  tft.setCursor(10, 100);      //xpos, ypos
  tft.setFreeFont(FSB9);
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
```

## Expected Output
On the Serial Monitor (115200 baud):
- `NPD-NORVI EXPE-AIX TEST`
- `Failed to initialize ADS 1 .` (only if ADS1115 not detected at address `0x48`)
- Per-channel line printed as each reading completes: `VOLTAGE 1 : 5.02 V`, `VOLTAGE 2 : 7.48 V`, `VOLTAGE 3 : 0.00 V`, `VOLTAGE 4 : 9.87 V`

On the TFT display: "NORVI" / "EXPE-AV4 TEST" header, followed by "VOLTAGE 1 :" through "VOLTAGE 4 :" labels, each showing a live voltage value (e.g. `5.02 V`), refreshing continuously.

## Common Issues
| Symptom | Likely Cause | Fix |
|---|---|---|
| `Failed to initialize ADS 1 .` | ADS1115 not detected — wiring fault, or module's I2C address doesn't match the hardcoded `0x48` | Verify SDA/SCL on GPIO 8/9, and confirm the module's address matches `0x48` (this sketch does not support changing the address without editing the code) |
| Voltage readings show ~0V on all channels regardless of input | No voltage signal connected, or `ads1.begin()` failed silently | Confirm a live voltage signal is wired to the corresponding channel and check ADS1115 initialization status in Serial |
| Voltage readings are inaccurate / scaled incorrectly | `VOLTAGE_DIVIDER_RATIO` (0.2065) not matched to the module's actual resistor-divider network | Verify the divider ratio against the X-AV4 module's voltage-divider spec and recalibrate if needed |
| TFT shows nothing / blank screen | SPI pins (MISO 13 / MOSI 11 / SCLK 12) or Display CS (GPIO 45) miswired, or `tft.init()`/`tft.begin()` failing silently | Verify SPI wiring and TFT_eSPI `User_Setup` pin configuration matches board pinout |
| Upload fails / board not detected | Wrong Board/Tools settings | Match settings to: ESP32S3 Dev Module, UART0/Hardware CDC, QSPI PSRAM, 16MB flash, Huge APP partition |

## Related Files
- Module reference: `expansion_module_AV4.md`
- CPU board: `cpu_board_X1.md`