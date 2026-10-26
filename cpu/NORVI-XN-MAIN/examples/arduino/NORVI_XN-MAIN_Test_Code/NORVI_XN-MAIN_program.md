---
type: Test_program
function: system_diagnostics_test (I2C scan, PCA9536 buttons/LEDs, TFT + touch counter, PCA9539 + GPIO output walk, RTC, SD, Ethernet/NTP, WiFi AP, RS485 loopback)
compatible_cpu_boards: [NORVI XN-MAIN (V2, N16R2 only)]
compatible_modules: [PCA9539 (I2C 0x75, onboard IO expander), PCA9536 (I2C 0x41, onboard IO expander), DS3231 RTC (I2C 0x68), SD card (SPI), Ethernet controller (SPI), CST816S capacitive touch (I2C 0x15), ST7789 TFT display (SPI), RS485 transceiver]
status: completed
last_updated: 2026-10-22
---

# Example: NORVI XN-MAIN System Diagnostics / Board Bring-Up Test

## What this does
A single bring-up sketch that exercises every onboard peripheral of the NORVI XN-CPU-ESPS3 and reports the results on the Serial Monitor and the TFT (display driven by **LovyanGFX**). It runs the same tests as the NORVI X-CPU-ESPS3-X1 test. Tests, in order:

1. **I2C scan** - lists every device found with its name (touch `0x15`, PCA9536 `0x41`, DS3231 `0x68`, PCA9539 `0x75`)
2. **Touch controller version** - printed on Serial
3. **Output walk test** - 17 outputs switch ON one by one, hold, switch OFF, hold, repeat (non-blocking; names printed on Serial, not shown on the TFT)
4. **DS3231 RTC** - set to the compile time **only if it lost power**; 5 timestamps printed on Serial
5. **microSD card** - mount, card type and size
6. **Ethernet** - DHCP (10 s timeout) then an NTP query (seconds since 1900, Unix time, UTC time)
7. **WiFi SoftAP** - SSID is the board MAC address, password `12345678`
8. **RS485** - sends `RS485 01 SUCCESS` and prints any echo (needs a loopback to see the reply)
9. **TFT benchmark** - timings in microseconds on Serial (same tests as X1)
10. **Live in loop():**
    - **Touch test on the TFT and Serial** - gesture (**SINGLE CLICK, DOUBLE CLICK, LONG PRESS, SWIPE UP / DOWN / LEFT / RIGHT**), points, event, X and Y coordinates, plus a touch counter
    - **IO TEST (PCA9536)** - same sequence as X1: LED1 on 500 ms, LED2 on 500 ms, both off, then `PB1 Read` / `PB2 Read` printed; PB1 / PB3 presses are also reported immediately on Serial
    - RTC date/time on the TFT every second, and the RS485 test message once per second

A **TEST SUMMARY** table is printed on the Serial Monitor at the end of `setup()`.

> Note: this sketch is a multi-function factory test, not a single-purpose example. It should be used as the reference for "how to initialize peripheral X", **not run whole as a template**. Pull only the `setup()` / function fragments for the peripherals you actually need (a request for "just RTC over serial" must not carry SD, Ethernet, display or RS485 code).

## Hardware required
- CPU board: NORVI XN-MAIN - **V2 "N16R2" ONLY** (ESP32-S3 module, 16MB flash / 2MB QSPI PSRAM). This is the only supported revision.
- Onboard peripherals:
  - PCA9539 16-bit I2C IO expander (address `0x75`) - drives CS1-CS4 and OUTPUT19/20/33/34/38
  - PCA9536 4-bit I2C IO expander (address `0x41`) - drives onboard pushbuttons PB1/PB3 and LED1/LED2
  - DS3231 RTC (I2C `0x68`)
  - microSD card (SPI)
  - Ethernet controller (SPI)
  - CST816S capacitive touch controller (I2C `0x15`)
  - ST7789 TFT display 240x320 (via LovyanGFX, SPI)
  - RS485 transceiver SN65HVD72DGKR (UART2, **automatic direction control - no dedicated flow-control/FC pin**)
- Wiring / pin map (from firmware `#define`s and the `LGFX` class):

> RS485 note: the RS485 transceiver uses **automatic direction control**. There is no dedicated flow-control (FC/DE-RE) pin, so no GPIO is assigned to it and the firmware does not toggle one - only RXD (GPIO 16) and TXD (GPIO 15) are used.

| Signal | GPIO |
|---|---|
| I2C SDA | 8 |
| I2C SCL | 9 |
| RS485 RXD | 16 |
| RS485 TXD | 15 |
| Ethernet CS | 1 |
| SPI MISO | 13 |
| SPI MOSI | 11 |
| SPI SCLK | 12 |
| TFT Display CS | 45 |
| TFT Display DC | 46 |
| TFT Display RST (shared with touch RST) | 47 |
| Touch SDA / SCL / RST / IRQ | 8, 9, 47, 48 |
| SD Card CS | 42 |
| IO Expander Reset (PORT_RST) | 38 |
| OUTPUT5 / 6 / 7 / 10 / 14 / 21 / 39 / 40 | 5, 6, 7, 10, 14, 21, 39, 40 (direct GPIO) |
| OUTPUT11 / 12 / 13 | 35, 36, 37 (defined, **not driven by this test** - see Special notes) |
| Spare UART1 TX / RX | 17, 18 |
| CS1-CS4, OUTPUT19/20/33/34/38 | via PCA9539 pins PCA_A5-B5 (not direct GPIO) |
| Onboard PB1 / LED1 / LED2 / PB3 | via PCA9536 pins 0-3 |

## Special configuration points (NORVI XN)
These come from the NORVI XN reference documents (overview, examples index, system-diagnostics and display-touch-counter references) and apply to any XN sketch.

### USB CDC On Boot - when Enabled, when Disabled, and why
USB CDC On Boot, USB Mode and Upload Mode are the **only** board settings that change with the sketch; everything else in the table below is fixed.

| Situation | USB CDC On Boot | USB Mode | Upload Mode |
|---|---|---|---|
| **Default** - the sketch does not print to the Serial Monitor (production / OTA-oriented, e.g. the touch-counter UI example) | **Disabled** | Hardware CDC | UART0 / Hardware |
| The sketch uses Serial Monitor output or debugging (**this diagnostics sketch**) | **Enabled** | Hardware CDC and JTAG | UART0 / Hardware CDC |

