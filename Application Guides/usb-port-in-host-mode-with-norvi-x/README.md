# Pump Control with Run-Dry Interlock and Hour Counter on NORVI X-R8

Industrial pump control example for a motor contactor and a motor-ON lamp switched by **Relay 1** and **Relay 2** of the NORVI X-R8 expansion module, protected by an RS485 Modbus RTU pressure transmitter.

The sketch switches the 230 V AC coil of a motor contactor with relay 1 and a motor-ON lamp with relay 2. It reads the pressure transmitter over Modbus RTU. While the motor runs, and after a 6 s priming time, the pump is stopped and locked in FAULT if the pressure stays below `RUN_DRY_PRESSURE` for 2 s. Run hours and starts are saved in flash. The CPU’s built-in ST7789 TFT shows the motor state, the `NO1` / `NO2` relay states, the run hours, the pressure and touch START / STOP / RESET buttons. Push buttons PB1 and PB3 on the X1 also start, stop and reset the pump.

---

## Compatible Hardware

| Item | Details |
|------|---------|
| CPU | NORVI X-CPU-ESPS3-X1 **V2 (N16R2)** — ESP32-S3, 16 MB flash, QSPI PSRAM, 2.0″ ST7789 TFT (240 × 320) with CST816S touch |
| Expansion | NORVI X-R8 — 8-channel relay output, PCA9538 I/O expander, I2C address `0x73` (DIP 1 OFF, DIP 2 OFF) |
| Motor contactor | 230 V AC coil on relay 1 (COM1 / NO1), with an RC snubber |
| Motor-ON lamp | On relay 2 (COM2 / NO2) |
| Pressure sensor | RS485 Modbus RTU pressure transmitter, 24 V DC, connected to the X1 RS485 port |
| Supply | 24 V DC for the CPU and the transmitter |

---

## Wiring Table

| Terminal | Function | Connection |
|----------|----------|------------|
| COM1 | Relay 1 common | Supply to the contactor coil (230 V AC) |
| NO1 | Relay 1 normally open | Contactor coil A1 |
| COM2 | Relay 2 common | Supply to the motor-ON lamp |
| NO2 | Relay 2 normally open | Motor-ON lamp |
| Relay 3 to 8 | Unused | Not used in this example |
| RS485 A / B | X1 RS485 port | Transmitter A / B |
| 24 V / 0 V | Transmitter supply | Transmitter power |

**Notes**
- Relay 2 follows relay 1. Both switch on at START and off at STOP or a fault.
- Fit an RC snubber or a varistor across the coil terminals A1 / A2. Without one, the switching spikes can reset or glitch the controller.
- The relay only drives the **contactor coil**. The motor itself is switched by the contactor contacts.
- If the RS485 pressure readings fail, swap the A and B wires.
- Keep the RS485 and sensor wiring away from the AC and motor cables.
- Mains wiring must be done by a qualified person with the power off.

### I2C Address (DIP switches on X-R8)

| DIP 1 | DIP 2 | Address |
|-------|-------|---------|
| OFF | OFF | **0x73** |

On the device, set DIP 1 OFF and DIP 2 OFF, because the code is configured to use I2C address `0x73` (`R8_I2C_ADDR`).

### CPU Pin Map (used by this sketch)

| Signal | GPIO |
|--------|------|
| I2C SDA | 8 |
| I2C SCL | 9 |
| SPI MISO | 13 |
| SPI MOSI | 11 |
| SPI SCLK | 12 |
| TFT Display CS | 45 |
| TFT Display DC | 46 |
| TFT Display RST | 47 |
| Touch IRQ | 48 |
| X1 IO expander reset | 38 |
| X-R8 PCA9538 reset | 21 |
| RS485 RX | 16 |
| RS485 TX | 15 |
| Pump contactor | X-R8 relay 1 (COM1 / NO1) |
| Motor-ON lamp | X-R8 relay 2 (COM2 / NO2) |
| X-R8 (PCA9538) address | 0x73 |

---

## PCA9538 Relay Map and X1 On-board I/O

| PCA9538 Pin | X-R8 Relay | Sketch usage |
|-------------|------------|--------------|
| 1 | Relay 1 (COM1 / NO1) | Pump contactor coil (`PUMP_RELAY_PIN`) |
| 2 | Relay 2 (COM2 / NO2) | Motor-ON lamp (`LAMP_RELAY_PIN`) |

