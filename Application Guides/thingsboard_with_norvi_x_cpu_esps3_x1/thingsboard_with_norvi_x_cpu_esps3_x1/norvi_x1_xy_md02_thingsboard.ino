#include <WiFi.h>
#include <PubSubClient.h>
#include <ModbusMaster.h>

// ===================== LovyanGFX display setup (NORVI X1 official) =====================
#define LGFX_USE_V1
#include <LovyanGFX.hpp>

class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ST7789 _panel;
  lgfx::Bus_SPI      _bus;
public:
  LGFX(void) {
    {
      auto c = _bus.config();
      c.spi_host    = SPI2_HOST;
      c.spi_mode    = 0;
      c.freq_write  = 27000000;
      c.freq_read   = 16000000;
      c.spi_3wire   = false;
      c.use_lock    = true;
      c.dma_channel = SPI_DMA_CH_AUTO;
      c.pin_sclk    = 12;
      c.pin_mosi    = 11;
      c.pin_miso    = 13;
      c.pin_dc      = 46;
      _bus.config(c);
      _panel.setBus(&_bus);
    }
    {
      auto c = _panel.config();
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
      c.bus_shared = true;   // SD card and Ethernet share this SPI bus
      _panel.config(c);
    }
    setPanel(&_panel);
  }
};
LGFX tft;

// ---- EDIT THESE ----
const char* WIFI_SSID     = "YOUR_WIFI_NAME";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
const char* TB_SERVER     = "mqtt.thingsboard.cloud";    // or your own ThingsBoard host
const int   TB_PORT       = 1883;
const char* TB_TOKEN      = "YOUR_DEVICE_ACCESS_TOKEN";  // device access token

// ---- Hardware / timing ----
#define RS485_RX          16
#define RS485_TX          15
#define ETH_CS_PIN        1    // held HIGH so it does not disturb the shared SPI bus
#define SD_CS_PIN         42   // same
#define SENSOR_ID         1
#define SENSOR_BAUD       9600
#define SENSOR_SCALE      100.0f   // raw 3275 = 32.75
#define READ_INTERVAL     5000UL
#define PUBLISH_INTERVAL  10000UL

// ---- Colours (RGB565) ----
#define C_BG      0x0000
#define C_HDR     0x01E8
#define C_CARD    0x10A3
#define C_TEXT    0xFFFF
#define C_DIM     0x8410
#define C_GREEN   0x07E0
#define C_RED     0xF800
#define C_CYAN    0x07FF
#define C_HUM     0x551F

WiFiClient   wifiClient;
PubSubClient mqtt(wifiClient);
ModbusMaster node;

float temperature = 0, humidity = 0;
bool  sensorOk = false;
unsigned long lastRead = 0, lastPublish = 0, lastPublishOkMs = 0, lastWifiTry = 0, lastMqttTry = 0, lastDraw = 0;
bool  everPublished = false;

// ---------- Display (portrait 240 x 320) ----------
const int CARD_W = 224, CARD_H = 104, CARD_X = 8, CARD_Y1 = 40, CARD_Y2 = 152;

void drawCentered(const char* s, int cx, int y, float size, uint16_t fg, uint16_t bg) {
  tft.setTextSize(size);
  tft.setTextColor(fg, bg);
  int w = tft.textWidth(s);
  tft.setCursor(cx - w / 2, y);
  tft.print(s);
}

void drawStatic() {
  tft.fillScreen(C_BG);
  tft.fillRect(0, 0, 240, 30, C_HDR);
  tft.setTextSize(2);
  tft.setTextColor(C_TEXT, C_HDR);
  tft.setCursor(8, 7);
  tft.print("NORVI X1 + XY-MD02");

  int cx = CARD_X + CARD_W / 2;
  tft.fillRoundRect(CARD_X, CARD_Y1, CARD_W, CARD_H, 8, C_CARD);
  tft.fillRoundRect(CARD_X, CARD_Y2, CARD_W, CARD_H, 8, C_CARD);
  drawCentered("Temperature", cx, CARD_Y1 + 8,  2, C_DIM, C_CARD);
  drawCentered("Humidity",    cx, CARD_Y2 + 8,  2, C_DIM, C_CARD);
  drawCentered("Celsius",     cx, CARD_Y1 + 80, 2, C_DIM, C_CARD);
  drawCentered("% RH",        cx, CARD_Y2 + 80, 2, C_DIM, C_CARD);

  tft.setTextSize(2);
  tft.setTextColor(C_TEXT, C_BG);
  tft.setCursor(6 + 16,   270); tft.print("WiFi");
  tft.setCursor(76 + 16,  270); tft.print("MQTT");
  tft.setCursor(146 + 16, 270); tft.print("Sensor");
}

void drawDot(int x, bool ok) {
  tft.fillCircle(x + 6, 278, 6, ok ? C_GREEN : C_RED);
}

