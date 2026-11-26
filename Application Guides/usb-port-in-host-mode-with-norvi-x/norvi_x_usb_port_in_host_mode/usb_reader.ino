#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <stdarg.h>
#include <string.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <CST816S.h>
#include <USBHost.h>
#include <USBHostMSC.h>
#include <USBMSCFS.h>
#include "esp_log.h"

// ---------------- Pins (NORVI X1 / XN) ----------------
#define TFT_CS     45
#define TFT_DC     46
#define TFT_RST    -1   // Software reset so GPIO 47 stays free for TOUCH_INT
#define SPI_SCLK   12
#define SPI_MISO   13
#define SPI_MOSI   11
#define W5500_CS    1
#define SD_CS      42

// ---------------- Touchscreen (CST816S on I2C) ----------------
#define TOUCH_SDA   8
#define TOUCH_SCL   9
#define TOUCH_INT  47
#define TOUCH_RST  48
#define CST816S_ADDR 0x15

CST816S touch(TOUCH_SDA, TOUCH_SCL, TOUCH_INT, TOUCH_RST);
Adafruit_ST7789 tft(TFT_CS, TFT_DC, TFT_RST);

// ---------------- Colors ----------------
#define C_BG          0x0000
#define C_HEADER_BG   0x0015   // Deep royal/navy blue for header banner
#define C_CARD_BG     0x10A2   // Dark slate card fill
#define C_SCROLL_BG   0x18E3   // Scrollbar track
#define C_WHITE       0xFFFF
#define C_GREEN       0x07E0
#define C_RED         0xF800
#define C_YELLOW      0xFFE0
#define C_CYAN        0x07FF
#define C_GREY        0x8410

#define HEADER_H      52

// ---------------- USB Mass Storage Filesystem ----------------
USBMSCFSClass USBMSCFS;
#define USB_MSC_MOUNTPOINT "/usb"

struct AppState {
  int hostState;       // 0 = init, 1 = ok, 2 = fail
  bool i2cOk;          // true if I2C bus / CST816S responds properly
  bool mounted;        // USBHostMSC.mounted()
  bool fsReady;        // USBMSCFS.begin() succeeded
  bool rwDone;         // Read/write cycle finished for current plug-in
  uint64_t totalMB;
  uint64_t usedMB;
  char verdict[24];
  uint16_t verdictCol;
};

static AppState st = {0, false, false, false, false, 0, 0, "STARTING...", C_GREY};
static portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

// ---------------- Scrollable File Content Display Buffer ----------------
#define MAX_BUFFER_LINES 250   // Total scrollable wrapped lines stored in RAM
#define VISIBLE_LINES     18   // Rows visible on screen at once
#define LOG_COLS          35   // 35 chars * 6px = 210px text column (x=18..228)
#define LOG_START_Y      122
#define LOG_ROW_H         11

struct DisplayEntry {
  char text[LOG_COLS + 1];
  bool isFirstSegment;  // true = "> line", false = indented continuation
  bool justifyLine;     // true = non-final wrapped line (full-justify across column)
};

static DisplayEntry logBuf[MAX_BUFFER_LINES];
static int totalLines   = 0;
static int scrollOffset = 0;     // Index of top visible line
static bool autoFollow  = true;  // Auto-scroll to bottom while reading

// ---------------- Touch Scroll State ----------------
static bool     touchActive      = false;
static int      touchStartY      = 0;
static int      touchStartOffset = 0;
static uint32_t lastTouchSeenMs  = 0;

static void clearLog() {
  taskENTER_CRITICAL(&mux);
  totalLines   = 0;
  scrollOffset = 0;
  autoFollow   = true;
  memset(logBuf, 0, sizeof(logBuf));
  taskEXIT_CRITICAL(&mux);
}