The relays switch on when the pin is driven `HIGH` (`RELAY_ON_LEVEL`). The sketch writes the OFF level before it makes the pin an output, so the relays stay off at boot.

| X1 PCA9536 Pin | Function | Sketch usage |
|----------------|----------|--------------|
| IO0 | PB1 | Start / stop |
| IO1 | LED1 | On while the pump runs |
| IO2 | LED2 | Blinks while in FAULT |
| IO3 | PB3 | Reset fault |

The on-board LEDs are active-low (`LED_ON_LEVEL` = `LOW`).

---

## Modbus Pressure Sensor

The sketch reads one register from the transmitter with a Modbus RTU request (8N1 on the X1 RS485 port).

| Parameter | Value in sketch |
|-----------|-----------------|
| `MB_BAUD` | 9600 |
| `MB_SLAVE_ID` | 1 |
| `MB_FUNC` | `0x03` (holding register; use `0x04` for an input register) |
| `MB_REG_PRESSURE` | `0x0000` |
| `MB_SIGNED` | 0 (unsigned register) |
| `MB_SCALE` | 0.001 (bar per count) |
| `MB_OFFSET` | 0.0 |
| `PRESS_FILTER` | 0.5 (smaller = smoother) |
| `MB_POLL_MS` | 500 ms |
| `MB_TIMEOUT_MS` | 150 ms |

Check these values against the register map of your own transmitter.

### Pressure Conversion

```text
p = raw × MB_SCALE + MB_OFFSET

filtered = filtered + (p − filtered) × PRESS_FILTER

dry = (filtered < RUN_DRY_PRESSURE)
```

If three polls in a row fail, the reading is marked invalid (`SENSOR NO DATA`).

### Bench Testing Without a Sensor

Set `DRY_SOURCE` to `DRY_SRC_SIM` to run without the transmitter. In this mode the **RUN-DRY** tile on the TFT toggles a simulated low-water condition when touched, and START is blocked while the simulated water is low.

---

## Key Sketch Settings

| Constant | Value | Meaning |
|----------|-------|---------|
| `R8_I2C_ADDR` | 0x73 | X-R8 I2C address (DIP 1 OFF, DIP 2 OFF) |
| `PUMP_RELAY_PIN` | 1 | PCA9538 pin for the contactor (relay 1) |
| `LAMP_RELAY_PIN` | 2 | PCA9538 pin for the motor-ON lamp (relay 2) |
| `RELAY_ON_LEVEL` | HIGH | Pin level that switches a relay on |
| `DRY_SOURCE` | `DRY_SRC_MODBUS` | Run-dry source (`DRY_SRC_SIM` for bench testing) |
| `PRESSURE_UNIT` | "bar" | Unit shown on the TFT and Serial |
| `RUN_DRY_PRESSURE` | 0.50 bar | Trip limit while running |
| `START_BYPASS_MS` | 6000 ms | Priming time, run-dry check ignored |
| `DRY_DEBOUNCE_MS` | 2000 ms | Low-pressure time before the trip |
| `SENSOR_FAIL_MS` | 3000 ms | No-data time before the trip |
| `MIN_OFF_MS` | 5000 ms | Restart lockout after a stop or fault |
| `SAVE_INTERVAL_MS` | 60000 ms | Hour counter save period while running |
| `TOUCH_FLIP_X` / `TOUCH_FLIP_Y` | 0 / 0 | Set to 1 if the touch is mirrored |

---

## Required Libraries

