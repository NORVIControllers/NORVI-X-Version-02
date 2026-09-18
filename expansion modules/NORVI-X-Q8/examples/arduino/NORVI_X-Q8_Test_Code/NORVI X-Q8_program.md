---
type: Test_program
function: transistor_output_test (8-channel transistor output test via PCA9538 with TFT display)
compatible_cpu_boards: [NORVI X-CPU-ESPS3-X1],[NORVI X-CPU-ESPS3-X2],[NORVI X-CPU-ESPS3-X3]
compatible_modules: [NORVI X-Q8 (8-channel transistor output expansion, PCA9538 I2C IO expander)]
status: completed
last_updated: 2026-08-17
---

# Example: NORVI X-Q8 Transistor Output Test

## What this does
Sequentially toggles all 8 transistor outputs (TR1–TR8) ON then OFF via a PCA9538 I2C IO expander, displaying each output's live ON/OFF state on the TFT screen while logging every state change to Serial, then repeats continuously.

## Hardware required
- CPU board: NORVI X-CPU-ESPS3-X1 OR X2, X3
- Expansion module(s): NORVI X-Q8 (8-channel transistor output module, PCA9538 I2C IO expander)
  - PCA9538 I2C address is DIP-switch selectable; this sketch uses `0x73` by default and supported the address range of 0x70-0x73
- Wiring / pin map (from firmware `#define`s):

| Signal | GPIO |
|---|---|
| I2C SDA | 8 |
| I2C SCL | 9 |
| SPI MISO | 13 |
| SPI MOSI | 11 |
| SPI SCLK | 12 |
| TFT Display CS | 45 |
| PCA9538 Reset (PCA_RESET) | 21 |
| Touch SDA / SCL / RST / IRQ | 8, 9, 47, 48 |
| TR1–TR8 (via PCA9538) | PCA9538 pins 1, 2, 3, 4, 5, 6, 7, 8 |

## Libraries required
| Library | Version |
|---|---|
| Wire (ESP32 core) | 2.0.0 |
| WiFi (ESP32 core) | 2.0.0 |
| SPI (ESP32 core) | 2.0.0 |
| PCA9538 |
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

//"NPD-NORVI X-R8"
//2025.11.01

#include <Wire.h>
#include <WiFi.h>
#include <SPI.h>
#include <PCA9538.h>
#include "TFT_eSPI.h"
#include <CST816S.h>
#include "Free_Fonts.h" // Include the header file attached to this sketch

// I2C address of PCA9538
#define PCA9538_ADDR 0x73

#define SDA   8     
#define SCL   9     

#define MISO 13
#define MOSI 11
#define SCLK 12

#define DSP_CS 45

#define PCA_RESET 21

#define TR1   1 
#define TR2   2
#define TR3   3
#define TR4   4
#define TR5   5
#define TR6   6
#define TR7   7
#define TR8   8

PCA9538 io(PCA9538_ADDR);

CST816S touch(8, 9, 47, 48);  // sda, scl, rst, irq

TFT_eSPI tft = TFT_eSPI();

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("NPD-NORVI X-DI8 TEST");
  delay(1000);

  pinMode(PCA_RESET, OUTPUT);
  delay(100);
  digitalWrite(PCA_RESET, HIGH);

  Wire.begin(SDA, SCL);
  delay(100);

  I2C_SCAN();
  delay(1000);

  io.pinMode(TR1, OUTPUT);
  io.pinMode(TR2, OUTPUT);
  io.pinMode(TR3, OUTPUT);
  io.pinMode(TR4, OUTPUT);
  io.pinMode(TR5, OUTPUT);
  io.pinMode(TR6, OUTPUT);
  io.pinMode(TR7, OUTPUT);
  io.pinMode(TR8, OUTPUT);

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

  tft.setCursor(10, 40);      //xpos, ypos
  tft.print("EXPE-X-Q8 TEST");
}