static void pushLogSegment(const char *txt, bool isFirst, bool justify) {
  taskENTER_CRITICAL(&mux);
  if (totalLines < MAX_BUFFER_LINES) {
    strlcpy(logBuf[totalLines].text, txt, sizeof(logBuf[0].text));
    logBuf[totalLines].isFirstSegment = isFirst;
    logBuf[totalLines].justifyLine    = justify;
    totalLines++;
  } else {
    // Shift buffer up if file exceeds MAX_BUFFER_LINES
    memmove(&logBuf[0], &logBuf[1], sizeof(DisplayEntry) * (MAX_BUFFER_LINES - 1));
    strlcpy(logBuf[MAX_BUFFER_LINES - 1].text, txt, sizeof(logBuf[0].text));
    logBuf[MAX_BUFFER_LINES - 1].isFirstSegment = isFirst;
    logBuf[MAX_BUFFER_LINES - 1].justifyLine    = justify;
  }

  if (autoFollow) {
    int maxScroll = totalLines - VISIBLE_LINES;
    scrollOffset = (maxScroll > 0) ? maxScroll : 0;
  }
  taskEXIT_CRITICAL(&mux);
}

// Word-wraps a paragraph cleanly with consistent left alignment (no stray leading spaces)
static void printFileLine(const char *line) {
  const size_t maxChars = LOG_COLS;
  size_t len = strlen(line);
  size_t pos = 0;
  bool isFirst = true;

  // Trim leading spaces on original line for uniform indent
  while (pos < len && line[pos] == ' ') pos++;

  if (pos >= len) {
    pushLogSegment("", true, false);
    return;
  }

  while (pos < len) {
    size_t remaining = len - pos;

    // Final line of paragraph fits within column -> left-aligned (not stretched)
    if (remaining <= maxChars) {
      char seg[LOG_COLS + 1];
      strlcpy(seg, line + pos, sizeof(seg));
      size_t sLen = strlen(seg);
      while (sLen > 0 && seg[sLen - 1] == ' ') seg[--sLen] = '\0';
      pushLogSegment(seg, isFirst, false);
      break;
    }

    // Look backwards from maxChars for a clean space boundary
    size_t split = maxChars;
    while (split > 0 && line[pos + split] != ' ') {
      split--;
    }

    // If a single word is longer than maxChars, hard-wrap at maxChars
    if (split == 0) {
      split = maxChars;
    }

    char seg[LOG_COLS + 1];
    memcpy(seg, line + pos, split);
    seg[split] = '\0';

    // Trim trailing spaces so right-edge justification aligns flush
    size_t sLen = strlen(seg);
    while (sLen > 0 && seg[sLen - 1] == ' ') seg[--sLen] = '\0';

    pushLogSegment(seg, isFirst, true);
    isFirst = false;

    pos += split;
    // Skip spaces between wrapped lines so next row starts flush at x = 18
    while (pos < len && line[pos] == ' ') pos++;
  }
}

static void setVerdict(const char *txt, uint16_t col) {
  taskENTER_CRITICAL(&mux);
  strlcpy(st.verdict, txt, sizeof(st.verdict));
  st.verdictCol = col;
  taskEXIT_CRITICAL(&mux);
}

// ---------------- Display Functions ----------------
static String lineCache[VISIBLE_LINES];
static String bigCache;
static String indicatorCache;
static int    lastThumbY = -1;
static int    lastThumbH = -1;

// Beater Dot Color Rules:
// - RED:    I2C issue (or USB host init failure)
// - GREEN:  All working & pendrive connected
// - YELLOW: All working, no pendrive connected
static uint16_t getBeatColor() {
  bool i2cHealthy, driveMounted;
  int hostSt;

  taskENTER_CRITICAL(&mux);
  i2cHealthy   = st.i2cOk;
  driveMounted = st.mounted;
  hostSt       = st.hostState;
  taskEXIT_CRITICAL(&mux);

  if (!i2cHealthy || hostSt == 2) return C_RED;
  if (driveMounted)               return C_GREEN;
  return C_YELLOW;
}

static void tickHeartbeat(bool forceDraw = false) {
  static uint32_t lastBeat = 0;
  static bool beat = false;
  static uint16_t lastActiveCol = 0;

  uint16_t activeCol = getBeatColor();
  uint32_t now = millis();

  if (forceDraw || (now - lastBeat >= 1000) || (activeCol != lastActiveCol && beat)) {
    if (now - lastBeat >= 1000) {
      lastBeat = now;
      beat = !beat;
    } else if (forceDraw) {
      beat = true;
    }
    lastActiveCol = activeCol;
    tft.fillCircle(228, 14, 5, beat ? activeCol : C_HEADER_BG);
  }
}

