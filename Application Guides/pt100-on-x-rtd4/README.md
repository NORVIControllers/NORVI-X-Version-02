# PT100 3-Wire Temperature on X-RTD4

Connect a 3-wire PT100 probe to a NORVI X-RTD4 expansion module and read it with a NORVI X-CPU-ESPS3-X1 over I²C. The X-RTD4 digitises the probe with a MAX31865 RTD-to-digital converter and an on-board STM32 microcontroller. It returns a linearised temperature, the resistance and a fault byte in a CRC-protected frame. The sketch requests the configured channels once per second, validates every frame, and shows the temperature, resistance and fault state of each channel on the controller's TFT and on Serial.

The same method suits motor windings, transformer windings, bearing housings, pump casings and any zone that stays inside the rating of the probe.

> **Safety:** Disconnect all power before wiring, and let a qualified electrician install the system. This monitor provides indication only. Do not use it as a motor or transformer protection device.

## What You Need

| Component | Role | Key details |
|-----------|------|-------------|
| NORVI X-CPU-ESPS3-X1 | Controller | ESP32-S3, 16 MB flash, 2 MB QSPI PSRAM, 2.0 in ST7789 TFT (240 × 320), 24 V DC supply |
| NORVI X-RTD4 | RTD input | 4 RTD channels, MAX31865 (15-bit) shared through a multiplexer, STM32F103C8T6 bridge, PT100/PT1000 by DIP switch, I²C address by DIP switch (default 0x3F) |
| PT100 probe | Field sensor | 3-wire, R₀ = 100 Ω, Class A or B. For windings, an embedded sensor or a probe in a pocket, as the equipment manufacturer specifies |
| 24 V DC supply | Power | Feeds the X-CPU. The X-RTD4 takes its power through the expansion port (80 mA at 24 V) |

Software: Arduino IDE with the ESP32 core and the **LovyanGFX** library.

## System Overview

Signal path: PT100 probe → X-RTD4 (multiplexer, MAX31865, STM32) → I²C → X-CPU (ESP32-S3) → TFT and Serial.

1. **Sensing.** The PT100 element rises from 100 Ω at 0 °C to about 138.5 Ω at 100 °C and 168.5 Ω at 180 °C.
2. **Multiplexing and conversion.** An analog multiplexer connects one of the four channels at a time to a single MAX31865. The converter excites the element and returns a 15-bit reading.
3. **Bridging.** The on-board STM32 handles the multiplexing and the converter's SPI interface, then answers the CPU on the shared I²C bus.
4. **Processing.** The ESP32-S3 requests up to two channels per transaction, checks the CRC of every 12-byte frame, and stores temperature, resistance and fault per channel.
5. **Indication.** Changed values are redrawn on the TFT, and one line per channel is printed on Serial.

## X-RTD4 at a Glance

| Parameter | Value |
|-----------|-------|
| Inputs | 4 RTD channels, terminals 1 to 8 (RTD1+ / RTD1− to RTD4+ / RTD4−), terminal 9 GND |
| Converter | MAX31865, 15-bit, with automatic fault detection |
| Interface to CPU | I²C through the expansion bus, address set by a 4-way DIP switch: 0x3F (all OFF) down to 0x30 (all ON) |
| RTD type switch | ON = PT100, OFF = PT1000 |
| Supply | 24 V DC through the expansion port, 80 mA at 24 V |
| Environment | −40 to +85 °C, IP20, DIN rail, 23 × 81 × 104 mm |
| Indicators | One LED per channel (1 to 4), an A LED and a power LED |

## Wiring

1. Power off the whole system.
2. Mount the X-RTD4 on the DIN rail beside the X-CPU and seat it firmly on the expansion bus connector.
3. Set the address DIP switches all OFF (0x3F) and the RTD type switch to **ON** for PT100.
4. Identify the probe leads with a multimeter. The two leads of the matched pair read about 0 Ω to each other.
5. Wire the probe to channel 1 as in the table below.
6. Connect the 24 V DC supply to the POWER terminals of the X-CPU. The RS-485 terminals are not used.
7. Power on and connect the USB cable.

### Wiring table

| Probe lead | X-RTD4 terminal | Function |
|------------|-----------------|----------|
| Single lead (red) | 1 – RTD1+ | Positive input, channel 1 |
| One lead of the matched pair (black) | 2 – RTD1− | Negative input, channel 1 |
| Other lead of the matched pair (black) | 9 – GND | Third lead used for the 3-wire connection |

Channels 2 to 4 are wired the same way on terminals 3 and 4, 5 and 6, and 7 and 8. GND (terminal 9) is shared.

### Pin allocation

