# 10 kΩ NTC Thermistor on NORVI X-T4

Industrial temperature monitoring example for a 10 kΩ NTC thermistor connected to **Channel 1** of the NORVI X-T4 expansion module.

The sketch reads the thermistor through the on-board ADS1115 16-bit ADC, converts resistance to temperature with the Beta equation, applies light filtering, and displays the result on the CPU’s built-in ST7789 TFT with a flicker-free industrial UI (7-segment value, LED bar gauge, 5-minute trend, rate-of-rise, and alarm banner).

---

## Compatible Hardware

| Item | Details |
|------|---------|
| CPU | NORVI X-CPU-ESPS3-X1 **V2 (N16R2 only)** — ESP32-S3, 16 MB flash, QSPI PSRAM, 2.0″ ST7789 TFT (240 × 320) |
| Expansion | NORVI X-T4 — 4-channel NTC input, ADS1115, I2C address `0x48` (default) |
| Sensor | 10 kΩ NTC thermistor (R25 = 10 kΩ), 2-wire, non-polarised |
| Example probe | Littelfuse 103JG1K (B = 3892 K) — replace `B_COEFFICIENT` with your probe’s value |
| Supply | 24 V DC (CPU ≥ 1 A recommended; X-T4 draws ≈ 40 mA via expansion port) |

---

## Wiring Table

| Terminal | Function | Connection |
|----------|----------|------------|
| TM1+ | Channel 1 positive | One lead of the NTC |
| TM1− | Channel 1 negative | Other lead of the NTC |
| AGND | Analog ground | Shared reference (optional for short runs) |
| TM2+ / TM2− | Channel 2 | Unused in this example |
| TM3+ / TM3− | Channel 3 | Unused |
| TM4+ / TM4− | Channel 4 | Unused |

**Notes**
- NTC is non-polarised — either lead may go to + or −.
- Keep sensor wiring short and away from AC / relay / motor cables.
- For long runs use shielded twisted-pair and ground the shield at one end only.

### I2C Address (DIP switches on X-T4)

| DIP 1 | DIP 2 | DIP 3 | DIP 4 | Address |
|-------|-------|-------|-------|---------|
| OFF | OFF | OFF | ON | **0x48** (default) |
| ON | OFF | OFF | OFF | 0x49 |
| OFF | OFF | ON | OFF | 0x4A |
| OFF | ON | OFF | OFF | 0x4B |

On the device, set the DIP switches for address 0x48 (DIP 1 OFF, DIP 2 OFF, DIP 3 OFF, DIP 4 ON), because the code is also configured to use I2C address 0x48.

### CPU Pin Map (used by this sketch)

| Signal | GPIO |
|--------|------|
| I2C SDA | 8 |
| I2C SCL | 9 |
| SPI MISO | 13 |
| SPI MOSI | 11 |
| SPI SCLK | 12 |
| TFT CS | 45 |
| TFT DC | 46 |
| TFT RST | 47 |
| ADS1115 address | 0x48 |

---

## ADS1115 Register / Channel Map

| ADS1115 Input | X-T4 Terminal | Sketch usage |
|---------------|---------------|--------------|
| AIN0 | TM1+ / TM1− | Channel 1 (this example) |
| AIN1 | TM2+ / TM2− | — |
| AIN2 | TM3+ / TM3− | — |
| AIN3 | TM4+ / TM4− | — |

Configuration used in the sketch:
- Gain: `GAIN_ONE` (±4.096 V)
- Data rate: `RATE_ADS1115_860SPS`
- Single-ended reads

---

## Temperature Conversion (Beta equation)

```text
R_ntc = R_series × (V_supply / V_adc − 1)

1/T = 1/T0 + (1/B) × ln(R_ntc / R25)

T(°C) = 1/T − 273.15
```

| Parameter | Value in sketch |
|-----------|-----------------|
| `SUPPLY_VOLTAGE` | 3.3 V |
| `SERIES_RESISTOR` | 10 000 Ω |
| `THERMISTOR_NOMINAL` (R25) | 10 000 Ω |
| `TEMPERATURE_NOMINAL` (T0) | 25 °C |
| `B_COEFFICIENT` | 3892 K (Littelfuse 103JG1K) |

---

## Key Sketch Settings

| Constant | Value | Meaning |
|----------|-------|---------|
| `MAX_SAFE_TEMP` | 300 °C | Over-temperature alarm |
| `TEMP_HYSTERESIS` | 5 °C | Alarm clears below 295 °C |
| `WARN_TEMP` | 255 °C (85 % of limit) | Amber warning zone |
| `RATE_OF_RISE_LIMIT` | 15 °C/min | Rate-of-rise alarm |
| `GAUGE_MAX` | 400 °C | Bar-gauge full scale |
| `SENSE_INTERVAL` | 100 ms | ADC sample period |
| `TREND_INTERVAL` | 3 s | Trend sample period |
| `DISPLAY_INTERVAL` | 200 ms | TFT refresh |
| `TREND_POINTS` | 100 | 5 minutes of history |
| `ROR_SAMPLES` | 20 | 60 s rate window |