- **Why Enabled here:** this sketch prints the I2C scan, test results, output walk and TEST SUMMARY through `Serial`. With CDC On Boot Disabled the USB serial port does not exist at boot and nothing appears in the Serial Monitor.
- **Why Disabled otherwise:** it is the production default. A sketch that never uses `Serial` (for example the touchscreen counter) should stay Disabled; add `Serial.begin(115200)` and switch to Enabled only when serial visibility is needed.
- Rule of thumb: default to Disabled; switch the three settings together to Enabled only when serial output is wanted.

### Other XN-specific points
- **Partition Scheme is different from the X-series:** XN uses **Minimal SPIFFS (1.9MB APP w/ OTA / 190KB SPIFFS)**, not Huge APP. Zigbee Mode must be **Disabled**.
- **Touch before display:** `touch.begin()` must be called **before** `tft.init()`. They share the reset pin (GPIO 47); initializing the display first resets the touch controller and leaves the display blank.
- **Wake the IO expanders first:** drive `PORT_RST` (GPIO 38) HIGH and wait about 100 ms before `io.begin()` or any `ioport` (PCA9539) call - the chips are held in reset until then. Done once in `setup()`.
- **SPI CS isolation:** display (CS 45), SD (CS 42) and Ethernet (CS 1) share SCLK/MOSI/MISO. Drive all their CS pins HIGH right after `SPI.begin()` and before initializing any one of them. This sketch is the canonical reference for that pattern. If a sketch uses only one SPI device it is not needed.
- **Display config is board-fixed:** the `LGFX` class (bus pins, panel pins, 240x320) and `CST816S touch(8, 9, 47, 48)` (SDA, SCL, RST, IRQ) are fixed for the board - copy them as-is into any sketch that uses the display or touch.
- **LovyanGFX version and settings:** this test uses LovyanGFX **1.2.25** with the same display settings as the X1 test (27 MHz write clock, `readable = false`, `invert = true`, `rgb_order = false`, `bus_shared = true`). The XN reference sketches list 1.2.33; the display configuration is otherwise identical.
- **Only initialize what you use:** if touch is not used, drop the touch include, object and `touch.begin()`; if PCA9536/PCA9539 are not used, the `PORT_RST` wake-up is not needed.
- **OUTPUT11-13 (GPIO 35/36/37) are not used by the test:** they are not available when PSRAM is enabled (same as the X1 test), so the walk test skips them. They are listed in the pin table only for reference.
- **GPIO 21 (OUTPUT21) is shared with the expansion-module PCA reset line (PCA_RESET):** the walk test toggles it (as the X1 test does), which resets any connected expansion module for a moment. Disconnect expansion modules, or skip OUTPUT21, if that matters.
- **RTC is set only if it lost power:** the compile time is written to the DS3231 only when `rtc.lostPower()` is true, so a running RTC keeps its time across reflashes.
- **Ethernet DHCP uses a 10 s timeout** (`Ethernet.begin(mac, 10000)`) so boot is not blocked for long when no cable is connected; hardware-missing and cable-unplugged cases are reported separately.
- **Fast / slow loop split:** touch, the IO TEST LED chaser and the output walk run every loop without blocking (`millis()` timers); the RTC display and the RS485 test run once per second. Reusable for "poll instantly, update display periodically".

## Libraries required
| Library | Version |
|---|---|
| Wire (ESP32 core) | 3.x (bundled with core) |
| WiFi (ESP32 core) | 3.x (bundled with core) |
| SPI (ESP32 core) | 3.x (bundled with core) |
| FS (ESP32 core) | 3.x (bundled with core) |
| SD (ESP32 core) | 3.x (bundled with core) |
| Ethernet | 2.0.2 |
| PCA9539 | local header (`PCA9539.h`, bundled with the sketch - not a Library Manager library) |
| PCA9536D | 1.0.0 |
| RTClib | 2.1.4 |
| LovyanGFX | 1.2.25 |
| CST816S | 1.3.0 |

**ESP32 Board Package version:** any 3.x version (ESP32 Arduino core 3.x, IDF 5.x). The core-bundled libraries use the version of the installed 3.x board package.

**Arduino IDE Board/Tools settings:**
| Setting | Value |
|---|---|
| Board | ESP32S3 Dev Module |
| Upload Speed | 115200 |
| USB Mode | Hardware CDC and JTAG |
| USB CDC On Boot | **Enabled** (this sketch uses Serial Monitor; use Disabled / Hardware CDC / UART0 / Hardware if no serial output is needed) |
| USB Firmware MSC On Boot | Disabled |
| USB DFU On Boot | Disabled |
| Upload Mode | UART0 / Hardware CDC |
| CPU Frequency | 240MHz (WiFi) |
| Flash Mode | QIO 80MHz |
| Flash Size | 16MB (128Mb) |
| Partition Scheme | Minimal SPIFFS (1.9MB APP w/ OTA / 190KB SPIFFS) |
| Core Debug Level | None |
| PSRAM | QSPI PSRAM |
| Arduino Runs On | Core 1 |
| Events Run On | Core 1 |
| Erase All Flash Before Sketch Upload | Disabled |
| JTAG Adapter | Disabled |
| Zigbee Mode | Disabled |