static void drawCenteredText(const char *text, int topY, const GFXfont *font, uint16_t col) {
  int16_t x1, y1;
  uint16_t w, h;
  tft.setFont(font);
  tft.setTextSize(1);
  tft.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);

  int x = (240 - (int)w) / 2 - x1;
  int baselineY = topY - y1;  // Converts top-edge Y to FreeFont baseline Y

  tft.setTextColor(col);
  tft.setCursor(x, baselineY);
  tft.print(text);
  tft.setFont(NULL);          // Reset to default font
}

// Renders a text line either left-aligned or full-justified across targetWidthPx
static void drawJustifiedLine(const char *text, int startX, int y, int targetWidthPx, bool justify, uint16_t col) {
  tft.setFont(NULL);
  tft.setTextSize(1);
  tft.setTextColor(col);

  int len = strlen(text);
  if (!justify || len <= 1) {
    tft.setCursor(startX, y);
    tft.print(text);
    return;
  }

  // Count inter-word spaces
  int spaces = 0;
  for (int i = 0; i < len; i++) {
    if (text[i] == ' ') spaces++;
  }

  int textPx  = len * 6;
  int extraPx = targetWidthPx - textPx;

  if (spaces == 0 || extraPx <= 0) {
    tft.setCursor(startX, y);
    tft.print(text);
    return;
  }

  int baseGap   = extraPx / spaces;
  int remainder = extraPx % spaces;
  int curX      = startX;
  int spaceIdx  = 0;

  for (int i = 0; i < len; i++) {
    if (text[i] == ' ') {
      curX += 6 + baseGap + (spaceIdx < remainder ? 1 : 0);
      spaceIdx++;
    } else {
      tft.drawChar(curX, y, (unsigned char)text[i], col, C_BG, 1);
      curX += 6;
    }
  }
}

// Uses exact same font face (NULL, size 1) as "FILE CONTENT:" subheading
static void drawFileRow(int rowIdx, const DisplayEntry &entry, bool isEmpty) {
  char key[84];
  if (isEmpty) {
    snprintf(key, sizeof(key), "EMPTY");
  } else {
    snprintf(key, sizeof(key), "%d|%d|%s",
             entry.isFirstSegment ? 1 : 0,
             entry.justifyLine ? 1 : 0,
             entry.text);
  }

  if (lineCache[rowIdx] == key) return;
  lineCache[rowIdx] = key;

  int topY = LOG_START_Y + (rowIdx * LOG_ROW_H);
  tft.fillRect(0, topY, 232, LOG_ROW_H, C_BG); // Leave x=233..239 for scrollbar

  if (isEmpty) return;

  tft.setFont(NULL);
  tft.setTextSize(1);

  if (entry.isFirstSegment) {
    tft.setTextColor(C_YELLOW);
    tft.setCursor(6, topY + 2);
    tft.print(">");
  }

  // Consistent left column at x=18, justified across 210px (x=18..228)
  drawJustifiedLine(entry.text, 18, topY + 2, LOG_COLS * 6, entry.justifyLine, C_WHITE);
}

// Right-side visual scrollbar + top-right line range indicator
static void drawScrollIndicators(int count, int offset) {
  char ind[24];
  if (count == 0) {
    snprintf(ind, sizeof(ind), "EMPTY");
  } else {
    int firstShown = offset + 1;
    int lastShown  = offset + VISIBLE_LINES;
    if (lastShown > count) lastShown = count;
    snprintf(ind, sizeof(ind), "%d-%d/%d", firstShown, lastShown, count);
  }

  if (indicatorCache != ind) {
    indicatorCache = ind;
    tft.fillRect(130, 100, 108, 12, C_BG);
    tft.setFont(NULL);
    tft.setTextSize(1);
    tft.setTextColor(C_GREY);
    int w = strlen(ind) * 6;
    tft.setCursor(236 - w, 102);
    tft.print(ind);
  }

  const int trackX = 234;
  const int trackY = LOG_START_Y;
  const int trackW = 4;
  const int trackH = VISIBLE_LINES * LOG_ROW_H; // 198 px

  int thumbY = trackY;
  int thumbH = trackH;

  if (count > VISIBLE_LINES) {
    thumbH = (trackH * VISIBLE_LINES) / count;
    if (thumbH < 16) thumbH = 16;
    int maxOffset = count - VISIBLE_LINES;
    thumbY = trackY + ((trackH - thumbH) * offset) / maxOffset;
  }

  if (thumbY != lastThumbY || thumbH != lastThumbH) {
    lastThumbY = thumbY;
    lastThumbH = thumbH;
    tft.fillRect(trackX, trackY, trackW, trackH, C_SCROLL_BG);
    if (count > 0) {
      tft.fillRoundRect(trackX, thumbY, trackW, thumbH, 2,
                        (count > VISIBLE_LINES) ? C_CYAN : C_GREY);
    }
  }
}

