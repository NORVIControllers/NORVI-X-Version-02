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
