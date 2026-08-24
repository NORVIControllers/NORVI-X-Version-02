---
type: Test_program
function: analog_output_dac_write_0-10v_0-20ma (4-channel serial-controlled DAC output over I2C, master side)
compatible_cpu_boards: [NORVI X-CPU-ESPS3-X1],[NORVI X-CPU-ESPS3-X2],[NORVI X-CPU-ESPS3-X3]
compatible_modules: [NORVI X-AQ4 (4-channel analog output expansion, STM32F103C8Tx I2C slave co-processor driving onboard DAC)]
status: completed
last_updated: 2026-08-17
---

# Example: NORVI X-AQ4 Analog Output (Voltage/Current) Serial Control

## What this does
Accepts serial commands (`CHx=value`) from the user to set one of 4 analog output channels — CH1/CH2 as 0–10V and CH3/CH4 as 0–20mA — converts the value to a 12-bit DAC code, and writes it over I2C to the onboard STM32F103C8Tx co-processor (acting as I2C slave), which drives the actual DAC hardware. The STM32 firmware is factory-programmed and fixed on the module; this ESP32 sketch is the master-side driver and is the only part intended to be modified/extended.

## Hardware required
- CPU board: NORVI X-CPU-ESPS3-X1 OR X2, X3 
- Expansion module(s): NORVI X-AQ4 (4-channel analog output module; onboard STM32F103C8Tx acts as fixed I2C slave co-processor driving the DAC — its firmware is not user-modifiable)
  - I2C slave address is DIP-switch selectable, base `0x50`, 4-bit DIP-configurable offset giving 16 possible addresses in the range `0x50`–`0x5F`. This sketch uses `0x5F` (`DAC_ADDR`) by default — update to match the module's DIP switch setting.
- Wiring / pin map (from firmware `#define`s):

| Signal | GPIO |
|---|---|
| I2C SDA | 8 |
| I2C SCL | 9 |

## Libraries required
| Library | Version |
|---|---|
| Wire (ESP32 core) | 2.0.0 |

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
```

## Expected Output
On the Serial Monitor (115200 baud), enter commands manually or from a script:
- Boot banner:
====================================
ESP32 DAC SERIAL CONTROL READY
CH1=5 (0-10V)
CH2=7.5 (0-10V)
CH3=12 (0-20mA)
CH4=20 (0-20mA)

- Sending `CH1=5` returns: `CH1 = 5.00 V | DAC Code = 2047` (example)
- Sending an out-of-range value (e.g. `CH3=25`) returns: `Current range: 0-20mA`
- Sending a malformed command (e.g. `CH5=1` or missing `=`) returns: `Invalid command!` or `Format: CHx=value`

## Common Issues
| Symptom | Likely Cause | Fix |
|---|---|---|
| `Invalid command!` for a seemingly valid input | Command doesn't start with `CH`, or command wasn't typed in the exact `CHx=value` format | Send commands exactly as `CH1=5`, `CH3=12.5`, etc. via Serial Monitor with newline line-ending enabled |
| No output change on any channel despite valid command accepted | `DAC_ADDR` (`0x5F`) doesn't match the module's actual DIP-switch-configured I2C address (`0x50`–`0x5F`) | Check the X-AQ4 module's DIP switch setting and update `DAC_ADDR` in the sketch to match |
| I2C write fails silently / channel never updates | Wiring fault on SDA/SCL (GPIO 8/9), or STM32F103C8Tx slave co-processor not powered/booted | Verify I2C wiring and confirm the module's onboard STM32 is powered; the STM32 firmware itself is fixed and not modifiable from this sketch |
| Voltage/current channel outputs a short-circuit or fault indicator on the module (LEDs off) | Output overloaded or shorted downstream, detected by the STM32 slave's internal AD5754-class error register | Check wiring/load on the affected channel; the slave firmware auto-zeroes a faulted channel's DAC code until the fault clears |
| Values slightly off from requested (e.g. 5.00V reads as 4.99V) | Expected quantization — 12-bit DAC resolution (4095 steps) over the 0–10V/0–20mA range | This is normal DAC resolution rounding, not a fault |

## Related Files
- Module reference: `expansion_module_AQ4.md`
- CPU board: `cpu_board_X1.md`