| Function | GPIO | Notes |
|----------|------|-------|
| I²C (SDA / SCL) | IO8 / IO9 | The only signals the X-RTD4 uses on the expansion port, apart from power. Shared with the X-CPU's own devices (touch 0x15, PCA9536 0x41, RTC 0x68, PCA9539 0x75) |
| SPI (MOSI / MISO / SCLK) | IO11 / IO13 / IO12 | Display bus. The sketch calls `SPI.begin(SCLK, MISO, MOSI)` |
| TFT (CS / DC / RST) | IO45 / IO46 / IO47 | ST7789 panel, driven through LovyanGFX |
| X-RTD4 address | 0x3F | All address DIP switches OFF. Matches `RTD_SLAVE_ADDR` in the sketch |

### Wiring practice

- A PT100 is passive and has no polarity, so it needs no supply of its own.
- Use three leads of the same gauge and length. The compensation assumes the leads are equal, so unequal leads leave an error.
- Check the probe before wiring: about 100 Ω at 0 °C, about 110 Ω at room temperature, and about 0 Ω between the matched pair.
- Use shielded, twisted cable rated for the temperature it passes through. Ground the shield at one end only.
- Keep sensor cables short and away from motor, VFD, AC and relay wiring. Cross power cables at 90° if you must cross them.
- Tighten every terminal. A loose lead gives a fault or a reading that drifts.
- Do not connect or disconnect the module while the system is powered.

## How a PT100 Is Read with 3-Wire Compensation

A PT100 is a platinum resistor with 100 Ω at 0 °C and a slope of about 0.385 Ω/°C. The converter measures its resistance, so any resistance in the cable adds to the reading. With two wires, 1 Ω of cable reads as about 2.6 °C. A 20 m run of 0.5 mm² copper has roughly 0.69 Ω per lead, which is about 3.6 °C of error for the two leads in the measuring loop.

A third lead lets the converter measure the lead resistance and subtract it, leaving the element resistance. This works only when the three leads are matched.

| Parameter | PT100 | PT1000 |
|-----------|-------|--------|
| Resistance at 0 °C | 100 Ω | 1000 Ω |
| Change per °C | about 0.385 Ω | about 3.85 Ω |
| Effect of lead resistance | Higher | Lower |
| Signal strength | Lower | Higher |
| RTD type switch on X-RTD4 | ON | OFF |

> **Note:** The X-RTD4 user guide shows each channel as a 2-wire input with GND as a shared reference. This tutorial connects the third lead to the GND terminal. Do not assume the lead is being compensated. Run the lead-resistance test below once, with the probe replaced by a resistor, and record the result.

## Resistance to Temperature Reference

The sketch does not linearise anything. The X-RTD4 returns the temperature in °C and the resistance as 32-bit floats, and the sketch displays them as received. For reference, the PT100 characteristic above 0 °C (IEC 60751) is:

```
R(T) = R0 × (1 + A×T + B×T²)
R0 = 100 Ω, A = 3.9083e-3 /°C, B = -5.775e-7 /°C²
```

Class A tolerance is ±(0.15 + 0.002·|T|) °C and Class B is ±(0.30 + 0.005·|T|) °C.

| Temp. (°C) | Ideal R (Ω) | Class A (±°C) | Class B (±°C) | Typical use |
|------------|-------------|---------------|---------------|-------------|
| 0 | 100.00 | 0.15 | 0.30 | Ice-water check |
| 25 | 109.73 | 0.20 | 0.43 | Room temperature |
| 50 | 119.40 | 0.25 | 0.55 | |
| 100 | 138.51 | 0.35 | 0.80 | |
| 130 | 149.83 | 0.41 | 0.95 | Class B insulation limit |
| 155 | 159.19 | 0.46 | 1.08 | Class F insulation limit |
| 180 | 168.48 | 0.51 | 1.20 | Class H insulation limit |

> **Note:** On PT100 the `Res` value reads ten times the element resistance. In the working setup the display shows 32.51 °C and 1126.44 Ω, while a PT100 at 32.51 °C is 112.64 Ω. The temperature is correct. Treat the temperature as the primary reading and divide `Res` by 10 if you need ohms.

## Motor Windings and Transformers

### Probe placement

- Use the winding sensors the manufacturer fitted, or the pocket or thermowell the manufacturer specifies. A probe taped to the outside of a casing reads the casing, not the hot spot.
- For a three-phase machine, read one probe per phase or per winding on channels 1 to 3, and use channel 4 for a bearing, core or oil pocket.
- Route all the probe cables the same way and keep them matched in length and gauge.

### Setting limits

Take the limits from the nameplate and the manufacturer's documents, not from this guide. The insulation thermal class sets the ceiling for the winding temperature (IEC 60085).