| Library | Version |
|---------|---------|
| Wire, SPI, Preferences | ESP32 core (bundled) |
| PCA9538 | [NORVI-X-Version-02 / libraries / PCA9538](https://github.com/NORVIControllers/NORVI-X-Version-02/tree/main/libraries/PCA9538) |
| PCA9536D | 2.0.1 |
| CST816S | 1.3.0 |
| LovyanGFX | 1.2.25 (`#define LGFX_USE_V1`) |

Install the libraries via **Sketch → Include Library → Manage Libraries**. The PCA9538 library comes from the NORVI-X-Version-02 repository.

### Arduino IDE board settings

| Setting | Value |
|---------|-------|
| Board | ESP32S3 Dev Module |
| Upload Speed | 115200 |
| USB Mode | Hardware CDC and JTAG |
| USB CDC On Boot | Enabled |
| Upload Mode | UART0 / Hardware CDC |
| CPU Frequency | 240MHz (WiFi) |
| Flash Mode | QIO 80MHz |
| Flash Size | 16MB (128Mb) |
| Partition Scheme | Huge APP (3MB No OTA/1MB SPIFFS) |
| Core Debug Level | None |
| PSRAM | QSPI PSRAM |

---

## Quick Start

1. Power off the system.
2. Seat the X-R8 on the expansion bus and set DIP 1 OFF and DIP 2 OFF (address `0x73`).
3. Wire the contactor coil to **COM1 / NO1** and fit an RC snubber or varistor across A1 / A2. Wire the motor-ON lamp to **COM2 / NO2**.
4. Wire the RS485 pressure transmitter to the X1 RS485 port (A / B) and supply it with 24 V DC.
5. Check that the transmitter’s Modbus settings match the `MB_*` values in the sketch.
6. Power on (24 V DC).
7. Open `pump_control_system_with_run-dry_interlock_on_x_r8.ino` in Arduino IDE.
8. Select the board settings above and the correct COM port.
9. Upload the sketch.
10. Open the Serial Monitor at 115200 baud, then press START on the TFT, press PB1, or send `s`. Observe the motor state, relay states, run hours and pressure on the TFT.

### Controls

| Control | Action |
|---------|--------|
| TFT START button | Start the pump (enabled only when stopped) |
| TFT STOP button | Stop the pump (enabled only while running) |
| TFT RESET button | Reset a fault (enabled only in FAULT) |
| PB1 | Start when stopped, stop when running |
| PB3 | Reset a fault |
| Hold the RUN HOURS tile for 3 s | Reset the run hours and starts (not while running) |
| PB1 + PB3 held together for 5 s | Reset the run hours and starts (not while running) |

### Serial commands (115200 baud)

| Key | Action |
|-----|--------|
| `s` | Start the pump |
| `x` | Stop the pump |
| `r` | Reset a fault |
| `p` | Print the pressure and the run-dry limit |

### Commissioning checklist

- **Relays:** with the contactor and lamp wired, press START and confirm relay 1 pulls in the contactor and relay 2 lights the lamp. Press STOP and confirm both drop out.
- **Pressure reading:** send `p` and compare with a reference gauge. Correct `MB_SCALE` if the values differ.
- **Run-dry trip:** with the pump running, let the pressure fall below `RUN_DRY_PRESSURE` after the 6 s priming time. Confirm the trip after 2 s, both relays off and `ALARM: RUN DRY` on the TFT.
- **Sensor loss:** with the pump running past priming, disconnect the transmitter. Confirm a `SENSOR FAIL` trip once there has been no valid reading for 3 s.
- **Hour counter:** run the pump, stop it, power-cycle the controller and confirm the run hours and starts are kept.

---

## Pump States and Faults

| State | Condition | Relays |
|-------|-----------|--------|
| STOPPED | Pump off, ready to start | Relay 1 and 2 off |
| PRIMING | First 6 s after a start, run-dry and sensor checks ignored | Relay 1 and 2 on |
| RUNNING | After priming, run-dry interlock active | Relay 1 and 2 on |
| FAULT | A fault tripped, latched until reset | Relay 1 and 2 off |

| Fault | Display text | Condition |
|-------|--------------|-----------|
| Run dry | `RUN DRY` | Pressure below `RUN_DRY_PRESSURE` for 2 s after priming |
| Sensor failure | `SENSOR FAIL` | No valid pressure reading for 3 s after priming |
| Relay module lost | `R8 I/O LOST` | X-R8 stops answering on I2C (checked once per second while running) |

A start is refused with a message on the TFT if the pump is in FAULT (`RESET FAULT FIRST`), the X-R8 is not found (`X-R8 NOT FOUND`), or less than 5 s have passed since the last stop or fault (`RESTART LOCKOUT`). The pump is always stopped after a reboot, with both relays off.

### Display States

| State | TFT header | MOTOR panel | Pressure tile |
|-------|------------|-------------|---------------|
| Stopped | `PUMP CONTROL` with a grey `STOP` chip | `STOPPED`, `NO1 OFF`, `NO2 OFF` | Pressure with `STANDBY` |
| Priming | Amber `PRIME` chip | `PRIMING`, `NO1 ON`, `NO2 ON` | Pressure with `PRIMING` |
| Running | Green `RUN` chip | `RUNNING`, `NO1 ON`, `NO2 ON` | Pressure with `OK` or `LOW` |
| Fault | Blinking red `ALARM: <fault>` | `FAULT`, `NO1 OFF`, `NO2 OFF` | Fault text in red |
| No sensor data | Unchanged | Unchanged | `SENSOR NO DATA` in amber |

The TFT also shows the **RUN HOURS** counter in `HHHHH:MM` format, with the number of starts and the current session time. LED1 is on while the pump runs. LED2 blinks while in FAULT.

---

## Expected Serial Output

At boot:

```text
=== NORVI X1 + X-R8 PUMP CONTROL ===
X-R8 found at 0x73, relay 1 (NO1, contactor) and relay 2 (NO2, lamp) OFF
Stored: 0 s total run time, 0 starts
Run-dry source: Modbus RTU pressure (ID 1, 9600 baud), limit 0.50 bar
```

On start, stop and a run-dry trip (the number in square brackets is the time in ms since boot):

```text
[15230] PUMP START (NO1 ON, lamp NO2 ON)
[48760] PUMP STOP (NO1 OFF, lamp NO2 OFF) total=33s
[73410] FAULT: RUN DRY -> NO1 / NO2 OFF (pressure 0.20 bar, limit 0.50)
```

If the module does not answer, Serial prints `X-R8 NOT FOUND at 0x73 - check DIP switches / stacking`.

---

## File Contents

| File | Description |
|------|-------------|
| `pump_control_system_with_run-dry_interlock_on_x_r8.ino` | Complete Arduino sketch |
| `pump_control_system_with_run-dry_interlock_on_x_r8.md` | Extended application notes |
| `README.md` | This document |

---

## FAQ (9 Q&A)

**Q1. Why does Serial print `X-R8 NOT FOUND`?**  
A. The module is not seated, or the DIP address does not match `R8_I2C_ADDR`. Power off, reseat the X-R8, and set DIP 1 and DIP 2 OFF for `0x73`.

**Q2. The relay is on at boot, or it works the wrong way round. What do I change?**  
A. The relay logic level differs from the sketch. Change `RELAY_ON_LEVEL` (`HIGH` or `LOW`).

**Q3. Why do I get `SENSOR NO DATA` or `SENSOR FAIL`?**  
A. The Modbus settings are wrong, the A / B wires are swapped, or the transmitter has no power. Check `MB_BAUD`, `MB_SLAVE_ID` and `MB_REG_PRESSURE`, swap A and B, and measure 24 V at the transmitter.

**Q4. The pressure reads wrong. How do I correct it?**  
A. `MB_SCALE` does not match the register units. Compare the reading with a reference gauge and correct `MB_SCALE` (and `MB_OFFSET` if needed).

**Q5. Why does `RUN DRY` trip right after a start?**  
A. The pressure builds slowly, or the limit is too high. Increase `START_BYPASS_MS` or lower `RUN_DRY_PRESSURE`.

**Q6. The controller glitches or resets when the contactor switches. Why?**  
A. There is no suppressor on the coil. Fit an RC snubber or a varistor across A1 / A2.

**Q7. Why does START do nothing?**  
A. Check the message on the TFT. `RESET FAULT FIRST` means the pump is in FAULT and needs a reset. `RESTART LOCKOUT` means less than 5 s have passed since the last stop or fault (`MIN_OFF_MS`). `X-R8 NOT FOUND` means the relay module is not answering.

**Q8. The touch is mirrored. How do I fix it?**  
A. The panel orientation differs. Set `TOUCH_FLIP_X` and / or `TOUCH_FLIP_Y` to 1.

**Q9. Why is the TFT blank?**  
A. Check the pins in the `LGFX` class (CS 45, DC 46, RST 47) and the Arduino IDE board settings above, especially USB CDC On Boot, Flash Size and PSRAM.

---

## References

- NORVI X-R8 User Guide and Datasheet (norvi.io/docs)
- [NORVI-X-Version-02 GitHub](https://github.com/NORVIControllers/NORVI-X-Version-02)
- Application guide folder: `Application Guides/pump_control_system_with_norvi_x`

---

*Last updated: 26 November 2026*