---

## Required Libraries

| Library | Notes |
|---------|-------|
| Wire | ESP32 core |
| SPI | ESP32 core |
| Adafruit ADS1X15 | + Adafruit BusIO |
| LovyanGFX | `#define LGFX_USE_V1` |

Install via **Sketch → Include Library → Manage Libraries**.

### Arduino IDE board settings (recommended)

| Setting | Value |
|---------|-------|
| Board | ESP32S3 Dev Module |
| USB CDC On Boot | Enabled |
| Flash Size | 16 MB (128 Mb) |
| PSRAM | QSPI PSRAM |
| Upload Speed | 115200 |
| CPU Frequency | 240 MHz (WiFi) |

---

## Quick Start

1. Power off the system.
2. Seat the X-T4 on the expansion bus and set DIP switches for address `0x48`.
3. Connect the 10 kΩ NTC across **TM1+** and **TM1−**.
4. Power on (24 V DC).
5. Open `Industrial_Furnace_Or_Oven_Temperature_Monitor.ino` in Arduino IDE.
6. Select the board settings above and the correct COM port.
7. Upload the sketch.
8. Observe temperature, trend, rate-of-rise and alarm status on the TFT. Serial prints a status line every second at 115200 baud.

---

## Alarm States

| State | Condition |
|-------|-----------|
| Normal | Temperature below warning threshold |
| Warning | ≥ 85 % of `MAX_SAFE_TEMP` |
| Over-temp | > `MAX_SAFE_TEMP` (latched, clears with hysteresis) |
| Rate alarm | Rate of rise > 15 °C/min (60 s window) |
| Sensor fault | Open or short detected for 5 consecutive bad reads |

---

## File Contents

| File | Description |
|------|-------------|
| `Industrial_Furnace_Or_Oven_Temperature_Monitor.ino` | Complete Arduino sketch |
| `Industrial_Furnace_Or_Oven_Temperature_Monitor.md` | Extended application notes |
| `README.md` | This document |

---

## FAQ (8 Q&A)

**Q1. Why does the reading show SENSOR OPEN?**  
A. Check that the NTC is firmly connected to TM1+/TM1−, that the X-T4 is seated on the expansion bus, and that the I2C address matches the sketch (`0x48`). Run an I2C scanner to confirm the ADS1115 is detected.

**Q2. Can I use a different B-value thermistor?**  
A. Yes. Change `B_COEFFICIENT` (and `THERMISTOR_NOMINAL` if R25 is not 10 kΩ) to the values given in your probe datasheet.

**Q3. How do I read the other three channels?**  
A. Call `ads1.readADC_SingleEnded(1)`, `(2)` or `(3)` and pass the raw value to the same `readThermistorC()` function. Map them to TM2, TM3 and TM4 respectively.

**Q4. The temperature is offset by a few degrees. How do I correct it?**  
A. Adjust the small compensation term inside `readThermistorC()` or calibrate against a known reference (ice water ≈ 0 °C, precision 10 kΩ resistor ≈ 25 °C).

**Q5. What is the maximum cable length for the NTC?**  
A. Keep runs as short as practical. For longer distances use shielded twisted-pair cable and ground the shield at one end only to minimise noise.

**Q6. Why is the TFT blank or flickering?**  
A. Confirm the board is the V2 N16R2 variant (QSPI PSRAM), that LovyanGFX is installed, and that SPI CS lines of any Ethernet/SD devices are held high if those peripherals are present.

**Q7. Can I measure temperatures over 300 °C using the NORVI X-T4 module?**  
A. No. Standard NTC thermistors supported by the X-T4 are restricted to lower ranges (typically below 150 °C to 200 °C). Attempting to measure over 300 °C with an NTC thermistor will cause the sensor material to degrade, lose calibration, or fail permanently.

**Q8. What alternative NORVI X expansions and sensors should I use for temperatures over 300 °C?**  
A. For high-temperature industrial processes exceeding 300 °C, use one of the following specialized expansion modules:

- NORVI-X-RTD4: Interfaces with PT100 or PT1000 RTD sensors. It provides high precision (±0.1 °C) and laboratory-grade stability up to +850 °C, making it ideal for high-precision industrial ovens and heating   chambers.

- NORVI-X-TC4: Interfaces with K-type thermocouples. It covers an extensive range from 0 °C up to 1093 °C, making it the standard choice for heavy industrial furnaces, kilns, and high-heat exhaust systems operating well over 300 °C.

---

## References

- [NORVI X-T4 User Guide](https://norvi.io/docs/norvi-x-t4-user-guide/)
- [NORVI X-T4 Datasheet](https://norvi.io/docs/norvi-x-t4-datasheet/)
- [NORVI-X-Version-02 GitHub](https://github.com/NORVIControllers/NORVI-X-Version-02)
- Application guide folder: `Application Guides/ntc-thermistor-on-x-t4`

---

*Last updated: 30 October 2026*
