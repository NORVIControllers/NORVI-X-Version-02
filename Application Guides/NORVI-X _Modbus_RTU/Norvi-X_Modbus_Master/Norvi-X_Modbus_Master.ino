
#include <Wire.h>
#include <SPI.h>
#include "TFT_eSPI.h"
#include "Free_Fonts.h"
#include <PCA9536D.h>
#include <ModbusRTU.h>

/* ---------------- PIN DEFINITIONS (from NORVI-X test code) --------------- */
#define SDA        8
#define SCL        9

#define RS485_RXD  16
#define RS485_TXD  15
#define RS485_DE   -1        // -1 = auto-direction transceiver (NORVI-X). Set to a GPIO if your unit needs DE/RE.

#define MISO       13
#define MOSI       11
#define SCLK       12
#define DSP_CS     45
#define PORT_RST   38

// Onboard status IO expander (PCA9536)
#define IO_PB1     0
#define IO_LED1    1         // used as GREEN / OK
#define IO_LED2    2         // used as RED / FAULT
#define IO_PB3     3
/* ------------------------------------------------------------------------ */

/* ---------------- MODBUS CONFIG ----------------------------------------- */
#define RS485_BAUD   9600            // Modbus RTU baud (match your slaves)
#define SLAVE_ID     1               // target slave address
#define HREG_START   0               // first holding register to read
#define HREG_COUNT   4               // number of registers to read
#define POLL_MS      500             // poll interval
#define OFFLINE_AFTER 3              // consecutive failures -> mark OFFLINE
/* ------------------------------------------------------------------------ */

TFT_eSPI  tft = TFT_eSPI();
PCA9536   io;
ModbusRTU mb;

bool      ledOk       = false;       // PCA9536 present?
uint16_t  hr[HREG_COUNT] = {0};      // last good register values
bool      online      = false;
uint8_t   failStreak  = 0;
uint32_t  pollCount   = 0;
uint32_t  errCount    = 0;
uint8_t   lastResult  = 0;           // last Modbus::ResultCode
uint32_t  lastPoll    = 0;
bool      txBusy      = false;

/* Modbus transaction callback -------------------------------------------- */
bool onData(Modbus::ResultCode event, uint16_t, void*) {
  txBusy     = false;
  lastResult = event;
  pollCount++;

  if (event == Modbus::EX_SUCCESS) {
    failStreak = 0;
    online     = true;
  } else {
    errCount++;
    if (++failStreak >= OFFLINE_AFTER) online = false;
  }
  return true;
}

/* ---------------------------- DISPLAY ----------------------------------- */
#define COL_BG      TFT_BLACK
#define COL_BAR     TFT_NAVY
#define COL_LABEL   TFT_CYAN
#define COL_VALUE   TFT_WHITE
#define COL_OK      TFT_GREEN
#define COL_ERR     TFT_RED

static const int ROW_Y[HREG_COUNT] = { 96, 122, 148, 174 };

void drawStaticUI() {
  tft.fillScreen(COL_BG);
  tft.fillRect(0, 0, tft.width(), 34, COL_BAR);
  tft.setFreeFont(FSB12);
  tft.setTextColor(TFT_YELLOW, COL_BAR);
  tft.setCursor(6, 24);
  tft.print("MODBUS MASTER");

  tft.setFreeFont(FSB9);
  tft.setTextColor(COL_LABEL, COL_BG);
  tft.setCursor(6, 56);  tft.print("Slave");
  tft.setCursor(70, 56); tft.setTextColor(COL_VALUE, COL_BG); tft.print(SLAVE_ID);

  const char* lbl[HREG_COUNT] = { "REG0", "REG1", "REG2", "REG3" };
  for (int i = 0; i < HREG_COUNT; i++) {
    tft.setTextColor(COL_LABEL, COL_BG);
    tft.setCursor(6, ROW_Y[i]); tft.print(lbl[i]);
  }
}

void updateUI() {
  tft.setFreeFont(FSB9);
  tft.setTextPadding(90);

  // link status
  tft.setTextColor(online ? COL_OK : COL_ERR, COL_BG);
  tft.setTextDatum(TL_DATUM);
  tft.drawString(online ? "ONLINE " : "OFFLINE", 120, 44);
  
  // register values
  tft.setTextColor(COL_VALUE, COL_BG);
  for (int i = 0; i < HREG_COUNT; i++) {
    tft.drawNumber(hr[i], 70, ROW_Y[i] - 12);
  }

  // footer counters
  tft.setTextPadding(0);
  tft.setTextColor(TFT_DARKGREY, COL_BG);
  char buf[40];
  snprintf(buf, sizeof(buf), "poll:%lu err:%lu", pollCount, errCount);
  tft.drawString("                    ", 6, tft.height() - 20);
  tft.drawString(buf, 6, tft.height() - 20);
}

/* ---------------------------- SETUP ------------------------------------- */
void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("NORVI-X CPU | Modbus RTU MASTER");

  // Status LEDs (non-fatal if missing)
  Wire.begin(SDA, SCL);
  ledOk = io.begin();
  if (ledOk) {
    io.pinMode(IO_LED1, OUTPUT);
    io.pinMode(IO_LED2, OUTPUT);
    io.digitalWrite(IO_LED1, HIGH);   // off
    io.digitalWrite(IO_LED2, HIGH);
  }

  // Display
  SPI.begin(SCLK, MISO, MOSI);
  pinMode(PORT_RST, OUTPUT); digitalWrite(PORT_RST, HIGH); // enable IO expander rail
  tft.init();
  tft.setRotation(0);
  drawStaticUI();

  // RS-485 / Modbus
  Serial2.begin(RS485_BAUD, SERIAL_8N1, RS485_RXD, RS485_TXD);
  mb.begin(&Serial2, RS485_DE);   // -1 => auto-direction, no DE toggling
  mb.master();
}

/* ---------------------------- LOOP -------------------------------------- */
void loop() {
  // Non-blocking poll
  if (!txBusy && millis() - lastPoll >= POLL_MS) {
    lastPoll = millis();
    if (mb.slave() == 0) {                 // bus free
      txBusy = true;
      mb.readHreg(SLAVE_ID, HREG_START, hr, HREG_COUNT, onData);
    }
  }

  mb.task();      // MUST run every loop
  yield();

  // Refresh display + LEDs at a steady rate
  static uint32_t lastUi = 0;
  if (millis() - lastUi >= 200) {
    lastUi = millis();
    updateUI();
    if (ledOk) {
      io.digitalWrite(IO_LED1, online ? LOW  : HIGH);  // green when ONLINE
      io.digitalWrite(IO_LED2, online ? HIGH : LOW);   // red when OFFLINE
    }
  }
}
