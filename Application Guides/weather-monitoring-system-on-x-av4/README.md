# Industrial Weather Monitoring System on NORVI X-AV4

Industrial weather monitoring example for three 0–10 V transmitters (humidity, wind speed and wind direction) connected to **Channels 1 to 3** of the NORVI X-AV4 expansion module.

The sketch reads the transmitters through the on-board ADS1115 16-bit ADC, undoes the input divider to get terminal volts, scales the result to %RH, m/s and degrees, averages 8 readings per update (wind direction is averaged as a vector), and displays everything on the CPU’s built-in ST7789 TFT with a flicker-free industrial UI (segmented wind-speed gauge, compass dial with needle, 16-point compass name, humidity gauge, and a header status LED). Over-range inputs and an offline X-AV4 are detected and shown on screen.

---

## Compatible Hardware

| Item                | Details                                                                                                     |
| ------------------- | ----------------------------------------------------------------------------------------------------------- |
| CPU                 | NORVI X-CPU-ESPS3-X1 — ESP32-S3, 16 MB flash, 2 MB QSPI PSRAM, 2.0″ ST7789 TFT (240 × 320)                   |
| Expansion           | NORVI X-AV4 — 4-channel 0–10 V input, ADS1115 16-bit ADC, I2C address `0x49` (DIP 1 ON)                      |
| Humidity sensor     | Sentera DSTHF-3 — 0–100 %RH, ±3 %RH, output AO2 set to 0–10 V, 18–34 V DC, 80 mA                             |
| Wind speed sensor   | WG2/O-10 — 0–50 m/s = 0–10 V, ±0.5 m/s or ±3 % of reading, 13–30 V DC or 24 V AC/DC, 50 mA                   |
| Wind direction sensor | WRG/O-40 — 0–360° = 0–10 V, ±1.5°, 1° resolution, 15–28 V DC or 24 V AC                                    |
| Cable               | Shielded twisted pair for signal wires, shield connected at one end only                                    |
| Supply              | 24 V DC (CPU ≥ 1 A recommended; X-AV4 draws ≈ 40 mA via expansion port; size for sensor heaters, see Wiring Notes) |
| Optional (commissioning) | Adjustable 0–10 V source or calibrator, multimeter, compass, reference hygrometer                      |

**X-AV4 specifications:** 24 V DC, 40 mA · 4 analog inputs, 0–10 V · ADS1115 16-bit, 3.3 V · operating temperature −40 to +85 °C · IP20, DIN rail, push-in terminals (AIN1 … AIN4, AGND).

---

## Wiring Table

| Terminal | Function              | Connection                                                      |
| -------- | --------------------- | --------------------------------------------------------------- |
| AIN1     | Humidity input        | DSTHF-3 output **AO2** (0–10 V = 0–100 %RH)                     |
| AIN2     | Wind speed input      | WG2/O-10 signal output (0–10 V = 0–50 m/s)                      |
| AIN3     | Wind direction input  | WRG/O-40 signal output (0–10 V = 0–360°)                        |
| AIN4     | Spare input           | Unused in this example                                          |
| AGND     | Analog ground         | GND of all three transmitters and supply 0 V (joined once at the X-AV4) |

**Notes**

- Supply 24 V DC to each transmitter in parallel; each signal output goes to its AIN terminal.
- For the DSTHF-3, wire the supply GND to the supply 0 V and the GND beside AO2 to AGND as **separate wires**.
- Never apply more than 10 V to an input. The sketch flags anything above 10.5 V as a fault.
- Use shielded twisted pair for signal wires and ground the shield at one end only.
- Keep sensor wiring away from motor, VFD and relay cables.

### I2C Address (DIP switches on X-AV4)

| DIP 1 | DIP 2 | DIP 3 | DIP 4 | Address            |
| ----- | ----- | ----- | ----- | ------------------ |
| ON    | OFF   | OFF   | OFF   | **0x49** (this sketch) |
| OFF   | OFF   | OFF   | ON    | 0x48               |
| OFF   | OFF   | ON    | OFF   | 0x4A               |
| OFF   | ON    | OFF   | OFF   | 0x4B               |