| Class | Maximum (°C) | Practical use |
|-------|--------------|---------------|
| B | 130 | Older and smaller machines |
| F | 155 | Common industrial motors and dry-type transformers |
| H | 180 | High-temperature duty |

Set a warning point below the trip point. A sensor in or near the winding reads lower than the hottest spot, so apply the margin the equipment manufacturer recommends. The sketch only displays values and has no alarm.

## IDE Setup and Configuration

- Select **ESP32S3 Dev Module**, then set USB CDC On Boot to **Enabled**, Flash Size to **16MB (128Mb)** and PSRAM to **QSPI PSRAM**. Without USB CDC On Boot the Serial Monitor stays empty.
- Install **LovyanGFX** from the Library Manager. Wire and SPI come with the ESP32 core. No sensor library is needed.
- Open `RTD4_PT100.ino` from the `pt100-on-x-rtd4` folder of the NORVI-X-Version-02 repository. In the configuration section, set the RTD type to PT100 (0), the channel list to 1 to 4 and the channel count to 4. The values are held in RAM only, so the sketch returns to these defaults after every reset.
- Upload the sketch and open the Serial Monitor at 115200 baud.

**Serial configuration.** Send `C` in the Serial Monitor, then one line of RTD type and channels: `100,1` (PT100, channel 1), `100,1-4` (all four) or `100,1,3` (channels 1 and 3). Invalid input leaves the configuration unchanged.

**Behaviour.** Each scan requests the channels, checks the CRC of every frame and skips bad frames, so the last good value stays on screen. After two failed transmissions the sketch clears the I²C bus and restarts it. After three scans in a row without a valid frame the display shows NO RESPONSE. The sketch retries every 100 ms until the first valid frame, then reads once per second.

## Register Map (I²C Frame Protocol)

The X-RTD4 does not use a register-addressed interface. It answers a framed request on its I²C address (default 0x3F). Every scan sends a request, waits 15 ms, then reads 12 bytes per channel. A request carries at most two channels, so four channels need two transactions.

### Request (write)

| Byte | Content |
|------|---------|
| 0 | Command, 0x01 |
| 1 | RTD type: 0 = PT100, 1 = PT1000 |
| 2, 3 | Channel numbers (1 to 4), one or two |

### Reply (read, 12 bytes per channel)

| Byte | Content |
|------|---------|
| 0 | Not used by the sketch |
| 1 | Channel number |
| 2 to 5 | Temperature in °C, 32-bit float |
| 6 to 9 | Resistance, 32-bit float |
| 10 | Fault byte (0 = no fault) |
| 11 | CRC-8 of bytes 0 to 10 (reflected polynomial 0x8C, initial value 0) |

### Address DIP switch

| Switches | Address |
|----------|---------|
| All OFF | 0x3F |
| All ON | 0x30 |

## Reading the Screen and Fault Reference

| Row | Colour | Content |
|-----|--------|---------|
| NORVI RTD MASTER | Yellow | Title |
| RTD Type | Cyan | PT100 or PT1000, as set in the sketch |
| Ch *n* : Temp | White and green | Channel number and temperature in °C |
| Res | Orange | Resistance reported by the module (see the PT100 note above) |
| Fault | Red | Fault byte, 0 when healthy |
| NO RESPONSE | Red and white | Shown instead of old values when the module stops answering |

| Condition | What you see | Meaning and action |
|-----------|--------------|--------------------|
| Normal | Fault: 0 | The converter reports no fault. The temperature is valid |
| Converter fault | Fault: non-zero | The converter flagged the channel, typically an open or shorted probe or lead. Do not use the temperature. Check the wiring |
| Bad frame | CRC ERROR on Serial | The frame is discarded and the last good value stays. Check the I²C wiring and seating |
| No reply | NO RESPONSE and the address on the TFT | Three scans in a row returned no valid frame. Check seating, power and the address switches |

## Commissioning and Test

### Checklist

- **Supply.** The 24 V DC supply is stable and wired to the X-CPU POWER terminals. The module's power LED is on.
- **Switches.** Address DIP switches all OFF, RTD type switch ON, sketch set to type 0 (PT100). The switch and the sketch must agree.
- **I²C scan.** The scan below lists 0x3F together with 0x15, 0x41, 0x68 and 0x75 from the X-CPU.
- **Channel LED.** The LED of each channel in use lights while it is being read.

To scan the bus, run any standard Arduino I²C scanner sketch.

### Resistor substitution

Replace the probe with a precision 100 Ω resistor between terminals 1 and 2, and link terminal 2 to terminal 9 so that the resistor sees the same connection as the probe. The display should read about 0 °C. A 138.5 Ω resistor should read about 100 °C.

### Lead-resistance test

