#include <Wire.h>
#include <TFT_eSPI.h>
#include "Free_Fonts.h"
#include <PCA9536D.h>

// NORVI X hardware pins from 4391.ino
#define SDA_PIN   8
#define SCL_PIN   9
#define RS485_RXD 16
#define RS485_TXD 15
#define RS485_FC  41

#define MISO 13
#define MOSI 11
#define SCLK 12

#define GSM_RX    18
#define GSM_TX    17
#define GSM_RST   39

#define IO_PB1  0
#define IO_LED1 1
#define IO_LED2 2
#define IO_PB3  3

// ATV320 Modbus settings
static const uint8_t  VFD_SLAVE_ID = 1;
static const uint32_t MODBUS_BAUD = 19200;

// Default scanner map addresses from ATV320 Modbus manual.
// Some masters/libraries use 0-based addressing, so sketch auto-detects and applies offset.
static const uint16_t REG_NC1_CMD  = 12761; // COM Scan Out1 val (CMD)
static const uint16_t REG_NC2_LFRD = 12762; // COM Scan Out2 val (LFRD)
static const uint16_t REG_NM1_ETA  = 12741; // COM Scan In1 val  (ETA)
static const uint16_t REG_ETA_PARAM = 3201;  // ETA direct logical address
static const uint16_t REG_LFT_PARAM = 7121;  // Last fault occurred
static const uint16_t REG_FNB_PARAM = 7393;  // Fault counter
static const uint16_t REG_DP1_PARAM = 7201;  // Fault history n-1 start

static const uint16_t CMD_RUN_BIT_FWD = 0x0001; // CD00
static const uint16_t CMD_RUN_BIT_REV = 0x0004; // CD02
static const uint16_t CMD_STOP     = 0x0000;
static const uint16_t CMD_RUN_FWD  = CMD_RUN_BIT_FWD;
static const uint16_t CMD_RUN_REV  = CMD_RUN_BIT_REV;

static const uint16_t SPEED_MIN_DECIHZ = 450;   // 45.0 Hz
static const uint16_t SPEED_MAX_DECIHZ = 1100;  // 110.0 Hz
static const uint16_t SPEED_STEP_DECIHZ = 50;  // 5.0 Hz per press
static const unsigned long PB1_DOUBLE_PRESS_MS = 350;
static const unsigned long REVERSE_STOP_DELAY_MS = 1200;
static const float REF_GAIN = 3.0f;
static const uint8_t HEALTH_REG_COUNT = 4;

const char APN[] = ""; //REPLACE WITH NETWORK PROVIDER'S APN
const char APN_USER[] = "";
const char APN_PASS[] = "";
const char SIM_PIN[] = "";
const char THINGSBOARD_HOST[] = "mqtt.thingsboard.cloud";
const uint16_t THINGSBOARD_PORT = 1883;
const char THINGSBOARD_ACCESS_TOKEN[] = "";      // REPLACE WITH THINGSBOARD DEVICE ACCESS TOKEN
const unsigned long TELEMETRY_PUBLISH_MS = 30000;

PCA9536 io;
TFT_eSPI tft = TFT_eSPI();
TaskHandle_t vfdTaskHandle = nullptr;
TaskHandle_t gsmTaskHandle = nullptr;

volatile bool runCommand = false;
volatile bool reverseCommand = false;
volatile uint16_t speedSetpointDeciHz = 450; // 45.0 Hz
volatile uint16_t speedCommandDeciHz = 450;
uint16_t healthRegs[HEALTH_REG_COUNT] = {0};
volatile uint16_t etaWord = 0;
volatile int16_t rfrdDeciHz = 0;
volatile int16_t motorPowerPct = 0;
volatile int16_t motorTorquePct = 0;
volatile int16_t motorCurrentRaw = 0;
uint16_t faultHistoryRegs[8] = {0};
volatile uint16_t lastFaultOccurred = 0;
volatile uint16_t faultCounter = 0;
volatile bool modbusOnline = false;
int8_t addressOffset = 0; // 0 or -1
int8_t etaAddressOffset = 0; // 0 or -1 for direct ETA read
int8_t historyAddressOffset = 0; // 0 or -1 for direct fault-history reads
volatile bool reverseChangePending = false;
volatile bool resumeAfterReverse = false;
volatile unsigned long reverseStopStartMs = 0;
volatile bool mqttConnected = false;
volatile bool gprsConnected = false;
bool modemReady = false;
unsigned long lastTelemetryPublishMs = 0;
unsigned long lastModemRetryMs = 0;
unsigned long lastFaultHistoryPollMs = 0;
unsigned long motorRunStartMs = 0;
unsigned long motorRunSecondsTotal = 0;
unsigned long lastRunDurationSeconds = 0;
uint32_t motorStartCount = 0;
bool motorRunningLatched = false;