On the device, set DIP 1 ON and DIP 2–4 OFF, because the code is configured to use I2C address 0x49.

### CPU Pin Map (used by this sketch)

| Signal                  | GPIO |
| ----------------------- | ---- |
| I2C SDA                 | 8    |
| I2C SCL                 | 9    |
| SPI MISO                | 13   |
| SPI MOSI                | 11   |
| SPI SCLK                | 12   |
| TFT CS                  | 45   |
| TFT DC                  | 46   |
| TFT RST                 | 47   |
| ADS1115 (X-AV4) address | 0x49 |

Other CPU chip-selects on the shared SPI bus: SD CS IO42, Ethernet CS IO1 (RST IO2, INT IO4).

---

## ADS1115 Register / Channel Map

| ADS1115 Input | X-AV4 Terminal | Sketch usage                    |
| ------------- | -------------- | ------------------------------- |
| AIN0          | AIN1           | Humidity (`readVolts(0)`)       |
| AIN1          | AIN2           | Wind speed (`readVolts(1)`)     |
| AIN2          | AIN3           | Wind direction (`readVolts(2)`) |
| AIN3          | AIN4           | — (spare, `readVolts(3)`)       |

Configuration used in the sketch:

- Gain: `GAIN_ONE` (±4.096 V at the ADC, 125 µV per count, equal to 13.1 V at the terminal)
- Data rate: library default (128 SPS, about 8 ms per conversion)
- Single-ended, single-shot reads

---

## Voltage Conversion

```
V_terminal = V_adc / DIVIDER                    (clamped at 0 V)

Humidity (%RH)  = V_terminal / 10 × 100
Wind speed (m/s) = V_terminal / 10 × 50
Direction (°)    = atan2(Σ sin a, Σ cos a),   a = V_terminal / 10 × 2π   (add 360° if negative)
```

The X-AV4 divides each 0–10 V input down so it fits the ADS1115 (3.3 V supply). With `DIVIDER` = 0.3125 (3.3 kΩ series, 1.5 kΩ to ground: 1.5 / (3.3 + 1.5)), a full-scale 10 V signal reaches the ADC as 3.125 V. For an offset range such as 2–10 V, use `min + (V − 2) / 8 × (max − min)`.

| Parameter      | Value in sketch                          |
| -------------- | ---------------------------------------- |
| `DIVIDER`      | 0.3125 (X-AV4 input divider)             |
| `HUM_MAX_PCT`  | 100 %RH at 10 V                          |
| `WIND_MAX_MS`  | 50 m/s at 10 V (WG2/O-10 full scale)     |
| Direction span | 360° at 10 V                             |

### Reference values for commissioning

Ideal values for `DIVIDER` = 0.3125. Real modules vary with resistor tolerance.

| Terminal       | ADC input | Expected count | Humidity | Wind speed | Direction |
| -------------- | --------- | -------------- | -------- | ---------- | --------- |
| 0.0 V          | 0.000 V   | 0              | 0 %RH    | 0 m/s      | 0°        |
| 2.5 V          | 0.781 V   | 6250           | 25 %RH   | 12.5 m/s   | 90°       |
| 5.0 V          | 1.563 V   | 12500          | 50 %RH   | 25 m/s     | 180°      |
| 7.5 V          | 2.344 V   | 18750          | 75 %RH   | 37.5 m/s   | 270°      |
| 10.0 V         | 3.125 V   | 25000          | 100 %RH  | 50 m/s     | 360°      |
| 10.5 V (limit) | 3.281 V   | 26250          | FAULT    | FAULT      | FAULT     |

Resolution: one count is 0.4 mV at the terminal, which is 0.004 %RH, 0.002 m/s or 0.014°. This is finer than the transmitters themselves (±3 %RH, ±0.5 m/s or ±3 %, ±1.5°).

---

## Key Sketch Settings

