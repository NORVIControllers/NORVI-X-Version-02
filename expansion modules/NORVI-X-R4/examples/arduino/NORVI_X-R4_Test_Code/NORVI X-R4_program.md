---
type: Test_program
function: relay_output_test (4-channel relay output test with pattern cycling and TFT display)
compatible_cpu_boards: [NORVI X-CPU-ESPS3-X1],[NORVI X-CPU-ESPS3-X2],[NORVI X-CPU-ESPS3-X3]
compatible_modules: [NORVI X-R4 (4-channel relay output expansion, PCA9538 I2C IO expander)]
status: completed
last_updated: 2026-08-17
---

# Example: NORVI X-R4 Relay Output Test

## What this does
Drives 4 relay outputs via a PCA9538 I2C IO expander through a repeating sequence of test patterns (All On/Off, Sequential, Odds vs Evens, Chaser, Binary Counter), showing the active pattern name and relay/state on the TFT screen while logging pattern names to Serial.

## Hardware required
- CPU board: NORVI X-CPU-ESPS3-X1 OR X2, X3 
- Expansion module(s): NORVI X-R4 (4-channel relay output module, PCA9538 I2C IO expander at address `0x70 OR 0x71 OR 0x72 OR 0x73`)
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
| Relay 1–4 (via PCA9538) | PCA9538 pins 1, 2, 3, 4 |

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

//"NPD-NORVI X-R4"
//2025.10.31

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

// Relay Pin Definitions for PCA9538
const int relayPins[] = {1, 2, 3, 4};

PCA9538 io(PCA9538_ADDR);

CST816S touch(8, 9, 47, 48);  // sda, scl, rst, irq

TFT_eSPI tft = TFT_eSPI();

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("NPD-NORVI X-R4 TEST");
  delay(1000);

  pinMode(PCA_RESET, OUTPUT);
  delay(100);
  digitalWrite(PCA_RESET, HIGH);

  Wire.begin(SDA, SCL);
  delay(100);

  I2C_SCAN();
  delay(1000);

  // Initialize PCA9538 Pins
  for(int i=0; i<4; i++) {
    io.pinMode(relayPins[i], OUTPUT);
    io.digitalWrite(relayPins[i], LOW);
  }

  SPI.begin(SCLK, MISO, MOSI); // Ensure these pin numbers are correct 
  delay(1000);

  tft.init();
  tft.begin();
  tft.setRotation(0);
  drawHeader();
  
}

void loop() {

  // 1. All On / All Off
  displayPatternName("ALL ON / OFF");
  setAllRelays(HIGH);
  delay(2000);
  setAllRelays(LOW);
  delay(1000);

  // 2. Sequential Test (The original pattern)
  displayPatternName("SEQUENTIAL");
  for(int i=0; i<4; i++) {
    updateRelay(i, HIGH);
    delay(300);
    updateRelay(i, LOW);
    delay(100);
  }

  // 3. Odds and Evens
  displayPatternName("ODDS vs EVENS");
  for(int repeat=0; repeat<4; repeat++) {
    toggleOddsEvens(true);  // Odds On
    delay(500);
    toggleOddsEvens(false); // Evens On
    delay(500);
  }
  setAllRelays(LOW);

  // 4. Chaser (Knight Rider style)
  displayPatternName("CHASER");
  for(int i=0; i<4; i++) { 
    updateRelay(i, HIGH); 
    if(i>0) updateRelay(i-1, LOW);
    delay(150); 
  }
  for(int i=3; i>=0; i--) { 
    updateRelay(i, HIGH); 
    if(i<3) updateRelay(i, LOW);
    delay(150); 
  }
  setAllRelays(LOW);

  // 5. Binary Counter (Fun visual)
  displayPatternName("BINARY COUNTER");
  for(int count=0; count <= 15; count++) { // Counting to 15 for brevity
    displayBinary(count);
    delay(400);
  }
  setAllRelays(LOW);
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

// --- Helper Functions ---

void drawHeader() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN);
  tft.setFreeFont(FSB12);
  tft.setCursor(60, 25);
  tft.print("NORVI");
  tft.setCursor(40, 50);
  tft.print("X-R4 TEST");
  tft.drawFastHLine(0, 60, 240, TFT_WHITE);
}