void loop() {

  tft.fillScreen(TFT_BLACK);            // Clear screen
  tft.setTextColor(TFT_GREEN);
  tft.setFreeFont(FSB12);
  
  tft.setCursor(60, 20);      //xpos, ypos
  tft.print("NORVI");

  tft.setCursor(20, 40);      //xpos, ypos
  tft.print("EXPE-Q8 TEST");

  tft.setTextColor(TFT_YELLOW);
  tft.setCursor(0, 70);      //xpos, ypos
  tft.setFreeFont(FSB9);
  tft.print("TR 1 :");

  tft.setCursor(0, 100);      //xpos, ypos
  tft.print("TR 2 :");
  
  tft.setCursor(0, 130);      //xpos, ypos
  tft.print("TR 3 :");
  
  tft.setCursor(0, 160);      //xpos, ypos
  tft.print("TR 4 :");
  
  tft.setCursor(0, 190);      //xpos, ypos
  tft.print("TR 5 :");
  
  tft.setCursor(0, 220);      //xpos, ypos
  tft.print("TR 6 :");
  
  tft.setCursor(0, 250);      //xpos, ypos
  tft.print("TR 7 :");
  
  tft.setCursor(0, 280);      //xpos, ypos
  tft.print("TR 8 :");
 
  
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(120, 70);      //xpos, ypos
  tft.print("ON");
  Serial.println("TR 1 : ON");
  io.digitalWrite(TR1,HIGH);
  delay(500);
  tft.setCursor(170, 70);      //xpos, ypos
  tft.print("OFF");
  Serial.println("TR 1 : OFF");
  io.digitalWrite(TR1,LOW);
  delay(500);

  tft.setCursor(120, 100);      //xpos, ypos
  tft.print("ON");
  Serial.println("TR 2 : ON");
  io.digitalWrite(TR2,HIGH);
  delay(500);
  tft.setCursor(170, 100);      //xpos, ypos
  tft.print("OFF");
  Serial.println("TR 2 : OFF");
  io.digitalWrite(TR2,LOW);
  delay(500);
  

  tft.setCursor(120, 130);      //xpos, ypos
  tft.print("ON");
  Serial.println("TR 3 : ON");
  io.digitalWrite(TR3,HIGH);
  delay(500);
  tft.setCursor(170, 130);      //xpos, ypos
  tft.print("OFF");
  Serial.println("TR 3 : OFF");
  io.digitalWrite(TR3,LOW);
  delay(500);
  

  tft.setCursor(120, 160);      //xpos, ypos
  tft.print("ON");
  Serial.println("TR 4 : ON");
  io.digitalWrite(TR4,HIGH);
  delay(500);
  tft.setCursor(170, 160);      //xpos, ypos
  tft.print("OFF");
  Serial.println("TR 4 : OFF");
  io.digitalWrite(TR4,LOW);
  delay(500);

  tft.setCursor(120, 190);      //xpos, ypos
  tft.print("ON");
  Serial.println("TR 5 : ON");
  io.digitalWrite(TR5,HIGH);
  delay(500);
  tft.setCursor(170, 190);      //xpos, ypos
  tft.print("OFF");
  Serial.println("TR 5 : OFF");
  io.digitalWrite(TR5,LOW);
  delay(500);

  tft.setCursor(120, 220);      //xpos, ypos
  tft.print("ON");
  Serial.println("TR 6 : ON");
  io.digitalWrite(TR6,HIGH);
  delay(500);
  tft.setCursor(170, 220);      //xpos, ypos
  tft.print("OFF");
  Serial.println("TR 6 : OFF");
  io.digitalWrite(TR6,LOW);
  delay(500);

  tft.setCursor(120, 250);      //xpos, ypos
  tft.print("ON");
  Serial.println("TR 7 : ON");
  io.digitalWrite(TR7,HIGH);
  delay(500);
  tft.setCursor(170, 250);      //xpos, ypos
  tft.print("OFF");
  Serial.println("TR 7 : OFF");
  io.digitalWrite(TR7,LOW);
  delay(500);

  tft.setCursor(120, 280);      //xpos, ypos
  tft.print("ON");
  Serial.println("TR 8 : ON");
  io.digitalWrite(TR8,HIGH);
  delay(500);
  tft.setCursor(170, 280);      //xpos, ypos
  tft.print("OFF");
  Serial.println("TR 8 : OFF");
  io.digitalWrite(TR8,LOW);
  delay(500);
  
  
  Serial.println("........................................................................");

  delay(100);
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
- `NPD-NORVI X-DI8 TEST`
- I2C scan results, e.g. `I2C device found at address 0x73  !`
- Per-channel state log each cycle: `TR 1 : ON`, `TR 1 : OFF`, `TR 2 : ON`, `TR 2 : OFF` ... through `TR 8 : ON`, `TR 8 : OFF`
- Separator line `........................................................................` printed once per full loop cycle

On the TFT display: "NORVI" / "EXPE-Q8 TEST" header, followed by "TR 1 :" through "TR 8 :" labels, each showing "ON" then "OFF" as the outputs are sequentially toggled, refreshing continuously.

## Common Issues
| Symptom | Likely Cause | Fix |
|---|---|---|
| No I2C device found at 0x73 | PCA9538 not detected —  DIP switch address doesn't match `PCA9538_ADDR`, or reset pin not held HIGH | Verify SDA/SCL on GPIO 8/9, confirm PCA_RESET (GPIO 21) is driven HIGH before Wire init, and check the module's DIP switch setting matches `0x73` (or update `PCA9538_ADDR` to match) |
| Outputs don't switch connected load even though TFT/Serial shows ON | PCA9538 output not physically connected to the transistor driver stage, or load requires separate power supply | Check wiring from PCA9538 output pins to the X-Q8 transistor driver circuit and confirm board power |
| TFT shows nothing / blank screen | SPI pins (MISO 13 / MOSI 11 / SCLK 12) or Display CS (GPIO 45) miswired, or `tft.init()`/`tft.begin()` failing silently | Verify SPI wiring and TFT_eSPI `User_Setup` pin configuration matches board pinout |
| Upload fails / board not detected | Wrong Board/Tools settings | Match settings to: ESP32S3 Dev Module, UART0/Hardware CDC, QSPI PSRAM, 16MB flash, Huge APP partition |

## Related Files
- Module reference: `expansion_module_Q8.md`
- CPU board: `cpu_board_X1.md`