This test shows whether your unit compensates the leads. Keep the 100 Ω resistor as the element and add a 2.0 Ω resistor in series with each of the three leads:

| Connection | Terminal | Note |
|------------|----------|------|
| Element end A through 2.0 Ω | 1 – RTD1+ | Simulates the single lead |
| Element end B through 2.0 Ω | 2 – RTD1− | First lead of the matched pair |
| Element end B through 2.0 Ω | 9 – GND | Second lead of the matched pair |

Compare the reading with the one from the resistor-substitution step.

- If it stays within about 1 °C of the previous reading, the module compensates the leads.
- If it rises by about 10 °C (4 Ω of added loop resistance at 0.385 Ω/°C), the module reads the channel as a 2-wire input. Keep the probe cable short, and do not rely on 3-wire compensation.

### Probe checks

- **Ice water.** With the probe in melting ice water the reading settles near 0 °C within the probe tolerance.
- **Warm by hand.** The temperature rises steadily when you hold the probe.
- **Open lead.** Disconnect the red lead. Fault should become non-zero, and the temperature must not be used.

### Expected output

Serial prints one line per channel and per scan, at 115200 baud:

```
Ch 1 Temp: 32.51 Res: 1126.44 Fault: 0
```

## Troubleshooting

| Symptom | Likely cause | Remedy |
|---------|--------------|--------|
| Serial Monitor empty | USB CDC On Boot disabled or wrong port | Enable USB CDC On Boot, reselect the port, set 115200 baud, press reset |
| Display blank | Wrong board settings, or display pins changed | Use the IDE settings above. Keep CS 45, DC 46, RST 47 in the LGFX class |
| NO RESPONSE | Module not seated, no power, or address mismatch | Power off and reseat. Check the module's power LED. Match `RTD_SLAVE_ADDR` to the address switches |
| Temperature far from expected | RTD type switch and sketch disagree | Set the switch to ON for PT100 and `rtdType = 0`, or OFF and 1 for PT1000 |
| Res is ten times high on PT100 | Resistance scaling on PT100 | Use the temperature, divide Res by 10 for ohms |
| Fault non-zero | Open or shorted lead, loose terminal | Check terminals 1, 2 and 9 and measure the probe with power off |
| Reading a few degrees high | Unequal or long leads, or no compensation | Use matched leads and run the lead-resistance test |
| CRC ERROR | Noise or poor seating on the I²C bus | Reseat the module, keep it away from motor cabling, shorten any extension |
| Setting lost after reset | Configuration is held in RAM | Edit the defaults in the sketch |

## Q&A

**1. Can an ESP32 read a PT100 directly?**
It can read a voltage, but its ADC is noisy and non-linear, and it cannot excite the element or remove cable resistance. The X-RTD4 does the excitation and conversion and hands the CPU a clean temperature over I²C.

**2. PT100 or PT1000?**
Both work. PT1000 gives about ten times the signal and is less affected by cable resistance, so it suits long runs. Match the RTD type switch and the sketch to the probe.

**3. Why does 3-wire matter for a motor winding?**
The terminal box can be several metres from the winding. Cable resistance adds to the reading at about 2.6 °C per ohm. A matched third lead lets the converter subtract it, provided the compensation is active on your unit. Run the lead-resistance test to check.

**4. Can I read more than one probe?**
Yes. Send `C`, then `100,1-4` for all four channels. The channels are scanned one after another through the multiplexer.

**5. Can I use more than one X-RTD4?**
Yes, with a different address on each module through the DIP switches. The sketch holds a single `RTD_SLAVE_ADDR`, so a second module needs a second address and request loop.

**6. Is this a protection relay?**
No. It displays and logs temperature. Use a certified protection device for tripping.

**7. What does a non-zero Fault value mean?**
The converter's automatic fault detection flagged the channel, typically an open or shorted probe or lead. The temperature for that channel must not be used. Check terminals 1, 2 and 9 with power off and measure the probe. To test, disconnect the red lead and Fault should become non-zero.

**8. Why does the display show 1126.44 Ω for a PT100 at 32.51 °C?**
On PT100 the `Res` value is reported ten times the element resistance. A PT100 at 32.51 °C is 112.64 Ω, so the temperature is correct and `Res` just needs dividing by 10 if you want ohms.

## Resources

- NORVI X-RTD4 datasheet and user guide: https://norvi.io/docs/norvi-x-rtd4-datasheet/
- NORVI-X-Version-02 repository (libraries, expansion module examples): https://github.com/NORVIControllers/NORVI-X-Version-02
- This tutorial's code: `Application Guides / pt100-on-x-rtd4` in the NORVI-X-Version-02 repository
- MAX31865 RTD-to-digital converter datasheet (Analog Devices)
- Technical support: support@norvi.io