bool lastPb1 = false;
bool lastPb3 = false;
uint8_t pb1PressCount = 0;
unsigned long pb1LastPressMs = 0;
unsigned long lastPollMs = 0;
unsigned long lastEtaLogMs = 0;
bool lastFaultState = false;
bool lastModbusOnlineState = false;

String lastLinkText;
String lastRunText;
String lastDirText;
String lastFooterText;
String lastSetFreqText;
String lastTxFreqText;
String lastDirectionValueText;
String lastVfdFbText;
String lastTelemetryText;

void serviceDriveCommunication();
void logDriveStateChanges();
bool gsmResponseHasAssignedIp(const String &response);
bool readDriveStatusWord();
bool readFaultHistory();
String buildLiveTelemetryPayload();
String buildFaultTelemetryPayload();
bool publishThingsBoard(const String &payload);
const char *faultCodeToText(uint16_t code);
float scaledFeedbackHz(int16_t deciHz);
void updateRunMetrics();
void handleButtons();
void handleReverseTransition();
void vfdTask(void *parameter);
void gsmTask(void *parameter);

uint16_t crc16_modbus(const uint8_t *buf, uint16_t len) {
  uint16_t crc = 0xFFFF;
  for (uint16_t pos = 0; pos < len; pos++) {
    crc ^= (uint16_t)buf[pos];
    for (int i = 0; i < 8; i++) {
      if (crc & 0x0001) {
        crc >>= 1;
        crc ^= 0xA001;
      } else {
        crc >>= 1;
      }
    }
  }
  return crc;
}

void rs485Send(const uint8_t *tx, size_t txLen) {
  digitalWrite(RS485_FC, HIGH);
  delayMicroseconds(200);
  Serial2.write(tx, txLen);
  Serial2.flush();
  delayMicroseconds(200);
  digitalWrite(RS485_FC, LOW);
}

bool modbusReadHolding(uint16_t reg, uint16_t qty, uint16_t *out) {
  uint8_t req[8];
  req[0] = VFD_SLAVE_ID;
  req[1] = 0x03;
  req[2] = (uint8_t)(reg >> 8);
  req[3] = (uint8_t)(reg & 0xFF);
  req[4] = (uint8_t)(qty >> 8);
  req[5] = (uint8_t)(qty & 0xFF);
  uint16_t crc = crc16_modbus(req, 6);
  req[6] = (uint8_t)(crc & 0xFF);
  req[7] = (uint8_t)(crc >> 8);

  while (Serial2.available()) Serial2.read();
  rs485Send(req, sizeof(req));

  const uint16_t expected = 5 + (qty * 2);
  uint8_t resp[64];
  uint16_t idx = 0;
  unsigned long t0 = millis();
  while (millis() - t0 < 120) {
    while (Serial2.available() && idx < sizeof(resp)) {
      resp[idx++] = (uint8_t)Serial2.read();
    }
    if (idx >= expected) break;
  }

  if (idx < expected) return false;
  if (resp[0] != VFD_SLAVE_ID || resp[1] != 0x03) return false;
  if (resp[2] != (qty * 2)) return false;
  uint16_t crcRx = (uint16_t)resp[expected - 2] | ((uint16_t)resp[expected - 1] << 8);
  uint16_t crcCalc = crc16_modbus(resp, expected - 2);
  if (crcRx != crcCalc) return false;

  for (uint16_t i = 0; i < qty; i++) {
    out[i] = ((uint16_t)resp[3 + i * 2] << 8) | resp[4 + i * 2];
  }
  return true;
}

bool modbusWriteMulti(uint16_t startReg, const uint16_t *values, uint16_t qty) {
  if (qty == 0 || qty > 16) return false;

  uint8_t req[64];
  uint16_t p = 0;
  req[p++] = VFD_SLAVE_ID;
  req[p++] = 0x10;
  req[p++] = (uint8_t)(startReg >> 8);
  req[p++] = (uint8_t)(startReg & 0xFF);
  req[p++] = (uint8_t)(qty >> 8);
  req[p++] = (uint8_t)(qty & 0xFF);
  req[p++] = (uint8_t)(qty * 2);
  for (uint16_t i = 0; i < qty; i++) {
    req[p++] = (uint8_t)(values[i] >> 8);
    req[p++] = (uint8_t)(values[i] & 0xFF);
  }
  uint16_t crc = crc16_modbus(req, p);
  req[p++] = (uint8_t)(crc & 0xFF);
  req[p++] = (uint8_t)(crc >> 8);

  while (Serial2.available()) Serial2.read();
  rs485Send(req, p);

  uint8_t resp[8];
  uint8_t idx = 0;
  unsigned long t0 = millis();
  while (millis() - t0 < 120) {
    while (Serial2.available() && idx < sizeof(resp)) {
      resp[idx++] = (uint8_t)Serial2.read();
    }
    if (idx >= sizeof(resp)) break;
  }

  if (idx < sizeof(resp)) return false;
  if (resp[0] != VFD_SLAVE_ID || resp[1] != 0x10) return false;
  uint16_t crcRx = (uint16_t)resp[6] | ((uint16_t)resp[7] << 8);
  uint16_t crcCalc = crc16_modbus(resp, 6);
  if (crcRx != crcCalc) return false;

  uint16_t ackReg = ((uint16_t)resp[2] << 8) | resp[3];
  uint16_t ackQty = ((uint16_t)resp[4] << 8) | resp[5];
  return (ackReg == startReg && ackQty == qty);
}

