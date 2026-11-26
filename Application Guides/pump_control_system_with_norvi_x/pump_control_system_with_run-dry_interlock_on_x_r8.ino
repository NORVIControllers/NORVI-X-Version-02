// NORVI X1 + X-R8 pump control with run-dry interlock and run-hour counter
// Relay 1 = contactor coil, relay 2 = motor-ON lamp
// Serial commands (115200): s = start, x = stop, r = reset fault, p = print pressure

// Types first, so the Arduino auto-generated prototypes can see them
#include <stdint.h>

struct Rect { int x, y, w, h; };

enum PumpState { ST_STOPPED = 0, ST_RUNNING = 1, ST_FAULT = 2 };
enum FaultCode { FLT_NONE = 0, FLT_DRY = 1, FLT_SENSOR = 2, FLT_IO = 3 };

struct Btn {
  uint8_t  pin;
  uint8_t  idle;
  bool     raw;
  bool     pressed;
  uint32_t chg;
  bool     fired;
};

#include <Wire.h>
#include <SPI.h>
#include <Preferences.h>
#include <PCA9538.h>
#include <PCA9536D.h>
#include <CST816S.h>

#define LGFX_USE_V1
#include <LovyanGFX.hpp>

// ============================ USER CONFIGURATION ============================
#define R8_I2C_ADDR     0x73
#define PUMP_RELAY_PIN  1
#define LAMP_RELAY_PIN  2
#define RELAY_ON_LEVEL  HIGH

#define DRY_SRC_SIM     0
#define DRY_SRC_MODBUS  1
#define DRY_SOURCE      DRY_SRC_MODBUS     // or DRY_SRC_SIM for bench testing

// Run-dry trip pressure: pump stops if pressure stays below this while running
#define PRESSURE_UNIT   "bar"
static const float RUN_DRY_PRESSURE = 0.50f;      // trip limit

// Protection timing
static const uint32_t START_BYPASS_MS   = 6000;    // priming time
static const uint32_t DRY_DEBOUNCE_MS   = 2000;    // low-pressure time to trip
static const uint32_t SENSOR_FAIL_MS    = 3000;    // no-data time to trip
static const uint32_t MIN_OFF_MS        = 5000;    // restart lockout
static const uint32_t SAVE_INTERVAL_MS  = 60000;   // hour counter save period

// Modbus RTU pressure sensor (check against your sensor's register map)
#define MB_BAUD          9600
#define MB_SLAVE_ID      1
#define MB_FUNC          0x03      // 0x03 holding, 0x04 input register
#define MB_REG_PRESSURE  0x0000
#define MB_SIGNED        0
static const float    MB_SCALE       = 0.001f;     // unit per count
static const float    MB_OFFSET      = 0.0f;
static const float    PRESS_FILTER   = 0.5f;       // smaller = smoother
static const uint32_t MB_POLL_MS     = 500;
static const uint32_t MB_TIMEOUT_MS  = 150;

// Touch orientation
#define TOUCH_FLIP_X 0
#define TOUCH_FLIP_Y 0

// Onboard LEDs are active-low
#define LED_ON_LEVEL LOW

// ================================= PINS =====================================
#define PIN_SDA        8
#define PIN_SCL        9
#define PIN_MISO       13
#define PIN_MOSI       11
#define PIN_SCLK       12
#define PIN_PORT_RST   38
#define PIN_PCA_RESET  21
#define PIN_RS485_RX   16
#define PIN_RS485_TX   15

#define IO_PB1   0
#define IO_LED1  1
#define IO_LED2  2
#define IO_PB3   3

// ============================== DISPLAY SETUP ===============================
class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ST7789 _panel;
  lgfx::Bus_SPI      _bus;
public:
  LGFX(void) {
    { auto c = _bus.config();
      c.spi_host    = SPI2_HOST;
      c.spi_mode    = 0;
      c.freq_write  = 27000000;
      c.freq_read   = 16000000;
      c.spi_3wire   = false;
      c.use_lock    = true;
      c.dma_channel = SPI_DMA_CH_AUTO;
      c.pin_sclk    = PIN_SCLK;
      c.pin_mosi    = PIN_MOSI;
      c.pin_miso    = PIN_MISO;
      c.pin_dc      = 46;
      _bus.config(c);
      _panel.setBus(&_bus);
    }
    { auto c = _panel.config();
      c.pin_cs   = 45;
      c.pin_rst  = 47;
      c.pin_busy = -1;
      c.panel_width  = 240;
      c.panel_height = 320;
      c.offset_x = 0;
      c.offset_y = 0;
      c.offset_rotation = 0;
      c.readable   = false;
      c.invert     = true;
      c.rgb_order  = false;
      c.dlen_16bit = false;
      c.bus_shared = true;
      _panel.config(c);
    }
    setPanel(&_panel);
  }
};