// Verdict Status Block (216 x 32 px)
static void drawStatusCard(const char *text, uint16_t col) {
  char key[48];
  snprintf(key, sizeof(key), "%s|%u", text, col);
  if (bigCache == key) return;
  bigCache = key;

  const int cardX = 12;
  const int cardY = 58;
  const int cardW = 216;
  const int cardH = 32;

  // Card background & colored border
  tft.fillRoundRect(cardX, cardY, cardW, cardH, 5, C_CARD_BG);
  tft.drawRoundRect(cardX, cardY, cardW, cardH, 5, col);

  // Left accent bar
  tft.fillRoundRect(cardX + 2, cardY + 4, 4, cardH - 8, 2, col);

  // Vertically centered status text
  drawCenteredText(text, cardY + 9, &FreeSans9pt7b, col);
}

static void drawHeader() {
  tft.fillScreen(C_BG);
  tft.fillRect(0, 0, 240, HEADER_H, C_HEADER_BG);
  drawCenteredText("NORVI X1", 7, &FreeSansBold12pt7b, C_WHITE);
  drawCenteredText("USB HOST MODE", 31, &FreeSans9pt7b, C_WHITE);

  // Subheading for File Content area (default font NULL, size 1)
  tft.drawFastHLine(0, 95, 240, C_GREY);
  tft.setFont(NULL);
  tft.setTextSize(1);
  tft.setTextColor(C_CYAN);
  tft.setCursor(6, 102);
  tft.print("FILE CONTENT:");
  tft.drawFastHLine(0, 115, 240, 0x2945); // Subtle dark divider under subheading

  for (int i = 0; i < VISIBLE_LINES; i++) lineCache[i] = "";
  bigCache       = "";
  indicatorCache = "";
  lastThumbY     = -1;
  lastThumbH     = -1;
}

static void updateDisplay() {
  AppState s;
  int count, offset;

  taskENTER_CRITICAL(&mux);
  s      = st;
  count  = totalLines;
  offset = scrollOffset;
  taskEXIT_CRITICAL(&mux);

  // Clamp scroll offset safely
  int maxOffset = (count > VISIBLE_LINES) ? (count - VISIBLE_LINES) : 0;
  if (offset < 0) offset = 0;
  if (offset > maxOffset) offset = maxOffset;

  // 1. Status Card
  drawStatusCard(s.verdict, s.verdictCol);

  // 2. Scrollbar & Line Counter
  drawScrollIndicators(count, offset);

  // 3. Visible File Content Window
  for (int i = 0; i < VISIBLE_LINES; i++) {
    int lineIdx = offset + i;
    if (lineIdx < count) {
      DisplayEntry entryCopy;
      taskENTER_CRITICAL(&mux);
      entryCopy = logBuf[lineIdx];
      taskEXIT_CRITICAL(&mux);
      drawFileRow(i, entryCopy, false);
    } else {
      DisplayEntry emptyEntry = {"", false, false};
      drawFileRow(i, emptyEntry, true);
    }
  }
}