String gsmSendCommand(const String &command, uint32_t waitMs) {
  String response;
  Serial.print("[GSM TX] ");
  Serial.println(command);
  Serial1.println(command);
  unsigned long startMs = millis();
  while (millis() - startMs < waitMs) {
    while (Serial1.available()) {
      response += (char)Serial1.read();
    }
    delay(2);
  }
  Serial.print("[GSM RX] ");
  if (response.length() == 0) {
    Serial.println("<no response>");
  } else {
    Serial.println(response);
  }
  return response;
}

bool gsmResponseHasOk(const String &response) {
  return response.indexOf("OK") >= 0;
}

void initGsmHardware() {
  pinMode(GSM_RST, OUTPUT);
  digitalWrite(GSM_RST, LOW);
  delay(200);
  digitalWrite(GSM_RST, HIGH);
  delay(2000);

  Serial1.begin(115200, SERIAL_8N1, GSM_RX, GSM_TX);
  delay(500);
}

bool modemBasicInit() {
  if (!gsmResponseHasOk(gsmSendCommand("AT", 1000))) return false;
  gsmSendCommand("ATE0", 1000);
  gsmSendCommand("AT+CFUN=1", 3000);
  gsmSendCommand("AT+CPIN?", 2000);
  gsmSendCommand("AT+CSQ", 1000);
  gsmSendCommand("AT+CREG?", 1000);
  gsmSendCommand("AT+CGATT?", 1000);
  gsmSendCommand("AT+CPSI?", 1500);
  gsmSendCommand("AT+CNMP=2", 2000);

  if (String(SIM_PIN).length() > 0) {
    gsmSendCommand("AT+CPIN=\"" + String(SIM_PIN) + "\"", 3000);
  }

  if (String(APN).length() > 0) {
    gsmSendCommand("AT+CGDCONT=1,\"IP\",\"" + String(APN) + "\"", 2000);
  }

  return true;
}

bool waitForNetworkRegistration(uint32_t timeoutMs) {
  unsigned long startMs = millis();
  while (millis() - startMs < timeoutMs) {
    String response = gsmSendCommand("AT+CREG?", 1500);
    if (response.indexOf("+CREG: 0,1") >= 0 || response.indexOf("+CREG: 0,5") >= 0 ||
        response.indexOf("+CREG: 0,6") >= 0 || response.indexOf("+CREG: 1,1") >= 0 ||
        response.indexOf("+CREG: 1,5") >= 0) {
      return true;
    }
    delay(2000);
  }
  return false;
}

bool gsmResponseHasAssignedIp(const String &response) {
  int prefix = response.indexOf("+CGPADDR:");
  if (prefix < 0) return false;
  int comma = response.indexOf(',', prefix);
  if (comma < 0) return false;
  int lineEnd = response.indexOf('\n', comma + 1);
  if (lineEnd < 0) lineEnd = response.length();
  String ip = response.substring(comma + 1, lineEnd);
  ip.trim();
  return ip.length() > 0 && ip != "0.0.0.0";
}

bool connectGprs() {
  gsmSendCommand("AT+CGATT?", 1000);
  gsmSendCommand("AT+CGATT=1", 3000);
  if (String(APN).length() > 0) {
    gsmSendCommand("AT+CGDCONT=1,\"IP\",\"" + String(APN) + "\"", 2000);
  }
  gsmSendCommand("AT+CGACT=1,1", 3000);
  String ipResp = gsmSendCommand("AT+CGPADDR=1", 1500);
  String netOpenResp = gsmSendCommand("AT+NETOPEN", 5000);
  if (gsmResponseHasAssignedIp(ipResp)) {
    gprsConnected = true;
    return true;
  }
  if (netOpenResp.indexOf("+NETOPEN: 0") >= 0 || netOpenResp.indexOf("already opened") >= 0) {
    ipResp = gsmSendCommand("AT+CGPADDR=1", 1500);
  }
  gprsConnected = gsmResponseHasAssignedIp(ipResp);
  return gprsConnected;
}