| Constant       | Value     | Meaning                                              |
| -------------- | --------- | ---------------------------------------------------- |
| `ADS_ADDR`     | 0x49      | X-AV4 I2C address (DIP 1 ON)                         |
| `DIVIDER`      | 0.3125    | X-AV4 input divider, terminal V to ADC V             |
| `HUM_MAX_PCT`  | 100 %RH   | Humidity at 10 V                                     |
| `WIND_MAX_MS`  | 50 m/s    | Wind speed at 10 V (check your sensor datasheet)     |
| `OVER_RANGE_V` | 10.5 V    | Above this a channel shows FAULT                     |
| `SAMPLES`      | 8         | Readings averaged per channel per update             |
| `UPDATE_MS`    | 500 ms    | Screen and Serial update period                      |

---

## Required Libraries

| Library                 | Version        | Notes                      |
| ----------------------- | -------------- | -------------------------- |
| Wire                    | ESP32 core     | Bundled                    |
| SPI                     | ESP32 core     | Bundled                    |
| math.h                  | ESP32 core     | Bundled                    |
| Adafruit ADS1X15        | 2.6.2          | + Adafruit BusIO 1.17.4    |
| LovyanGFX               | 1.2.36          | `#define LGFX_USE_V1`      |

ESP32 board package: **3.x (version 3 or above)**. Install libraries via **Sketch → Include Library → Manage Libraries**.

### Arduino IDE board settings (recommended: Arduino IDE 1.8.19)

| Setting                 | Value                                              |
| ----------------------- | -------------------------------------------------- |
| Board                   | ESP32S3 Dev Module                                 |
| Upload Speed            | 115200                                             |
| USB Mode                | Hardware CDC and JTAG                              |
| USB CDC On Boot         | Enabled                                            |
| CPU Frequency           | 240 MHz (WiFi)                                     |
| Flash Mode              | QIO 80 MHz                                         |
| Flash Size              | 16 MB (128 Mb)                                     |
| Partition Scheme        | Default 4MB with spiffs (1.2MB APP/1.5MB SPIFFS)   |
| PSRAM                   | QSPI PSRAM                                         |
| Core Debug Level        | None                                               |

Leave the remaining options at their defaults (USB Firmware MSC On Boot and USB DFU On Boot disabled, Upload Mode UART0 / Hardware CDC, Arduino and Events run on Core 1, Erase All Flash disabled, JTAG Adapter disabled).

---

## Quick Start

1. Power off the system.
2. Seat the X-AV4 on the expansion bus and set DIP 1 ON, DIP 2–4 OFF (address `0x49`).
3. Wire the humidity output (AO2) to **AIN1**, wind speed to **AIN2** and wind direction to **AIN3**. Connect all transmitter GND wires to Power supply GND. **AGND** connect to A02 specific GND.
4. Power on (24 V DC).
5. Open `NORVI_X-AV4_Weather_Monitoring_System.ino` in Arduino IDE.
6. Select the board settings above and the correct COM port.
7. Upload the sketch.
8. Observe wind speed, wind direction and humidity on the TFT. Serial prints one status line every 500 ms at 115200 baud.

Expected Serial output:

```
wind=17.1 m/s  dir=223 deg SW  hum=58.5 %
```

If the module does not answer, Serial prints `X-AV4 not found` every 500 ms (check DIP address and seating).

### Commissioning checklist

- **Voltage injection:** with the transmitters disconnected, apply 0, 2.5, 5, 7.5 and 10 V between each input and AGND and compare with the reference table. Do not exceed 10 V.
- **Wind direction:** turn the vane to N, E, S and W and compare with a compass. Align the north mark as the installation instructions describe (the sketch has no direction offset).
- **Wind speed:** spin the cups by hand. The value rises and returns to about 0 m/s when they stop.
- **Humidity:** the voltage at AIN1 should equal %RH ÷ 10 (58.5 %RH is 5.85 V). Compare with a reference hygrometer in the same air.
- **Fault response:** power off, remove the X-AV4, power on and confirm `---` and `no X-AV4` with a red LED. Power off, reseat the module and power on to confirm normal operation.

---

## Display States

