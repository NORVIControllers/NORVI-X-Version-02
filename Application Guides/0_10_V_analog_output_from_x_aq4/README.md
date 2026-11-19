# 0-10 V Analog Output from NORVI X-AQ4

Industrial speed-reference example that generates a 0-10 V signal on **Channel 1 (AOUT1)** of the NORVI X-AQ4 expansion module and feeds it to the AI1 input of a Schneider Electric Altivar Machine ATV320 drive running a three-phase motor.

The sketch limits the requested value to 0-10 V, converts it to a 13-bit DAC code (0-8191) with `voltageToDAC()`, and writes it to the X-AQ4 over I2C. The operator sets the voltage with "+" and "-" touch buttons on the CPU's built-in ST7789 TFT (0.1 V per press) or by typing a number from 0 to 10 in the Serial Monitor. The display shows the commanded voltage and a status dot that turns red if an I2C write fails.

---

## Compatible Hardware

| Item | Details |
| --- | --- |
| CPU | NORVI X1 (CPU-S3), ESP32-S3, 2.0″ ST7789 TFT (240 × 320) with CST816S capacitive touch |
| Expansion | NORVI X-AQ4, 4-channel analog output (AOUT1-AOUT4), I2C address `0x5F` in the sketch. **Only AOUT1 is used** |
| Drive | Altivar Machine ATV320U11M2C, 1.1 kW, single-phase 200-240 V input, AI1 0-10 V |
| Motor | Y2A-6324, three-phase induction motor, connected in **240 V delta** |
| Supply | 24 V DC for the X1 and a single-phase 200-240 V supply for the drive |

---

## Wiring Table

| Terminal | Function | Connection |
| --- | --- | --- |
| X-AQ4 AOUT1 (terminal 1) | Analog output 1, 0-10 V | ATV320 AI1 |
| X-AQ4 AGND (terminal 2) | Analog ground | ATV320 COM |
| AOUT2 / AOUT3 / AOUT4 | Channels 2-4 | Unused in this example |
| ATV320 P24 | 24 V | ATV320 STO |
| ATV320 P24 via a run switch | Run command | ATV320 DI1 (forward, source logic) |

**Notes**

- Use a switch on DI1 so that the operator decides when the motor may start.
- A permanent link between P24 and STO removes the safe torque off function. Accept this only on a test bench.
- Use a shielded pair for the reference signal if the drive is far from the X1.

### Power and Motor Wiring

| ATV320 terminal | Wire | Motor terminals it serves |
| --- | --- | --- |
| R/L1 | Line (L) | Single-phase supply, through the breaker / RCD |
| S/L2 | Neutral (N) | Single-phase supply |
| PE | Earth | Supply earth and motor earth |
| U/T1 | Grey | Link U1 + W2 |
| V/T2 | Black | Link V1 + U2 |
| W/T3 | Brown | Link W1 + V2 |

- The ATV320U11M2C cannot deliver more than 240 V, so the motor must stay in the **240 V delta** connection.
- Wire colours are as read from the test motor. Verify them on your motor before connecting.
- Swapping any two of U, V and W reverses the direction of rotation.

### I2C Address (X-AQ4)

Set the X-AQ4 address switches so that the module answers at `0x5F`, because the sketch uses `AQ4_ADDR = 0x5F`. See the X-AQ4 datasheet for the switch positions.

### CPU Pin Map (used by this sketch)

| Signal | GPIO |
| --- | --- |
| I2C SDA | 8 |
| I2C SCL | 9 |
| SPI MOSI | 11 |
| SPI SCLK | 12 |
| SPI MISO | 13 |
| TFT CS | 45 |
| TFT DC | 46 |
| TFT RST | 47 |
| Touch (SDA / SCL / RST / INT) | 8 / 9 / 47 / 48 |
| X-AQ4 address | 0x5F |

---

## X-AQ4 Register / Channel Map