bool mqttConnectThingsBoard() {

    if (String(THINGSBOARD_ACCESS_TOKEN).length() == 0) {
        return false;
    }

    gsmSendCommand("AT+CMQTTDISC=0,120", 1000);
    gsmSendCommand("AT+CMQTTREL=0", 1000);
    gsmSendCommand("AT+CMQTTSTOP", 3000);

    delay(2000);

    gsmSendCommand("AT+CMQTTSTART", 5000);
    delay(2000);
    
String clientId = "STM32Client-" + String((uint32_t)random(0xFFFF), HEX);

gsmSendCommand("AT+CMQTTACCQ=0,\"" + clientId + "\",0", 2000);
delay(1000);

    gsmSendCommand("AT+CMQTTWILLTOPIC=0,2", 1000);
    gsmSendCommand("01\x1A", 1000);
    delay(1000);

    gsmSendCommand("AT+CMQTTWILLMSG=0,6,1", 1000);
    gsmSendCommand("qwerty\x1A", 1000);
    delay(1000);

    String cmd =
        "AT+CMQTTCONNECT=0,\"tcp://" +
        String(THINGSBOARD_HOST) + ":" +
        String(THINGSBOARD_PORT) +
        "\",60,1,\"" +
        String(THINGSBOARD_ACCESS_TOKEN) +
        "\"";

    String response = gsmSendCommand(cmd, 20000);
delay(2000);
    Serial.println(response);
//
    mqttConnected = response.indexOf("+CMQTTCONNECT: 0,0") >= 0;

    return mqttConnected;
}
  
 bool publishThingsBoard(const String &payload) {
    const String topic = "v1/devices/me/telemetry";

    String response = gsmSendCommand("AT+CMQTTTOPIC=0," + String(topic.length()), 2000);
    if (response.indexOf(">") < 0) return false;

    response = gsmSendCommand(topic, 2000);
    if (response.indexOf("OK") < 0) return false;

    response = gsmSendCommand("AT+CMQTTPAYLOAD=0," + String(payload.length()), 2000);
    if (response.indexOf(">") < 0) return false;

    response = gsmSendCommand(payload, 5000);
    if (response.indexOf("OK") < 0) return false;

    response = gsmSendCommand("AT+CMQTTPUB=0,1,60,0,0", 7000);
    return response.indexOf("+CMQTTPUB: 0,0") >= 0;
  }

void ensureTelemetryLink() {
  if (String(THINGSBOARD_ACCESS_TOKEN).length() == 0) return;
  if (millis() - lastModemRetryMs < 10000) return;
  lastModemRetryMs = millis();

  if (!modemReady) {
    modemReady = modemBasicInit();
    if (!modemReady) return;
  }
  if (!waitForNetworkRegistration(60000)) return;
  if (!gprsConnected && !connectGprs()) return;
  if (!mqttConnected) {
    mqttConnectThingsBoard();
  }
}

float scaledFeedbackHz(int16_t deciHz) {
  if (REF_GAIN <= 0.0f) return deciHz / 10.0f;
  return (deciHz / 10.0f) / REF_GAIN;
}

void updateRunMetrics() {
  const int16_t motionThresholdDeciHz = 5; // 0.5 Hz
  bool motorRunningNow = modbusOnline && !etaFaultActive() && abs(rfrdDeciHz) >= motionThresholdDeciHz;
  unsigned long now = millis();

  if (motorRunningNow && !motorRunningLatched) {
    motorRunningLatched = true;
    motorRunStartMs = now;
    motorStartCount++;
  } else if (!motorRunningNow && motorRunningLatched) {
    unsigned long runSeconds = (now - motorRunStartMs) / 1000UL;
    motorRunSecondsTotal += runSeconds;
    lastRunDurationSeconds = runSeconds;
    motorRunningLatched = false;
  }
}

uint16_t buildCmdWord() {
  if (!runCommand) return CMD_STOP;
  return reverseCommand ? CMD_RUN_REV : CMD_RUN_FWD;
}

uint16_t buildSpeedCommand() {
  float scaled = speedSetpointDeciHz * REF_GAIN;
  if (scaled < 0.0f) scaled = 0.0f;
  if (scaled > 5990.0f) scaled = 5990.0f;
  return (uint16_t)(scaled + 0.5f);
}

bool writeCommandAndSpeed() {
  speedCommandDeciHz = buildSpeedCommand();
  uint16_t frame[2] = {buildCmdWord(), speedCommandDeciHz};
  return modbusWriteMulti((uint16_t)(REG_NC1_CMD + addressOffset), frame, 2);
}