void updateDisplay() {
  static char lastT[12] = "", lastH[12] = "", lastAgo[24] = "";
  static uint16_t lastTc = 0, lastHc = 0;
  static int8_t lw = -1, lm = -1, ls = -1;

  // values
  char t[12], h[12];
  uint16_t tc = C_DIM, hc = C_DIM;
  if (sensorOk) {
    snprintf(t, sizeof(t), "%.2f", temperature);
    snprintf(h, sizeof(h), "%.2f", humidity);
    tc = temperature > 35.0f ? C_RED : (temperature < 5.0f ? C_CYAN : C_GREEN);
    hc = C_HUM;
  } else {
    strcpy(t, "--");
    strcpy(h, "--");
  }
  int cx = CARD_X + CARD_W / 2;
  if (strcmp(t, lastT) || tc != lastTc) {
    tft.fillRect(CARD_X + 4, CARD_Y1 + 30, CARD_W - 8, 44, C_CARD);
    drawCentered(t, cx, CARD_Y1 + 34, 5, tc, C_CARD);
    strcpy(lastT, t); lastTc = tc;
  }
  if (strcmp(h, lastH) || hc != lastHc) {
    tft.fillRect(CARD_X + 4, CARD_Y2 + 30, CARD_W - 8, 44, C_CARD);
    drawCentered(h, cx, CARD_Y2 + 34, 5, hc, C_CARD);
    strcpy(lastH, h); lastHc = hc;
  }

  // status dots
  int8_t w = WiFi.status() == WL_CONNECTED, m = mqtt.connected(), s = sensorOk;
  if (w != lw) { drawDot(6,   w); lw = w; }
  if (m != lm) { drawDot(76,  m); lm = m; }
  if (s != ls) { drawDot(146, s); ls = s; }

  // time since last successful send
  char ago[24];
  if (!everPublished) {
    strcpy(ago, "Sent: waiting...");
  } else {
    unsigned long sec = (millis() - lastPublishOkMs) / 1000;
    if (sec < 60) snprintf(ago, sizeof(ago), "Sent %lus ago", sec);
    else          snprintf(ago, sizeof(ago), "Sent %lum ago", sec / 60);
  }
  if (strcmp(ago, lastAgo)) {
    tft.fillRect(0, 292, 240, 28, C_BG);
    tft.setTextSize(2);
    tft.setTextColor(C_DIM, C_BG);
    tft.setCursor(8, 296);
    tft.print(ago);
    strcpy(lastAgo, ago);
  }
}

// ---------- Network ----------
void keepWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;
  if (millis() - lastWifiTry < 15000UL) return;
  lastWifiTry = millis();
  Serial.println("Wi-Fi (re)connecting...");
  WiFi.disconnect();
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void keepMQTT() {
  if (mqtt.connected() || WiFi.status() != WL_CONNECTED) return;
  if (millis() - lastMqttTry < 5000UL) return;
  lastMqttTry = millis();
  Serial.print("Connecting to ThingsBoard MQTT... ");
  // ThingsBoard: username = device access token, empty password
  if (mqtt.connect("norvi-x1", TB_TOKEN, NULL)) {
    Serial.println("connected");
  } else {
    Serial.print("failed, state=");
    Serial.println(mqtt.state());   // 5 = not authorised (check the token)
  }
}

// ---------- Sensor ----------
void readSensor() {
  uint8_t r = node.readInputRegisters(0x0001, 2);
  if (r == node.ku8MBSuccess) {
    temperature = (int16_t)node.getResponseBuffer(0) / SENSOR_SCALE;  // signed
    humidity    = node.getResponseBuffer(1) / SENSOR_SCALE;
    sensorOk = true;
    Serial.printf("Sensor: %.2f C, %.2f %%RH\n", temperature, humidity);
  } else {
    sensorOk = false;
    Serial.printf("Modbus read failed, error 0x%02X (0xE2 = timeout: check wiring/address/baud)\n", r);
  }
}

void publishTelemetry() {
  char payload[64];
  snprintf(payload, sizeof(payload), "{\"temperature\":%.2f,\"humidity\":%.2f}", temperature, humidity);
  if (mqtt.publish("v1/devices/me/telemetry", payload)) {
    everPublished = true;
    lastPublishOkMs = millis();
    Serial.print("Published: ");
    Serial.println(payload);
  } else {
    Serial.println("Publish failed");
  }
}

void setup() {
  Serial.begin(115200);

  // Keep other devices on the shared SPI bus deselected before the display starts
  pinMode(ETH_CS_PIN, OUTPUT); digitalWrite(ETH_CS_PIN, HIGH);
  pinMode(SD_CS_PIN, OUTPUT);  digitalWrite(SD_CS_PIN, HIGH);

  tft.init();
  tft.setRotation(0);          // portrait, 240 x 320 (use 2 if upside down)
  drawStatic();

  Serial2.begin(SENSOR_BAUD, SERIAL_8N1, RS485_RX, RS485_TX);
  node.begin(SENSOR_ID, Serial2);

  mqtt.setServer(TB_SERVER, TB_PORT);
  mqtt.setSocketTimeout(5);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  lastWifiTry = millis();

  readSensor();
  updateDisplay();
}

void loop() {
  keepWiFi();
  keepMQTT();
  mqtt.loop();

  unsigned long now = millis();

  if (now - lastRead >= READ_INTERVAL) {
    lastRead = now;
    readSensor();
    updateDisplay();
  }

  if (now - lastPublish >= PUBLISH_INTERVAL && mqtt.connected() && sensorOk) {
    lastPublish = now;
    publishTelemetry();
    updateDisplay();
  }

  if (now - lastDraw >= 1000UL) {   // refresh status dots and the "sent ago" counter
    lastDraw = now;
    updateDisplay();
  }
}