| Output | X-AQ4 Terminal | Write register | Sketch usage |
| --- | --- | --- | --- |
| AOUT1 | AOUT1 / AGND | 3 | Channel 1 (this example) |
| AOUT2 | AOUT2 | 5 | n/a |
| AOUT3 | AOUT3 | 7 | n/a |
| AOUT4 | AOUT4 | 9 | n/a |

I2C frames used by the sketch:

- Configuration frame: `0x01`, `cfg`. Channel 1 receives the value 0 (the setting of the working 0-10 V output). Channels 3 and 4 receive 1 and are not used.
- Channel write frame: `1 + 2n`, high byte, low byte, where n is the channel number. The value range is 0-8191.

---

## Voltage to DAC Conversion

```
DAC = (V / 10) × 8191

1 count ≈ 1.22 mV
```

| Parameter | Value in sketch |
| --- | --- |
| Voltage range | 0-10 V |
| DAC code range | 0-8191 |
| Resolution | about 1.22 mV per count |
| Touch step | 0.1 V per press |

---

## Key Sketch Settings

| Constant / Setting | Value | Meaning |
| --- | --- | --- |
| `AQ4_ADDR` | `0x5F` | X-AQ4 I2C address |
| Output channel | 1 (AOUT1) | Channel written by the sketch |
| Input limit | 0-10 V | Requested value is limited to this range |
| Touch step | 0.1 V | Change per "+" or "-" press |
| Serial baud rate | 115200 | Serial Monitor, line ending Newline |

---

## ATV320 Drive Settings

Enter the motor nameplate values first (motor block), then the reference and command parameters. The factory settings already select 2-wire control, the combined profile and AI1 as the speed reference.

| Code | Name | Value |
| --- | --- | --- |
| UnS | Nominal motor voltage | 240 V (delta) |
| FrS | Nominal motor frequency | 50 Hz |
| nPr / nCr / nSP / COS / ItH | Power, current, speed, power factor, thermal current | Nameplate values |
| `Ctt` | Motor control type | **`StD`** (Standard V/Hz) |
| tCC | 2/3-wire control | 2C (factory) |
| CHCF | Profile | SIM (factory) |
| Fr1 | Reference channel 1 | AI1 (factory) |
| AI1t | AI1 type | 10U, 0-10 V (factory) |
| LSP | Low speed (at 0 V) | 0 Hz |
| HSP | High speed (at 10 V) | 50 Hz |

**Important:** change `Ctt` from the factory `UUC` to `StD` before the first run. On a small frame-63 motor, `UUC` can make the drive trip on overcurrent when the reference is above 0 V.

The full parameter list with menu paths is in the application guide: [0_10_V_analog_output_from_X_AQ4.md](0_10_V_analog_output_from_X_AQ4.md).

---

## Required Libraries

| Library | Notes |
| --- | --- |
| Wire | ESP32 core |
| SPI | ESP32 core |
| LovyanGFX | TFT driver |
| CST816S | Touch controller |

Install via **Sketch → Include Library → Manage Libraries**.

### Arduino IDE board settings (recommended)

| Setting | Value |
| --- | --- |
| Board | ESP32S3 Dev Module |
| USB CDC On Boot | Enabled |
| Flash Size | 16 MB (128 Mb) |
| PSRAM | QSPI PSRAM |

---

## Quick Start

1. Switch off the mains and the 24 V supply.
2. Set the X-AQ4 address switches for `0x5F` and seat the module on the X1 expansion port.
3. Wire AOUT1 to AI1 and AGND to COM on the ATV320.
4. Wire P24 to STO, and a run switch from P24 to DI1 (switch open).
5. Wire the drive supply and the motor in 240 V delta.
6. Set the ATV320 parameters, including `Ctt = StD`.
7. Power on (24 V DC) and open the sketch in the Arduino IDE.
8. Select the board settings above and the correct COM port, then upload the sketch.
9. Set 0 V, close the DI1 switch, then raise the voltage with "+" or from the Serial Monitor (115200 baud, Newline).
10. Observe the commanded voltage on the TFT and the motor frequency on the drive.

---

## Status Indication