bool readHealthData() {
  if (!modbusReadHolding((uint16_t)(REG_NM1_ETA + addressOffset), HEALTH_REG_COUNT, healthRegs)) return false;

  motorPowerPct = (int16_t)healthRegs[0];
  rfrdDeciHz = (int16_t)healthRegs[1];
  motorTorquePct = (int16_t)healthRegs[2];
  motorCurrentRaw = (int16_t)healthRegs[3];
  return true;
}

bool readDriveStatusWord() {
  uint16_t etaValue = 0;
  if (!modbusReadHolding((uint16_t)(REG_ETA_PARAM + etaAddressOffset), 1, &etaValue)) return false;
  etaWord = etaValue;
  return true;
}

bool readFaultHistory() {
  uint16_t regValue = 0;
  if (!modbusReadHolding((uint16_t)(REG_LFT_PARAM + historyAddressOffset), 1, &regValue)) return false;
  lastFaultOccurred = regValue;

  if (!modbusReadHolding((uint16_t)(REG_FNB_PARAM + historyAddressOffset), 1, &regValue)) return false;
  faultCounter = regValue;

  if (!modbusReadHolding((uint16_t)(REG_DP1_PARAM + historyAddressOffset), 8, faultHistoryRegs)) return false;
  return true;
}

const char *faultCodeToText(uint16_t code) {
  switch (code) {
    case 0: return "NOF";
    case 2: return "EEF1";
    case 3: return "CFF";
    case 4: return "CFI";
    case 5: return "SLF1";
    case 6: return "ILF";
    case 7: return "CNF";
    case 8: return "EPF1";
    case 9: return "OCF";
    case 10: return "CRF1";
    case 11: return "SPF";
    case 12: return "ANF";
    case 16: return "OHF";
    case 17: return "OLF";
    case 18: return "OBF";
    case 19: return "OSF";
    case 20: return "OPF1";
    case 21: return "PHF";
    case 22: return "USF";
    case 23: return "SCF1";
    case 24: return "SOF";
    case 25: return "TNF";
    case 26: return "INF1";
    case 27: return "INF2";
    case 28: return "INF3";
    case 29: return "INF4";
    case 30: return "EEF2";
    case 32: return "SCF3";
    case 33: return "OPF2";
    case 34: return "COF";
    case 35: return "BLF";
    case 38: return "EPF2";
    case 41: return "BRF";
    case 42: return "SLF2";
    case 44: return "SSF";
    case 45: return "SLF3";
    case 49: return "PTFL";
    case 50: return "OTFL";
    case 51: return "INF9";
    case 52: return "INFA";
    case 53: return "INFB";
    case 54: return "TJF";
    case 55: return "SCF4";
    case 56: return "SCF5";
    case 58: return "FCF1";
    case 59: return "FCF2";
    case 64: return "LCF";
    case 67: return "HDF";
    case 68: return "INF6";
    case 69: return "INFE";
    case 71: return "LFF3";
    case 73: return "HCF";
    case 76: return "DLF";
    case 77: return "CFI2";
    case 99: return "CSF";
    case 100: return "ULF";
    case 101: return "OLC";
    case 105: return "ASF";
    case 107: return "SAFF";
    case 108: return "FBE";
    case 109: return "FBES";
    default: return "UNK";
  }
}

void logDriveStateChanges() {
  bool faultNow = etaFaultActive();
  if (modbusOnline != lastModbusOnlineState) {
    Serial.print("[VFD] Modbus ");
    Serial.println(modbusOnline ? "ONLINE" : "OFFLINE");
    lastModbusOnlineState = modbusOnline;
  }

  if (faultNow != lastFaultState) {
    Serial.print("[VFD] Fault state ");
    Serial.print(faultNow ? "ACTIVE" : "CLEARED");
    Serial.print(" ETA=0x");
    Serial.println(etaWord, HEX);
    lastFaultState = faultNow;
  }

  if (faultNow && millis() - lastEtaLogMs >= 5000) {
    lastEtaLogMs = millis();
    Serial.print("[VFD] ETA=0x");
    Serial.print(etaWord, HEX);
    Serial.print(" RFRD=");
    Serial.print(rfrdDeciHz / 10.0f, 1);
    Serial.print("Hz OPR=");
    Serial.print(motorPowerPct);
    Serial.print("% OTR=");
    Serial.print(motorTorquePct);
    Serial.print("% CMD=0x");
    Serial.println(buildCmdWord(), HEX);
  }
}