// ---------------- Bidirectional Touch Drag Scrolling Task ----------------
static void touchScrollTask() {
  if (!st.i2cOk) return;

  uint32_t now = millis();

  if (!touch.available()) {
    // Keep drag session alive across brief CST816S polling gaps (140 ms)
    if (touchActive && (now - lastTouchSeenMs > 140)) {
      touchActive = false;
    }
    return;
  }

  lastTouchSeenMs = now;

  // TFT uses setRotation(2) (180 deg inverted relative to native rotation 0)
  int ty = 319 - (int)touch.data.y;

  // Only start touches inside the file content pane (y >= 115)
  if (!touchActive && ty < 115) return;

  int count;
  taskENTER_CRITICAL(&mux);
  count = totalLines;
  taskEXIT_CRITICAL(&mux);

  if (count <= VISIBLE_LINES) return; // Nothing to scroll if all lines fit on screen

  int maxOffset = count - VISIBLE_LINES;

  if (!touchActive) {
    touchActive = true;
    touchStartY = ty;
    taskENTER_CRITICAL(&mux);
    touchStartOffset = scrollOffset;
    autoFollow       = false; // User took manual control of scrolling
    taskEXIT_CRITICAL(&mux);
  } else {
    // Reversed scroll direction:
    // Drag finger DOWN (ty > touchStartY) -> deltaY > 0 -> scroll DOWN to later lines
    // Drag finger UP   (ty < touchStartY) -> deltaY < 0 -> scroll UP to earlier lines
    int deltaY     = ty - touchStartY;
    int deltaLines = deltaY / 8; // 8px per line for smooth, responsive scrolling
    int newOffset  = constrain(touchStartOffset + deltaLines, 0, maxOffset);

    bool changed = false;
    taskENTER_CRITICAL(&mux);
    if (newOffset != scrollOffset) {
      scrollOffset = newOffset;
      changed = true;
    }
    taskEXIT_CRITICAL(&mux);

    if (changed) {
      updateDisplay();
    }
  }
}

// ---------------- Pen Drive Read & Copy Operations ----------------
static void runUsbReadWrite() {
  clearLog();
  updateDisplay();

  if (!USBMSCFS.begin(USB_MSC_MOUNTPOINT, 10, false)) {
    setVerdict("FAT32 MOUNT FAIL", C_RED);
    return;
  }

  uint64_t tot = USBMSCFS.totalBytes() / (1024 * 1024);
  uint64_t usd = USBMSCFS.usedBytes() / (1024 * 1024);

  taskENTER_CRITICAL(&mux);
  st.fsReady = true;
  st.totalMB = tot;
  st.usedMB  = usd;
  taskEXIT_CRITICAL(&mux);

  // 1. Open original file (READ ONLY)
  setVerdict("OPENING FILE...", C_YELLOW);
  updateDisplay();
  delay(450); // Visual pause so operator sees file open stage
  tickHeartbeat();

  File rf = USBMSCFS.open("/norvi_log.txt", FILE_READ);

  if (!rf || rf.isDirectory()) {
    setVerdict("SOURCE FILE FAIL", C_RED);

    if (rf) rf.close();
    USBMSCFS.end();
    return;
  }

  // Calculate visual pacing proportional to file size
  size_t fileSize = rf.size();
  uint32_t totalChunks = (fileSize + 127) / 128;
  if (totalChunks == 0) totalChunks = 1;

  // Target ~1.2s to 3.2s total visual read progression scaled by file size
  uint32_t targetTotalMs = 1200 + (fileSize / 4);
  if (targetTotalMs > 3200) targetTotalMs = 3200;
  uint32_t chunkDelayMs = targetTotalMs / totalChunks;
  if (chunkDelayMs < 35)  chunkDelayMs = 35;
  if (chunkDelayMs > 180) chunkDelayMs = 180;

  // 2. Create destination file
  File wf = USBMSCFS.open("/norvi_log_cpy.txt", FILE_WRITE);

  if (!wf || wf.isDirectory()) {
    setVerdict("COPY OPEN FAIL", C_RED);

    rf.close();
    if (wf) wf.close();
    USBMSCFS.end();
    return;
  }

  setVerdict("READING & COPYING...", C_YELLOW);
  updateDisplay();
  delay(350);

  // 3. Read original, write identical bytes to copy, and display file content
  uint8_t data[128];
  char rawLine[256];
  size_t rawLen = 0;
  bool copyOK = true;

  while (rf.available()) {
    size_t n = rf.read(data, sizeof(data));
    if (n == 0) break;

    // Write the exact bytes read from the original file
    size_t written = wf.write(data, n);
    if (written != n) {
      copyOK = false;
      break;
    }

    // Print file content to TFT display
    for (size_t i = 0; i < n; i++) {
      char c = (char)data[i];

      if (c == '\n') {
        // Remove CR from CRLF for display only
        if (rawLen > 0 && rawLine[rawLen - 1] == '\r') {
          rawLen--;
        }
        rawLine[rawLen] = '\0';
        printFileLine(rawLine);
        rawLen = 0;
      } else {
        if (rawLen < sizeof(rawLine) - 1) {
          rawLine[rawLen++] = c;
        } else {
          rawLine[rawLen] = '\0';
          printFileLine(rawLine);
          rawLen = 0;
          rawLine[rawLen++] = c;
        }
      }
    }

    updateDisplay();
    tickHeartbeat();
    delay(chunkDelayMs); // Proportional visual pacing per chunk
    yield();
  }

  // Display final line if file does not end with newline
  if (rawLen > 0) {
    if (rawLine[rawLen - 1] == '\r') {
      rawLen--;
    }
    rawLine[rawLen] = '\0';
    printFileLine(rawLine);
    updateDisplay();
  }

  // 4. Close both files
  rf.close();
  wf.close();

  if (!copyOK) {
    setVerdict("COPY FAILED", C_RED);
    USBMSCFS.end();
    return;
  }

  // 5. Update storage information
  usd = USBMSCFS.usedBytes() / (1024 * 1024);

  // 6. Unmount filesystem safely
  USBMSCFS.end();

  taskENTER_CRITICAL(&mux);
  st.usedMB = usd;
  st.rwDone = true;
  taskEXIT_CRITICAL(&mux);

  // 7. Final status
  setVerdict("R/W COMPLETE", C_GREEN);
}

