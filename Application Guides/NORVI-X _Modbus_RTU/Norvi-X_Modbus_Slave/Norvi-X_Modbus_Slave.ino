
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
#define RS485_DE   -1        // -1 = auto-direction transceiver (NORVI-X)

#define MISO       13
#define MOSI       11
#define SCLK       12
#define DSP_CS     45
#define PORT_RST   38

#define IO_PB1     0
#define IO_LED1    1         // GREEN / heartbeat
#define IO_LED2    2         // RED / activity
#define IO_PB3     3
/* ------------------------------------------------------------------------ */

/* ---------------- MODBUS CONFIG ----------------------------------------- */
#define RS485_BAUD   9600
#define SLAVE_ID     1
#define HREG_COUNT   4       // holding regs 0..3
#define COIL_COUNT   2       // coils 0..1
/* ------------------------------------------------------------------------ */

TFT_eSPI  tft = TFT_eSPI();
PCA9536   io;
ModbusRTU mb;

bool     ledOk    = false;
uint32_t reqCount = 0;       // requests served (counts on any Rx activity)

/* Count served requests via callback on each register access -------------- */
uint16_t cbAccess(TRegister*, uint16_t val) {
  reqCount++;
  return val;
}

/* ---------------------------- DISPLAY ----------------------------------- */
#define COL_BG    TFT_BLACK
#define COL_BAR   TFT_DARKGREEN
#define COL_LABEL TFT_CYAN
#define COL_VALUE TFT_WHITE

static const int ROW_Y[HREG_COUNT] = { 96, 122, 148, 174 };

void drawStaticUI() {
  tft.fillScreen(COL_BG);
  tft.fillRect(0, 0, tft.width(), 34, COL_BAR);
  tft.setFreeFont(FSB12);
  tft.setTextColor(TFT_YELLOW, COL_BAR);
  tft.setCursor(6, 24);
  tft.print("MODBUS SLAVE");

  tft.setFreeFont(FSB9);
  tft.setTextColor(COL_LABEL, COL_BG);
  tft.setCursor(6, 56);  tft.print("ID");
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
  tft.setTextColor(COL_VALUE, COL_BG);
  for (int i = 0; i < HREG_COUNT; i++)
    tft.drawNumber(mb.Hreg(i), 70, ROW_Y[i] - 12);

  tft.setTextPadding(0);
  tft.setTextColor(TFT_DARKGREY, COL_BG);
  char buf[32];
  snprintf(buf, sizeof(buf), "served:%lu", reqCount);
  tft.drawString("                 ", 6, tft.height() - 20);
  tft.drawString(buf, 6, tft.height() - 20);
}

/* ---------------------------- SETUP ------------------------------------- */
void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("NORVI-X CPU | Modbus RTU SLAVE");

  Wire.begin(SDA, SCL);
  ledOk = io.begin();
  if (ledOk) {
    io.pinMode(IO_LED1, OUTPUT);
    io.pinMode(IO_LED2, OUTPUT);
    io.digitalWrite(IO_LED1, HIGH);
    io.digitalWrite(IO_LED2, HIGH);
  }

  SPI.begin(SCLK, MISO, MOSI);
  pinMode(PORT_RST, OUTPUT); digitalWrite(PORT_RST, HIGH);
  tft.init();
  tft.setRotation(0);
  drawStaticUI();

  // RS-485 / Modbus
  Serial2.begin(RS485_BAUD, SERIAL_8N1, RS485_RXD, RS485_TXD);
  mb.begin(&Serial2, RS485_DE);
  mb.slave(SLAVE_ID);

  // Register map
  for (int i = 0; i < HREG_COUNT; i++) {
    mb.addHreg(i, 0);
    mb.onGetHreg(i, cbAccess);          // count reads
    mb.onSetHreg(i, cbAccess);          // count writes
  }
  for (int i = 0; i < COIL_COUNT; i++) mb.addCoil(i, false);
}

/* ---------------------------- LOOP -------------------------------------- */
void loop() {
  mb.task();          // handle incoming frames
  yield();

  // Update the data this slave exposes (replace with real sensor/IO reads)
  static uint32_t lastUpd = 0;
  if (millis() - lastUpd >= 250) {
    lastUpd = millis();
    mb.Hreg(0, (uint16_t)(millis() / 1000));      // uptime seconds
    mb.Hreg(1, (uint16_t)random(0, 1000));        // e.g. analog value
    mb.Hreg(2, mb.Coil(0) ? 1 : 0);               // reflect coil0
    mb.Hreg(3, 0xBEEF);                           // fixed marker
  }

  static uint32_t lastUi = 0;
  if (millis() - lastUi >= 300) {
    lastUi = millis();
    updateUI();
    if (ledOk) {                                   // heartbeat
      static bool hb = false; hb = !hb;
      io.digitalWrite(IO_LED1, hb ? LOW : HIGH);
    }
  }
}