## Full Sketch
```cpp
//"NPD-NORVI XN-MAIN (N16R2)"
/*
  System diagnostics / board bring-up (same tests as the NORVI X-CPU-ESPS3-X1 test). Tests, in order:
    1. I2C scan                      (touch, PCA9536, RTC, PCA9539)
    2. Touch controller version      (printed on Serial)
    3. PCA9536 IO TEST               (LED1 / LED2 chaser, PB1 / PB3 read-back - same as X1)
    4. PCA9539 + direct GPIO outputs (walking ON / OFF test, names on Serial)
    5. DS3231 RTC                    (set to compile time only if it lost power, 5 timestamps on Serial)
    6. SD card                       (mount, type, size)
    7. Ethernet (DHCP + NTP)         (UTC time on Serial)
    8. WiFi SoftAP
    9. RS485 test message            (flow control is automatic - no direction pin)
   10. TFT benchmark                 (timings on Serial)
  Then the TFT shows the touch test (gesture, points, event, X, Y), a touch counter and the RTC time.
  Gestures: SINGLE CLICK, DOUBLE CLICK, LONG PRESS, SWIPE UP / DOWN / LEFT / RIGHT.
  A summary of all results is printed on the Serial Monitor at the end of setup().
*/

#include <Wire.h>
#include <WiFi.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include "PCA9539.h"
#include <PCA9536D.h>
#include "RTClib.h"
#include <SPI.h>
#include "FS.h"
#include "SD.h"
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

#define RS485_RXD 16
#define RS485_TXD 15
// RS485 flow control is automatic on this board - no manual direction pin

#define ETH_CS 1
#define DSP_CS 45
#define SD_chipSelect 42

#define MISO 13
#define MOSI 11
#define SCLK 12

#define CS4      PCA_A5
#define CS3      PCA_A6
#define CS2      PCA_A7
#define CS1      PCA_B0
#define OUTPUT19 PCA_B1
#define OUTPUT20 PCA_B2
#define OUTPUT33 PCA_B3
#define OUTPUT34 PCA_B4
#define OUTPUT38 PCA_B5

#define PORT_RST 38

#define OUTPUT5  5       // Direct GPIO
#define OUTPUT6  6
#define OUTPUT7  7
#define OUTPUT10 10
#define OUTPUT11 35     //********
#define OUTPUT12 36     // Not allowed with PSRAM (not used in the output test)
#define OUTPUT13 37     //*******
#define OUTPUT14 14
#define OUTPUT21 21
#define OUTPUT39 39
#define OUTPUT40 40

#define TX1 17           // Spare UART1
#define RX1 18

// PCA9536 pins (onboard indicators & buttons)
#define IO_PB1  0   // Button 1
#define IO_LED1 1   // ERROR indicator
#define IO_LED2 2   // RUN indicator
#define IO_PB3  3   // Button 2

// ---------------- OBJECTS ----------------
PCA9539 ioport(0x75);
PCA9536 io;
RTC_DS3231 rtc;
CST816S touch(8, 9, 47, 48);  // sda, scl, rst, irq

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

// ---------------- ETHERNET & NTP ----------------
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
unsigned int localPort = 8888;
const char timeServer[] = "time.nist.gov";
const int NTP_PACKET_SIZE = 48;
byte packetBuffer[NTP_PACKET_SIZE];
EthernetUDP Udp;

// ---------------- OUTPUT TEST ----------------
#define OUTPUT_COUNT    17        // 8 direct GPIO + 5 on PCA9539 + CS1-CS4 on PCA9539
#define OUTPUT_STEP_MS  150       // time between switching the next output ON
#define OUTPUT_HOLD_MS  1000      // pause with all ON, and with all OFF

int outputIndex = 0;              // number of outputs currently ON
unsigned long lastOutputStep = 0;
unsigned long outputDelay = OUTPUT_HOLD_MS;

// ---------------- STATE ----------------
int touchCounter = 0;
bool isTouched = false;
unsigned long lastScreenUpdate = 0;

// Last touch event (shown on the TFT and Serial)
String lastGesture = "NONE";
int lastPoints = 0, lastEvent = 0, lastX = 0, lastY = 0;

// PCA9536 IO TEST (LED chaser) state
int ioStep = 0;
unsigned long lastIoStep = 0;
bool pb1Last = false, pb3Last = false;

// Test results (printed in the summary at the end of setup)
int  i2cDeviceCount = 0;
bool okPca9536  = false;
bool okRtc      = false;
bool okSd       = false;
bool okEthernet = false;
bool okNtp      = false;
bool okWifi     = false;
bool okRs485    = false;

// ---------------- FUNCTION PROTOTYPES ----------------
void I2C_SCAN();
const char* i2cDeviceName(byte address);
void PCA9536_INIT();
void TOUCH_VERSION();
void DISPLAY_INIT();
void DISPLAY_BENCHMARK();
void DISPLAY_DRAW_UI();
void showTouchRows();
void touchTask();
void ioTestTask();
void OUTPUTS_INIT();
void outputSet(int index, bool level);
const char* outputName(int index);
void allOutputsOff();
void outputTestTask();
void RTC_Check();
void SD_CHECK();
void ETHERNET_CHECK();
void WIFI_CHECK();
void RS485_CHECK();
void sendNTPpacket(const char * address);
void printSummary();

// ---------------- SETUP ----------------
void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial2.begin(115200, SERIAL_8N1, RS485_RXD, RS485_TXD);

  Serial.println("NPD-NORVI XN-CPU-ESPS3");
  delay(1000);

  pinMode(PORT_RST, OUTPUT);
  digitalWrite(PORT_RST, HIGH);   // WAKE UP PCA9536 & PCA9539
  delay(100);

  Wire.begin(SDA, SCL);
  SPI.begin(SCLK, MISO, MOSI);
  delay(100);

  // Isolate SPI slaves (display, SD, Ethernet share the bus)
  pinMode(ETH_CS, OUTPUT); digitalWrite(ETH_CS, HIGH);
  pinMode(SD_chipSelect, OUTPUT); digitalWrite(SD_chipSelect, HIGH);
  pinMode(DSP_CS, OUTPUT); digitalWrite(DSP_CS, HIGH);

  I2C_SCAN();
  delay(1000);

  PCA9536_INIT();

  touch.begin();                  // touch BEFORE display: they share the reset pin (GPIO 47)
  TOUCH_VERSION();
  DISPLAY_INIT();

  OUTPUTS_INIT();

  RTC_Check();
  SD_CHECK();
  ETHERNET_CHECK();
  WIFI_CHECK();
  RS485_CHECK();

  DISPLAY_BENCHMARK();
  DISPLAY_DRAW_UI();

  printSummary();
}

// ---------------- LOOP ----------------
void loop() {

  // ==========================================
  // FAST TASKS (touch, IO TEST, output test)
  // ==========================================
  touchTask();
  ioTestTask();
  outputTestTask();

  // ==========================================
  // SLOW TASKS (run once per second)
  // ==========================================
  if (millis() - lastScreenUpdate >= 1000) {
    lastScreenUpdate = millis();

    // 1. UPDATE RTC TIME
    DateTime now = rtc.now();
    tft.setFont(&fonts::FreeSansBold9pt7b);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(10, 197);
    tft.printf("%02d/%02d/%04d", now.month(), now.day(), now.year());
    tft.setCursor(10, 219);
    tft.printf("%02d:%02d:%02d", now.hour(), now.minute(), now.second());

    // 2. RUN RS485 TEST (flow control is automatic)
    Serial2.println("RS485 01 SUCCESS");
    delay(10);
    while (Serial2.available()) {
      Serial.write(Serial2.read());
    }
  }

  delay(10);
}

// ---------------- INITIALIZATION ----------------
void PCA9536_INIT() {
  if (io.begin() == false) {
    Serial.println("PCA9536 not detected.");
    okPca9536 = false;
  } else {
    io.pinMode(IO_PB1, INPUT);
    io.pinMode(IO_LED1, OUTPUT);   // ERROR LED
    io.pinMode(IO_LED2, OUTPUT);   // RUN LED
    io.pinMode(IO_PB3, INPUT);

    // Turn both LEDs OFF initially (HIGH = OFF)
    io.digitalWrite(IO_LED1, HIGH);
    io.digitalWrite(IO_LED2, HIGH);
    okPca9536 = true;
    Serial.println("PCA9536 OK (buttons & LEDs ready).");
  }
}

// ---------------- TOUCH ----------------
void TOUCH_VERSION() {
  Serial.print(touch.data.version);
  Serial.print("\t");
  Serial.print(touch.data.versionInfo[0]);
  Serial.print("-");
  Serial.print(touch.data.versionInfo[1]);
  Serial.print("-");
  Serial.println(touch.data.versionInfo[2]);
}

void DISPLAY_INIT() {
  Serial.println("LovyanGFX library test!");
  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);
}

// TFT benchmark (same tests as the X1 test, timings in microseconds on Serial)
void DISPLAY_BENCHMARK() {
  yield(); Serial.println(F("Benchmark                Time (microseconds)"));

  yield(); Serial.print(F("Screen fill              "));
  yield(); Serial.println(testFillScreen());

  yield(); Serial.print(F("Text                     "));
  yield(); Serial.println(testText());

  yield(); Serial.print(F("Lines                    "));
  yield(); Serial.println(testLines(TFT_CYAN));

  yield(); Serial.print(F("Horiz/Vert Lines         "));
  yield(); Serial.println(testFastLines(TFT_RED, TFT_BLUE));

  yield(); Serial.print(F("Rectangles (outline)     "));
  yield(); Serial.println(testRects(TFT_GREEN));

  yield(); Serial.print(F("Rectangles (filled)      "));
  yield(); Serial.println(testFilledRects(TFT_YELLOW, TFT_MAGENTA));

  yield(); Serial.print(F("Circles (filled)         "));
  yield(); Serial.println(testFilledCircles(10, TFT_MAGENTA));

  yield(); Serial.print(F("Circles (outline)        "));
  yield(); Serial.println(testCircles(10, TFT_WHITE));

  yield(); Serial.print(F("Triangles (outline)      "));
  yield(); Serial.println(testTriangles());

  yield(); Serial.print(F("Triangles (filled)       "));
  yield(); Serial.println(testFilledTriangles());

  yield(); Serial.print(F("Rounded rects (outline)  "));
  yield(); Serial.println(testRoundRects());

  yield(); Serial.print(F("Rounded rects (filled)   "));
  yield(); Serial.println(testFilledRoundRects());

  yield(); Serial.println(F("Done!")); yield();
}

// Static screen: titles and labels (the touch rows and the RTC are updated in loop)
void DISPLAY_DRAW_UI() {
  tft.setRotation(0);
  tft.setTextDatum(lgfx::textdatum_t::top_left);   // GFX font cursor = top-left of the text
  tft.setTextSize(1);
  tft.fillScreen(TFT_BLACK);

  tft.setTextColor(TFT_YELLOW);
  tft.setFont(&fonts::FreeSansBold12pt7b);
  tft.setCursor(60, 10);
  tft.print("NORVI XN");

  tft.setFont(&fonts::FreeSansBold9pt7b);
  tft.setTextColor(TFT_RED);
  tft.setCursor(10, 50);
  tft.print("Touch Test");

  tft.setTextColor(TFT_CYAN);
  tft.setCursor(10, 175);
  tft.print("RTC Time:");

  tft.setTextColor(TFT_RED);
  tft.setCursor(10, 255);
  tft.print("IO TEST");

  showTouchRows();
}

void OUTPUTS_INIT() {
  // PCA9539 outputs
  ioport.pinMode(CS4, OUTPUT); ioport.pinMode(CS3, OUTPUT);
  ioport.pinMode(CS2, OUTPUT); ioport.pinMode(CS1, OUTPUT);
  ioport.pinMode(OUTPUT19, OUTPUT); ioport.pinMode(OUTPUT20, OUTPUT);
  ioport.pinMode(OUTPUT33, OUTPUT); ioport.pinMode(OUTPUT34, OUTPUT);
  ioport.pinMode(OUTPUT38, OUTPUT);

  // Direct GPIO outputs (OUTPUT11-13 are not used - see pin list)
  pinMode(OUTPUT5,  OUTPUT); pinMode(OUTPUT6,  OUTPUT);
  pinMode(OUTPUT7,  OUTPUT); pinMode(OUTPUT10, OUTPUT);
  pinMode(OUTPUT14, OUTPUT); pinMode(OUTPUT21, OUTPUT);
  pinMode(OUTPUT39, OUTPUT); pinMode(OUTPUT40, OUTPUT);

  allOutputsOff();
  Serial.println("Outputs ready (17 outputs, walking test runs in loop).");
}

// Gesture, points, event, X, Y and the touch counter (redrawn only when a touch arrives)
void showTouchRows() {
  tft.setFont(&fonts::FreeSansBold9pt7b);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(10, 75);
  tft.printf("%-14s", lastGesture.c_str());
  tft.setCursor(10, 97);
  tft.printf("Pts:%d  Evt:%d   ", lastPoints, lastEvent);
  tft.setCursor(10, 119);
  tft.printf("X:%-4d Y:%-4d", lastX, lastY);
  tft.setTextColor(TFT_MAGENTA, TFT_BLACK);
  tft.setCursor(10, 141);
  tft.printf("Touches: %-5d", touchCounter);
}

void touchTask() {
  if (!touch.available()) {
    isTouched = false;
    return;
  }

  String g = touch.gesture();                 // SINGLE CLICK / DOUBLE CLICK / LONG PRESS / SWIPE ...
  if (g != "NONE") lastGesture = g;           // keep the last real gesture on screen
  lastPoints = touch.data.points;
  lastEvent  = touch.data.event;
  lastX      = touch.data.x;
  lastY      = touch.data.y;

  if (!isTouched) {                           // count each new touch once
    touchCounter++;
    isTouched = true;
  }

  Serial.print(g);                    Serial.print("\t");
  Serial.print(lastPoints);           Serial.print("\t");
  Serial.print(lastEvent);            Serial.print("\t");
  Serial.print(lastX);                Serial.print("\t");
  Serial.println(lastY);

  showTouchRows();
}

// ---------------- PCA9536 IO TEST (same sequence as X1) ----------------
// LED1 on 500 ms -> LED2 on 500 ms -> both off + PB1 / PB3 read-back -> pause -> repeat.
// LEDs are active LOW. Buttons read LOW while pressed.
void ioTestTask() {
  if (!okPca9536) return;

  // report button presses / releases immediately
  bool pb1 = (io.digitalRead(IO_PB1) == LOW);
  bool pb3 = (io.digitalRead(IO_PB3) == LOW);
  if (pb1 != pb1Last) { Serial.println(pb1 ? "PB1 pressed" : "PB1 released"); pb1Last = pb1; }
  if (pb3 != pb3Last) { Serial.println(pb3 ? "PB3 pressed" : "PB3 released"); pb3Last = pb3; }

  unsigned long wait = (ioStep == 2) ? 1000 : 500;
  if (millis() - lastIoStep < wait) return;
  lastIoStep = millis();

  switch (ioStep) {
    case 0:
      Serial.println("IO TEST");
      io.digitalWrite(IO_LED1, LOW);  io.digitalWrite(IO_LED2, HIGH);
      break;
    case 1:
      io.digitalWrite(IO_LED1, HIGH); io.digitalWrite(IO_LED2, LOW);
      break;
    case 2:
      io.digitalWrite(IO_LED1, HIGH); io.digitalWrite(IO_LED2, HIGH);
      Serial.print("PB1 Read: "); Serial.println(io.digitalRead(IO_PB1));
      Serial.print("PB2 Read: "); Serial.println(io.digitalRead(IO_PB3));
      break;
  }
  ioStep = (ioStep + 1) % 3;
}

// ---------------- OUTPUT TEST ----------------

// Switch one output by its position in the test order (0 .. OUTPUT_COUNT-1)
void outputSet(int index, bool level) {
  switch (index) {
    case 0:  digitalWrite(OUTPUT5,  level); break;
    case 1:  digitalWrite(OUTPUT6,  level); break;
    case 2:  digitalWrite(OUTPUT7,  level); break;
    case 3:  digitalWrite(OUTPUT10, level); break;
    case 4:  digitalWrite(OUTPUT14, level); break;
    case 5:  ioport.digitalWrite(OUTPUT19, level); break;
    case 6:  ioport.digitalWrite(OUTPUT20, level); break;
    case 7:  digitalWrite(OUTPUT21, level); break;
    case 8:  ioport.digitalWrite(OUTPUT33, level); break;
    case 9:  ioport.digitalWrite(OUTPUT34, level); break;
    case 10: ioport.digitalWrite(OUTPUT38, level); break;
    case 11: digitalWrite(OUTPUT39, level); break;
    case 12: digitalWrite(OUTPUT40, level); break;
    case 13: ioport.digitalWrite(CS1, level); break;
    case 14: ioport.digitalWrite(CS2, level); break;
    case 15: ioport.digitalWrite(CS3, level); break;
    case 16: ioport.digitalWrite(CS4, level); break;
  }
}

const char* outputName(int index) {
  static const char* const names[OUTPUT_COUNT] = {
    "OUTPUT5",  "OUTPUT6",  "OUTPUT7",  "OUTPUT10", "OUTPUT14",
    "OUTPUT19", "OUTPUT20", "OUTPUT21", "OUTPUT33", "OUTPUT34",
    "OUTPUT38", "OUTPUT39", "OUTPUT40", "CS1",      "CS2",
    "CS3",      "CS4"
  };
  return names[index];
}

void allOutputsOff() {
  for (int i = 0; i < OUTPUT_COUNT; i++) {
    outputSet(i, LOW);
  }
  outputIndex = 0;
}

// Non-blocking walking test: switches the outputs ON one by one, holds, then all OFF, holds, repeat
void outputTestTask() {
  if (millis() - lastOutputStep < outputDelay) return;
  lastOutputStep = millis();

  if (outputIndex < OUTPUT_COUNT) {
    outputSet(outputIndex, HIGH);
    Serial.printf("%s ON\n", outputName(outputIndex));
    outputIndex++;
    outputDelay = (outputIndex == OUTPUT_COUNT) ? OUTPUT_HOLD_MS : OUTPUT_STEP_MS;
  } else {
    allOutputsOff();
    Serial.println("All outputs OFF");
    outputDelay = OUTPUT_HOLD_MS;
  }
}

// ---------------- UTILITY FUNCTIONS ----------------
const char* i2cDeviceName(byte address) {
  switch (address) {
    case 0x15: return "CST816S touch";
    case 0x41: return "PCA9536 buttons/LEDs";
    case 0x68: return "DS3231 RTC";
    case 0x75: return "PCA9539 I/O expander";
    default:   return "";
  }
}

void I2C_SCAN() {
  byte address;
  i2cDeviceCount = 0;
  Serial.println("\nScanning I2C...");
  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    if (Wire.endTransmission() == 0) {
      Serial.printf("I2C device found at address 0x%02X  !  %s\n", address, i2cDeviceName(address));
      i2cDeviceCount++;
      delay(1);
    }
  }
  Serial.printf("I2C scan complete: %d device(s) found\n", i2cDeviceCount);
}

void RTC_Check() {
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    okRtc = false;
  } else {
    if (rtc.lostPower()) {
      Serial.println("RTC lost power, lets set the time!");
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));   // set RTC to the compile time of this sketch
    }
    okRtc = true;
    Serial.println("RTC Initialized.");
    for (int a = 0; a < 5; a++) {               // print 5 timestamps, as in X1
      DateTime now = rtc.now();
      Serial.printf("%04d/%02d/%02d %s %02d:%02d:%02d\n", now.year(), now.month(), now.day(),
                    daysOfTheWeek[now.dayOfTheWeek()], now.hour(), now.minute(), now.second());
      delay(1000);
    }
  }
}

void SD_CHECK() {
  Serial.print("\nInitializing SD card...");
  if (SD.begin(SD_chipSelect)) {
    okSd = true;
    uint8_t cardType = SD.cardType();
    Serial.println("Card Mount: success");
    Serial.print("Card Type: ");
    if (cardType == CARD_MMC)       Serial.println("MMC");
    else if (cardType == CARD_SD)   Serial.println("SDSC");
    else if (cardType == CARD_SDHC) Serial.println("SDHC");
    else                            Serial.println("Unknown");
    Serial.printf("Card Size: %lluMB\n", SD.cardSize() / (1024 * 1024));
  } else {
    okSd = false;
    Serial.println("NO SD card");
  }
}

void ETHERNET_CHECK() {
  Serial.println("\nInitializing Ethernet...");
  Ethernet.init(ETH_CS);

  if (Ethernet.begin(mac, 10000) == 0) {            // DHCP, 10 second timeout
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet controller not found");
    } else if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable is not connected");
    } else {
      Serial.println("Failed to configure Ethernet using DHCP");
    }
    okEthernet = false;
    return;
  }

  okEthernet = true;
  Serial.print("Ethernet OK, IP address: ");
  Serial.println(Ethernet.localIP());

  Udp.begin(localPort);
  sendNTPpacket(timeServer);
  delay(1000);
  if (Udp.parsePacket()) {
    Udp.read(packetBuffer, NTP_PACKET_SIZE);
    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    unsigned long epoch = secsSince1900 - 2208988800UL;
    okNtp = true;
    Serial.print("Seconds since Jan 1 1900 = ");
    Serial.println(secsSince1900);
    Serial.print("Unix time = ");
    Serial.println(epoch);
    Serial.printf("The UTC time is %02lu:%02lu:%02lu\n", (epoch % 86400L) / 3600, (epoch % 3600) / 60, epoch % 60);
  } else {
    okNtp = false;
    Serial.println("No NTP response");
  }
}

void sendNTPpacket(const char * address) {
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  packetBuffer[0] = 0b11100011;
  packetBuffer[2] = 6;
  packetBuffer[3] = 0xEC;
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;
  Udp.beginPacket(address, 123);
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}

void WIFI_CHECK() {
  Serial.println("\nInitializing Wi-Fi (SoftAP mode)...");
  WiFi.mode(WIFI_MODE_APSTA);

  byte wifiMac[6];
  WiFi.macAddress(wifiMac);
  char macText[18];
  snprintf(macText, sizeof(macText), "%02X:%02X:%02X:%02X:%02X:%02X",
           wifiMac[0], wifiMac[1], wifiMac[2], wifiMac[3], wifiMac[4], wifiMac[5]);
  Serial.printf("MAC Address: %s\n", macText);

  const char* password = "12345678";
  okWifi = WiFi.softAP(macText, password);          // SSID = MAC address

  if (okWifi) {
    Serial.print("Access Point IP address: ");
    Serial.println(WiFi.softAPIP());
    Serial.printf("Access Point SSID: %s\n", macText);
    Serial.println("Wi-Fi Access Point is active!");
  } else {
    Serial.println("Wi-Fi Access Point failed to start");
  }
}

void RS485_CHECK() {
  Serial.println("\nTesting RS485 (flow control is automatic)...");
  while (Serial2.available()) Serial2.read();       // clear anything old
  Serial2.println("RS485 01 SUCCESS");
  delay(100);

  String reply = "";
  while (Serial2.available()) {
    reply += (char)Serial2.read();
  }
  reply.trim();

  if (reply.indexOf("SUCCESS") >= 0) {
    okRs485 = true;
    Serial.println("RS485 echo received: " + reply);
  } else {
    okRs485 = false;
    Serial.println("No RS485 echo (connect a loopback to see the message come back)");
  }
}

void printSummary() {
  Serial.println("\n================ TEST SUMMARY ================");
  Serial.printf("I2C devices : %d found (expected 4: 0x15, 0x41, 0x68, 0x75)\n", i2cDeviceCount);
  Serial.printf("PCA9536     : %s\n", okPca9536  ? "OK" : "NOT FOUND");
  Serial.printf("RTC DS3231  : %s\n", okRtc      ? "OK" : "NOT FOUND");
  Serial.printf("SD card     : %s\n", okSd       ? "OK" : "NOT FOUND");
  Serial.printf("Ethernet    : %s\n", okEthernet ? "OK" : "FAILED");
  Serial.printf("NTP         : %s\n", okNtp      ? "OK" : "NO RESPONSE");
  Serial.printf("Wi-Fi AP    : %s\n", okWifi     ? "OK" : "FAILED");
  Serial.printf("RS485 echo  : %s\n", okRs485    ? "OK" : "NO ECHO");
  Serial.println("==============================================");
  Serial.println("Touch the screen to see gesture and X/Y; press PB1 / PB3 to see the button messages.\n");
}

// ---------------- TFT BENCHMARK (same tests as the X1 test) ----------------

unsigned long testFillScreen() {
  unsigned long start = micros();
  tft.fillScreen(TFT_BLACK);
  tft.fillScreen(TFT_RED);
  tft.fillScreen(TFT_GREEN);
  tft.fillScreen(TFT_BLUE);
  tft.fillScreen(TFT_BLACK);
  return micros() - start;
}

unsigned long testText() {
  tft.fillScreen(TFT_BLACK);
  unsigned long start = micros();
  tft.setTextDatum(lgfx::textdatum_t::top_left);   // built-in font is positioned from its top-left
  tft.setFont(&fonts::Font0);   // built-in GLCD-style font for the benchmark text
  tft.setCursor(0, 0);
  tft.setTextColor(TFT_WHITE);  tft.setTextSize(1);
  tft.println("Hello World!");
  tft.setTextColor(TFT_YELLOW); tft.setTextSize(2);
  tft.println(1234.56);
  tft.setTextColor(TFT_RED);    tft.setTextSize(3);
  tft.println(0xDEADBEEF, HEX);
  tft.println();
  tft.setTextColor(TFT_GREEN);
  tft.setTextSize(5);
  tft.println("Groop");
  tft.setTextSize(2);
  tft.println("I implore thee,");
  tft.setTextSize(1);
  tft.println("my foonting turlingdromes.");
  tft.println("And hooptiously drangle me");
  tft.println("with crinkly bindlewurdles,");
  tft.println("Or I will rend thee");
  tft.println("in the gobberwarts");
  tft.println("with my blurglecruncheon,");
  tft.println("see if I don't!");
  return micros() - start;
}

unsigned long testLines(uint16_t color) {
  unsigned long start, t;
  int           x1, y1, x2, y2,
                w = tft.width(),
                h = tft.height();

  tft.fillScreen(TFT_BLACK);

  x1 = y1 = 0;
  y2    = h - 1;
  start = micros();
  for (x2 = 0; x2 < w; x2 += 6) tft.drawLine(x1, y1, x2, y2, color);
  x2    = w - 1;
  for (y2 = 0; y2 < h; y2 += 6) tft.drawLine(x1, y1, x2, y2, color);
  t     = micros() - start; // fillScreen doesn't count against timing

  tft.fillScreen(TFT_BLACK);

  x1    = w - 1;
  y1    = 0;
  y2    = h - 1;
  start = micros();
  for (x2 = 0; x2 < w; x2 += 6) tft.drawLine(x1, y1, x2, y2, color);
  x2    = 0;
  for (y2 = 0; y2 < h; y2 += 6) tft.drawLine(x1, y1, x2, y2, color);
  t    += micros() - start;

  tft.fillScreen(TFT_BLACK);

  x1    = 0;
  y1    = h - 1;
  y2    = 0;
  start = micros();
  for (x2 = 0; x2 < w; x2 += 6) tft.drawLine(x1, y1, x2, y2, color);
  x2    = w - 1;
  for (y2 = 0; y2 < h; y2 += 6) tft.drawLine(x1, y1, x2, y2, color);
  t    += micros() - start;

  tft.fillScreen(TFT_BLACK);

  x1    = w - 1;
  y1    = h - 1;
  y2    = 0;
  start = micros();
  for (x2 = 0; x2 < w; x2 += 6) tft.drawLine(x1, y1, x2, y2, color);
  x2    = 0;
  for (y2 = 0; y2 < h; y2 += 6) tft.drawLine(x1, y1, x2, y2, color);

  return micros() - start;
}

unsigned long testFastLines(uint16_t color1, uint16_t color2) {
  unsigned long start;
  int           x, y, w = tft.width(), h = tft.height();

  tft.fillScreen(TFT_BLACK);
  start = micros();
  for (y = 0; y < h; y += 5) tft.drawFastHLine(0, y, w, color1);
  for (x = 0; x < w; x += 5) tft.drawFastVLine(x, 0, h, color2);

  return micros() - start;
}

unsigned long testRects(uint16_t color) {
  unsigned long start;
  int           n, i, i2,
                cx = tft.width()  / 2,
                cy = tft.height() / 2;

  tft.fillScreen(TFT_BLACK);
  n     = min(tft.width(), tft.height());
  start = micros();
  for (i = 2; i < n; i += 6) {
    i2 = i / 2;
    tft.drawRect(cx - i2, cy - i2, i, i, color);
  }

  return micros() - start;
}

unsigned long testFilledRects(uint16_t color1, uint16_t color2) {
  unsigned long start, t = 0;
  int           n, i, i2,
                cx = tft.width()  / 2 - 1,
                cy = tft.height() / 2 - 1;

  tft.fillScreen(TFT_BLACK);
  n = min(tft.width(), tft.height());
  for (i = n - 1; i > 0; i -= 6) {
    i2    = i / 2;
    start = micros();
    tft.fillRect(cx - i2, cy - i2, i, i, color1);
    t    += micros() - start;
    // Outlines are not included in timing results
    tft.drawRect(cx - i2, cy - i2, i, i, color2);
  }

  return t;
}

unsigned long testFilledCircles(uint8_t radius, uint16_t color) {
  unsigned long start;
  int x, y, w = tft.width(), h = tft.height(), r2 = radius * 2;

  tft.fillScreen(TFT_BLACK);
  start = micros();
  for (x = radius; x < w; x += r2) {
    for (y = radius; y < h; y += r2) {
      tft.fillCircle(x, y, radius, color);
    }
  }

  return micros() - start;
}

unsigned long testCircles(uint8_t radius, uint16_t color) {
  unsigned long start;
  int           x, y, r2 = radius * 2,
                      w = tft.width()  + radius,
                      h = tft.height() + radius;

  // Screen is not cleared for this one -- this is
  // intentional and does not affect the reported time.
  start = micros();
  for (x = 0; x < w; x += r2) {
    for (y = 0; y < h; y += r2) {
      tft.drawCircle(x, y, radius, color);
    }
  }

  return micros() - start;
}

unsigned long testTriangles() {
  unsigned long start;
  int           n, i, cx = tft.width()  / 2 - 1,
                      cy = tft.height() / 2 - 1;

  tft.fillScreen(TFT_BLACK);
  n     = min(cx, cy);
  start = micros();
  for (i = 0; i < n; i += 5) {
    tft.drawTriangle(
      cx    , cy - i, // peak
      cx - i, cy + i, // bottom left
      cx + i, cy + i, // bottom right
      tft.color565(0, 0, i));
  }

  return micros() - start;
}

unsigned long testFilledTriangles() {
  unsigned long start, t = 0;
  int           i, cx = tft.width()  / 2 - 1,
                   cy = tft.height() / 2 - 1;

  tft.fillScreen(TFT_BLACK);
  start = micros();
  for (i = min(cx, cy); i > 10; i -= 5) {
    start = micros();
    tft.fillTriangle(cx, cy - i, cx - i, cy + i, cx + i, cy + i,
                     tft.color565(0, i, i));
    t += micros() - start;
    tft.drawTriangle(cx, cy - i, cx - i, cy + i, cx + i, cy + i,
                     tft.color565(i, i, 0));
  }

  return t;
}

unsigned long testRoundRects() {
  unsigned long start;
  int           w, i, i2,
                cx = tft.width()  / 2 - 1,
                cy = tft.height() / 2 - 1;

  tft.fillScreen(TFT_BLACK);
  w     = min(tft.width(), tft.height());
  start = micros();
  for (i = 0; i < w; i += 6) {
    i2 = i / 2;
    tft.drawRoundRect(cx - i2, cy - i2, i, i, i / 8, tft.color565(i, 0, 0));
  }

  return micros() - start;
}

unsigned long testFilledRoundRects() {
  unsigned long start;
  int           i, i2,
                cx = tft.width()  / 2 - 1,
                cy = tft.height() / 2 - 1;

  tft.fillScreen(TFT_BLACK);
  start = micros();
  for (i = min(tft.width(), tft.height()); i > 20; i -= 6) {
    i2 = i / 2;
    tft.fillRoundRect(cx - i2, cy - i2, i, i, i / 8, tft.color565(0, i, 0));
  }

  return micros() - start;
}
```