| State           | Condition                    | Display                                           | Header LED                    |
| --------------- | ---------------------------- | ------------------------------------------------- | ----------------------------- |
| Normal          | All channels 0 to 10.5 V     | Values, gauges and compass needle                 | Green, blinking each update   |
| Wind speed zone | Gauge colour                 | Green below 25 m/s, amber 25 to 40 m/s, red from 40 m/s | Unchanged               |
| Channel fault   | A channel above 10.5 V       | `FAULT` and `check wiring` in red, gauge empty    | Red                           |
| X-AV4 offline   | ADS1115 does not answer      | `---` and `no X-AV4` on all channels              | Red                           |

The sketch restarts the ADC automatically when the X-AV4 answers again.

---

## File Contents

| File                                          | Description                |
| --------------------------------------------- | -------------------------- |
| `NORVI_X-AV4_Weather_Monitoring_System.ino`   | Complete Arduino sketch    |
| `NORVI_X-AV4_Weather_Monitoring_System.md`    | Extended application notes |
| `README.md`                                   | This document              |

---

## FAQ (8 Q&A)

**Q1. Why do all channels show `---` and `no X-AV4`?**  
A. The ADS1115 is not answering at `ADS_ADDR`. Power off and reseat the X-AV4, set DIP 1 ON (DIP 2–4 OFF) for `0x49`, and run an I2C scanner to confirm `0x49` is detected.

**Q2. Why does one channel show `FAULT`?**  
A. The input is above 10.5 V. Common causes are the wrong sensor variant, supply voltage on the signal wire, or a faulty transmitter. Power off, check the wiring, use the 0–10 V variants (-10 or -40) and measure the voltage between the AIN terminal and AGND.

**Q3. Can I use a sensor with a different range, for example 0–30 m/s or 2–10 V?**  
A. Yes. Change `WIND_MAX_MS` (or `HUM_MAX_PCT`) to the full-scale value in your sensor datasheet. For an offset range such as 2–10 V, use `min + (V − 2) / 8 × (max − min)` in place of the linear formula.

**Q4. How do I read the fourth channel?**  
A. Call `readVolts(3)` for AIN4 and apply your own span, for example `value = readVolts(3) / 10 × span`.

**Q5. The wind direction is wrong by a fixed angle. How do I correct it?**  
A. The north mark of the vane is probably not aligned. Rotate the mounting to align it, or add an offset to the angle in the sketch (the sketch has no direction offset by default).

**Q6. All values are wrong by the same percentage. How do I calibrate?**  
A. Apply a known voltage (for example 5.00 V and 10.00 V) between an input and AGND, convert the displayed value back to volts, and set `DIVIDER (new) = DIVIDER (old) × displayed volts / applied volts`. Example: 10.00 V shows as 9.70 V, so the new value is 0.3125 × 0.97 = 0.3031. A correct setting shows 25.0 m/s at 5.00 V on the wind speed input.

**Q7. A reading is stuck at zero. Why is an open signal wire not flagged as a fault?**  
A. A 0–10 V signal has no live zero, so an open signal wire reads as 0 V and cannot be told apart from a real 0 value. Check the supply at the transmitter, the voltage between AIN and AGND, and that the transmitter 0 V is tied to AGND. You can also add a low-limit check (for example humidity below 1 %RH) to flag a suspect channel.

**Q8. Why is the TFT blank, white or flickering?**  
A. Confirm CS 45, DC 46 and RST 47 in the `LGFX` class, and that USB CDC On Boot, Flash Size and PSRAM match the board settings above (the display buffer needs PSRAM). For flicker or noise, the Ethernet or SD CS lines may be floating on the shared SPI bus: in `setup()`, before `tft.init()`, call `pinMode(1, OUTPUT)` and `pinMode(42, OUTPUT)`, then drive both HIGH.

---

## References

- [NORVI-X-Version-02 GitHub](https://github.com/NORVIControllers/NORVI-X-Version-02)
- NORVI X-AV4 User Guide and Datasheet (norvi.io/docs)
- Sentera DSTHF-3, WG2/O-10 and WRG/O-40 transmitter datasheets
- Application guide folder: `Application Guides/weather-monitoring-on-x-av4`

---

*Last updated: 11 November 2026*
