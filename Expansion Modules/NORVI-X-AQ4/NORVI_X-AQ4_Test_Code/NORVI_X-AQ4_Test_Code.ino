/*
 * NORVI X-AQ4-v2
 */
#include <Wire.h>

// ================= I2C =================
#define SDA_PIN 8     // Change if needed
#define SCL_PIN 9
#define DAC_ADDR 0x5F

// =========================================================
void setup() {

  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);

  delay(300);

  config_dac(DAC_ADDR, 0, 0, 1, 1);

  Serial.println("====================================");
  Serial.println("  ESP32 DAC SERIAL CONTROL READY");
  Serial.println("====================================");
  Serial.println("CH1=5      (0-10V)");
  Serial.println("CH2=7.5    (0-10V)");
  Serial.println("CH3=12     (0-20mA)");
  Serial.println("CH4=20     (0-20mA)");
  Serial.println("====================================");
}

// =========================================================
void loop() {

  if (Serial.available()) {

    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    cmd.toUpperCase();

    processCommand(cmd);
  }
}

// =========================================================
// PROCESS COMMAND
// =========================================================
void processCommand(String cmd) {

  if (!cmd.startsWith("CH")) {
    Serial.println("Invalid command!");
    return;
  }

  int ch = cmd.substring(2, 3).toInt();
  int eqIndex = cmd.indexOf('=');

  if (ch < 1 || ch > 4 || eqIndex < 0) {
    Serial.println("Format: CHx=value");
    return;
  }

  float val = cmd.substring(eqIndex + 1).toFloat();
  uint16_t dac_value = 0;

  // -------- CH1 & CH2 (0–10V) --------
  if (ch == 1 || ch == 2) {

    if (val < 0 || val > 10) {
      Serial.println("Voltage range: 0-10V");
      return;
    }

    dac_value = (val / 10.0) * 4095.0;
  }

  // -------- CH3 & CH4 (0–20mA) --------
  if (ch == 3 || ch == 4) {

    if (val < 0 || val > 20) {
      Serial.println("Current range: 0-20mA");
      return;
    }

    dac_value = (val / 20.0) * 4095.0;
  }

  write_channel(DAC_ADDR, ch, dac_value);

  Serial.print("CH");
  Serial.print(ch);
  Serial.print(" = ");
  Serial.print(val);

  if (ch <= 2)
    Serial.print(" V");
  else
    Serial.print(" mA");

  Serial.print(" | DAC Code = ");
  Serial.println(dac_value);
}

// =========================================================
// WRITE DAC CHANNEL
// =========================================================
void write_channel(byte device_address, unsigned int channel, unsigned int value) {

  Wire.beginTransmission(device_address);
  Wire.write(1 + (channel * 2));
  Wire.write(value >> 8);
  Wire.write(value & 0xFF);
  Wire.endTransmission();
}

// =========================================================
// DAC CONFIG
// =========================================================
void config_dac(byte device_address,
                unsigned int c1,
                unsigned int c2,
                unsigned int c3,
                unsigned int c4) {

  unsigned int tft = 0;
  tft |= (c4 << 3);
  tft |= (c3 << 2);
  tft |= (c2 << 1);
  tft |= c1;

  Wire.beginTransmission(device_address);
  Wire.write(0x01);
  Wire.write(tft);
  Wire.endTransmission();
}