LGFX         tft;
PCA9538      relayIo(R8_I2C_ADDR);
PCA9536      ledIo;
CST816S      touch(PIN_SDA, PIN_SCL, 47, 48);
Preferences  prefs;

// ================================ COLOURS ===================================
constexpr uint16_t C565(uint8_t r, uint8_t g, uint8_t b) {
  return (uint16_t)(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
}
static const uint16_t C_BG     = C565(10, 14, 20);
static const uint16_t C_HDR    = C565(18, 24, 34);
static const uint16_t C_PANEL  = C565(22, 29, 40);
static const uint16_t C_BORDER = C565(52, 63, 80);
static const uint16_t C_TEXT   = C565(226, 232, 240);
static const uint16_t C_DIM    = C565(122, 134, 152);
static const uint16_t C_GREEN  = C565(0, 214, 118);
static const uint16_t C_AMBER  = C565(255, 176, 0);
static const uint16_t C_RED    = C565(235, 64, 52);
static const uint16_t C_RED_DK = C565(110, 22, 18);
static const uint16_t C_CYAN   = C565(0, 176, 224);

// ============================= UI GEOMETRY ==================================
static const Rect R_MOTOR     = {  6,  42, 228,  98 };
static const Rect R_HOURS     = {  6, 146, 228,  76 };
static const Rect R_DRY       = {  6, 228, 228,  44 };
static const Rect R_BTN_START = {  6, 278,  72,  36 };
static const Rect R_BTN_STOP  = { 84, 278,  72,  36 };
static const Rect R_BTN_RESET = {162, 278,  72,  36 };

static inline bool hit(const Rect& r, int x, int y) {
  return x >= r.x && x < r.x + r.w && y >= r.y && y < r.y + r.h;
}

// ============================== PUMP STATE ==================================
PumpState state = ST_STOPPED;
FaultCode fault = FLT_NONE;

bool     relayOn      = false;
bool     r8Ok         = false;
bool     ledOk        = false;
bool     simDry       = false;

uint32_t runStartMs   = 0;
uint32_t lastStopMs   = (uint32_t)(0 - MIN_OFF_MS);
uint32_t lastTick     = 0;
uint32_t runAccumMs   = 0;
uint32_t lastSaveMs   = 0;
uint32_t lastIoCheck  = 0;
bool     dryActive    = false;
uint32_t dryStartMs   = 0;
bool     sensBad      = false;
uint32_t sensBadMs    = 0;

uint32_t totalSec     = 0;      // persisted
uint32_t sessionSec   = 0;
uint32_t starts       = 0;      // persisted

// Filtered pressure
struct Sense { bool valid; bool dry; float value; } sense = { true, false, 0.0f };

char     msg[40]      = "";
uint32_t msgUntil     = 0;

// ============================== HELPERS =====================================
static void setMsg(const char* m, uint32_t ms = 3000) {
  strlcpy(msg, m, sizeof(msg));
  msgUntil = millis() + ms;
}

static bool isPriming() {
  return state == ST_RUNNING && (millis() - runStartMs) < START_BYPASS_MS;
}

static bool r8Present() {
  Wire.beginTransmission(R8_I2C_ADDR);
  return Wire.endTransmission() == 0;
}

// Lamp on relay 2 follows the contactor relay
static void relaySet(bool on) {
  uint8_t lvl = (uint8_t)(on ? RELAY_ON_LEVEL : !RELAY_ON_LEVEL);
  relayIo.digitalWrite(PUMP_RELAY_PIN, lvl);
  relayIo.digitalWrite(LAMP_RELAY_PIN, lvl);
  relayOn = on;
}

static void persist() {
  prefs.putULong("sec", totalSec);
  prefs.putULong("starts", starts);
}

static const char* faultText(FaultCode f) {
  switch (f) {
    case FLT_DRY:    return "RUN DRY";
    case FLT_SENSOR: return "SENSOR FAIL";
    case FLT_IO:     return "R8 I/O LOST";
    default:         return "";
  }
}

// ============================ MODBUS (optional) =============================
#if DRY_SOURCE == DRY_SRC_MODBUS
static uint16_t crc16(const uint8_t* d, size_t n) {
  uint16_t c = 0xFFFF;
  for (size_t i = 0; i < n; i++) {
    c ^= d[i];
    for (int b = 0; b < 8; b++) c = (c & 1) ? (c >> 1) ^ 0xA001 : (c >> 1);
  }
  return c;
}

static bool mbReadReg(uint8_t id, uint8_t func, uint16_t reg, uint16_t& val) {
  while (Serial2.available()) Serial2.read();
  uint8_t req[8] = { id, func, (uint8_t)(reg >> 8), (uint8_t)(reg & 0xFF), 0x00, 0x01, 0, 0 };
  uint16_t c = crc16(req, 6);
  req[6] = c & 0xFF;
  req[7] = c >> 8;
  Serial2.write(req, 8);
  Serial2.flush();

  uint8_t rsp[7];
  size_t n = 0;
  uint32_t t0 = millis();
  while (n < 7 && (millis() - t0) < MB_TIMEOUT_MS) {
    if (Serial2.available()) rsp[n++] = Serial2.read();
    else delay(1);
  }
  if (n < 7) return false;
  if (rsp[0] != id || rsp[1] != func || rsp[2] != 2) return false;
  uint16_t rc = crc16(rsp, 5);
  if ((rc & 0xFF) != rsp[5] || (rc >> 8) != rsp[6]) return false;
  val = ((uint16_t)rsp[3] << 8) | rsp[4];
  return true;
}
#endif

// =============================== SENSING ====================================
// Sensor is polled continuously; the dry decision is used only while running
static void senseUpdate() {
#if DRY_SOURCE == DRY_SRC_SIM
  sense.valid = true;
  sense.dry   = simDry;
  sense.value = 0.0f;
#else
  static uint32_t lastPoll = 0;
  static uint8_t  fails    = 0;
  static bool     haveAvg  = false;
  static float    avg      = 0.0f;
  uint32_t now = millis();
  if (now - lastPoll >= MB_POLL_MS) {
    lastPoll = now;
    uint16_t raw;
    if (mbReadReg(MB_SLAVE_ID, MB_FUNC, MB_REG_PRESSURE, raw)) {
      fails = 0;
#if MB_SIGNED
      float p = (float)(int16_t)raw * MB_SCALE + MB_OFFSET;
#else
      float p = (float)raw * MB_SCALE + MB_OFFSET;
#endif
      if (!haveAvg) { avg = p; haveAvg = true; }
      else          { avg += (p - avg) * PRESS_FILTER; }
      sense.valid = true;
      sense.value = avg;
      sense.dry   = (avg < RUN_DRY_PRESSURE);
    } else {
      if (fails < 255) fails++;
      if (fails >= 3) { sense.valid = false; haveAvg = false; }
    }
  }
#endif
}

// ============================ PUMP COMMANDS =================================
static void requestStart() {
  uint32_t now = millis();
  if (state == ST_FAULT)   { setMsg("RESET FAULT FIRST");       return; }
  if (state != ST_STOPPED) return;
  if (!r8Present())        { setMsg("X-R8 NOT FOUND");           return; }
  if ((now - lastStopMs) < MIN_OFF_MS) { setMsg("RESTART LOCKOUT");   return; }
#if DRY_SOURCE == DRY_SRC_SIM
  if (simDry)              { setMsg("START BLOCKED: NO WATER");  return; }
#endif
  relaySet(true);
  state       = ST_RUNNING;
  runStartMs  = now;
  lastTick    = now;
  runAccumMs  = 0;
  sessionSec  = 0;
  dryActive   = false;
  sensBad     = false;
  lastSaveMs  = now;
  lastIoCheck = now;
  starts++;
  persist();
  Serial.printf("[%lu] PUMP START (NO1 ON, lamp NO2 ON)\n", (unsigned long)now);
}

static void stopPump() {
  if (state != ST_RUNNING) return;
  relaySet(false);
  state      = ST_STOPPED;
  lastStopMs = millis();
  persist();
  Serial.printf("[%lu] PUMP STOP (NO1 OFF, lamp NO2 OFF) total=%lus\n", (unsigned long)millis(), (unsigned long)totalSec);
}

static void tripFault(FaultCode code) {
  relaySet(false);
  fault      = code;
  state      = ST_FAULT;
  lastStopMs = millis();
  persist();
#if DRY_SOURCE == DRY_SRC_MODBUS
  Serial.printf("[%lu] FAULT: %s -> NO1 / NO2 OFF (pressure %.2f %s, limit %.2f)\n", (unsigned long)millis(),
                faultText(code), sense.value, PRESSURE_UNIT, RUN_DRY_PRESSURE);
#else
  Serial.printf("[%lu] FAULT: %s -> NO1 / NO2 OFF\n", (unsigned long)millis(), faultText(code));
#endif
}

static void resetFault() {
  if (state != ST_FAULT) return;
#if DRY_SOURCE == DRY_SRC_SIM
  if (simDry) { setMsg("STILL DRY - CHECK WATER"); return; }
#endif
  if (!r8Present()) { setMsg("X-R8 NOT FOUND"); return; }
  fault      = FLT_NONE;
  state      = ST_STOPPED;
  lastStopMs = millis();
  Serial.println("Fault reset");
}

static void resetHours() {
  if (state == ST_RUNNING) return;
  totalSec = 0;
  starts   = 0;
  persist();
  setMsg("HOUR COUNTER RESET");
  Serial.println("Hour counter reset");
}

// ============================= SUPERVISION ==================================
static void supervise() {
  uint32_t now = millis();
  // Run-dry lock is active ONLY while the motor is running
  if (state != ST_RUNNING) { lastTick = now; dryActive = false; sensBad = false; return; }

  // Run-hour counter
  uint32_t dt = now - lastTick;
  lastTick = now;
  runAccumMs += dt;
  while (runAccumMs >= 1000) {
    runAccumMs -= 1000;
    totalSec++;
    sessionSec++;
  }
  if (now - lastSaveMs >= SAVE_INTERVAL_MS) {
    lastSaveMs = now;
    persist();
  }

  // Relay module health
  if (now - lastIoCheck >= 1000) {
    lastIoCheck = now;
    if (!r8Present()) { tripFault(FLT_IO); return; }
  }

  // Run-dry interlock (ignored while priming)
  if (isPriming()) { dryActive = false; sensBad = false; return; }

  if (!sense.valid) {
    if (!sensBad) { sensBad = true; sensBadMs = now; }
    if (now - sensBadMs >= SENSOR_FAIL_MS) tripFault(FLT_SENSOR);
    return;
  }
  sensBad = false;

  if (sense.dry) {
    if (!dryActive) { dryActive = true; dryStartMs = now; }
    if (now - dryStartMs >= DRY_DEBOUNCE_MS) tripFault(FLT_DRY);
  } else {
    dryActive = false;
  }
}

// ============================ BUTTONS / LEDS ================================
Btn pb1 = { IO_PB1, 1, false, false, 0, false };
Btn pb3 = { IO_PB3, 1, false, false, 0, false };
uint32_t comboSince = 0;

static void btnUpdate(Btn& b) {
  uint32_t now = millis();
  bool p = (ledIo.digitalRead(b.pin) != b.idle);
  if (p != b.raw) { b.raw = p; b.chg = now; }
  if ((now - b.chg) > 40 && b.pressed != b.raw) {
    b.pressed = b.raw;
    if (b.pressed) b.fired = true;
  }
}

static void readButtons() {
  if (!ledOk) return;
  btnUpdate(pb1);
  btnUpdate(pb3);

  if (pb1.fired) {
    pb1.fired = false;
    if (state == ST_RUNNING) stopPump(); else requestStart();
  }
  if (pb3.fired) {
    pb3.fired = false;
    resetFault();
  }

  // PB1 + PB3 held together 5 s while stopped: reset hour counter
  if (pb1.pressed && pb3.pressed) {
    if (!comboSince) comboSince = millis();
    if (comboSince != 0xFFFFFFFF && millis() - comboSince >= 5000) { resetHours(); comboSince = 0xFFFFFFFF; }
  } else {
    comboSince = 0;
  }
}

static void ledUpdate() {
  if (!ledOk) return;
  bool runLed = (state == ST_RUNNING);
  bool errLed = (state == ST_FAULT) && ((millis() / 400) & 1);
  ledIo.digitalWrite(IO_LED1, (uint8_t)(runLed ? LED_ON_LEVEL : !LED_ON_LEVEL));
  ledIo.digitalWrite(IO_LED2, (uint8_t)(errLed ? LED_ON_LEVEL : !LED_ON_LEVEL));
}

// ================================ TOUCH =====================================
uint32_t lastTouchSeen = 0;
uint32_t lastTouchAct  = 0;
uint32_t holdStart     = 0;
bool     holdDone      = false;

static void onTap(int x, int y) {
  if (hit(R_BTN_START, x, y))      { requestStart(); }
  else if (hit(R_BTN_STOP, x, y))  { stopPump(); }
  else if (hit(R_BTN_RESET, x, y)) { resetFault(); }
#if DRY_SOURCE == DRY_SRC_SIM
  else if (hit(R_DRY, x, y))       { simDry = !simDry; Serial.printf("SIM water %s\n", simDry ? "LOW (dry)" : "OK"); }
#endif
}

static void handleTouch() {
  uint32_t now = millis();
  if (touch.available()) {
    int x = touch.data.x;
    int y = touch.data.y;
#if TOUCH_FLIP_X
    x = 239 - x;
#endif
#if TOUCH_FLIP_Y
    y = 319 - y;
#endif
    lastTouchSeen = now;

    if (hit(R_HOURS, x, y)) {
      if (holdStart == 0) {
        holdStart = now;
        holdDone  = false;
        if (state != ST_RUNNING) setMsg("HOLD 3 S TO RESET HOURS", 1500);
      } else if (!holdDone && (now - holdStart) >= 3000) {
        holdDone = true;
        resetHours();
      }
    } else {
      holdStart = 0;
    }

    if ((now - lastTouchAct) >= 350) {
      lastTouchAct = now;
      onTap(x, y);
    }
  }
  if (holdStart && (now - lastTouchSeen) > 500) holdStart = 0;
}

// ================================= UI =======================================
static void txt(const char* s, int x, int y, const lgfx::IFont* f, uint16_t fg, uint16_t bg,
                lgfx::textdatum_t d = lgfx::textdatum_t::top_left, int pad = 0) {
  tft.setFont(f);
  tft.setTextDatum(d);
  tft.setTextColor(fg, bg);
  tft.setTextPadding(pad);
  tft.drawString(s, x, y);
  tft.setTextPadding(0);
}

static void panel(const Rect& r) {
  tft.fillRoundRect(r.x, r.y, r.w, r.h, 8, C_PANEL);
  tft.drawRoundRect(r.x, r.y, r.w, r.h, 8, C_BORDER);
}

static void lamp(int cx, int cy, uint16_t outer, uint16_t mid, uint16_t core, bool lit) {
  tft.fillCircle(cx, cy, 34, outer);
  tft.fillCircle(cx, cy, 29, mid);
  tft.fillCircle(cx, cy, 23, core);
  if (lit) tft.fillCircle(cx - 8, cy - 9, 6, C565(210, 255, 230));
}

static void drawHeader() {
  bool fl    = (state == ST_FAULT);
  bool blink = fl && ((millis() / 500) & 1);
  uint16_t bg = fl ? (blink ? C_RED : C_RED_DK) : C_HDR;

  tft.fillRect(0, 0, 240, 34, bg);
  tft.fillRect(0, 34, 240, 2, fl ? C_RED : C_CYAN);

  if (fl) {
    char t[32];
    snprintf(t, sizeof(t), "ALARM: %s", faultText(fault));
    txt(t, 120, 17, &fonts::FreeSansBold9pt7b, C_TEXT, bg, lgfx::textdatum_t::middle_center);
  } else {
    txt("PUMP CONTROL", 8, 17, &fonts::FreeSans9pt7b, C_TEXT, bg, lgfx::textdatum_t::middle_left);
    const char* chip; uint16_t cc;
    if (state == ST_RUNNING) {
      if (isPriming()) { chip = "PRIME"; cc = C_AMBER; }
      else             { chip = "RUN";   cc = C_GREEN; }
    } else               { chip = "STOP";  cc = C_BORDER; }
    tft.fillRoundRect(164, 6, 70, 22, 6, cc);
    txt(chip, 199, 17, &fonts::FreeSansBold9pt7b, (state == ST_STOPPED) ? C_TEXT : C_BG, cc,
        lgfx::textdatum_t::middle_center);
  }
}

static void drawMotor() {
  panel(R_MOTOR);
  int cx = R_MOTOR.x + 48;
  int cy = R_MOTOR.y + 49;
  int x0 = R_MOTOR.x + 96;

  const char* word; uint16_t wc;
  if (state == ST_FAULT) {
    word = "FAULT";   wc = C_RED;
    lamp(cx, cy, C565(110, 24, 20), C565(180, 40, 34), C565(235, 64, 52), true);
  } else if (state == ST_RUNNING && isPriming()) {
    word = "PRIMING"; wc = C_AMBER;
    lamp(cx, cy, C565(110, 76, 0), C565(190, 130, 0), C565(255, 176, 0), true);
  } else if (state == ST_RUNNING) {
    word = "RUNNING"; wc = C_GREEN;
    lamp(cx, cy, C565(0, 80, 50), C565(0, 140, 85), C565(0, 225, 120), true);
  } else {
    word = "STOPPED"; wc = C_DIM;
    lamp(cx, cy, C565(48, 58, 72), C565(30, 38, 50), C565(24, 30, 40), false);
  }

  txt("MOTOR", x0, R_MOTOR.y + 10, &fonts::FreeSans9pt7b, C_DIM, C_PANEL);
  txt(word, x0, R_MOTOR.y + 32, &fonts::FreeSansBold12pt7b, wc, C_PANEL);

  // Relay states
  char s1[12], s2[12];
  snprintf(s1, sizeof(s1), "NO1 %s", relayOn ? "ON" : "OFF");
  snprintf(s2, sizeof(s2), "NO2 %s", relayOn ? "ON" : "OFF");
  uint16_t sc = relayOn ? C_GREEN : C_DIM;
  txt(s1, x0,      R_MOTOR.y + 68, &fonts::Font2, sc, C_PANEL);
  txt(s2, x0 + 62, R_MOTOR.y + 68, &fonts::Font2, sc, C_PANEL);
}

static void drawHours(bool force) {
  static char lastHrs[16] = "";
  static char lastSub[48] = "";

  char hrs[16];
  snprintf(hrs, sizeof(hrs), "%05lu:%02lu",
           (unsigned long)(totalSec / 3600), (unsigned long)((totalSec / 60) % 60));
  char sub[48];
  snprintf(sub, sizeof(sub), "STARTS %lu   SESSION %02lu:%02lu:%02lu",
           (unsigned long)starts, (unsigned long)(sessionSec / 3600),
           (unsigned long)((sessionSec / 60) % 60), (unsigned long)(sessionSec % 60));

  if (force) {
    panel(R_HOURS);
    txt("RUN HOURS", R_HOURS.x + 12, R_HOURS.y + 6, &fonts::Font2, C_DIM, C_PANEL);
    txt("HHHHH:MM", R_HOURS.x + R_HOURS.w - 12, R_HOURS.y + 6, &fonts::Font2, C_DIM, C_PANEL,
        lgfx::textdatum_t::top_right);
    lastHrs[0] = 0;
    lastSub[0] = 0;
  }
  if (strcmp(hrs, lastHrs) != 0) {
    txt(hrs, R_HOURS.x + 12, R_HOURS.y + 22, &fonts::Orbitron_Light_32, C_CYAN, C_PANEL,
        lgfx::textdatum_t::top_left, 190);
    strlcpy(lastHrs, hrs, sizeof(lastHrs));
  }
  if (strcmp(sub, lastSub) != 0) {
    txt(sub, R_HOURS.x + 12, R_HOURS.y + 58, &fonts::Font2, C_DIM, C_PANEL,
        lgfx::textdatum_t::top_left, 204);
    strlcpy(lastSub, sub, sizeof(lastSub));
  }
}

// Run-dry / pressure tile
static void drawDry(bool force) {
  static char     last1[44] = "";
  static char     last2[44] = "";
  static uint16_t lastC1 = 0, lastC2 = 0;

  char l1[44], l2[44];
  uint16_t c1 = C_DIM, c2 = C_GREEN;
  uint32_t now = millis();
  bool msgOn = (int32_t)(msgUntil - now) > 0;

  if (msgOn) {
    strlcpy(l1, msg, sizeof(l1));
    c1 = C_AMBER;
  } else if (state == ST_FAULT) {
    strlcpy(l1, "PUMP STOPPED - RESET", sizeof(l1));
  } else {
#if DRY_SOURCE == DRY_SRC_SIM
    snprintf(l1, sizeof(l1), "RUN-DRY  [SIM - TAP TO TOGGLE]");
#else
    snprintf(l1, sizeof(l1), "RUN-DRY LIMIT < %.2f %s", RUN_DRY_PRESSURE, PRESSURE_UNIT);
#endif
  }

  if (state == ST_FAULT) {
    strlcpy(l2, faultText(fault), sizeof(l2));
    c2 = C_RED;
  } else if (!sense.valid) {
    strlcpy(l2, "SENSOR NO DATA", sizeof(l2));
    c2 = C_AMBER;
  } else {
#if DRY_SOURCE == DRY_SRC_SIM
    if (sense.dry) { strlcpy(l2, "WATER LOW (SIM)", sizeof(l2)); c2 = C_AMBER; }
    else           { strlcpy(l2, "WATER OK (SIM)",  sizeof(l2)); c2 = C_GREEN; }
#else
    if (state == ST_RUNNING && isPriming()) {
      snprintf(l2, sizeof(l2), "%.2f %s  PRIMING", sense.value, PRESSURE_UNIT);
      c2 = C_AMBER;
    } else if (state == ST_RUNNING) {
      snprintf(l2, sizeof(l2), "%.2f %s  %s", sense.value, PRESSURE_UNIT, sense.dry ? "LOW" : "OK");
      c2 = sense.dry ? C_AMBER : C_GREEN;
    } else {
      snprintf(l2, sizeof(l2), "%.2f %s  STANDBY", sense.value, PRESSURE_UNIT);
      c2 = C_DIM;
    }
#endif
  }

  if (force || strcmp(l1, last1) || strcmp(l2, last2) || c1 != lastC1 || c2 != lastC2) {
    panel(R_DRY);
    txt(l1, R_DRY.x + 12, R_DRY.y + 4,  &fonts::Font2,             c1, C_PANEL);
    txt(l2, R_DRY.x + 12, R_DRY.y + 21, &fonts::FreeSansBold9pt7b, c2, C_PANEL);
    strlcpy(last1, l1, sizeof(last1));
    strlcpy(last2, l2, sizeof(last2));
    lastC1 = c1;
    lastC2 = c2;
  }
}

static void button(const Rect& r, const char* label, uint16_t col, bool en) {
  if (en) {
    tft.fillRoundRect(r.x, r.y, r.w, r.h, 6, col);
    txt(label, r.x + r.w / 2, r.y + r.h / 2, &fonts::FreeSansBold9pt7b, C_BG, col,
        lgfx::textdatum_t::middle_center);
  } else {
    tft.fillRoundRect(r.x, r.y, r.w, r.h, 6, C_PANEL);
    tft.drawRoundRect(r.x, r.y, r.w, r.h, 6, C_BORDER);
    txt(label, r.x + r.w / 2, r.y + r.h / 2, &fonts::FreeSansBold9pt7b, C_DIM, C_PANEL,
        lgfx::textdatum_t::middle_center);
  }
}

static void drawButtons() {
  button(R_BTN_START, "START", C_GREEN, state == ST_STOPPED);
  button(R_BTN_STOP,  "STOP",  C_RED,   state == ST_RUNNING);
  button(R_BTN_RESET, "RESET", C_AMBER, state == ST_FAULT);
}

static void uiInit() {
  tft.fillScreen(C_BG);
  drawHours(true);
  drawDry(true);
}

static void uiUpdate() {
  static int32_t kHdr = -1, kMotor = -1, kBtn = -1;
  bool prime = isPriming();
  bool blink = (state == ST_FAULT) && ((millis() / 500) & 1);

  int32_t h = (int32_t)state | ((int32_t)prime << 4) | ((int32_t)fault << 5) | ((int32_t)blink << 8);
  if (h != kHdr) { kHdr = h; drawHeader(); }

  int32_t m = (int32_t)state | ((int32_t)prime << 4) | ((int32_t)fault << 5) | ((int32_t)relayOn << 8);
  if (m != kMotor) { kMotor = m; drawMotor(); }

  int32_t b = (int32_t)state;
  if (b != kBtn) { kBtn = b; drawButtons(); }

  drawHours(false);
  drawDry(false);
}

// ================================ SERIAL ====================================
static void handleSerial() {
  while (Serial.available()) {
    char c = Serial.read();
    switch (c) {
      case 's': requestStart(); break;
      case 'x': stopPump();     break;
      case 'r': resetFault();   break;
      case 'p':
#if DRY_SOURCE == DRY_SRC_MODBUS
        if (sense.valid) Serial.printf("Pressure %.3f %s (run-dry limit %.2f)\n", sense.value, PRESSURE_UNIT, RUN_DRY_PRESSURE);
        else             Serial.println("Pressure sensor: no data");
#else
        Serial.println("Pressure: sensor not used (simulated source)");
#endif
        break;
      default: break;
    }
  }
}

// ================================ SETUP / LOOP ==============================
void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("\n=== NORVI X1 + X-R8 PUMP CONTROL ===");

  pinMode(PIN_PCA_RESET, OUTPUT);
  digitalWrite(PIN_PCA_RESET, HIGH);
  pinMode(PIN_PORT_RST, OUTPUT);
  digitalWrite(PIN_PORT_RST, HIGH);
  delay(100);

  Wire.begin(PIN_SDA, PIN_SCL);
  delay(50);

  // Relay OFF level first, then make the pin an output
  r8Ok = r8Present();
  if (r8Ok) {
    relayIo.digitalWrite(PUMP_RELAY_PIN, (uint8_t)(!RELAY_ON_LEVEL));
    relayIo.digitalWrite(LAMP_RELAY_PIN, (uint8_t)(!RELAY_ON_LEVEL));
    relayIo.pinMode(PUMP_RELAY_PIN, OUTPUT);
    relayIo.pinMode(LAMP_RELAY_PIN, OUTPUT);
    relaySet(false);
    Serial.printf("X-R8 found at 0x%02X, relay 1 (NO1, contactor) and relay 2 (NO2, lamp) OFF\n", R8_I2C_ADDR);
  } else {
    Serial.printf("X-R8 NOT FOUND at 0x%02X - check DIP switches / stacking\n", R8_I2C_ADDR);
    setMsg("X-R8 NOT FOUND", 8000);
  }

  touch.begin();

  ledOk = ledIo.begin();
  if (ledOk) {
    ledIo.pinMode(IO_PB1, INPUT);
    ledIo.pinMode(IO_LED1, OUTPUT);
    ledIo.pinMode(IO_LED2, OUTPUT);
    ledIo.pinMode(IO_PB3, INPUT);
    ledIo.digitalWrite(IO_LED1, (uint8_t)(!LED_ON_LEVEL));
    ledIo.digitalWrite(IO_LED2, (uint8_t)(!LED_ON_LEVEL));
    // Learn released button level
    pb1.idle = ledIo.digitalRead(IO_PB1);
    pb3.idle = ledIo.digitalRead(IO_PB3);
  } else {
    Serial.println("PCA9536 (buttons/LEDs) not detected");
  }

  SPI.begin(PIN_SCLK, PIN_MISO, PIN_MOSI);
  delay(50);
  tft.init();
  tft.setRotation(0);

  prefs.begin("pump", false);
  totalSec = prefs.getULong("sec", 0);
  starts   = prefs.getULong("starts", 0);
  Serial.printf("Stored: %lu s total run time, %lu starts\n", (unsigned long)totalSec, (unsigned long)starts);

#if DRY_SOURCE == DRY_SRC_MODBUS
  Serial2.begin(MB_BAUD, SERIAL_8N1, PIN_RS485_RX, PIN_RS485_TX);
  sense.valid = false;
  Serial.printf("Run-dry source: Modbus RTU pressure (ID %d, %d baud), limit %.2f %s\n",
                MB_SLAVE_ID, MB_BAUD, RUN_DRY_PRESSURE, PRESSURE_UNIT);
#else
  Serial.println("Run-dry source: simulated (touch the RUN-DRY tile to toggle)");
#endif

  uiInit();
}

void loop() {
  handleSerial();
  readButtons();
  handleTouch();
  senseUpdate();
  supervise();
  ledUpdate();
  uiUpdate();
  delay(5);
}
