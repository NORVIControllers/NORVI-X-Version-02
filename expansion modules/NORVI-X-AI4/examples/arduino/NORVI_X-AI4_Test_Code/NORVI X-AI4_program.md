---
type: Test_program
function: analog_current_read_4-20ma (4-channel ADS1115 analog current input test with TFT display)
compatible_cpu_boards: [NORVI X-CPU-ESPS3-X1],[NORVI X-CPU-ESPS3-X2],[NORVI X-CPU-ESPS3-X3]
compatible_modules: [NORVI X-AI4 (4-channel analog current input expansion, ADS1115 I2C 16-bit ADC)]
status: completed
last_updated: 2026-08-17
---

# Example: NORVI X-AI4 Analog Current Input Test

## What this does
Reads 4 single-ended analog current channels via an ADS1115 ADC, converts the raw ADC counts to mA using a calibration factor, and continuously displays the live current readings on the TFT screen while printing all four channel values to Serial each cycle.

## Hardware required
- CPU board: NORVI X-CPU-ESPS3-X1 OR X2, X3
- Expansion module(s): NORVI X-AI4 (4-channel analog current input module, ADS1115 I2C ADC)
  - ADS1115 I2C address is DIP-switch selectable: `0x49` (DIP1 ON), `0x4B` (DIP2 ON), `0x4A` (DIP3 ON), `0x48` (DIP4 ON) — this sketch uses `0x49` by default
- Wiring / pin map (from firmware `#define`s):

| Signal | GPIO |
|---|---|
| I2C SDA | 8 |
| I2C SCL | 9 |
| SPI MISO | 13 |
| SPI MOSI | 11 |
| SPI SCLK | 12 |
| TFT Display CS | 47 |
| Touch SDA / SCL / RST / IRQ | 8, 9, 45, 48 |

## Libraries required
| Library | Version |
|---|---|
| Wire (ESP32 core) | 2.0.0 |
| WiFi (ESP32 core) | 2.0.0 |
| SPI (ESP32 core) | 2.0.0 |
| Adafruit_ADS1X15 | not specified in sketch |
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
//"NPD-NORVI EXPE AI4-V2 (OPSRAM)

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

#define DSP_CS 47

#define ADS_ADDR 0x49     // DIP1(ON)-0X49  DIP2(ON)-0x4B  DIP3(ON)-0x4A  DIP4(ON)-0x48

Adafruit_ADS1115 ads1;
const float mA_Factor = 4.096 / 3269.826;

CST816S touch(8, 9, 45, 48);  // sda, scl, rst, irq

TFT_eSPI tft = TFT_eSPI();

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("NPD-NORVI X-AI4 TEST");
  delay(1000);

  Wire.begin(SDA, SCL);
  delay(100);


  if (!ads1.begin(ADS_ADDR)) {
    Serial.println("Failed to initialize ADS 1 .");
    // while (1);
  }
  ads1.setGain(GAIN_ONE);

  SPI.begin(SCLK, MISO, MOSI); // Ensure these pin numbers are correct
  delay(1000);

  tft.init();
  tft.begin();
  tft.setRotation(0);

  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN);
  tft.setFreeFont(FSB12);

  tft.setCursor(60, 20);
  tft.print("NORVI");

  tft.setCursor(40, 40);
  tft.print("X-AI4 TEST");

  tft.setTextColor(TFT_YELLOW);
  tft.setFreeFont(FSB9);

  tft.setCursor(10, 100);
  tft.print("CURRENT 1 :");

  tft.setCursor(10, 130);
  tft.print("CURRENT 2 :");

  tft.setCursor(10, 160);
  tft.print("CURRENT 3 :");

  tft.setCursor(10, 190);
  tft.print("CURRENT 4 :");
}

void loop() {

  int16_t adc0, adc1, adc2, adc3;

  adc0 = ads1.readADC_SingleEnded(0);
  adc1 = ads1.readADC_SingleEnded(1);
  adc2 = ads1.readADC_SingleEnded(2);
  adc3 = ads1.readADC_SingleEnded(3);

  float current0 = adc0 * mA_Factor;
  float current1 = adc1 * mA_Factor;
  float current2 = adc2 * mA_Factor;
  float current3 = adc3 * mA_Factor;

  char buf[20];

  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  dtostrf(current0, 5, 2, buf);
  tft.setCursor(170, 100);
  tft.print(buf);
  tft.print(" mA");

  dtostrf(current1, 5, 2, buf);
  tft.setCursor(170, 130);
  tft.print(buf);
  tft.print(" mA");

  dtostrf(current2, 5, 2, buf);
  tft.setCursor(170, 160);
  tft.print(buf);
  tft.print(" mA");

  dtostrf(current3, 5, 2, buf);
  tft.setCursor(170, 190);
  tft.print(buf);
  tft.print(" mA");

  // Print all channels once
  Serial.printf("CH1: %.2f mA  CH2: %.2f mA  CH3: %.2f mA  CH4: %.2f mA\n",
                current0, current1, current2, current3);

  delay(50);
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
- `NPD-NORVI X-AI4 TEST`
- `Failed to initialize ADS 1 .` (only if ADS1115 not detected at the configured address)
- Continuous per-cycle line: `CH1: %.2f mA  CH2: %.2f mA  CH3: %.2f mA  CH4: %.2f mA` (e.g. `CH1: 4.02 mA  CH2: 8.15 mA  CH3: 12.33 mA  CH4: 20.01 mA`)

On the TFT display: "NORVI" / "X-AI4 TEST" header, followed by "CURRENT 1 :" through "CURRENT 4 :" labels, each showing a live current value in mA (e.g. `4.02 mA`), refreshing continuously.

## Common Issues
| Symptom | Likely Cause | Fix |
|---|---|---|
| `Failed to initialize ADS 1 .` | ADS1115 not detected — wiring fault, or DIP-switch address doesn't match `ADS_ADDR` (`0x49`) hardcoded in the sketch | Verify SDA/SCL on GPIO 8/9, and confirm the module's DIP switch setting matches `0x49`, or update `ADS_ADDR` to match the actual DIP configuration (`0x48`/`0x4A`/`0x4B`) |
| Current readings show ~0 mA on all channels regardless of input | No 4-20mA signal source connected, or `ads1.begin()` failed silently | Confirm a live current loop signal is wired to the corresponding channel and check ADS1115 initialization status in Serial |
| Current readings are inaccurate / scaled incorrectly | `mA_Factor` calibration constant not matched to the module's actual shunt resistor value | Verify the `mA_Factor` (4.096 / 3269.826) against the X-AI4 module's shunt resistor spec and recalibrate if needed |
| TFT shows nothing / blank screen | SPI pins (MISO 13 / MOSI 11 / SCLK 12) or Display CS (GPIO 47) miswired, or `tft.init()`/`tft.begin()` failing silently | Verify SPI wiring and TFT_eSPI `User_Setup` pin configuration matches board pinout |
| Upload fails / board not detected | Wrong Board/Tools settings | Match settings to: ESP32S3 Dev Module, UART0/Hardware CDC, QSPI PSRAM, 16MB flash, Huge APP partition |

## Related Files
- Module reference: `expansion_module_AI4.md`
- CPU board: `cpu_board_X1.md`