| State | Condition |
| --- | --- |
| Normal | Green status dot, I2C write succeeds |
| I2C write error | Red status dot, Serial prints "I2C Write Error: 2" (no module at the address) or "I2C Write Error: 3" (module does not accept the data) |

Expected drive response with LSP = 0 Hz and HSP = 50 Hz:

| X1 output (AOUT1) | Drive frequency |
| --- | --- |
| 0 V | 0 Hz |
| 1 V | about 5 Hz |
| 5.8 V | about 29 Hz |
| 10 V | 50 Hz |

---

## File Contents

| File | Description |
| --- | --- |
| Arduino sketch (`.ino`) | Complete Arduino sketch |
| `0_10_V_analog_output_from_X_AQ4.md` | Extended application notes |
| `README.md` | This document |

---

## FAQ (8 Q&A)

**Q1. Why not generate the 0-10 V signal directly from the ESP32?**  
A. The ESP32-S3 outputs 0-3.3 V logic levels. A filtered PWM signal would need level shifting, scaling and filtering to give a stable 0-10 V. The X-AQ4 provides a ready 0-10 V output with a defined resolution and is set over I2C.

**Q2. Why AI1 and not AI3?**  
A. AI1 is the 0-10 V voltage input of the ATV320. AI3 is the current input (0-20 mA or 4-20 mA). Use AI3 only if the X-AQ4 channel is set to current output.

**Q3. Can the drive be controlled over Modbus instead?**  
A. Yes. The ATV320 has an embedded Modbus port, and the X1 can set the run command and the frequency over RS-485. The analog method needs no protocol set-up and is the simplest to commission, but it carries only the speed reference. Run and stop still need a separate input.

**Q4. Can I reverse the motor?**  
A. Yes. Wire a second switch between P24 and DI2, which is the reverse input at the factory. Do not reverse a mechanism at speed without a suitable deceleration time.

**Q5. Does 0 V stop the motor?**  
A. With LSP = 0 Hz the drive output goes to 0 Hz, but the drive stays in the run state. Open the DI1 switch to stop.

**Q6. Can I use AOUT2, AOUT3 or AOUT4 later?**  
A. Yes. Each channel is written independently with `write_channel(AQ4_ADDR, n, value)`, where n is 1, 2, 3 or 4. The wiring and scaling are the same as AOUT1. Check the configuration value sent in `config_dac()`, because the sketch currently sends 1 for channels 3 and 4.

**Q7. Why does the motor twitch and trip on a small motor at 1 V?**  
A. The factory setting of `Ctt` may be `UUC` (Sensorless Flux Vector Control). In this mode the drive injects test pulses to measure the rotor phase-shift angle, which fails on a low-current frame-63 motor. Set `Ctt` = `StD`.

**Q8. Why does the drive display read differently from the X1 display?**  
A. The X1 shows the voltage it commands on AOUT1. The ATV320 shows whichever monitoring value is selected, which is not the AI1 voltage by default. Also check that `Fr1` = `AI1` and `CHCF` = `SIM`, and measure AOUT1 to AGND with a multimeter to confirm the real voltage.

---

## Safety

- Disconnect all power before wiring. A qualified electrician must install the mains, the drive and the motor.
- This set-up is a bench demonstration. Do not use it as a safety system or as an emergency stop.
- The drive cannot protect a very small motor thermally at the lowest accepted current setting. Fit a motor protection device suited to the motor current.

---

## References

- [NORVI X-AQ4](https://shop.norvi.lk/products/norvi-x-aq4)
- [NORVI-X-Version-02 GitHub](https://github.com/NORVIControllers/NORVI-X-Version-02)
- Application guide: [0_10_V_analog_output_from_X_AQ4.md](0_10_V_analog_output_from_X_AQ4.md)
- Schneider Electric, Altivar Machine ATV320 Programming Manual, NVE41295.06
- [LovyanGFX](https://github.com/lovyan03/LovyanGFX)
- [CST816S](https://github.com/fbiego/CST816S)

---

*Last updated: 19 November 2026*