## Expected Output
On the Serial Monitor (115200 baud):
- `NPD-NORVI XN-CPU-ESPS3`
- I2C scan with device names, e.g. `I2C device found at address 0x15  !  CST816S touch`, `0x41  PCA9536 buttons/LEDs`, `0x68  DS3231 RTC`, `0x75  PCA9539 I/O expander`, then `I2C scan complete: 4 device(s) found`
- `PCA9536 OK (buttons & LEDs ready).` and the touch controller version bytes
- `Outputs ready (17 outputs, walking test runs in loop).`
- RTC status (preceded by `RTC lost power, lets set the time!` only if the RTC had lost power) and 5 timestamps
- SD card mount status, card type and size in MB (or `NO SD card`)
- Ethernet DHCP result and IP address; if successful, NTP query with `Seconds since Jan 1 1900`, `Unix time` and the UTC time (or `No NTP response`)
- WiFi SoftAP: MAC address (used as SSID), AP IP address, SSID confirmation
- RS485 result (`RS485 echo received: ...` or `No RS485 echo (connect a loopback ...)`)
- `LovyanGFX library test!` followed by the benchmark timings in microseconds for each graphics primitive
- A **TEST SUMMARY** block, one line per test (`OK` / `NOT FOUND` / `FAILED` / `NO RESPONSE` / `NO ECHO`)
- In loop():
  - on every touch one tab-separated line: gesture, points, event, X, Y (e.g. `SINGLE CLICK  1  0  120  160`)
  - `IO TEST`, `PB1 Read: 1`, `PB2 Read: 1` once per LED cycle (0 while the button is held), and `PB1 pressed` / `PB1 released` (same for PB3)
  - `OUTPUT5 ON`, `OUTPUT6 ON` ... `CS4 ON`, `All outputs OFF`, repeating
  - `RS485 01 SUCCESS` echoed back once per second if an RS485 loopback is connected