// ---------------- I2C Health Check ----------------
static bool checkI2cTouchHealth() {
  Wire.beginTransmission(CST816S_ADDR);
  return (Wire.endTransmission() == 0);
}

// ---------------- Arduino Setup & Loop ----------------
void setup() {
  // Park other devices on the shared SPI bus
  pinMode(W5500_CS, OUTPUT); digitalWrite(W5500_CS, HIGH);
  pinMode(SD_CS, OUTPUT);    digitalWrite(SD_CS, HIGH);

  // Initialize I2C & CST816S Capacitive Touchscreen
  Wire.begin(TOUCH_SDA, TOUCH_SCL);
  touch.begin();
  delay(50);
  st.i2cOk = checkI2cTouchHealth();

  // Initialize SPI & ST7789 TFT Display
  SPI.begin(SPI_SCLK, SPI_MISO, SPI_MOSI, -1);
  tft.init(240, 320);
  tft.setSPISpeed(10000000);
  tft.setRotation(2);
  tft.setTextWrap(false);

  drawHeader();
  setVerdict("INIT USB HOST...", C_YELLOW);
  updateDisplay();
  tickHeartbeat(true);

  esp_log_level_set("*", ESP_LOG_WARN);

  if (!USBHost.begin()) {
    st.hostState = 2;
    setVerdict("HOST INIT FAIL", C_RED);
    updateDisplay();
    tickHeartbeat(true);
    return;
  }

  st.hostState = 1;
  setVerdict("PLUG PENDRIVE...", C_YELLOW);
  updateDisplay();
  tickHeartbeat(true);
}

void loop() {
  static uint32_t lastUi = 0, lastI2cCheck = 0;

  USBHost.task();
  touchScrollTask();

  // Periodic I2C bus health verification (every 2 seconds when idle)
  if (!touchActive && (millis() - lastI2cCheck >= 2000)) {
    lastI2cCheck = millis();
    bool ok = checkI2cTouchHealth();
    taskENTER_CRITICAL(&mux);
    st.i2cOk = ok;
    taskEXIT_CRITICAL(&mux);
  }

  bool currentlyMounted = USBHostMSC.mounted();

  if (currentlyMounted && !st.rwDone) {
    taskENTER_CRITICAL(&mux);
    st.mounted = true;
    taskEXIT_CRITICAL(&mux);

    setVerdict("PENDRIVE FOUND", C_CYAN);
    updateDisplay();
    tickHeartbeat(true);
    delay(550); // Visual pause so operator sees detection state

    runUsbReadWrite();
    updateDisplay();
  } else if (!currentlyMounted && (st.mounted || st.rwDone)) {
    // Pen drive was unplugged: reset state and clear file output
    taskENTER_CRITICAL(&mux);
    st.mounted = false;
    st.fsReady = false;
    st.rwDone  = false;
    st.totalMB = 0;
    st.usedMB  = 0;
    taskEXIT_CRITICAL(&mux);

    clearLog();
    setVerdict("PLUG PENDRIVE...", C_YELLOW);
    updateDisplay();
    tickHeartbeat(true);
  }

  if (millis() - lastUi >= 250) {
    lastUi = millis();
    updateDisplay();
  }

  tickHeartbeat();
  delay(10);
}