void serviceDriveCommunication() {
  if (millis() - lastPollMs < 200) return;
  lastPollMs = millis();

  bool w = writeCommandAndSpeed();
  bool r = readHealthData();
  bool s = readDriveStatusWord();
  bool h = true;
  if (millis() - lastFaultHistoryPollMs >= 5000) {
    lastFaultHistoryPollMs = millis();
    h = readFaultHistory();
  }
  modbusOnline = (w && r && s && h);

  io.digitalWrite(IO_LED1, runCommand ? HIGH : LOW);
  io.digitalWrite(IO_LED2, modbusOnline ? HIGH : LOW);
  if (modbusOnline) updateRunMetrics();
  logDriveStateChanges();
}

void vfdTask(void *parameter) {
  (void)parameter;
  for (;;) {
    handleButtons();
    handleReverseTransition();
    serviceDriveCommunication();
    vTaskDelay(pdMS_TO_TICKS(20));
  }
}

void gsmTask(void *parameter) {
  (void)parameter;
  for (;;) {
    if (String(THINGSBOARD_ACCESS_TOKEN).length() > 0) {
      if (!mqttConnected) {
        ensureTelemetryLink();
      }

      if (mqttConnected && (millis() - lastTelemetryPublishMs >= TELEMETRY_PUBLISH_MS)) {
        lastTelemetryPublishMs = millis();
        bool liveOk = publishThingsBoard(buildLiveTelemetryPayload());
        bool faultOk = false;
        if (liveOk) {
          vTaskDelay(pdMS_TO_TICKS(500));
          faultOk = publishThingsBoard(buildFaultTelemetryPayload());
        }
        if (!(liveOk && faultOk)) {
          mqttConnected = false;
        }
      }
    }
    vTaskDelay(pdMS_TO_TICKS(250));
  }
}

bool readButtonPressed(uint8_t pin) {
  return io.digitalRead(pin) == LOW;
}

bool etaLimitActive() {
  return (etaWord & (1U << 11)) != 0;
}

bool etaFaultActive() {
  return (etaWord & (1U << 3)) != 0;
}

const char *directionText() {
  return reverseCommand ? "REV" : "FWD";
}

void drawHeader() {
  tft.fillRect(0, 0, 240, 34, TFT_DARKCYAN);
  tft.drawFastHLine(0, 34, 240, TFT_CYAN);
  tft.setTextColor(TFT_WHITE, TFT_DARKCYAN);
  tft.setFreeFont(FSS12);
  tft.setCursor(12, 24);
  tft.print("VFD CONTROL");
}

void drawValueCard(int x, int y, int w, int h, uint16_t borderColor, const char *label, const String &value, uint16_t valueColor) {
  tft.drawRoundRect(x, y, w, h, 8, borderColor);
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.setFreeFont(FSS9);
  tft.setCursor(x + 10, y + 20);
  tft.print(label);

  tft.setTextColor(valueColor, TFT_BLACK);
  tft.setFreeFont(FSB12);
  tft.setCursor(x + 10, y + 46);
  tft.print(value);
}

void drawStatusBarFrame() {
  tft.fillRoundRect(8, 42, 224, 26, 6, TFT_NAVY);
  tft.setTextColor(TFT_WHITE, TFT_NAVY);
  tft.setFreeFont(FM9);
  tft.setCursor(186, 59);
  tft.print("ID1");
}

String buildFooterText() {
  if (etaFaultActive()) return "DRIVE FAULT ACTIVE";
  if (etaLimitActive()) return "REF LIMITED BY HSP/LSP";
  if (runCommand && reverseCommand) return "REV CMD ACTIVE";
  if (String(THINGSBOARD_ACCESS_TOKEN).length() == 0) return "SET TB TOKEN TO ENABLE MQTT";
  if (!mqttConnected) return "GSM MQTT OFFLINE";
  return "PB1:RUN/STOP  DBL:DIR";
}

uint16_t footerTextColor(const String &msg) {
  if (msg == "DRIVE FAULT ACTIVE") return TFT_RED;
  if (msg == "REF LIMITED BY HSP/LSP" || msg == "REV CMD ACTIVE") return TFT_ORANGE;
  return TFT_LIGHTGREY;
}

void drawFooterFrame() {
  tft.drawFastHLine(0, 210, 240, TFT_DARKGREY);
}