On the TFT display: "NORVI XN" title, then "Touch Test" with the last gesture, `Pts` / `Evt`, `X` / `Y` coordinates and `Touches: n`; the RTC date and time (updated every second); and the "IO TEST" label. The PCA9536 LEDs chase (LED1, LED2, off) continuously.

## Common Issues
| Symptom | Likely Cause | Fix |
|---|---|---|
| Nothing appears in the Serial Monitor | USB CDC On Boot is Disabled | Set USB CDC On Boot: Enabled, USB Mode: Hardware CDC and JTAG, Upload Mode: UART0 / Hardware CDC |
| `PCA9536 not detected.` / PCA9539 outputs do not switch | IO expanders still held in reset, or I2C wiring fault | Confirm `PORT_RST` (GPIO 38) is driven HIGH with a ~100 ms wait before `io.begin()`; verify SDA/SCL on GPIO 8/9 and that the board is the V2 N16R2 variant |
| Display stays blank | `tft.init()` called before `touch.begin()` (shared reset GPIO 47) | Call `touch.begin()` first, then `tft.init()` |
| Display colors wrong or inverted | Panel invert / RGB order setting differs from the panel in use | Toggle `c.invert` or `c.rgb_order` in the `LGFX` class |
| Display or SD/Ethernet misbehave when used together | SPI bus contention | Drive ETH_CS (1), SD CS (42) and DSP_CS (45) HIGH right after `SPI.begin()` |
| `Couldn't find RTC` | DS3231 not detected on the I2C bus | Check I2C bus and RTC connection |
| RTC time wrong after reflashing | RTC is set only when it lost power | Remove the RTC battery briefly, or call `rtc.adjust(...)` once |
| `NO SD card found` | No card inserted | Insert SD card; verify SPI wiring and CS pin 42 |
| `Ethernet controller not found` / `Ethernet cable is not connected` / `Failed to configure Ethernet using DHCP` | Controller not detected, cable unplugged, or no DHCP server | Check SPI/CS 1, cable/link and DHCP availability |
| `No RS485 echo` | No loopback connected (the test only echoes with a loopback) | Connect RS485 A/B to a loopback or another device; do not add a direction pin - flow control is automatic |
| Connected expansion module resets during the output test | OUTPUT21 (GPIO 21) is shared with PCA_RESET | Disconnect the module or skip OUTPUT21 in the test |
| Upload fails / board not detected | Wrong Board/Tools settings | Match settings exactly to the table above (ESP32S3 Dev Module, QSPI PSRAM, 16MB flash, Minimal SPIFFS partition) |

## Related Files
- CPU board: `cpu_board_XN.md`