void displayPatternName(String name) {
  tft.fillRect(0, 65, 240, 30, TFT_BLACK); // Clear sub-header area
  tft.setTextColor(TFT_YELLOW);
  tft.setFreeFont(FSB9);
  tft.setCursor(10, 85);
  tft.print("MODE: " + name);
  Serial.println("Pattern: " + name);
}

// Controls a single relay and updates the TFT
void updateRelay(int index, bool state) {
  io.digitalWrite(relayPins[index], state);
  
  int yPos = 110 + (index * 25);
  tft.setFreeFont(FSB9);
  tft.setCursor(10, yPos);
  
  // Clear the line
  tft.fillRect(0, yPos-15, 240, 20, TFT_BLACK);
  
  tft.setTextColor(TFT_WHITE);
  tft.printf("RELAY %d : ", index + 1);
  
  if(state) {
    tft.setTextColor(TFT_RED);
    tft.print("ON");
  } else {
    tft.setTextColor(TFT_BLUE);
    tft.print("OFF");
  }
}

void setAllRelays(bool state) {
  for(int i=0; i<4; i++) {
    io.digitalWrite(relayPins[i], state);
  }
  
  // Update UI
  tft.fillRect(0, 100, 240, 220, TFT_BLACK);
  tft.setCursor(60, 200);
  tft.setTextColor(TFT_WHITE);
  tft.setFreeFont(FSB12);
  tft.print(state ? "ALL ON" : "ALL OFF");
}

void toggleOddsEvens(bool odds) {
  for(int i=0; i<4; i++) {
    bool state = (odds) ? (i % 2 == 0) : (i % 2 != 0);
    io.digitalWrite(relayPins[i], state);
  }
}

void displayBinary(int num) {
  for (int i = 0; i < 4; i++) {
    bool bit = bitRead(num, i);
    io.digitalWrite(relayPins[i], bit);
  }
  tft.fillRect(0, 100, 240, 220, TFT_BLACK);
  tft.setCursor(80, 200);
  tft.setFreeFont(FSB12);
  tft.setTextColor(TFT_CYAN);
  tft.print("VAL: ");
  tft.print(num);
}
```

## Expected Output
On the Serial Monitor (115200 baud):
- `NPD-NORVI X-R4 TEST`
- I2C scan results, e.g. `I2C device found at address `0x70 OR 0x71 OR 0x72 OR 0x73`  !` , `I2C device found at address 0x75  !`, `I2C device found at address 0x68  !` , `I2C device found at address 0x41  !`  , `I2C device found at address 0x15  !` 
- Pattern name printed at the start of each cycle, e.g. `Pattern: ALL ON / OFF`, `Pattern: SEQUENTIAL`, `Pattern: ODDS vs EVENS`, `Pattern: CHASER`, `Pattern: BINARY COUNTER`

On the TFT display: "NORVI" / "X-R4 TEST" header with a horizontal divider, followed by the active pattern name (e.g. "MODE: SEQUENTIAL"), then live relay states (RELAY 1–4 : ON/OFF) or "ALL ON"/"ALL OFF" or "VAL: n" depending on the current pattern, cycling continuously through all 5 patterns.

## Common Issues
| Symptom | Likely Cause | Fix |
|---|---|---|
| No I2C device found at 0x73 | PCA9538 not detected - reset pin not held HIGH, or wrong address | Verify SDA/SCL on GPIO 8/9, confirm PCA_RESET (GPIO 21) is driven HIGH before Wire init, check PCA9538 address strapping from dip switch |
| Relays don't switch even though TFT shows ON/OFF correctly | PCA9538 output not physically connected to relay driver, or relay board needs separate power supply | Check wiring from PCA9538 output pins to the X-R4 relay driver circuit and confirm relay board power |
| TFT shows nothing / blank screen | SPI pins (MISO 13 / MOSI 11 / SCLK 12) or Display CS (GPIO 45) configured incorrectly in the code, or `tft.init()`/`tft.begin()` failing silently | Verify SPI wiring and TFT_eSPI `User_Setup` pin configuration matches board pinout |
| Upload fails / board not detected | Wrong Board/Tools settings | Match settings to: ESP32S3 Dev Module, UART0/Hardware CDC, QSPI PSRAM, 16MB flash, Huge APP partition |

## Related Files
- Module reference: `expansion_module_R4.md`
- CPU board: `cpu_board_X1.md`