void updateStatusBar(bool force) {
  String linkText = modbusOnline ? "LINK OK" : "LINK OFF";
  String runText = runCommand ? "RUN" : "STOP";
  String dirText = directionText();

  if (force || linkText != lastLinkText) {
    tft.fillRect(14, 46, 68, 16, TFT_NAVY);
    tft.setTextColor(modbusOnline ? TFT_GREEN : TFT_RED, TFT_NAVY);
    tft.setFreeFont(FM9);
    tft.setCursor(14, 59);
    tft.print(linkText);
    lastLinkText = linkText;
  }

  if (force || runText != lastRunText) {
    tft.fillRect(88, 46, 50, 16, TFT_NAVY);
    tft.setTextColor(runCommand ? TFT_GREENYELLOW : TFT_ORANGE, TFT_NAVY);
    tft.setFreeFont(FM9);
    tft.setCursor(88, 59);
    tft.print(runText);
    lastRunText = runText;
  }

  if (force || dirText != lastDirText) {
    tft.fillRect(146, 46, 34, 16, TFT_NAVY);
    tft.setTextColor(reverseCommand ? TFT_ORANGE : TFT_CYAN, TFT_NAVY);
    tft.setFreeFont(FM9);
    tft.setCursor(146, 59);
    tft.print(dirText);
    lastDirText = dirText;
  }
}

void updateCardValue(int x, int y, int w, int h, const String &value, String &cache, uint16_t valueColor, bool force) {
  if (!force && value == cache) return;

  tft.fillRect(x + 8, y + 24, w - 16, h - 16, TFT_BLACK);
  tft.setTextColor(valueColor, TFT_BLACK);
  tft.setFreeFont(FSB12);
  tft.setCursor(x + 10, y + 46);
  tft.print(value);
  cache = value;
}

void updateFooter(bool force) {
  String msg = buildFooterText();
  if (!force && msg == lastFooterText) return;

  tft.fillRect(0, 212, 240, 24, TFT_BLACK);
  uint16_t msgColor = TFT_LIGHTGREY;
  msgColor = footerTextColor(msg);
  tft.setTextColor(msgColor, TFT_BLACK);
  tft.setFreeFont(FM9);
  tft.setCursor(8, 228);
  tft.print(msg);
  lastFooterText = msg;
}

void drawUIFrame() {
  tft.fillScreen(TFT_BLACK);
  drawHeader();
  drawStatusBarFrame();
  drawValueCard(8, 78, 108, 58, TFT_CYAN, "SET FREQ", "", TFT_YELLOW);
  drawValueCard(124, 78, 108, 58, TFT_GREEN, "TORQUE", "", TFT_GREENYELLOW);
  drawValueCard(8, 144, 108, 58, TFT_ORANGE, "DIRECTION", "", TFT_CYAN);
  drawValueCard(124, 144, 108, 58, TFT_MAGENTA, "VFD FB", "", TFT_WHITE);
  drawFooterFrame();
}

void refreshUI(bool force) {
  updateStatusBar(force);
  updateCardValue(8, 78, 108, 58, String(speedSetpointDeciHz / 10.0f, 1) + "Hz", lastSetFreqText, TFT_YELLOW, force);
  updateCardValue(124, 78, 108, 58, String(motorTorquePct) + "%", lastTxFreqText, TFT_GREENYELLOW, force);
  updateCardValue(8, 144, 108, 58, String(directionText()), lastDirectionValueText, reverseCommand ? TFT_ORANGE : TFT_CYAN, force);
  updateCardValue(124, 144, 108, 58, String(scaledFeedbackHz(rfrdDeciHz), 1) + "Hz", lastVfdFbText, TFT_WHITE, force);
  updateFooter(force);
}

String buildLiveTelemetryPayload() {
  unsigned long liveRunSeconds = motorRunSecondsTotal;
  if (motorRunningLatched) {
    liveRunSeconds += (millis() - motorRunStartMs) / 1000UL;
  }

  String payload = "{";
  payload += "\"sw\":" + String(etaWord);
  payload += ",\"af\":" + String(scaledFeedbackHz(rfrdDeciHz), 1);
  payload += ",\"opr\":" + String(motorPowerPct);
  payload += ",\"otr\":" + String(motorTorquePct);
  payload += ",\"mc\":" + String(motorCurrentRaw);
  payload += ",\"run\":" + String(runCommand ? 1 : 0);
  payload += ",\"rev\":" + String(reverseCommand ? 1 : 0);
  payload += ",\"mb\":" + String(modbusOnline ? 1 : 0);
  payload += ",\"fault\":" + String(etaFaultActive() ? 1 : 0);
  payload += ",\"warn\":" + String((etaWord & (1U << 7)) ? 1 : 0);
  payload += ",\"sod\":" + String((etaWord & (1U << 6)) ? 1 : 0);
  payload += ",\"run_s\":" + String(liveRunSeconds);
  payload += ",\"last_run_s\":" + String(lastRunDurationSeconds);
  payload += ",\"starts\":" + String(motorStartCount);
  payload += "}";
  return payload;
}

String buildFaultTelemetryPayload() {
  String payload = "{";
  payload += "\"lft\":\"" + String(faultCodeToText(lastFaultOccurred)) + "\"";
  payload += ",\"fnb\":" + String(faultCounter);
  payload += ",\"dp1\":\"" + String(faultCodeToText(faultHistoryRegs[0])) + "\"";
  payload += ",\"dp2\":\"" + String(faultCodeToText(faultHistoryRegs[1])) + "\"";
  payload += ",\"dp3\":\"" + String(faultCodeToText(faultHistoryRegs[2])) + "\"";
  payload += ",\"dp4\":\"" + String(faultCodeToText(faultHistoryRegs[3])) + "\"";
  payload += ",\"dp5\":\"" + String(faultCodeToText(faultHistoryRegs[4])) + "\"";
  payload += ",\"dp6\":\"" + String(faultCodeToText(faultHistoryRegs[5])) + "\"";
  payload += ",\"dp7\":\"" + String(faultCodeToText(faultHistoryRegs[6])) + "\"";
  payload += ",\"dp8\":\"" + String(faultCodeToText(faultHistoryRegs[7])) + "\"";
  payload += "}";
  return payload;
}

void handlePb1SinglePress() {
  runCommand = !runCommand;
}

void handlePb1DoublePress() {
  if (runCommand) {
    reverseChangePending = true;
    resumeAfterReverse = true;
    runCommand = false;
    reverseStopStartMs = millis();
  } else {
    reverseCommand = !reverseCommand;
  }
}

void handleReverseTransition() {
  if (!reverseChangePending) return;
  if (millis() - reverseStopStartMs < REVERSE_STOP_DELAY_MS) return;

  reverseCommand = !reverseCommand;
  reverseChangePending = false;

  if (resumeAfterReverse) {
    runCommand = true;
    resumeAfterReverse = false;
  }
}

void handleButtons() {
  bool pb1 = readButtonPressed(IO_PB1);
  bool pb3 = readButtonPressed(IO_PB3);
  unsigned long now = millis();

  if (pb1 && !lastPb1) {
    if (pb1PressCount == 0) {
      pb1PressCount = 1;
      pb1LastPressMs = now;
    } else if ((now - pb1LastPressMs) <= PB1_DOUBLE_PRESS_MS) {
      handlePb1DoublePress();
      pb1PressCount = 0;
    } else {
      pb1PressCount = 1;
      pb1LastPressMs = now;
    }
  }

  if (pb1PressCount == 1 && (now - pb1LastPressMs) > PB1_DOUBLE_PRESS_MS) {
    handlePb1SinglePress();
    pb1PressCount = 0;
  }

  if (pb3 && !lastPb3) {
    speedSetpointDeciHz += SPEED_STEP_DECIHZ;
    if (speedSetpointDeciHz > SPEED_MAX_DECIHZ) {
      speedSetpointDeciHz = SPEED_MIN_DECIHZ;
    }
  }

  lastPb1 = pb1;
  lastPb3 = pb3;
}

void setup() {
  Serial.begin(115200);

  pinMode(RS485_FC, OUTPUT);
  digitalWrite(RS485_FC, LOW);
  initGsmHardware();

  Wire.begin(SDA_PIN, SCL_PIN);
  if (!io.begin()) {
    while (true) {
      delay(1000);
    }
  }

  io.pinMode(IO_PB1, INPUT);
  io.pinMode(IO_PB3, INPUT);
  io.pinMode(IO_LED1, OUTPUT);
  io.pinMode(IO_LED2, OUTPUT);

  SPI.begin(SCLK, MISO, MOSI); // Ensure these pin numbers are correct
  delay(1000);

  Serial2.begin(MODBUS_BAUD, SERIAL_8E1, RS485_RXD, RS485_TXD);

  tft.init();
  tft.setRotation(0);
  drawUIFrame();

  // Auto detect address base (manual logical addresses vs zero-based offset)
  addressOffset = 0;
  if (!readHealthData()) {
    addressOffset = -1;
    if (!readHealthData()) {
      addressOffset = 0;
    }
  }
  etaAddressOffset = 0;
  if (!readDriveStatusWord()) {
    etaAddressOffset = -1;
    if (!readDriveStatusWord()) {
      etaAddressOffset = 0;
    }
  }
  historyAddressOffset = 0;
  if (!readFaultHistory()) {
    historyAddressOffset = -1;
    if (!readFaultHistory()) {
      historyAddressOffset = 0;
    }
  }

  speedCommandDeciHz = buildSpeedCommand();
  xTaskCreatePinnedToCore(vfdTask, "vfdTask", 8192, nullptr, 1, &vfdTaskHandle, 0);
  xTaskCreatePinnedToCore(gsmTask, "gsmTask", 12288, nullptr, 0, &gsmTaskHandle, 1);
  refreshUI(true);
}

void loop() {
  refreshUI(false);
  delay(50);
}
